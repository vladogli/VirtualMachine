#include "cpu.h"
#define IP 0x00
#define READ_IP ProcData->Read2Bytes(IP)
#define WRITE_IP(x) ProcData->Write2Bytes(IP, x)
#define SP 0x10
#define READ_SP     ProcData->Read2Bytes(SP)
#define WRITE_SP(x) ProcData->Write2Bytes(SP,x)
CPU::CPU() {
	functions = new std::function<void(void)>*[0x100];
	for (unsigned short i = 0; i < 0x100; i++) {
		functions[i] = nullptr;
	}
	matrix  = new memory(0x3840); // 160 x 90 symbols
	RAM     = new memory(0xFFFF);
	outPort = new memory(0xFFFF);
	stack   = new memory(0xFFFF);
	ProcData= new memory(0x29);
}

void CPU::PushStack(unsigned short _Val) {
	unsigned short SP_Val = READ_SP;
	if (SP_Val >= 0xFFFE) {
		throw STACK_OVERFLOW;
	}
	stack->Write2Bytes(SP_Val, _Val);
	WRITE_SP(SP_Val + 2);
}
unsigned short CPU::PopStack() {
	unsigned short SP_Val = READ_SP;
	if (SP_Val == 0) {
		throw STACK_UNDERFLOW;
	}
	WRITE_SP(SP_Val - 2);
	return stack->Read2Bytes(SP_Val);
}
void CPU::op_exit() {

}
void CPU::op_int_store() {}
void CPU::op_int_print() {}
void CPU::op_int_tostring() {}
void CPU::op_int_random() {}

/* 0x10 - 0x1F */
void CPU::op_jump_to() {}
void CPU::op_jump_z() {}
void CPU::op_jump_nz() {}

/* 0x20 - 0x2F */
void CPU::op_xor() {}
void CPU::op_or() {}
void CPU::op_add() {}
void CPU::op_and() {}
void CPU::op_sub() {}
void CPU::op_mul() {}
void CPU::op_div() {}
void CPU::op_inc() {}
void CPU::op_dec() {}

/* 0x30 - 0x3F */
void CPU::op_string_store() {}
void CPU::op_string_print() {}
void CPU::op_string_concat() {}
void CPU::op_string_system() {}
void CPU::op_string_toint() {}

/* 0x40 - 0x4F */
void CPU::op_cmp_reg() {}
void CPU::op_cmp_immediate() {}
void CPU::op_cmp_string() {}
void CPU::op_is_string() {}
void CPU::op_is_integer() {}

/* 0x50 - 0x5F */
void CPU::op_nop() {
	WRITE_IP(READ_IP + 1);
}
void CPU::op_reg_store() {
	unsigned short SavedIP = READ_IP;
	BYTE dest = RAM->Read(SavedIP + 1);
	BYTE source = RAM->Read(SavedIP + 2);
	if (!isReg(dest) || !isReg(source)) {
		op_exit();
		return;
	}
	WriteToReg(dest, ReadFromReg(source));
	WRITE_IP(SavedIP + 3);
}


