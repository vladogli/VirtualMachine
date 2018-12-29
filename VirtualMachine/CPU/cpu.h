#pragma once
#include "../memory/memory.h"
#include <string>
#include <functional>
#define ADDR unsigned short
#define SIZE unsigned short
#define BYTE unsigned char
#define MATRIX_X_MAX_SIZE 160
#define MATRIX_Y_MAX_SIZE 90
class CPU {
public: // Variables 
// Memory
	//160 x 90 symbols
	memory *memMatrix;
	BYTE **matrix;
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

	ASM(name)  Addr           Desc                   BYTE-CODE 
	IP         0x00-0x01      Instruction Pointer    0x00
	AX         0x02-0x03      Accumulator            0x01, AL - 0x20, AH - 0x21
	CX         0x04-0x05      Count       Register   0x02, CL - 0x22, CH - 0x23
	DX         0x06-0x07      Data        Register   0x03, DL - 0x24, DH - 0x25
	BX         0x08-0x09      Base        Register   0x04, BL - 0x26, BH - 0x27
	SI         0x0A-0x0B      Source      Index      0x05
	DI         0x0C-0x0D      Destination Index      0x06
	BP         0x0E-0x0F      Base        Pointer    0x07
	SP         0x10-0x11      Stack       Pointer    0x08

	Matrix Registers
	ASM(name)  Addr           Desc                   BYTE-CODE 
	MX         0x10           Matrix      X Pointer  0x30
	MY         0x11           Matrix      Y Pointer  0x31

	FLAGS
	ASM(name)  Addr           Desc                   BYTE-CODE 
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

	// BYTE-CODE            NAME                    HOW TO USE

	// 0x00                 EXT                     EXT
	void					op_exit();              // exit

	// 0x01                 ITOS                    ITOS #$REG
	void					op_int_tostring();      // itos





	/* 0x10 - 0x1F */

	// BYTE-CODE            NAME                    HOW TO USE

	// 0x10                 JMP                     JMP $LLHH
	void					op_jump_to();           // Jump to given addr

	// 0x11                 JZ                      JZ $LLHH
	void					op_jump_z();            // Jump to given addr if Z flag is true

	// 0x12                 JNZ                     JNZ $LLHH
	void					op_jump_nz();           // Jump to given addr if Z flag is false





	/* 0x20 - 0x2F */

	// BYTE-CODE            NAME                    HOW TO USE

	// 0x20                 XOR                     XOR #REG, #REG
	void					op_xor();               // XOR next 2 regs

	// 0x21                 OR                      OR #REG, #REG
	void					op_or();                // OR  next 2 regs

	// 0x22                 AND                     AND #REG, #REG
	void					op_and();               // AND next 2 regs

	// 0x23                 ADD                     ADD #REG, #REG
	void					op_add();               // add next 2 regs

	// 0x24                 SUB                     SUB #REG, #REG
	void					op_sub();               // sub next 2 regs

	// 0x25                 MUL                     MUL #REG, #REG
	void					op_mul();               // multiply next 2 regs

	// 0x26                 DIV                     DIV #REG, #REG
	void					op_div();               // divide next 2 regs

	// 0x27                 INC                     INC #REG, #REG
	void					op_inc();               // increment next reg

	// 0x28                 DEC                     DEC #REG, #REG
	void					op_dec();               // decrement next reg





	/* 0x30 - 0x3F */

	// BYTE-CODE            NAME                     HOW TO USE

	// in first REG addr where we place a concated str
	// in second & third - strings what we should concat
	// 0x30                 CONCAT                   CONCAT #REG, #REG, #REG
	void					op_string_concat();      // Concat 2 strings

	// in second REG - addr of string
	// in first we place a result
	// 0x31                 TOI                      TOI #REG, #REG
	void					op_string_toint();       // Convert a string to int

	// in second REG - addr of string
	// in first we place a result
	// 0x32                 SLN                      SLN #REG, #REG
	void					op_string_size();        // put in register str size





