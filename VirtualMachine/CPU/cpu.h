#pragma once
#include "../memory/memory.h"
#include <functional>
#define ADDR unsigned short
#define SIZE unsigned short
#define BYTE unsigned char

class CPU {
public: // Variables 
// Memory
	//160 x 90 symbols
	memory *matrix;
	memory *RAM;
	memory *inputPort;
	memory *outPort;
private: 
// Stack
	enum Exceptions {
		STACK_UNDERFLOW,
		STACK_OVERFLOW
	};
	memory *stack;
	void PushStack(unsigned short);
	unsigned short PopStack();
	/*
	Registers

	Name       Addr           Desc                   ASM
	IP         0x00-0x01      Instruction Pointer    0x00
	AX         0x02-0x03      Accumulator            0x01, AL - 0x20, AH - 0x21
	CX         0x04-0x05      Count       Register   0x02, CL - 0x22, CH - 0x23
	DX         0x06-0x07      Data        Register   0x03, DL - 0x24, DH - 0x25
	BX         0x08-0x09      Base        Register   0x04, BL - 0x26, BH - 0x27
	SI         0x0A-0x0B      Source      Index      0x05
	DI         0x0C-0x0D      Destination Index      0x06
	BP         0x0E-0x0F      Base        Pointer    0x07
	SP         0x10-0x11      Stack       Pointer    0x08
	


	FLAGS
	Name       Addr           Desc                   ASM
	CF         0x20           Carry     Flag         0x10
	PF         0x21           Parity    Flag         0x11
	AF         0x22           Auxiliary Carry  Flag  0x12
	ZF         0x23           Zero      Flag         0x13
	SF         0x24           Sign      Flag         0x14
	TF         0x25           Trap      Flag         0x15
	IF         0x26           Interrupt Enable Flag  0x16
	DF         0x27           Direction Flag         0x17
	OF         0x28           Overflow  Flag         0x18
	*/
	memory *ProcData;
	
// Opcodes
	std::function<void(void)>** functions;
private: // functions
	/* 0x00 - 0x0F */
	void					op_exit();
	void					op_int_store();
	void					op_int_print();
	void					op_int_tostring();
	void					op_int_random();

	/* 0x10 - 0x1F */
	void					op_jump_to();
	void					op_jump_z();
	void					op_jump_nz();

	/* 0x20 - 0x2F */
	void					op_xor();
	void					op_or();
	void					op_add();
	void					op_and();
	void					op_sub();
	void					op_mul();
	void					op_div();
	void					op_inc();
	void					op_dec();

	/* 0x30 - 0x3F */
	void					op_string_store();
	void					op_string_print();
	void					op_string_concat();
	void					op_string_system();
	void					op_string_toint();

	/* 0x40 - 0x4F */
	void					op_cmp_reg();
	void					op_cmp_immediate();
	void					op_cmp_string();
	void					op_is_string();
	void					op_is_integer();

	/* 0x50 - 0x5F */
	void					op_nop();                // No operation
	void					op_reg_store();          // Put reg1 value to reg2


	/* 0x60 - 0x6F */
	void					op_peek();               // Read from address  to register
	void					op_poke();               // Read from register to address
	void					op_memcpy();             // Copy size bytes of data from addr to addr
	void					op_memcpy_reg();         // like a memcpy, but addr saved in registers
	void					op_memcpy_stack();       // like a memcpy, but takes values from stack


	/* 0x70 - 0x7F */
	void					op_stack_push();         // Push to stack next 2 bytes
	void					op_stack_push_reg();     // Push to stack reg value
	void					op_stack_pop_reg();      // Pop from stack to register
	void					op_stack_ret();          // Return
	void					op_stack_call();         // Call point


// Register interaction funcs
	// Write 2 bytes to register 
	void					WriteToReg2							(BYTE, unsigned short);
	
	// Read 2 bytes from register 
	unsigned short			ReadFromReg2						(BYTE);
	
	// Write byte to register 
	void					WriteToReg							(BYTE, BYTE);
	
	// Read byte from register
	BYTE					ReadFromReg							(BYTE);
	
	// Get register addr in ProcData
	BYTE					GetRegAddr							(BYTE);
	
	// Register type
	// 1 - 1 byte
	// 0 - 2 byte
	bool					RegType								(BYTE);
	
	// is register
	// 1 - true
	// 0 - false
	bool					isReg								(BYTE);
	
	// is flag
	// 1 - true
	// 0 - false
	bool					isFlag								(BYTE);
public: 
	void NextOp();
public: // structors
	CPU();
};
#include "cpu.cpp"
#undef ADDR
#undef SIZE
#undef BYTE