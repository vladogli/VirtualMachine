#include "cpu.h"
CPU::CPU() {
	functions = new std::function<void(void)>*[0x100];
	for (unsigned short i = 0; i < 0x100; i++) {
		functions[i] = nullptr;
	}
	memMatrix = new memory(MATRIX_Y_MAX_SIZE * MATRIX_X_MAX_SIZE); // 160 x 90 symbols
	ProcData= new memory(0x30);
	matrix = new BYTE*[MATRIX_Y_MAX_SIZE];
	for (BYTE i = 0; i < MATRIX_Y_MAX_SIZE; i++) {
		matrix[i] = &memMatrix->mem[i * MATRIX_X_MAX_SIZE];
	}
	WRITE_IR(0xFF);
	WRITE_IP(0x410);
	WRITE_SP(0x40);
#define init_op(x,y) functions[x] = new std::function<void(void)>(std::bind(&CPU::y, this));
	init_op(0x0, op_exit);
	init_op(0x1, op_int_tostring);
	init_op(0x2, op_sleep);
	init_op(0x3, op_sleep_reg);
	init_op(0x4, op_jump_to);
	init_op(0x5, op_jump_ift);
	init_op(0x6, op_jump_ifnt);
	init_op(0x7, op_jump_or);
	init_op(0x8, op_jump_nor);
	init_op(0x9, op_jump_xor);
	init_op(0xA, op_jump_nxor);
	init_op(0xB, op_jump_le);
	init_op(0xC, op_jump_greater);
	init_op(0xD, op_set_f);
	init_op(0xE, op_clear_f);

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

	init_op(0x90, op_try_connect);
	init_op(0x91, op_close_connection);
	init_op(0x92, op_open_port);
	init_op(0x93, op_close_port);
	init_op(0x94, op_port_get_state);

	init_op(0xA0, op_getfileinfo);
	init_op(0xA1, op_load_part_reg);
	init_op(0xA2, op_load_part);
	init_op(0xA3, op_save_file_part);
	init_op(0xA4, op_save_file_part_reg);
	 
#undef init_op
}

void CPU::PushStack(unsigned short _Val) {
	unsigned short SP_Val = READ_SP;
	if (SP_Val >= 0xFF) {
		throw STACK_OVERFLOW;
	}
	RAM->Write2Bytes(SP_Val, _Val);
	WRITE_SP(SP_Val + 2);
}
unsigned short CPU::PopStack() {
	unsigned short SP_Val = READ_SP - 2;
	if (SP_Val < 0x40) {
		throw STACK_UNDERFLOW;
	}
	WRITE_SP(SP_Val);
	return RAM->Read2Bytes(SP_Val);
}
void CPU::op_exit() {
	WRITE_IP(0);
	closed = 1;
}
void CPU::op_int_tostring() {

}
void CPU::op_sleep() {
	unsigned short SavedIP = READ_IP;
	unsleep_clock = clock() + RAM->Read2Bytes(SavedIP + 1);
	WRITE_IP(SavedIP + 1);
}
void CPU::op_sleep_reg() {
	unsigned short SavedIP = READ_IP;
	BYTE reg = RAM->Read(SavedIP + 1);
	if (!IsReg(reg)) {
		return;
	}
	if (RegType(reg)) {
		unsleep_clock = clock() + ReadFromReg(reg);
	}
	else {
		unsleep_clock = clock() + ReadFromReg2(reg);
	}
	WRITE_IP(SavedIP + 1);
}

