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

#define SF 0x23
#define READ_SF     READ(SF)
#define WRITE_SF(x) WRITE(SF,x)

#define OF 0x23
#define READ_OF     READ(OF)
#define WRITE_OF(x) WRITE(OF,x)

#define IsReg2Bytes(x) (IsReg(x) && !RegType(x))

CPU::CPU() {
	functions = new std::function<void(void)>*[0x100];
	for (unsigned short i = 0; i < 0x100; i++) {
		functions[i] = nullptr;
	}
	memMatrix = new memory(0x3840); // 160 x 90 symbols
	stack   = new memory(0xFFFF);
	ProcData= new memory(0x40);
	matrix = new BYTE*[90];
	for (BYTE i = 0; i < 90; i++) {
		matrix[i] = &memMatrix->mem[i * 160];
	}
#define init_op(x,y) functions[x] = new std::function<void(void)>(std::bind(&CPU::y, this));
	init_op(0, op_exit);
	init_op(1, op_int_tostring);
	init_op(0x10, op_jump_to);
	init_op(0x11, op_jump_z);
	init_op(0x12, op_jump_nz);

	init_op(0x20, op_xor);
	init_op(0x21, op_or);
	init_op(0x22, op_and);
	init_op(0x23, op_add);
	init_op(0x24, op_sub);
	init_op(0x25, op_mul);
	init_op(0x26, op_div);
	init_op(0x27, op_inc);
	init_op(0x28, op_dec);

	init_op(0x30, op_string_concat);
	init_op(0x31, op_string_toint);
	init_op(0x32, op_string_size);

	init_op(0x40, op_cmp_reg);
	init_op(0x41, op_cmp_immediate);
	init_op(0x42, op_cmp_string);

	init_op(0x50, op_nop);
	init_op(0x51, op_reg_store);

	init_op(0x60, op_peek);
	init_op(0x61, op_poke);
	init_op(0x62, op_memcpy);
	init_op(0x63, op_memcpy_reg);
	init_op(0x64, op_memcpy_stack);

	init_op(0x70, op_stack_push);
	init_op(0x71, op_stack_push_reg);
	init_op(0x72, op_stack_pop_reg);
	init_op(0x73, op_stack_ret);
	init_op(0x74, op_stack_call);

	init_op(0x80, op_string_show);
	init_op(0x81, op_string_show_reg);
	init_op(0x82, op_int_show);
	init_op(0x83, op_int_show_reg);
	init_op(0x84, op_char_show);
	init_op(0x85, op_char_show_reg);
	init_op(0x86, op_scroll);
	init_op(0x87, op_nextline);
	init_op(0x88, op_page);
	 
#undef init_op
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
	WRITE_IP(0);
	closed = 1;
}
void CPU::op_int_tostring() {

}
/* 0x10 - 0x1F */
void CPU::op_jump_to() {
	ADDR _Value = RAM->Read2Bytes(READ_IP + 1);
	WRITE_IP(_Value);
	return;
}
void CPU::op_jump_z() {
	if (READ_ZF) {
		ADDR _Value = RAM->Read2Bytes(READ_IP + 1);
		WRITE_IP(_Value);
		return;
	}
	WRITE_IP(READ_IP + 3);
}
void CPU::op_jump_nz() {
	if (!READ_ZF) {
		ADDR _Value = RAM->Read2Bytes(READ_IP + 1);
		WRITE_IP(_Value);
		return;
	}
	WRITE_IP(READ_IP + 3);
}

