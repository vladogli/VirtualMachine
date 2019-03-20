#include "OPCodes.h"
void setRegValue(uint8_t *procMemory, uint8_t reg, uint8_t value) {
	if ((reg & 0x20) == 0x20) {
		procMemory[reg ^ 0x20] = value;
		return;
	}
	procMemory[reg + 0x10] = value;
}
// Set 2-byte value to the register
void setReg2Value(uint8_t *procMemory, uint8_t reg, uint16_t value) {
	procMemory[reg * 2] = value % 0x100;
	procMemory[reg * 2 + 1] = uint8_t(value / 0x100);
}
uint8_t getRegValue(uint8_t *procMemory, uint8_t reg) {
	if ((reg & 0x20) == 0x20) {
		return procMemory[reg ^ 0x20];
	}
	return procMemory[reg + 0x10];
}
uint16_t getReg2Value(uint8_t *procMemory, uint8_t reg) {
	return procMemory[reg * 2] + procMemory[reg * 2 + 1] * 0x100;
}

void ret(standartParams) {
	WriteToRegister(IP, GetValueFromStack);
}
void op_jump_if(standartParams) {
	uint16_t Value = ReadFromRegister(IP);
	if (ReadFromRegister(RAM->Read<uint8_t>(Value + 1))) {
		WriteToRegister(IP, RAM->Read<uint16_t>(Value + 2));
		return;
	}
	WriteToRegister(IP, Value + 4);
}
void op_jump_ifnt(standartParams) {
	uint16_t Value = ReadFromRegister(IP);
	if (!ReadFromRegister(RAM->Read<uint8_t>(Value + 1))) {
		WriteToRegister(IP, RAM->Read<uint16_t>(Value + 2));
		return;
	}
	WriteToRegister(IP, Value + 3);
}
void nop(MemoryService*, uint8_t *procMemory) {
	WriteToRegister(IP, ReadFromRegister(IP) + 1);
	return;
}
void push(standartParams) {
	uint16_t Value = ReadFromRegister(IP);
	WriteValueToStack(ReadFromRegister((*RAM).Read<uint8_t>(Value + 1)));
	WriteToRegister(IP, Value + 2);
}
void pop(standartParams) {
	uint16_t Value = ReadFromRegister(IP);
	WriteToRegister((*RAM).Read<uint8_t>(Value + 1),GetValueFromStack);
	WriteToRegister(IP, Value + 2);
}
void mov_reg_mem(standartParams) {
	uint16_t Value = ReadFromRegister(IP);
	WriteToRegister(RAM->Read<uint8_t>(Value + 1), RAM->Read<uint16_t>(Value + 2));
	WriteToRegister(IP, Value + 4);
}
void mov_mem_reg(standartParams) {
	uint16_t Value = ReadFromRegister(IP);
	RAM->Write(
		RAM->Read<uint16_t>(Value + 1), 
		ReadFromRegister(RAM->Read<uint8_t>(Value + 3)));
	WriteToRegister(IP, Value + 4);
}
void mov_reg_reg(standartParams) {
	uint16_t Value = ReadFromRegister(IP);
	WriteToRegister(RAM->Read<uint8_t>(Value + 1), ReadFromRegister(RAM->Read<uint8_t>(Value + 2)));
	WriteToRegister(IP, Value + 3);
}
void set_flag(standartParams) {
	uint16_t Value = ReadFromRegister(IP);
	WriteToRegister(RAM->Read<uint8_t>(Value + 1), true);
	WriteToRegister(IP, Value + 2);
}
void clc_flag(standartParams) {
	uint16_t Value = ReadFromRegister(IP);
	WriteToRegister(RAM->Read<uint8_t>(Value + 1), false);
	WriteToRegister(IP, Value + 2);
}

void jmp(standartParams) {
	WriteToRegister(IP, RAM->Read<uint16_t>(Value + 2));
	return;
}
void call(standartParams) {
	WriteValueToStack(ReadFromRegister(IP));
	WriteToRegister(IP, RAM->Read<uint16_t>(Value + 1));
	return;
}
void xor_reg(standartParams) {
	uint16_t Value = ReadFromRegister(IP);
	uint8_t reg = RAM->Read<uint8_t>(Value + 1);
	uint16_t result = 
		ReadFromRegister(reg) ^ 
		ReadFromRegister(RAM->Read<uint8_t>(Value + 2));
	WriteToRegister(reg, result);

	WriteToRegister(PF, result % 2);
	WriteToRegister(ZF, result == 0);

	WriteToRegister(IP, Value + 3);
}
void or_reg(standartParams) {
	uint16_t Value = ReadFromRegister(IP);
	uint8_t reg = RAM->Read<uint8_t>(Value + 1);
	uint16_t result =
		ReadFromRegister(reg) |
		ReadFromRegister(RAM->Read<uint8_t>(Value + 2));
	WriteToRegister(reg, result);

	WriteToRegister(PF, result % 2);
	WriteToRegister(ZF, result == 0);

	WriteToRegister(IP, Value + 3);
}
void and_reg(standartParams) {
	uint16_t Value = ReadFromRegister(IP);
	uint8_t reg = RAM->Read<uint8_t>(Value + 1);
	uint16_t result =
		ReadFromRegister(reg) &
		ReadFromRegister(RAM->Read<uint8_t>(Value + 2));
	WriteToRegister(reg, result);

	WriteToRegister(PF, result % 2);
	WriteToRegister(ZF, result == 0);

	WriteToRegister(IP, Value + 3);
}
void add_reg(standartParams) {
	uint16_t Value = ReadFromRegister(IP);
	uint8_t reg = RAM->Read<uint8_t>(Value + 1);
	int32_t result =
		int32_t(int16_t(ReadFromRegister(reg))) +
		int16_t(ReadFromRegister(RAM->Read<uint8_t>(Value + 2)));
	WriteToRegister(reg, uint16_t(result));

	WriteToRegister(PF, result % 2);
	WriteToRegister(ZF, result == 0);
	WriteToRegister(OF, result > 32767); 
	WriteToRegister(CF, result < 32768);
	WriteToRegister(SF, result < 0);

	WriteToRegister(IP, Value + 3);
}
void sub_reg(standartParams) {
	uint16_t Value = ReadFromRegister(IP);
	uint8_t reg = RAM->Read<uint8_t>(Value + 1);
	int32_t result =
		int32_t(int16_t(ReadFromRegister(reg))) -
		int16_t(ReadFromRegister(RAM->Read<uint8_t>(Value + 2)));
	WriteToRegister(reg, uint16_t(result));

	WriteToRegister(PF, result % 2);
	WriteToRegister(ZF, result == 0);
	WriteToRegister(OF, result > 32767);
	WriteToRegister(CF, result < 32768);
	WriteToRegister(SF, result < 0);

	WriteToRegister(IP, Value + 3);
}
void mul_reg(standartParams) {
	uint16_t Value = ReadFromRegister(IP);
	uint8_t reg = RAM->Read<uint8_t>(Value + 1);
	int32_t result =
		int32_t(int16_t(ReadFromRegister(reg))) *
		int16_t(ReadFromRegister(RAM->Read<uint8_t>(Value + 2)));
	WriteToRegister(reg, uint16_t(result));

	WriteToRegister(PF, result % 2);
	WriteToRegister(ZF, result == 0);
	WriteToRegister(OF, result > 32767);
	WriteToRegister(CF, result < 32768);
	WriteToRegister(SF, result < 0);

	WriteToRegister(IP, Value + 3);
}