void CPU::op_jump_to() {
	unsigned short SavedIP = READ_IP;
	ADDR addr = RAM->Read2Bytes(SavedIP + 1);
	WRITE_IP(addr);
}
void CPU::op_jump_ift() {
	unsigned short SavedIP = READ_IP;
	BYTE reg = RAM->Read(SavedIP + 1);
	if (!IsReg(reg)) {
		op_exit();
		return;
	}
	if (!ReadFromReg(reg)) {
		WRITE_IP(SavedIP + 4);
		return;
	}
	ADDR addr = RAM->Read2Bytes(SavedIP + 2);
	WRITE_IP(addr);
}
void CPU::op_jump_ifnt() {
	unsigned short SavedIP = READ_IP;
	BYTE reg = RAM->Read(SavedIP + 1);
	if (!IsReg(reg)) {
		op_exit();
		return;
	}
	if (ReadFromReg(reg)) {
		WRITE_IP(SavedIP + 4);
		return;
	}
	ADDR addr = RAM->Read2Bytes(SavedIP + 2);
	WRITE_IP(addr);
}
void CPU::op_jump_or() {
	unsigned short SavedIP = READ_IP;
	BYTE reg1 = RAM->Read(SavedIP + 1);
	BYTE reg2 = RAM->Read(SavedIP + 2);
	if (!IsReg(reg1)|| !IsReg(reg2)) {
		op_exit();
		return;
	}
	if (!(ReadFromReg(reg1) || ReadFromReg(reg2))) {
		WRITE_IP(SavedIP + 5);
		return;
	}
	ADDR addr = RAM->Read2Bytes(SavedIP + 3);
	WRITE_IP(addr);
}
void CPU::op_jump_nor() {
	unsigned short SavedIP = READ_IP;
	BYTE reg1 = RAM->Read(SavedIP + 1);
	BYTE reg2 = RAM->Read(SavedIP + 2);
	if (!IsReg(reg1) || !IsReg(reg2) ) {
		op_exit();
		return;
	}
	if ((ReadFromReg(reg1) || ReadFromReg(reg2))) {
		WRITE_IP(SavedIP + 5);
		return;
	}
	ADDR addr = RAM->Read2Bytes(SavedIP + 3);
	WRITE_IP(addr);
}
void CPU::op_jump_xor() {
	unsigned short SavedIP = READ_IP;
	BYTE reg1 = RAM->Read(SavedIP + 1);
	BYTE reg2 = RAM->Read(SavedIP + 2);
	if (!IsReg(reg1) || !IsReg(reg2)) {
		op_exit();
		return;
	}
	if (!(ReadFromReg(reg1) ^ ReadFromReg(reg2))) {
		WRITE_IP(SavedIP + 5);
		return;
	}
	ADDR addr = RAM->Read2Bytes(SavedIP + 3);
	WRITE_IP(addr);
}
void CPU::op_jump_nxor() {
	unsigned short SavedIP = READ_IP;
	BYTE reg1 = RAM->Read(SavedIP + 1);
	BYTE reg2 = RAM->Read(SavedIP + 2);
	if (!IsReg(reg1)|| !IsReg(reg2)) {
		op_exit();
		return;
	}
	if ((ReadFromReg(reg1) ^ ReadFromReg(reg2))) {
		WRITE_IP(SavedIP + 5);
		return;
	}
	ADDR addr = RAM->Read2Bytes(SavedIP + 3);
	WRITE_IP(addr);
}
void CPU::op_jump_le() {
	unsigned short SavedIP = READ_IP;
	ADDR addr = RAM->Read2Bytes(SavedIP + 1);
	if (((READ_SF ^ READ_OF) | READ_ZF)) {
		return;
	}
	WRITE_IP(addr);
}
void CPU::op_jump_greater() {
	unsigned short SavedIP = READ_IP;
	ADDR addr = RAM->Read2Bytes(SavedIP + 1);
	if (!((READ_SF ^ READ_OF) | READ_ZF)) {
		return;
	}
	WRITE_IP(addr);
}
void CPU::op_set_f() {
	unsigned short SavedIP = READ_IP;
	BYTE reg = RAM->Read(SavedIP + 1);
	if (!IsReg(reg) || !IsFlag(reg)) {
		op_exit();
		return;
	}
	WriteToReg(reg, 1);
	WRITE_IP(SavedIP + 2);
}
void CPU::op_clear_f() {
	unsigned short SavedIP = READ_IP;
	BYTE reg = RAM->Read(SavedIP + 1);
	if (!IsReg(reg) || !IsFlag(reg)) {
		op_exit();
		return;
	}
	WriteToReg(reg, 0);
	WRITE_IP(SavedIP + 2);
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

	int _Value = (int)(short int)SwitchReg(first) ^ (short int)SwitchReg(second);
	WriteReg(first, _Value);
	if (_Value == 0) {
		WRITE_ZF(1);
	}
	else {
		WRITE_ZF(0);
	}
	if (_Value % 2 == 0) {
		WRITE_PF(1);
	}
	else {
		WRITE_PF(0);
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
	int _Value = (int)(short int)SwitchReg(first) | (short int)SwitchReg(second);
	WriteReg(first, _Value);
	if (_Value == 0) {
		WRITE_ZF(1);
	}
	else {
		WRITE_ZF(0);
	}
	if (_Value % 2 == 0) {
		WRITE_PF(1);
	}
	else {
		WRITE_PF(0);
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
	int _Value = (int)(short int)SwitchReg(first) & (short int)SwitchReg(second);
	WriteReg(first, _Value);
	if (_Value == 0) {
		WRITE_ZF(1);
	}
	else {
		WRITE_ZF(0);
	}
	if (_Value % 2 == 0) {
		WRITE_PF(1);
	}
	else {
		WRITE_PF(0);
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
	int _Value = (int)(short int)SwitchReg(first) + (short int)SwitchReg(second);
	WriteReg(first, _Value);
	if (_Value == 0) {
		WRITE_ZF(1);
	}
	else {
		WRITE_ZF(0);
	}
	if (_Value % 2 == 0) {
		WRITE_PF(1);
	}
	else {
		WRITE_PF(0);
	}
	if (_Value < -0x8000 || _Value > 0x7FFF) {
		WRITE_CF(1);
	}
	else {
		WRITE_CF(0);
	}
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
	int _Value = (int)(short int)SwitchReg(first) - (short int)SwitchReg(second);
	WriteReg(first, _Value);
	if (_Value == 0) {
		WRITE_ZF(1);
	}
	else {
		WRITE_ZF(0);
	}
	if (_Value % 2 == 0) {
		WRITE_PF(1);
	}
	else {
		WRITE_PF(0);
	}
	if (_Value < -0x8000 || _Value > 0x7FFF) {
		WRITE_CF(1);
	}
	else {
		WRITE_CF(0);
	}
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
	int _Value = (int)(short int)SwitchReg(first) * (short int)SwitchReg(second);
	WriteReg(first, _Value);
	if (_Value == 0) {
		WRITE_ZF(1);
	}
	else {
		WRITE_ZF(0);
	}
	if (_Value % 2 == 0) {
		WRITE_PF(1);
	}
	else {
		WRITE_PF(0);
	}
	if (_Value <-0x8000 || _Value > 0x7FFF) {
		WRITE_CF(1);
	}
	else {
		WRITE_CF(0);
	}
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
	int _Value = (int)(short int)SwitchReg(first) / (short int)SwitchReg(second);
	WriteReg(first, _Value);
	if (_Value == 0) {
		WRITE_ZF(1);
	}
	else {
		WRITE_ZF(0);
	}
	if (_Value % 2 == 0) {
		WRITE_PF(1);
	}
	else {
		WRITE_PF(0);
	}
	if (_Value < -0x8000 || _Value > 0x7FFF) {
		WRITE_CF(1);
	}
	else {
		WRITE_CF(0);
	}
	WRITE_IP(SavedIP + 3);
}
void CPU::op_inc() {
	unsigned short SavedIP = READ_IP;
	BYTE reg = RAM->Read(SavedIP + 1);
	if (!IsReg2Bytes(reg)) {
		op_exit();
		return;
	}
	int _Value = (int)(short int)SwitchReg(reg) + 1;
	if (_Value == 0) {
		WRITE_ZF(1);
	}
	else {
		WRITE_ZF(0);
	}
	if (_Value % 2 == 0) {
		WRITE_PF(1);
	}
	else {
		WRITE_PF(0);
	}
	if (_Value < -0x8000 || _Value > 0x7FFF) {
		WRITE_CF(1);
	}
	else {
		WRITE_CF(0);
		WriteReg(reg, _Value);
	}
	WRITE_IP(SavedIP + 2);
}
void CPU::op_dec() {
	unsigned short SavedIP = READ_IP;
	BYTE reg = RAM->Read(SavedIP + 1);
	if (!IsReg2Bytes(reg)) {
		op_exit();
		return;
	}
	int _Value = (int)(short int)SwitchReg(reg) + 1;
	if (_Value == 0) {
		WRITE_ZF(1);
	}
	else {
		WRITE_ZF(0);
	}
	if (_Value % 2 == 0) {
		WRITE_PF(1);
	}
	else {
		WRITE_PF(0);
	}
	if (_Value < -0x8000 || _Value > 0x7FFF) {
		WRITE_CF(1);
	}
	else {
		WRITE_CF(0);
		WriteReg(reg, _Value);
	}
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
	int _Value = (int)(short int)SwitchReg(first) - (short int)SwitchReg(second);
	if (_Value == 0) {
		WRITE_ZF(1);
	}
	else {
		WRITE_ZF(0);
	}
	if (_Value % 2 == 0) {
		WRITE_PF(1);
	}
	else {
		WRITE_PF(0);
	}
	if (_Value < 0x8000 || _Value > 0x7FFF) {
		WRITE_CF(1);
	}
	else {
		WRITE_CF(0);
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
	int _Value = (int)(short int)SwitchReg(reg) - integer;
	if (_Value == 0) {
		WRITE_ZF(1);
	}
	else {
		WRITE_ZF(0);
	}
	if (_Value % 2 == 0) {
		WRITE_PF(1);
	}
	else {
		WRITE_PF(0);
	}
	if (_Value < -0x8000 || _Value > 0x7FFF) {
		WRITE_CF(1);
	}
	else {
		WRITE_CF(0);
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
	BYTE reg = RAM->Read(SavedIP + 1);
	unsigned short _Value = RAM->Read2Bytes(SavedIP + 2);
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
	ADDR addr = RAM->Read2Bytes(RAM->Read2Bytes(SavedIP + 1));
	BYTE reg = RAM->Read(SavedIP + 3);
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
	BYTE X = READ_MX;
	BYTE Y = READ_MY;
	if (X + _v.size() + 1 > MATRIX_X_MAX_SIZE) {
		if (Y + 1== MATRIX_Y_MAX_SIZE) {
			op_scroll();
			Y--;
		}
		else {
			Y++;
		}
		X = 0;
	}
	memcpy(&matrix[Y][X], _v.c_str(), _v.size());
	WRITE_MX((unsigned char)_v.size());
	WRITE_MY(Y);
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
	BYTE X = READ_MX;
	BYTE Y = READ_MY;
	if (X + _v.size() + 1 > MATRIX_X_MAX_SIZE) {
		if (Y + 1== MATRIX_Y_MAX_SIZE) {
			op_scroll();
			Y--;
		}
		else {
			Y++;
		}
		X = 0;
	}
	memcpy(&matrix[Y][X], _v.c_str(), _v.size());
	WRITE_MX((unsigned char)_v.size());
	WRITE_MY(Y);
	WRITE_IP(SavedIP + 1);
}
void CPU::op_int_show() {
	unsigned short SavedIP = READ_IP;
	unsigned short _Value = RAM->Read2Bytes(SavedIP + 1);
	BYTE X = READ_MX;
	BYTE Y = READ_MY;
	std::string _v = std::to_string(_Value);
	if (X + _v.size() + 1 > MATRIX_X_MAX_SIZE) {
		if (Y + 1 == MATRIX_Y_MAX_SIZE) {
			op_scroll();
		}
		else {
			Y++;
		}
		X = 0;
	}
	memcpy(&matrix[Y][X], _v.c_str(), _v.size());
	WRITE_MX(X + (unsigned char)_v.size());
	WRITE_MY(Y);
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
	BYTE X = READ_MX;
	BYTE Y = READ_MY;
	std::string _v = std::to_string(_Value);
	if (X + _v.size() + 1> MATRIX_X_MAX_SIZE) {
		if (Y + 1 == MATRIX_Y_MAX_SIZE) {
			op_scroll();
			Y--;
		}
		else {
			Y++;
		}
		X = 0;
	}
	else {
		X++;
	}
	memcpy(&matrix[Y][X], _v.c_str(), _v.size());
	WRITE_MX(X + (unsigned char)_v.size() + 1);
	WRITE_MY(Y);
	WRITE_IP(SavedIP + 2);
}
void CPU::op_char_show_reg() {
	unsigned short SavedIP = READ_IP;
	BYTE reg = RAM->Read(SavedIP + 1);
	if (!IsReg(reg)) {
		op_exit();
		return;
	}
	BYTE X = READ_MX;
	BYTE Y = READ_MY;
	BYTE _value = ReadFromReg(reg);
	if (_value == '\n' || _value == '\r') {
		op_nextline();
		WRITE_IP(SavedIP + 2);
		return;
	}
	if (_value == '\b') {
		matrix[Y][X] = ' ';
		if (X > 0) {
			WRITE_MX(X - 1);
		}
		WRITE_IP(SavedIP + 2);
		return;
	}
	if (X + 1 > MATRIX_X_MAX_SIZE) {
		if (Y + 1 == MATRIX_Y_MAX_SIZE) {
			op_scroll();
		}
		else {
			Y++;
		}
		X = 0;
	}
	else {
		X++;
	}
	matrix[Y][X] = _value;
	WRITE_MX(X);
	WRITE_MY(Y);
	WRITE_IP(SavedIP + 2);
}
void CPU::op_char_show() {
	unsigned short SavedIP = READ_IP;
	BYTE X = READ_MX;
	BYTE Y = READ_MY;
	if (X + 1> MATRIX_X_MAX_SIZE) {
		if (Y + 1 == MATRIX_Y_MAX_SIZE) {
			op_scroll();
		}
		else {
			Y++;
		}
		X = 0;
	}
	else {
		X++;
	}
	matrix[Y][X] = (BYTE)RAM->Read2Bytes(SavedIP + 1);
	WRITE_MX(X);
	WRITE_MY(Y);
	WRITE_IP(SavedIP + 2);
}
void CPU::op_scroll() {
	WRITE_IP(READ_IP + 1);
	if (READ_MY == 0) {
		return;
	}
	for (BYTE i = 1; i < MATRIX_Y_MAX_SIZE; i++) {
		memcpy(matrix[i - 1], matrix[i], MATRIX_X_MAX_SIZE);
	}
	memset(matrix[READ_MY], 0, MATRIX_X_MAX_SIZE);
	WRITE_MY(READ_MY-1);
}
void CPU::op_nextline() {
	BYTE Y = READ_MY + 1;
	if (Y >= MATRIX_Y_MAX_SIZE) {
		op_scroll();
		Y--;
	}
	WRITE_MX(0);
	WRITE_MY(Y);
}
void CPU::op_page() {
	memMatrix->Fill(0, memMatrix->memSize, 0);
	WRITE_MX(0);
	WRITE_MY(0);
	WRITE_IP(READ_IP + 1);
}

void CPU::op_try_connect() {
	unsigned long long int connectID = READ_CRH * 0x10000 + READ_CRL;
	BYTE error = IoConnectToNewDevice(connectID);
	ProcData->Write(0x1B, error);
	if (error == 0) {
		WRITE_CF(1);
	}
	else {
		WRITE_CF(0);
	}
	WRITE_IP(READ_IP + 1);
}
void CPU::op_close_connection() {
	unsigned long long int connectID = READ_CRH * 0x10000 + READ_CRL;
	WRITE_CF(IoIsPortOpened(connectID));
	IoCloseConnection(connectID);
	WRITE_IP(READ_IP + 1);
}
void CPU::op_open_port() {
	unsigned long long int connectID = READ_CRH * 0x10000 + READ_CRL;
	if (IoIsPortOpened(connectID)) {
		WRITE_CERR(1);
		return;
	}
	WRITE_CERR(0);
	OpenPort();
	WRITE_IP(READ_IP + 1);
}
void CPU::op_close_port() {
	unsigned long long int connectID = READ_CRH * 0x10000 + READ_CRL;
	if (IoIsPortOpened(connectID)) {
		WRITE_CERR(1);
		return;
	}
	WRITE_CERR(0);
	ClosePort();
	WRITE_IP(READ_IP + 1);
}
void CPU::op_port_get_state() {
	WRITE_CERR(GetPortState());
	WRITE_IP(READ_IP + 1);
}

void CPU::op_getfileinfo() {
	std::string _Value;
	ADDR addr = ProcData->Read2Bytes(0x29);
	BYTE itr = 1, nowByte = RAM->Read(addr);
	while (nowByte != 0) {
		if (itr > 64 || (nowByte < 32 || nowByte > 126)) {
			WRITE_FERR(2);
			WRITE_FS(0);
			return;
		}
		_Value += nowByte;
		nowByte = RAM->Read(addr + itr);
		itr++;
	}
	if (boost::filesystem::exists(_Value)) {
		WRITE_FS(1 + (BYTE)(boost::filesystem::file_size(_Value) / 0x100));
		WRITE_FERR(1);
	}
	else {
		WRITE_FS(0);
		WRITE_FERR(1);
	}
	WRITE_IP(READ_IP + 1);
}
void CPU::op_save_file_part_reg() {
	std::string _Value;
	ADDR addr = ProcData->Read2Bytes(0x29);
	BYTE itr = 1, nowByte = RAM->Read(addr);
	while (nowByte != 0) {
		if (itr > 64 || (nowByte < 32 || nowByte > 126)) {
			WRITE_FERR(2);
			WRITE_FS(0);
			return;
		}
		_Value += nowByte;
		nowByte = RAM->Read(addr + itr);
		itr++;
	}
	ADDR SavedIP = READ_IP;
	BYTE regPart = RAM->Read(SavedIP + 1);
	if (!IsReg(regPart)) {
		op_exit();
		return;
	}
	BYTE Part = (BYTE)SwitchReg(regPart);
	boost::filesystem::ifstream SaveValue(_Value);
	if (!SaveValue.is_open()) {
		WRITE_FS(0);
		WRITE_FERR(1);
		return;
	}
	BYTE* val = new BYTE[0x10000];
	std::string _V1, _V2;
	while (getline(SaveValue, _V2)) {
		_V1 += _V2;
	}
	memcpy(val, _V1.c_str(), _V1.size());
	memcpy(val + Part * 0x100, &RAM->RAM[0x300], 0x100);
	SaveValue.close();
	boost::filesystem::ofstream saveValue(_Value);
	for (int i = 0; i < 0x10000; i++) {
		saveValue << val[i];
	}
	saveValue.close();
	delete[] val;
	WRITE_IP(SavedIP + 2);
}
void CPU::op_save_file_part() {
	std::string _Value;
	ADDR addr = ProcData->Read2Bytes(0x29);
	BYTE itr = 1, nowByte = RAM->Read(addr);
	while (nowByte != 0) {
		if (itr > 64 || (nowByte < 32 || nowByte > 126)) {
			WRITE_FERR(2);
			WRITE_FS(0);
			return;
		}
		_Value += nowByte;
		nowByte = RAM->Read(addr + itr);
		itr++;
	}
	ADDR SavedIP = READ_IP;
	BYTE Part = RAM->Read(SavedIP + 1);
	boost::filesystem::ifstream SaveValue(_Value);
	if (!SaveValue.is_open()) {
		WRITE_FS(0);
		WRITE_FERR(1);
		return;
	}
	BYTE* val = new BYTE[0x10000];
	std::string _V1, _V2;
	while (getline(SaveValue, _V2)) {
		_V1 += _V2;
	}
	memcpy(val, _V1.c_str(), _V1.size());
	memcpy(val + Part * 0x100, &RAM->RAM[0x300], 0x100);
	SaveValue.close();
	boost::filesystem::ofstream saveValue(_Value);
	for (int i = 0; i < 0x10000; i++) {
		saveValue << val[i];
	}
	saveValue.close();
	delete[] val;
	WRITE_IP(SavedIP + 2);
}
void CPU::op_load_part_reg() {
	std::string _Value;
	ADDR addr = ProcData->Read2Bytes(0x29);
	BYTE itr = 1, nowByte = RAM->Read(addr);
	while (nowByte != 0) {
		if (itr > 64 || (nowByte < 32 || nowByte > 126)) {
			WRITE_FERR(2);
			WRITE_FS(0);
			return;
		}
		_Value += nowByte;
		nowByte = RAM->Read(addr + itr);
		itr++;
	}
	ADDR SavedIP = READ_IP;
	BYTE regPart = RAM->Read(SavedIP + 1);
	if (!IsReg(regPart)) {
		op_exit();
		return;
	}
	BYTE Part = (BYTE)SwitchReg(regPart);
	boost::filesystem::ifstream SaveValue(_Value);
	if (!SaveValue.is_open()) {
		WRITE_FS(0);
		WRITE_FERR(1);
		return;
	}
	BYTE* val = new BYTE[0x10000];
	std::string _V1, _V2;
	while (getline(SaveValue, _V2)) {
		_V1 += _V2;
	}
	memcpy(val, _V1.c_str(), _V1.size());
	memcpy(&RAM->RAM[0x300], val + Part * 0x100, 0x100);
	SaveValue.close();
	boost::filesystem::ofstream saveValue(_Value);
	for (int i = 0; i < 0x10000; i++) {
		saveValue << val[i];
	}
	saveValue.close();
	delete[] val;
	WRITE_IP(SavedIP + 2);
}
void CPU::op_load_part() {
	std::string _Value;
	ADDR addr = ProcData->Read2Bytes(0x29);
	BYTE itr = 1, nowByte = RAM->Read(addr);
	while (nowByte != 0) {
		if (itr > 64 || (nowByte < 32 || nowByte > 126)) {
			WRITE_FERR(2);
			WRITE_FS(0);
			return;
		}
		_Value += nowByte;
		nowByte = RAM->Read(addr + itr);
		itr++;
	}
	ADDR SavedIP = READ_IP;
	BYTE Part = RAM->Read(SavedIP + 1);
	boost::filesystem::ifstream SaveValue(_Value);
	if (!SaveValue.is_open()) {
		WRITE_FS(0);
		WRITE_FERR(1);
		return;
	}
	BYTE* val = new BYTE[0x10000];
	std::string _V1, _V2;
	while (getline(SaveValue, _V2)) {
		_V1 += _V2;
	}
	memcpy(val, _V1.c_str(), _V1.size());
	memcpy(&RAM->RAM[0x300], val + Part * 0x100, 0x100);
	SaveValue.close();
	boost::filesystem::ofstream saveValue(_Value);
	for (int i = 0; i < 0x10000; i++) {
		saveValue << val[i];
	}
	saveValue.close();
	delete[] val;
	WRITE_IP(SavedIP + 2);
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
	if (reg <= 0x8) {
		return reg * 2;
	}
	else if (IsFlag(reg)){
		return reg + 0x10;
	}
	else {
		if ((reg & 0x60) == 0x60) {
			return 0x29 + (reg ^ 0x60) * 2;
		}
		if ((reg & 0x50) == 0x50) {
			return 0x17 + (reg ^ 0x50) * 2;
		}
		if ((reg & 0x40) == 0x40) {
			return 0x14 + (reg ^ 0x40);
		}
		if ((reg & 0x30) == 0x30) {
			return 0x12 + (reg ^ 0x30);
		}
		else {
			return 0x02 + (reg ^ 0x20);
		}
	}
}
bool CPU::RegType(BYTE reg) {
	return (reg >= 0x20 && reg <= 0x28) || 
		(reg >= 0x30 && reg <= 0x31) || 
		(reg >= 0x10 && reg <= 0x18) || 
		reg == 0x40 ||
		reg == 0x62;
}
bool CPU::IsReg(BYTE reg) {
	return reg < 9 ||                // Registers
		(reg >= 0x20 && reg <= 0x28) ||  // Low-Registers
		(reg >= 0x10 && reg <= 0x18) ||  // Flags
		(reg >= 0x30 && reg <= 0x31) ||  // Matrix registers
		(reg >= 0x40 && reg <= 0x41) ||  // Interrupt registers
		(reg >= 0x50 && reg <= 0x52) ||  // ConnectToDevice registers
		(reg >= 0x60 && reg <= 0x62);    // Filesystem registers
}
bool CPU::IsFlag(BYTE reg) {
	return reg >= 10 && reg <= 18;
}
void CPU::NextOp() {
#ifdef DEBUG_MODE
	BYTE VIR = READ_IR;
	BYTE VIF = READ_IF;
	ADDR VIP = READ_IP;
	BYTE OP = RAM->Read(READ_IP);
#endif
	if (READ_IR != 0xFF) {
		if (!READ_IF) {
			PushStack(READ_IP);
			WRITE_IP(RAM->Read2Bytes(READ_IR * 2)); 
			WRITE_IR(0xFF);
		}
		closed = 0;
	}
	if (clock() < unsleep_clock) {
		return;
	}
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
void CPU::keyPressed(BYTE key) {
	WRITE_IV(key);
	WRITE_IR(0);
}