/* 0x20 - 0x2F */
void CPU::op_xor() {
	unsigned short SavedIP = READ_IP;
	BYTE first = RAM->Read(SavedIP + 1);
	BYTE second = RAM->Read(SavedIP + 2);
	if (!IsReg(first) || !IsReg(second)) {
		op_exit();
		return;
	}
	if (RegType(first)) {
		WRITE_ZF(ReadFromReg(first) ^ ReadFromReg(second));
	}
	else {
		WRITE_ZF(ReadFromReg2(first) ^ ReadFromReg2(second));
	}
	WRITE_IP(SavedIP + 3);
}
void CPU::op_or() {
	unsigned short SavedIP = READ_IP;
	BYTE first = RAM->Read(SavedIP + 1);
	BYTE second = RAM->Read(SavedIP + 2);
	if (!IsReg(first) || !IsReg(second)) {
		op_exit();
		return;
	}
	if (RegType(first)) {
		WRITE_ZF(ReadFromReg(first) || ReadFromReg(second));
	}
	else {
		WRITE_ZF(ReadFromReg2(first) || ReadFromReg2(second));
	}
	WRITE_IP(SavedIP + 3);
}
void CPU::op_and() {
	unsigned short SavedIP = READ_IP;
	BYTE first = RAM->Read(SavedIP + 1);
	BYTE second = RAM->Read(SavedIP + 2);
	if (!IsReg(first) || !IsReg(second)) {
		op_exit();
		return;
	}
	if (RegType(first)) {
		WRITE_ZF(ReadFromReg(first) == ReadFromReg(second));
	}
	else {
		WRITE_ZF(ReadFromReg2(first) == ReadFromReg2(second));
	}
	WRITE_IP(SavedIP + 3);
}
void CPU::op_add() {
	unsigned short SavedIP = READ_IP;
	BYTE first = RAM->Read(SavedIP + 1);
	BYTE second = RAM->Read(SavedIP + 2);
	if (!IsReg(first) || !IsReg(second) || IsFlag(first) || IsFlag(second)) {
		op_exit();
		return;
	}
	unsigned int _Value = (unsigned int)(ReadFromReg2(first)) + ReadFromReg2(second);
	if (_Value > 65535) {
		WRITE_OF(1);
	}
	else if (_Value == 0){
		WRITE_ZF(0);
	}
	else {
		WRITE_OF(0);
	}
	WriteToReg2(first, (unsigned short)_Value);
	WRITE_IP(SavedIP + 3);
}
void CPU::op_sub() {
	unsigned short SavedIP = READ_IP;
	BYTE first = RAM->Read(SavedIP + 1);
	BYTE second = RAM->Read(SavedIP + 2);
	if (!IsReg(first) || !IsReg(second) || IsFlag(first) || IsFlag(second)) {
		op_exit();
		return;
	}
	short _Value = (ReadFromReg2(first)) - ReadFromReg2(second);
	if (_Value < 0) {
		WRITE_SF(1);
		WRITE_ZF(0);
	}
	else if(_Value == 0){
		WRITE_SF(0);
		WRITE_ZF(1);
	}
	else {
		WRITE_ZF(0);
		WRITE_SF(0);
	}
	WriteToReg2(first, (unsigned short)_Value);
	WRITE_IP(SavedIP + 3);
}
void CPU::op_mul() {
	unsigned short SavedIP = READ_IP;
	BYTE first = RAM->Read(SavedIP + 1);
	BYTE second = RAM->Read(SavedIP + 2);
	if (!IsReg(first) || !IsReg(second) || IsFlag(first) || IsFlag(second)) {
		op_exit();
		return;
	}
	unsigned int _Value = (unsigned int)(ReadFromReg2(first)) * ReadFromReg2(second);
	if (_Value > 65535) {
		WRITE_OF(1);
	}
	else {
		WRITE_OF(0);
	}
	WriteToReg2(first, (unsigned short)_Value);
	WRITE_IP(SavedIP + 3);
}
void CPU::op_div() {
	unsigned short SavedIP = READ_IP;
	BYTE first  = RAM->Read(SavedIP + 1);
	BYTE second = RAM->Read(SavedIP + 2);
	if (!IsReg(first) || !IsReg(second) || IsFlag(first) || IsFlag(second)) {
		op_exit();
		return;
	}
	unsigned short _Value = ReadFromReg2(first) / ReadFromReg2(second);
	WriteToReg2(first, _Value);
	WRITE_IP(SavedIP + 3);
}
void CPU::op_inc() {
	unsigned short SavedIP = READ_IP;
	BYTE reg = RAM->Read(SavedIP + 1);
	if (!IsReg2Bytes(reg)) {
		op_exit();
		return;
	}
	short int _Value = ReadFromReg2(reg) + 1;
	WRITE_SF(0);
	WRITE_ZF(0);
	WRITE_OF(0);
	if (_Value < 0) {
		WRITE_SF(1);
	}
	else if (_Value == 0) {
		WRITE_ZF(1);
	}
	else if (_Value == 32767) {
		WRITE_OF(1);
	}
	WriteToReg2(reg, _Value);
	WRITE_IP(SavedIP + 2);
}
void CPU::op_dec() {
	unsigned short SavedIP = READ_IP;
	BYTE reg = RAM->Read(SavedIP + 1);
	if (!IsReg2Bytes(reg)) {
		op_exit();
		return;
	}
	short int _Value = ReadFromReg2(reg) - 1;
	WRITE_SF(0);
	WRITE_ZF(0);
	if (_Value < 0) {
		WRITE_SF(1);
	}
	else if (_Value == 0) {
		WRITE_ZF(1);
	}
	WriteToReg2(reg, (unsigned short)_Value);
	WRITE_IP(SavedIP + 2);
}