void div_reg(standartParams) {
	uint16_t Value = ReadFromRegister(IP);
	uint8_t reg = RAM->Read<uint8_t>(Value + 1);
	int32_t result =
		int32_t(int16_t(ReadFromRegister(reg))) /
		int16_t(ReadFromRegister(RAM->Read<uint8_t>(Value + 2)));
	WriteToRegister(reg, uint16_t(result));

	WriteToRegister(PF, result % 2);
	WriteToRegister(ZF, result == 0);
	WriteToRegister(OF, result > 32767);
	WriteToRegister(CF, result < 32768);
	WriteToRegister(SF, result < 0);

	WriteToRegister(IP, Value + 3);
}

void xor_flag(standartParams) {
	uint16_t Value = ReadFromRegister(IP);
	uint8_t reg = RAM->Read<uint8_t>(Value + 1);
	bool result =
		ReadFromRegister(reg) ^
		ReadFromRegister(RAM->Read<uint8_t>(Value + 2));
	WriteToRegister(reg, result);
	WriteToRegister(IP, Value + 3);
}
void or_flag(standartParams) {
	uint16_t Value = ReadFromRegister(IP);
	uint8_t reg = RAM->Read<uint8_t>(Value + 1);
	bool result =
		ReadFromRegister(reg) |
		ReadFromRegister(RAM->Read<uint8_t>(Value + 2));
	WriteToRegister(reg, result);
	WriteToRegister(IP, Value + 3);
}
void and_flag(standartParams) {
	uint16_t Value = ReadFromRegister(IP);
	uint8_t reg = RAM->Read<uint8_t>(Value + 1);
	bool result =
		ReadFromRegister(reg) &
		ReadFromRegister(RAM->Read<uint8_t>(Value + 2));
	WriteToRegister(reg, result);
	WriteToRegister(IP, Value + 3);
}
void inc_reg(standartParams) {
	uint16_t Value = ReadFromRegister(IP);
	uint8_t reg = RAM->Read<uint8_t>(Value + 1);
	int32_t result =
		int32_t(int16_t(ReadFromRegister(reg))) + 1;
	WriteToRegister(reg, uint16_t(result));

	WriteToRegister(PF, result % 2);
	WriteToRegister(ZF, result == 0);
	WriteToRegister(OF, result > 32767);
	WriteToRegister(SF, result < 0);

	WriteToRegister(IP, Value + 2);
}
void dec_reg(standartParams) {
	uint16_t Value = ReadFromRegister(IP);
	uint8_t reg = RAM->Read<uint8_t>(Value + 1);
	int32_t result =
		int32_t(int16_t(ReadFromRegister(reg))) - 1;
	WriteToRegister(reg, uint16_t(result));

	WriteToRegister(PF, result % 2);
	WriteToRegister(ZF, result == 0);
	WriteToRegister(OF, result < 32768);
	WriteToRegister(SF, result < 0);

	WriteToRegister(IP, Value + 2);
}
void not_reg(standartParams) {
	uint16_t Value = ReadFromRegister(IP);
	uint8_t reg = RAM->Read<uint8_t>(Value + 1);
	int32_t result =
		!int32_t(int16_t(ReadFromRegister(reg)));
	WriteToRegister(reg, uint16_t(result));

	WriteToRegister(PF, result % 2);
	WriteToRegister(ZF, result == 0);
	WriteToRegister(SF, result < 0);

	WriteToRegister(IP, Value + 2);
}

void not_flag(standartParams) {
	uint16_t Value = ReadFromRegister(IP);
	uint8_t reg = RAM->Read<uint8_t>(Value + 1);
	bool result =
		!int32_t(int16_t(ReadFromRegister(reg)));
	WriteToRegister(reg, uint16_t(result));
	WriteToRegister(IP, Value + 2);
}