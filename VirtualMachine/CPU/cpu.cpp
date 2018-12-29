#include "cpu.h"
#define READ(v)    ProcData->Read2Bytes(v)
#define WRITE(x,v) ProcData->Write2Bytes(x, v)

#define IP 0x00
#define READ_IP     READ(IP)
#define WRITE_IP(x) WRITE(IP, x)
#define SP 0x10
#define READ_SP     READ(SP)
#define WRITE_SP(x) WRITE(SP,x)

#define ZF 0x23
#define READ_ZF     READ(ZF)
#define WRITE_ZF(x) WRITE(ZF,x)

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
void CPU::op_string_concat() {
	unsigned short SavedIP = READ_IP;
	BYTE dest   = RAM->Read(SavedIP + 1);
	BYTE first  = RAM->Read(SavedIP + 2);
	BYTE second = RAM->Read(SavedIP + 3);
#define isReg2Bytes(x) (IsReg(x) && !RegType(x))
	if (isReg2Bytes(dest) && isReg2Bytes(first) && isReg2Bytes(second)) {
#undef isReg2Bytes
		ADDR destAddr = ReadFromReg2(dest);
		ADDR sourceFirst = ReadFromReg2(first);
		ADDR sourceSecond = ReadFromReg2(second);
		BYTE itr = 0, itr2 = 0;
		while (itr < 128) {
			BYTE ReadedValue = RAM->Read(sourceFirst + itr);
			if (ReadedValue == 0) {
				break;
			}
			RAM->Write(destAddr + itr, ReadedValue);
			itr++;
		}
		while (itr2 < 128) {
			BYTE ReadedValue = RAM->Read(sourceSecond + itr2);
			if (ReadedValue == 0) {
				break;
			}
			RAM->Write(destAddr + itr, ReadedValue);
			itr++;
			itr2++;
		}
	}
	else {
		op_exit();
		return;
	}
	WRITE_IP(SavedIP + 4);
}
void CPU::op_string_toint() {
	unsigned short SavedIP = READ_IP;
	BYTE reg    = RAM->Read(SavedIP + 1);
	BYTE strPtr = RAM->Read(SavedIP + 2);
	if (!IsReg(reg) || !IsReg(strPtr) || RegType(strPtr)) {
		op_exit();
		return;
	}
	ADDR str = ReadFromReg2(strPtr);
	unsigned short WriteValue = 0;
	int itr = 0;
	std::string _Value;
	while (_Value[itr] != 0) {
		if (itr >= 7) {
			op_exit();
			return;
		}
		_Value += RAM->Read(str + itr);
		itr++;
	}
	if (_Value.size() > 2 && _Value[0] == '0' && _Value[1] == 'x') {
		itr = 0;
		for (size_t x = _Value.size(); x > 1; x--,itr++) {
			if (_Value[x] >= '0' && _Value[x] <= '9') {
				WriteValue += (_Value[x] - '0') * (unsigned short)std::pow(0x10, itr);
			}
			else if(_Value[x] >= 'a' && _Value[x] >= 'f') {
				WriteValue += (_Value[x] = 'a') * (unsigned short)std::pow(0x10, itr);
			}
			else {
				break;
			}
		}
	}
	else {
		itr = 0;
		for (size_t x = _Value.size(); x > 1; x--, itr++) {
			if (_Value[x] >= '0' && _Value[x] <= '9') {
				WriteValue += (_Value[x] - '0') * (unsigned short)std::pow(10, itr);
			}
			else {
				break;
			}
		}
	}
	if (RegType(reg)) {// 1 byte
		WriteToReg(reg, (BYTE)WriteValue);
	}
	else {               // 2 bytes
		WriteToReg2(reg, WriteValue);
	}
	WRITE_IP(SavedIP + 3);
}
void CPU::op_string_size() {
	unsigned short SavedIP = READ_IP;
	BYTE dest = RAM->Read(SavedIP + 1);
	BYTE str = RAM->Read(SavedIP + 2);
	if (!IsReg(dest) || !IsReg(str) || RegType(dest)) {
		op_exit();
		return;
	}
	ADDR strAddr = ReadFromReg2(str);
	BYTE itr = 0;
	while (RAM->Read(strAddr+itr)!=0) {
		if (itr < 128) {
			WriteToReg(dest, 0);
		}
	}
	WRITE_IP(SavedIP + 3);
}

