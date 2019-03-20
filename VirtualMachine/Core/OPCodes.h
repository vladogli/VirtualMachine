#pragma once
#include "MemoryService.h"
// Set 1-byte value to the register
void setRegValue(uint8_t *procMemory, uint8_t reg, uint8_t value);
// Set 2-byte value to the register
void setReg2Value(uint8_t *procMemory, uint8_t reg, uint16_t value);
// Set 1-byte value to the register
uint8_t getRegValue(uint8_t *procMemory, uint8_t reg);
// Set 2-byte value to the register
uint16_t getReg2Value(uint8_t *procMemory, uint8_t reg);

#define IP uint8_t(0x00)
#define AX uint8_t(0x01)
#define BX uint8_t(0x02)
#define CX uint8_t(0x03)
#define DX uint8_t(0x04)
#define SI uint8_t(0x05)
#define DI uint8_t(0x06)
#define BP uint8_t(0x07)
#define SP uint8_t(0x08)
#define IV uint8_t(0x09)
#define IR uint8_t(0x0A)

#define CF uint8_t(0x10)
#define PF uint8_t(0x11)
#define AF uint8_t(0x12)
#define ZF uint8_t(0x13)
#define SF uint8_t(0x14)
#define TF uint8_t(0x15)
#define IF uint8_t(0x16)
#define DF uint8_t(0x17)
#define OF uint8_t(0x18)

#define AL uint8_t(0x20)
#define AH uint8_t(0x21)
#define BL uint8_t(0x22)
#define BH uint8_t(0x23)
#define CL uint8_t(0x24)
#define CH uint8_t(0x25)
#define DL uint8_t(0x26)
#define DH uint8_t(0x27)

uint16_t ReadFromRegister(uint8_t *procMemory, uint8_t reg) {
	if (reg < 0x0B) {
		return getReg2Value(procMemory, reg);
	}
	else if(reg<0x19) {
		return bool(getRegValue(procMemory, reg));
	}
	else if (reg < 0x28) {
		return getRegValue(procMemory, reg);
	}
	return 0;
}
void WriteToRegister(uint8_t *procMemory, uint8_t reg, uint16_t value) {
	if (reg < 0x0B) {
		setReg2Value(procMemory, reg, value);
	}
	else if (reg < 0x19) {
		setRegValue(procMemory, reg, bool(value));
	}
	else if (reg < 0x28) {
		setReg2Value(procMemory, reg, uint8_t(value));
	}
}
#define ReadFromRegister(x) ReadFromRegister(procMemory, x)
#define WriteToRegister(x,y) WriteToRegister(procMemory, x, y)
#define standartParams  MemoryService *RAM, uint8_t *procMemory

uint16_t GetValueFromStack(standartParams) {
	uint16_t ADDR = ReadFromRegister(SP);
	WriteToRegister(SP, uint16_t(ADDR - 2));
	return (*RAM).Read<uint16_t>(ADDR);
}
void WriteValueToStack(uint16_t value, standartParams) {
	uint16_t ADDR = ReadFromRegister(SP); 
	WriteToRegister(SP, uint16_t(ADDR + 2));
	(*RAM).Write(ADDR, value);
}
#define GetValueFromStack GetValueFromStack(RAM, procMemory)
#define WriteValueToStack(val) WriteValueToStack(val,RAM,procMemory)
// 0x00-0x0F
// idk
 /* Mnemonic           opcode    NumOfBytes
*  ret(urn)           0x00      0x01
*
*  Description:
*  Gets value from stack and going to this address
*
*  Example:
*  ret
*/
void ret(standartParams);
/* Mnemonic           opcode    NumOfBytes
*  jif $flag, #LLHH   0x01      0x04
*
*  Description:
*  Jumps to addr if flag == true
*
*  Example:
*  jif ZF, 1234h
*/
void op_jump_if(standartParams);

/* Mnemonic           opcode    NumOfBytes
*  jnif $flag, #LLHH  0x02      0x04
*
*  Description:
*  Jumps if flag == false
*
*  Example:
*  jnif ZF, 1234h
*/
void op_jump_ifnt(standartParams);

/* Mnemonic           opcode    NumOfBytes
*  nop                0x03      0x01
*
*  Description:
*  No operation
*
*  Example:
*  nop
*/
void nop(standartParams);

/* Mnemonic           opcode    NumOfBytes
*  push $reg          0x04      0x02
*
*  Description:
*  Push value from reg to stack
*
*  Example:
*  push ax
*/
void push(standartParams);