/* 0x30 - 0x3F */
void CPU::op_string_concat() {
	unsigned short SavedIP = READ_IP;
	BYTE dest   = RAM->Read(SavedIP + 1);
	BYTE first  = RAM->Read(SavedIP + 2);
	BYTE second = RAM->Read(SavedIP + 3);
	if (IsReg2Bytes(dest) && IsReg2Bytes(first) && IsReg2Bytes(second)) {
#undef IsReg2Bytes
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

#define MX 0x30
#define MY 0x31
/* 0x80 - 0x8F */
void CPU::op_string_show() {
	unsigned short SavedIP = READ_IP;
	ADDR addr = RAM->Read2Bytes(SavedIP+1);
	std::string _v;
	int itr = 0;
	BYTE v = RAM->Read(addr + itr);
	do {
		itr++;
		if (itr >= 64) {
			op_exit();
			return;
		}
		v = RAM->Read(addr + itr);
		_v += v;
	} while (v != 0);
	BYTE X = ReadFromReg(MX);
	BYTE Y = ReadFromReg(MY);
	if (X + _v.size() + 1 > MATRIX_X_MAX_SIZE) {
		if (Y == MATRIX_Y_MAX_SIZE) {
			op_scroll();
			Y--;
		}
		else {
			Y++;
		}
		X = 0;
	}
	memcpy(&matrix[Y][X], _v.c_str(), _v.size());
	WriteToReg(MX, X + (unsigned char)_v.size() + 1);
	WriteToReg(MY, Y);
	WRITE_IP(SavedIP + 3);
}
void CPU::op_string_show_reg() {
	unsigned short SavedIP = READ_IP;
	BYTE reg = RAM->Read(SavedIP + 1);
	if (!IsReg(reg) || RegType(reg)) {
		op_exit();
		return;
	}
	ADDR addr = ReadFromReg2(reg);
	std::string _v;
	int itr = 0;
	BYTE v = RAM->Read(addr + itr);
	do {
		itr++;
		if (itr >= 64) {
			op_exit();
			return;
		}
		v = RAM->Read(addr + itr);
		_v += v;
	} while (v != 0);
	BYTE X = ReadFromReg(MX);
	BYTE Y = ReadFromReg(MY);
	if (X + _v.size() + 1 > MATRIX_X_MAX_SIZE) {
		if (Y == MATRIX_Y_MAX_SIZE) {
			op_scroll();
			Y--;
		}
		else {
			Y++;
		}
		X = 0;
	}
	memcpy(&matrix[Y][X], _v.c_str(), _v.size());
	WriteToReg(MX, X + (unsigned char)_v.size() + 1);
	WriteToReg(MY, Y);
	WRITE_IP(SavedIP + 1);
}
void CPU::op_int_show() {
	unsigned short SavedIP = READ_IP;
	unsigned short _Value = RAM->Read2Bytes(SavedIP + 1);
	BYTE X = ReadFromReg(MX);
	BYTE Y = ReadFromReg(MY);
	std::string _v = std::to_string(_Value);
	if (X + _v.size() + 1 > MATRIX_X_MAX_SIZE) {
		if (Y == MATRIX_Y_MAX_SIZE) {
			op_scroll();
			Y--;
		}
		else {
			Y++;
		}
		X = 0;
	}
	memcpy(&matrix[Y][X], _v.c_str(), _v.size());
	WriteToReg(MX, X + (unsigned char)_v.size() + 1);
	WriteToReg(MY, Y);
	WRITE_IP(SavedIP + 3);

}
void CPU::op_int_show_reg() {
	unsigned short SavedIP = READ_IP;
	BYTE reg = RAM->Read(SavedIP + 1);
	if (!IsReg(reg)) {
		op_exit();
		return;
	}
	unsigned short _Value;
	if (RegType(reg)) {
		_Value = ReadFromReg(reg);
	}
	else {
		_Value = ReadFromReg2(reg);
	}
	BYTE X = ReadFromReg(MX);
	BYTE Y = ReadFromReg(MY);
	std::string _v = std::to_string(_Value);
	if (X + _v.size() + 1> MATRIX_X_MAX_SIZE) {
		if (Y == MATRIX_Y_MAX_SIZE) {
			op_scroll();
			Y--;
		}
		else {
			Y++;
		}
		X = 0;
	}
	memcpy(&matrix[Y][X], _v.c_str(), _v.size());
	WriteToReg(MX, X + (unsigned char)_v.size() + 1);
	WriteToReg(MY, Y);
	WRITE_IP(SavedIP + 2);
}
void CPU::op_char_show_reg() {
	unsigned short SavedIP = READ_IP;
	BYTE reg = RAM->Read(SavedIP + 1);
	if (!IsReg(reg)) {
		op_exit();
		return;
	}
	BYTE X = ReadFromReg(MX);
	BYTE Y = ReadFromReg(MY);
	if (X > MATRIX_X_MAX_SIZE) {
		if (Y + 1 == MATRIX_Y_MAX_SIZE) {
			op_scroll();
		}
		X = 0;
	}
	matrix[Y][X] = ReadFromReg(reg);
	WriteToReg(MX, X);
	WriteToReg(MY, Y);
	WRITE_IP(SavedIP + 2);
}
void CPU::op_char_show() {
	unsigned short SavedIP = READ_IP;
	BYTE X = ReadFromReg(MX);
	BYTE Y = ReadFromReg(MY);
	if (X > MATRIX_X_MAX_SIZE) {
		if (Y + 1 == MATRIX_Y_MAX_SIZE) {
			op_scroll();
		}
		X = 0;
	}
	matrix[Y][X] = RAM->Read(SavedIP + 1);
	WriteToReg(MX, X);
	WriteToReg(MY, Y);
	WRITE_IP(SavedIP + 2);
}
void CPU::op_scroll() {
	WRITE_IP(READ_IP + 1);
	if (ReadFromReg(MY) == 0) {
		return;
	}
	for (BYTE i = 1; i < 90; i++) {
		memcpy(matrix[i - 1], matrix[i], 160);
	}
	WriteToReg(MY, ReadFromReg(MY) - 1);
}
void CPU::op_nextline() {
	BYTE Y = ReadFromReg(MY) + 1;
	if (Y >= MATRIX_Y_MAX_SIZE) {
		op_scroll();
		Y--;
	}
	WriteToReg(MY, Y);
	WriteToReg(MX, 0);

}
void CPU::op_page() {
	memMatrix->Fill(0, memMatrix->memSize, 0);
	WriteToReg(MX, 0);
	WriteToReg(MY, 0);
	WRITE_IP(READ_IP + 1);
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
	else if (IsFlag(reg)){
		return reg + 0x10;
	}
	else {
		if ((reg & 0x20) == 0x20) {
			return 2 + (reg ^ 0x20);
		}
		else {
			return 0x10 + (reg ^ 0x30);
		}
	}
}
bool CPU::RegType(BYTE reg) {
	return (reg & 0x20) == 0x20 || IsFlag(reg) || (reg & 0x30) == 0x30;
}
bool CPU::IsReg(BYTE reg) {
	return reg < 9 ||                // Registers
		(reg >= 20 && reg <= 28) ||  // Low-Registers
		(reg >= 10 && reg <= 18) ||  // Flags
		(reg >= 30 && reg <= 31);    // Matrix Registers
}
bool CPU::IsFlag(BYTE reg) {
	return reg >= 10 && reg <= 18;
}


void CPU::NextOp() {
	if (closed) { 
		return; 
	}
	BYTE op = RAM->Read(READ_IP);
	if (functions[op] == nullptr) {
		op_nop();
		return;
	}
	(*functions[op])();
}
#undef READ
#undef WRITE

#undef IP
#undef READ_IP     
#undef WRITE_IP

#undef SP
#undef READ_SP
#undef WRITE_SP

#undef ZF 
#undef READ_ZF     
#undef WRITE_ZF

#undef SF 
#undef READ_SF     
#undef WRITE_SF

#undef OF 
#undef READ_OF     
#undef WRITE_OF

#undef IsReg2Bytes