/* 0x40 - 0x4F */
void CPU::op_cmp_reg() {
	unsigned short SavedIP = READ_IP;
	BYTE first  = RAM->Read(SavedIP + 1);
	BYTE second = RAM->Read(SavedIP + 2);
	if (!IsReg(first) || !IsReg(second) || RegType(first) != RegType(second)) {
		op_exit();
		return;
	}
	if (RegType(first)) {// 1 byte
		WRITE_ZF(ReadFromReg(first) == ReadFromReg(second));
	}
	else {               // 2 bytes
		WRITE_ZF(ReadFromReg2(first) == ReadFromReg2(second));
	}
	WRITE_IP(SavedIP + 3);
}
void CPU::op_cmp_immediate() {
	unsigned short SavedIP = READ_IP;
	BYTE reg = RAM->Read(SavedIP + 1);
	unsigned short integer = RAM->Read2Bytes(SavedIP + 2);
	if (!IsReg(reg)) {
		op_exit();
		return;
	}
	if (RegType(reg)) {// 1 byte
		WRITE_ZF(ReadFromReg(reg) == integer);
	}
	else {               // 2 bytes
		WRITE_ZF(ReadFromReg2(reg) == integer);
	}
	WRITE_IP(SavedIP + 4);
}
void CPU::op_cmp_string() {
	unsigned short SavedIP = READ_IP;
	BYTE regBuf = RAM->Read(SavedIP + 1);
	ADDR firstAddr, secondAddr;
	if (!IsReg(regBuf) || RegType(regBuf)) {
		op_exit();
		return;
	}
	firstAddr = ReadFromReg2(regBuf);
	regBuf = RAM->Read(SavedIP + 2);
	if (!IsReg(regBuf) || RegType(regBuf)) {
		op_exit();
		return;
	}
	secondAddr = ReadFromReg2(regBuf);
	BYTE itr = 0, itr2 = 0;
	while (RAM->Read(firstAddr+itr) != 0) {
		if (itr > 128) {
			op_exit();
			return;
		}
		itr++;
	}
	while (RAM->Read(secondAddr + itr2) != 0) {
		if (itr2 > 128) {
			op_exit();
			return;
		}
		itr2++;
	}
	if (itr != itr2) {
		WRITE_ZF(0);
	}
	else {
		itr = 0;
		bool FVal = 1;
		while (itr != itr2)
		{
			FVal &= RAM->Read(firstAddr + itr) == RAM->Read(secondAddr + itr);
			itr++;
		}
		WRITE_ZF(FVal);
	}
	WRITE_IP(SavedIP + 3);
}

/* 0x50 - 0x5F */
void CPU::op_nop() {
	WRITE_IP(READ_IP + 1);
}
void CPU::op_reg_store() {
	unsigned short SavedIP = READ_IP;
	BYTE dest = RAM->Read(SavedIP + 1);
	BYTE source = RAM->Read(SavedIP + 2);
	if (!IsReg(dest) || !IsReg(source)) {
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
	if (!IsReg(reg)) {
		op_exit();
		return;
	}
	if (RegType(reg)) { // 1-byte register
		WriteToReg(reg, _Value % 256);
	}
	else {              // 2-byte register
		WriteToReg2(reg, _Value);
	}
	WRITE_IP(SavedIP + 4);
}
void CPU::op_poke() {
	unsigned short SavedIP = READ_IP;
	BYTE reg = RAM->Read(SavedIP + 1);
	ADDR addr= RAM->Read2Bytes(RAM->Read2Bytes(SavedIP + 2));
	if (!IsReg(reg)) {
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
	if (!IsReg(dest) || !IsReg(source) || !IsReg(size) || !RegType(dest) || !RegType(source) || !RegType(size)) {
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
void CPU::op_stack_push() {
	unsigned short SavedIP = READ_IP;
	PushStack(RAM->Read2Bytes(SavedIP + 1));
	WRITE_IP(SavedIP + 3);
}
void CPU::op_stack_push_reg() {
	unsigned short SavedIP = READ_IP;
	BYTE reg = RAM->Read(SavedIP + 1);
	if (!IsReg(reg)) {
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
	if (!IsReg(reg)) {
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
	ProcData->Write2Bytes(GetRegAddr(reg), data);
}
unsigned short CPU::ReadFromReg2(BYTE reg) {
	return ProcData->Read2Bytes(GetRegAddr(reg));
}
void CPU::WriteToReg(BYTE reg, BYTE data) {
	ProcData->Write(GetRegAddr(reg), data);
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
bool CPU::IsReg(BYTE reg) {
	return reg < 0x28;
}
bool CPU::isFlag(BYTE reg) {
	return reg >= 10 && reg <= 18;
}
#undef IP
#undef READ_IP
#undef WRITE_IP
#undef READ
#undef WRITE