/* Mnemonic           opcode    NumOfBytes
*  pop $reg           0x05      0x02
*
*  Description:
*  Pop value from stack to reg
*
*  Example:
*  pop ax
*/
void pop(standartParams);

/* Mnemonic           opcode    NumOfBytes
*  mov $reg, #LLHH    0x06      0x04
*
*  Description:
*  Move memory value to register
*
*  Example:
*  mov ax, 1234h
*/
void mov_reg_mem(standartParams);

/* Mnemonic           opcode    NumOfBytes
*  mov #LLHH, $reg    0x07      0x04
*
*  Description:
*  Move register value to address
*
*  Example:
*  mov 1234h, ax
*/
void mov_mem_reg(standartParams);

/* Mnemonic           opcode    NumOfBytes
*  mov $reg, $reg     0x08      0x03
*
*  Description:
*  Move second register value to first
*
*  Example:
*  mov ax, bx 
*  from bx to ax
*/
void mov_reg_reg(standartParams);

/* Mnemonic           opcode    NumOfBytes
*  set $REG           0x09      0x02
*
*  Description:
*  Sets to flag true value
*
*  Example:
*  set ZF
*/
void set_flag(standartParams);

/* Mnemonic           opcode    NumOfBytes
*  clc $REG           0x0A      0x02
*
*  Description:
*  Sets to flag false value
*
*  Example:
*  clc ZF
*/
void clc_flag(standartParams);

/* Mnemonic           opcode    NumOfBytes
*  jmp #addr          0x0B      0x03
*
*  Description:
*  jump to addr
*
*  Example:
*  jmp 1234h
*/
void jmp(standartParams);

/* Mnemonic           opcode    NumOfBytes
*  call #addr         0x0C      0x03
*
*  Description:
*  push to stack now ip and jump to addr
*
*  Example:
*  call 1234h
*/
void call(standartParams);



// 0x10-0x1F
// Math instructions

/* 
  Register instructions.
  Every opcode takes 3 bytes. Name will be in description.
  Every operation places result in first register.
  Mnemonic:
  %Name% $reg, $reg
*/

/*
* Name        opcode
* xor         0x10
* Description: XOR.
*/
void xor_reg(standartParams);

/*
* Name        opcode
* or          0x11
* Description: OR.
*/
void or_reg(standartParams);

/*
* Name        opcode
* and         0x12
* Description: AND.
*/
void and_reg(standartParams);

/*
* Name        opcode
* add         0x13
* Description: ADD.
*/
void add_reg(standartParams);

/*
* Name        opcode
* sub         0x14
* Description: subtract.
*/
void sub_reg(standartParams);

/*
* Name        opcode
* mul         0x15
* Description: multiply.
*/
void mul_reg(standartParams);

/*
* Name        opcode
* div         0x16
* Description: divide.
*/
void div_reg(standartParams);

/*
  Flag instructions.
  Every logic opcode takes 3 bytes. Name will be in description.
  Every operation places result in first flag.
  Mnemonic:
  %Name% $flag, $flag
*/

/*
* Name        opcode
* xor         0x17
* Description: XOR.
*/
void xor_flag(standartParams);

/*
* Name        opcode
* or          0x18
* Description: OR.
*/
void or_flag(standartParams);

/*
* Name        opcode
* and         0x19
* Description: AND.
*/
void and_flag(standartParams);

/*
* Other.
* using 2 bytes.
*/

/*
* Name         opcode
* inc $reg     0x1A
* Description: increment register.
* 
*/
void inc_reg(standartParams);

/*
* Name         opcode
* dec $reg     0x1B
* Description: decrement register.
* 
*/
void dec_reg(standartParams);

/*
* Name        opcode
* and         0x1C
* Description: NOT register.
*/
void not_reg(standartParams);

/*
* Name        opcode
* and         0x1D
* Description: NOT flag.
*/
void not_flag(standartParams);


#include "OPCodes.cpp"

#undef GetValueFromStack 
#undef WriteValueToStack 
#undef IP 
#undef AX 
#undef BX 
#undef CX 
#undef DX 
#undef SI 
#undef DI 
#undef BP 
#undef SP 
#undef IV 
#undef IR 

#undef CF 
#undef PF 
#undef AF 
#undef ZF 
#undef SF 
#undef TF 
#undef IF 
#undef DF 
#undef OF 

#undef AL 
#undef AH 
#undef BL 
#undef BH 
#undef CL 
#undef CH 
#undef DL 
#undef DH 
#undef Register
#undef standartParams
#undef setValue
#undef getValue
#undef getType
#undef ReadFromRegister
#undef WriteToRegister