/* 0x60 - 0x6F */
void CPU::op_peek() {
	unsigned short SavedIP = READ_IP;
	unsigned short _Value = RAM->Read2Bytes(RAM->Read2Bytes(SavedIP + 1));
	BYTE reg  = RAM->Read(SavedIP + 3);
	if (!isReg(reg)) {
		op_exit();
		return;
	}
	if (RegType(reg)) { // 1-byte register
		WriteToReg(reg, _Value);
	}
	else {              // 2-byte register
		WriteToReg(reg, _Value);
	}
	WRITE_IP(SavedIP + 4);
}
void CPU::op_poke() {
	unsigned short SavedIP = READ_IP;
	BYTE reg = RAM->Read(SavedIP + 1);
	ADDR addr= RAM->Read2Bytes(RAM->Read2Bytes(SavedIP + 2));
	if (!isReg(reg)) {
		op_exit();
		return;
	}
	if (RegType(reg)) { // 1-byte register
		RAM->Write2Bytes(addr, ReadFromReg(reg));
	}
	else {              // 2-byte register
		RAM->Write2Bytes(addr, ReadFromReg2(reg));
	}
	WRITE_IP(SavedIP + 4);
}
void CPU::op_memcpy() {
	unsigned short SavedIP = READ_IP;
	ADDR dest = RAM->Read(SavedIP + 1);
	ADDR source = RAM->Read(SavedIP + 3);
	unsigned short size = RAM->Read(SavedIP + 5);
	RAM->Copy(dest, source, size);
	WRITE_IP(SavedIP + 7);
}
void CPU::op_memcpy_reg() {
	unsigned short SavedIP = READ_IP;
	BYTE dest   = RAM->Read(SavedIP + 1);
	BYTE source = RAM->Read(SavedIP + 2);
	BYTE size   = RAM->Read(SavedIP + 3);
	if (!isReg(dest) || !isReg(source) || !isReg(size) || !RegType(dest) || !RegType(source) || !RegType(size)) {
		op_exit();
		return;
	}
	RAM->Copy(ReadFromReg2(dest), ReadFromReg2(source), ReadFromReg2(size));
	WRITE_IP(SavedIP + 4);
}
void CPU::op_memcpy_stack() {
	ADDR dest;
	ADDR source;
	ADDR size;
	try {
		size = PopStack();
		source = PopStack();
		dest = PopStack();
	}
	catch(...) {
		op_exit();
		return;
	}
	WRITE_IP(READ_IP + 1);
}


/* 0x70 - 0x7F */
void CPU::op_stac÷î êk_push() {
	unsigned short SavedIP = READ_IP;
	PushStack(RAM->Read2Bytes(SavedIP + 1));
	WRITE_IP(SavedIP + 3);
}
void CPU::op_stack_push_reg() {
	unsigned short SavedIP = READ_IP;
	BYTE reg = RAM->Read(SavedIP + 1);
	if (!isReg(reg)) {
		op_exit();
		return;
	}
	if (RegType(reg)) {
		PushStack(ReadFromReg(reg));
	}
	else {
		PushStack(ReadFromReg2(reg));
	}
	WRITE_IP(SavedIP + 2);
}
void CPU::op_stack_pop_reg() {
	unsigned short _Value;
	try {
		_Value = PopStack();
	}
	catch (...) {
		op_exit();
		return;
	}
	unsigned short SavedIP = READ_IP;
	BYTE reg = RAM->Read(SavedIP + 1);
	if (!isReg(reg)) {
		op_exit();
		return;
	}
	if (RegType(reg)) {
		WriteToReg(reg, _Value % 256);
	}
	else {
		WriteToReg2(reg, _Value);
	}
	WRITE_IP(SavedIP + 2);
}
void CPU::op_stack_ret() {
	unsigned short _Value;
	try {
		_Value = PopStack();
	}
	catch (...) {
		op_exit();
		return;
	}
	WRITE_IP(_Value);
}
void CPU::op_stack_call() {
	PushStack(READ_IP + 3);
	op_jump_to();
}


void CPU::WriteToReg2(BYTE reg, unsigned short data) {
	ProcData->Write2Bytes(reg, data);
}
unsigned short CPU::ReadFromReg2(BYTE reg) {
	return ProcData->Read2Bytes(GetRegAddr(reg));
}
void CPU::WriteToReg(BYTE reg, BYTE data) {
	ProcData->Write(reg, data);
}
BYTE CPU::ReadFromReg(BYTE reg) {
	return ProcData->Read(GetRegAddr(reg));
}
BYTE CPU::GetRegAddr(BYTE reg) {
	if (!RegType(reg)) {
		return reg * 2;
	}
	else if (isFlag(reg)){
		return reg + 0x10;
	}
	else {
		return 2 + (reg ^ 0x20);
	}
}
bool CPU::RegType(BYTE reg) {
	return (reg & 0x20) == 0x20 || isFlag(reg);
}
bool CPU::isReg(BYTE reg) {
	return reg < 0x28;
}
bool CPU::isFlag(BYTE reg) {
	return reg >= 10 && reg <= 18;
}
#undef IP
#undef READ_IP
#undef WRITE_IP