	/* 0x40 - 0x4F */

	// BYTE-CODE            NAME                     HOW TO USE

	// 0x40                 CMP                      CMP #REG, #REG
	void					op_cmp_reg();            // Compare 2 registers

	// 0x41                 CMP                      CMP #REG, $LLHH
	void					op_cmp_immediate();      // Compare register with integer

	// 0x42                 CMPS                     CMPS #REG, #REG
	void					op_cmp_string();         // Compare 2 strings which are saved in registers





	/* 0x50 - 0x5F */

	// BYTE-CODE            NAME                     HOW TO USE

	// 0x50                 NOP
	void					op_nop();                // No operation

	// first  REG - dest
	// second REG - source
	// 0x51                 MOV                      MOV #REG, #REG                      
	void					op_reg_store();          // Put reg1 value to reg2





	/* 0x60 - 0x6F */

	// BYTE-CODE            NAME                     HOW TO USE

	// 0x60                 MOV                      MOV #REG, $LLHH
	void					op_peek();               // Read from address  to register

	// 0x61                 MOV                      MOV $LLHH, #REG
	void					op_poke();               // Read from register to address

	// first  - dest   addr
	// second - source addr
	// third  - size
	// 0x62                 CPY                      CPY $LLHH, $LLHH, $LLHH
	void					op_memcpy();             // Copy size bytes of data from addr to addr

	// first  - dest   addr
	// second - source addr
	// third  - size
	// 0x63                 CPY                      CPY #REG, #REG, #REG
	void					op_memcpy_reg();         // like a memcpy, but values saved in registers
	
	// Stack:
	// SIZE									 
	// source
	// size
	// 0x64                 CPY                      CPY
	void					op_memcpy_stack();       // like a memcpy, but takes values from stack





	/* 0x70 - 0x7F */

	// BYTE-CODE            NAME                     HOW TO USE

	// 0x70                 PUSH                     PUSH $LLHH
	void					op_stack_push();         // Push to stack next 2 bytes

	// 0x71                 PUSH                     PUSH #REG
	void					op_stack_push_reg();     // Push to stack reg value

	// 0x72                 POP                      POP #REG
	void					op_stack_pop_reg();      // Pop from stack to register
	
	// Stack:
	// Return addr
	// 0x73                 RET                      RET
	void					op_stack_ret();          // Return

	// 0x74                 CALL                     CALL $LLHH
	void					op_stack_call();         // Call point





	/* 0x80 - 0x8F */

	// BYTE-CODE            NAME                     HOW TO USE

	// 0x80                 SWSTR                    SWSTR $LLHH
	void					op_string_show();        // Show str from addr

	// 0x81                 SWSTR                    SWSTR #REG
	void					op_string_show_reg();    // Read from register pointer to str and show it

	// 0x82                 SWINT                    SWINT $LLHH
	void					op_int_show();           // Show int

	// 0x83                 SWINT                    SWINT #REG
	void					op_int_show_reg();       // Read from register int and show it

	// 0x84                 SWCHR                    SWCHR $LL
	void					op_char_show();          // Show char

	// 0x85                 SWCHR                    SWCHR #REG
	void					op_char_show_reg();      // Read from register char and show it

	// 0x86                 SCR                      SCR
	void                    op_scroll();             // Scrolls the screen 

	// 0x87                 ENDL                     ENDL
	void                    op_nextline();           // NEXT_LINE_OPERAND

	// 0x88                 CLS                      CLS
	void                    op_page();               // Clears the screen


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
	bool					IsReg                               (BYTE);
	
	// is flag
	// 1 - true
	// 0 - false
	bool					isFlag								(BYTE);
public: 
	bool closed = 0;
	void NextOp();
public: // structors
	CPU();
};
#include "cpu.cpp"
#undef ADDR
#undef SIZE
#undef BYTE