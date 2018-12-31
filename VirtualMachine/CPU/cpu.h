#pragma once
#include "../memory/memory.h"
#include <string>
#include <functional>
#include <ctime>
#include <boost/filesystem.hpp>
#define ADDR unsigned short
#define SIZE unsigned short
#define BYTE unsigned char
#define MATRIX_X_MAX_SIZE 150
#define MATRIX_Y_MAX_SIZE 60
#define READ(v)    ProcData->Read2Bytes(v)
#define WRITE(x,v) ProcData->Write2Bytes(x, v)
#define READ1(v) ProcData->Read(v)
#define WRITE1(x,v) ProcData->Write(x, v)

#define IP 0x00
#define READ_IP     READ(IP)
#define WRITE_IP(x) WRITE(IP, x)

#define SP 0x10
#define READ_SP     READ(SP)
#define WRITE_SP(x) WRITE(SP,x)

#define ZF 0x23
#define READ_ZF     READ1(ZF)
#define WRITE_ZF(x) WRITE1(ZF,x)

#define SF 0x24
#define READ_SF     READ1(SF)
#define WRITE_SF(x) WRITE1(SF,x)

#define CF 0x24
#define READ_CF     READ1(CF)
#define WRITE_CF(x) WRITE1(CF,x)


#define PF 0x21
#define READ_PF     READ1(PF)
#define WRITE_PF(x) WRITE1(PF,x)

#define OF 0x28
#define READ_OF     READ1(OF)
#define WRITE_OF(x) WRITE1(OF,x)

#define IR 0x14
#define READ_IR READ1(IR)
#define WRITE_IR(x) WRITE1(IR, x)

#define IV 0x15
#define READ_IV READ(IV)
#define WRITE_IV(x) WRITE(IV, x)

#define IF 0x26
#define READ_IF READ1(IF)
#define WRITE_IF(x) WRITE1(IF, x)

#define FERR 0x2D
#define WRITE_FERR(x) WRITE(FERR,x)
#define READ_FERR   READ(FERR)

#define CERR 0x1B
#define WRITE_CERR(x) WRITE(CERR,x)
#define READ_CERR   READ(CERR)

#define FS 0x2B
#define WRITE_FS(x) WRITE1(FS,x)
#define READ_FS   READ1(FS)

#define MX 0x12
#define WRITE_MX(x) WRITE1(MX,x)
#define READ_MX    READ1(MX)

#define MY 0x13
#define WRITE_MY(x) WRITE1(MY,x)
#define READ_MY    READ1(MY)

#define CRH 0x17 
#define WRITE_CRH(x) WRITE(CRH,x)
#define READ_CRH READ(CRH)

#define CRL 0x19
#define WRITE_CRL(x) WRITE(CRL,x)
#define READ_CRL READ(CRL)
#define IsReg2Bytes(x) (IsReg(x) && !RegType(x))

#define SwitchReg(reg) ((RegType(reg)) ? (ReadFromReg(reg)) : ReadFromReg2(reg))
#define WriteReg(reg, _Value) \
if(RegType(reg)) { WriteToReg(reg, (unsigned char)_Value); } \
else  { WriteToReg2(reg, (unsigned short)_Value); }


#define DEBUG_MODE
class CPU {
public: // Variables 
// Memory
	//150 x 60 symbols
	memory *memMatrix;
	BYTE **matrix;
/*
	********************************************************************************************************
	**                     First 64 bytes in RAM is a pointers to interrupt events                        **
	**  IR_VAL     NAME                     ADDR in memory       Value in IV         Description          **
	**  0x00       KEYBOARD_INPUT_EVENT     0x00                 Key Value           Processing a key     **
	**  0x01       ACCEPT_CONNECTION        0x02                 Device ID                                **
	**  0x02       NONE                     0x04                                                          **
	**  0x03       NONE                     0x06                                                          **
	**  0x04       NONE                     0x08                                                          **
	**  0x05       NONE                     0x0A                                                          **
	**  0x06       NONE                     0x0C                                                          **
	**  0x07       NONE                     0x0E                                                          **
	**  0x08       NONE                     0x10                                                          **
	**  0x09       NONE                     0x12                                                          **
	**  0x0A       NONE                     0x14                                                          **
	**  0x0B       NONE                     0x16                                                          **
	**  0x0C       NONE                     0x18                                                          **
	**  0x0D       NONE                     0x1A                                                          **
	**  0x0E       NONE                     0x1C                                                          **
	**  0x0F       NONE                     0x1E                                                          **
	**  0x10       NONE                     0x20                                                          **
	**  0x11       NONE                     0x22                                                          **
	**  0x12       NONE                     0x24                                                          **
	**  0x13       NONE                     0x26                                                          **
	**  0x14       NONE                     0x28                                                          **
	**  0x15       NONE                     0x2A                                                          **
	**  0x16       NONE                     0x2C                                                          **
	**  0x17       NONE                     0x2E                                                          **
	**  0x18       NONE                     0x30                                                          **
	**  0x19       NONE                     0x32                                                          **
	**  0x1A       NONE                     0x34                                                          **
	**  0x1B       NONE                     0x36                                                          **
	**  0x1C       NONE                     0x38                                                          **
	**  0x1D       NONE                     0x3A                                                          **
	**  0x1E       NONE                     0x3C                                                          **
	**  0x1F       NONE                     0x3E                                                          **
	********************************************************************************************************
*/
	//0x40  - 0xFF   - Num Stack
	//0x100 - 0x1FF  - pointers to IN
	//0x200 - 0x2FF  - pointers to OUT
	//0x300 - 0x3FF  - File buffer
	//0x400 - 0xFFFF - not reserved
	Ram *RAM;
	bool closed = 0;
private: 
// Stack
	enum Exceptions {
		STACK_UNDERFLOW,
		STACK_OVERFLOW
	};
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
	MX         0x12           Matrix      X Pointer  0x30
	MY         0x13           Matrix      Y Pointer  0x31

	Interrupt registers
	ASM(name)  Addr           Desc                   BYTE-CODE 
	IR         0x14           Interrupt Register     0x40
	IV         0x15-0x16      Interrupt value        0x41

	ConnectToDevice registers
	ASM(name)  Addr           Desc                   BYTE-CODE
	CRH        0x17-0x18      Connect register high  0x50
	CRL        0x19-0x1A      Connect register low   0x51
	CERR       0x1B-0x1C      Connection error buf   0x52
	
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

	Filesystem registers
	ASM(name)  Addr           Desc                   BYTE-CODE
	FNA        0x29-0x2A      Filename addr          0x60
	FS         0x2B           File size(in parts)    0x61
	FERR       0x2D           Filesystem error       0x62

	*/
	memory *ProcData;
	int unsleep_clock = 0;
// Opcodes
	std::function<void(void)>** functions;
private: // functions
	/* 0x00 - 0x0F */

	// BYTE-CODE            NAME                    HOW TO USE

	// 0x00                 EXT                     EXT
	void                    op_exit();              // exit

	// 0x01                 ITOS                    ITOS #REG
	void                    op_int_tostring();      // itos

	// 0x02                 SLP                     ITOS $LLHH
	void                    op_sleep();

	// 0x03                 SLP                     ITOS #REG
	void                    op_sleep_reg();

	
	// 0x04                 JMP                     JMP $LLHH
	void                    op_jump_to();           // Jump to given addr

	// OPERAND              EXPANSION               Desc
	// JO                   JIF OF, $LLHH           Jump if overflow
	// JC                   JIF CF, $LLHH           Jump if carry
	// JB                   JIF CF, $LLHH           Jump if below
	// JNAE                 JIF CF, $LLHH           Jump if not above or equal
	// JE                   JIF ZF, $LLHH           Jump if equal
	// JZ                   JIF ZF, $LLHH           Jump if zero
	// JS                   JIF SF, $LLHH           Jump if sign
	// JP                   JIF PF, $LLHH           Jump if parity
	// JPE                  JIF PF, $LLHH           Jump if parity even

	// 0x05                 JIF                     JIF  #FLAG, $LLHH
	void                    op_jump_ift();          // Jump to given addr if flag is true

	// OPERAND              EXPANSION               Desc
	// JNO                  JNIF OF, $LLHH          Jump if not overflow
	// JNC                  JNIF CF, $LLHH          Jump if not carry
	// JAE                  JNIF CF, $LLHH          Jump if above or equal
	// JNB                  JNIF CF, $LLHH          Jump if not below
	// JNE                  JNIF ZF, $LLHH          Jump if not equal
	// JNZ                  JNIF ZF, $LLHH          Jump if not zero
	// JNS                  JNIF SF, $LLHH          Jump if not sign
	// JNP                  JNIF PF, $LLHH          Jump if not parity
	// JPO                  JNIF PF, $LLHH          Jump if parity odd

	// 0x06                 JNIF                    JNIF  #FLAG, $LLHH
	void                    op_jump_ifnt();         // Jump to given addr if flag is true

	// OPERAND              EXPANSION               Desc
	// JBE                  JOR CF, ZF, $LLHH       Jump if below or equal
	// JNA                  JOR CF, ZF, $LLHH       Jump if not above
	// 0x06                 JOR                     JOR  #FLAG, #FLAG, $LLHH
	void                    op_jump_or();           // Jump to given addr if one of given flags is true

	// OPERAND              EXPANSION               Desc
	// JNBE                 JNOR CF, ZF, $LLHH      Jump if not below or equal
	// JA                   JNOR CF, ZF, $LLHH      Jump if above
	// 0x08                 JNOR                    JNOR  #FLAG, #FLAG, $LLHH
	void                    op_jump_nor();          // Jump to given addr if every flag is false
	

	// OPERAND              EXPANSION               Desc
	// JL                   JXOR SF, OF, $LLHH      Jump if less
	// JNGE                 JXOR SF, OF, $LLHH      Jump if not greater nor equal
	// 0x09                 JXOR                    JXOR  #FLAG, #FLAG, $LLHH
	void                    op_jump_xor();          // Jump to given addr if ONLY one of given flags is true


	// OPERAND              EXPANSION               Desc
	// JNL                  JNXOR SF, OF, $LLHH     Jump if not less
	// JGE                  JNXOR SF, OF, $LLHH     Jump if greater or equal
	// 0x0A                 JNXOR                   JNXOR  #FLAG, #FLAG, $LLHH
	void                    op_jump_nxor();         // Jump to given addr if given flags is equal

	// JLE (less or equal)
	// 0x0B                 JNG                     JLE $LLHH
	void                    op_jump_le();           // Jump to given addr if (SF xor OF) or ZF = 1

	// JNLE (not less nor equal)
	// 0x0C                 JG                      JG $LLHH
	void                    op_jump_greater();      // Jump to given addr if (SF xor OF) or ZF = 0

	// OPERAND              EXPANSION               Desc
	// STC                  STF CF                  Sets carry            flag to 1
	// STP                  STF PF                  Sets parity           flag to 1
	// STA                  STF AF                  Sets auxiliary carry  flag to 1
	// STZ                  STF ZF                  Sets zero             flag to 1
	// STS                  STF SF                  Sets sign             flag to 1
	// STT                  STF TF                  Sets trap             flag to 1
	// STI                  STF IF                  Sets interrupt enable flag to 1
	// STD                  STF DF                  Sets direction        flag to 1
	// STO                  STF OF                  Sets overflow         flag to 1
	// 0x0D                 STF                     STF #REG
	void                    op_set_f();             // Set the given flag to 1


	// CLC                  CLF CF                  Sets carry            flag to 0
	// CLP                  CLF PF                  Sets parity           flag to 0
	// CLA                  CLF AF                  Sets auxiliary carry  flag to 0
	// CLZ                  CLF ZF                  Sets zero             flag to 0
	// CLS                  CLF SF                  Sets sign             flag to 0
	// CLT                  CLF TF                  Sets trap             flag to 0
	// CLI                  CLF IF                  Sets interrupt enable flag to 0
	// CLD                  CLF DF                  Sets direction        flag to 0
	// CLO                  CLF OF                  Sets overflow         flag to 0
	// 0x0E                 CLF                     CLF #REG
	void                    op_clear_f();           // Set the given flag to 0


	/* 0x10 - 0x1F */
	



	/* 0x20 - 0x2F */

	// BYTE-CODE            NAME                    HOW TO USE

	// 0x20                 XOR                     XOR #REG, #REG
	void                    op_xor();               // XOR next 2 regs

	// 0x21                 OR                      OR #REG, #REG
	void                    op_or();                // OR  next 2 regs

	// 0x22                 AND                     AND #REG, #REG
	void                    op_and();               // AND next 2 regs

	// 0x23                 ADD                     ADD #REG, #REG
	void                    op_add();               // add next 2 regs

	// 0x24                 SUB                     SUB #REG, #REG
	void                    op_sub();               // sub next 2 regs

	// 0x25                 MUL                     MUL #REG, #REG
	void                    op_mul();               // multiply next 2 regs

	// 0x26                 DIV                     DIV #REG, #REG
	void                    op_div();               // divide next 2 regs

	// 0x27                 INC                     INC #REG, #REG
	void                    op_inc();               // increment next reg

	// 0x28                 DEC                     DEC #REG, #REG
	void                    op_dec();               // decrement next reg





	/* 0x30 - 0x3F */

	// BYTE-CODE            NAME                     HOW TO USE

	// in first REG addr where we place a concated str
	// in second & third - strings what we should concat
	// 0x30                 CONCAT                   CONCAT #REG, #REG, #REG
	void                    op_string_concat();      // Concat 2 strings

	// in second REG - addr of string
	// in first we place a result
	// 0x31                 TOI                      TOI #REG, #REG
	void                    op_string_toint();       // Convert a string to int

	// in second REG - addr of string
	// in first we place a result
	// 0x32                 SLN                      SLN #REG, #REG
	void                    op_string_size();        // put in register str size





	/* 0x40 - 0x4F */

	// BYTE-CODE            NAME                     HOW TO USE

	// 0x40                 CMP                      CMP #REG, #REG
	void                    op_cmp_reg();            // Compare 2 registers

	// 0x41                 CMP                      CMP #REG, $LLHH
	void                    op_cmp_immediate();      // Compare register with integer

	// 0x42                 CMPS                     CMPS #REG, #REG
	void                    op_cmp_string();         // Compare 2 strings which are saved in registers





	/* 0x50 - 0x5F */

	// BYTE-CODE            NAME                     HOW TO USE

	// 0x50                 NOP
	void                    op_nop();                // No operation

	// first  REG - dest
	// second REG - source
	// 0x51                 MOV                      MOV #REG, #REG                      
	void                    op_reg_store();          // Put reg1 value to reg2





	/* 0x60 - 0x6F */

	// BYTE-CODE            NAME                     HOW TO USE

	// 0x60                 MOV                      MOV #REG, $LLHH
	void                    op_peek();               // Write value  to register

	// 0x61                 MOV                      MOV $LLHH, #REG
	void                    op_poke();               // Write register value to addr 

	// first  - dest   addr
	// second - source addr
	// third  - size
	// 0x62                 CPY                      CPY $LLHH, $LLHH, $LLHH
	void                    op_memcpy();             // Copy size bytes of data from addr to addr

	// first  - dest   addr
	// second - source addr
	// third  - size
	// 0x63                 CPY                      CPY #REG, #REG, #REG
	void                    op_memcpy_reg();         // like a memcpy, but values saved in registers
	
	// Stack:
	// SIZE									 
	// source
	// size
	// 0x64                 CPY                      CPY
	void                    op_memcpy_stack();       // like a memcpy, but takes values from stack





	/* 0x70 - 0x7F */

	// BYTE-CODE            NAME                     HOW TO USE

	// 0x70                 PUSH                     PUSH $LLHH
	void                    op_stack_push();         // Push to stack next 2 bytes

	// 0x71                 PUSH                     PUSH #REG
	void                    op_stack_push_reg();     // Push to stack reg value

	// 0x72                 POP                      POP #REG
	void                    op_stack_pop_reg();      // Pop from stack to register
	
	// Stack:
	// Return addr
	// 0x73                 RET                      RET
	void                    op_stack_ret();          // Return

	// 0x74                 CALL                     CALL $LLHH
	void                    op_stack_call();         // Call point





	/* 0x80 - 0x8F */ // Matrix opcodes

	// BYTE-CODE            NAME                     HOW TO USE

	// 0x80                 SWSTR                    SWSTR $LLHH
	void                    op_string_show();        // Show str from addr

	// 0x81                 SWSTR                    SWSTR #REG
	void                    op_string_show_reg();    // Read from register pointer to str and show it

	// 0x82                 SWINT                    SWINT $LLHH
	void                    op_int_show();           // Show int

	// 0x83                 SWINT                    SWINT #REG
	void                    op_int_show_reg();       // Read from register int and show it

	// 0x84                 SWCHR                    SWCHR $LLHH
	void                    op_char_show();          // Show char

	// 0x85                 SWCHR                    SWCHR #REG
	void                    op_char_show_reg();      // Read from register char and show it
	
													 
	/*	C - cursor
	--------------              **      ********------
	********------               **     **********----
	**********----          ********    ************--
	************--               **     **************
	**************              **      C-------------
	C-------------                      --------------
	*/
	// 0x86                 SCR                      SCR
	void                    op_scroll();             // Scrolls the screen 


	/*	C - cursor
	********------              **      **********----
	**********----               **     ************--
	************--          ********    **************
	*************C               **     **************
	--------------              **      C-------------
	*/
	// 0x87                 ENDL                     ENDL
	void                    op_nextline();           // NEXT_LINE_OPERAND

	/*	C - cursor
	--------------              **      C-------------
	********------               **     --------------
	**********----          ********    --------------
	************--               **     --------------
	*************C              **      --------------
	*/
	// 0x88                 CLS                      CLS
	void                    op_page();               // Clears the screen

	/* 0x90 - 0x9F */ // Connection opcodes

	// Sets in Carry Flag true if connection was opened successfully. Sets false if something goes wrong.
	// If Carry Flag is false, sets in CERR error type.
	// 0 - Successfully connected.
	// 1 - The current connection isn't closed.
	// 2 - Device doesn't exist.
	// 3 - The port on the device is closed.
	// 4 - The port on our device is closed.
	// 5 - The device is already connected to another.
	// 6 - Our device does not have the permission to connect to this.
	// 0x90                 TRY_CONNECT               TRY_CONNECT
	void                    op_try_connect();         // Tries to connect to device, which is addr saved in CR register
	
	// Sets in Carry flag true if connection was closed successfully. Or false, if there was no connection at all.
	// 0x91                 CLOSE_CONNECT             CLOSE_CONNECT
	void                    op_close_connection();    // Closes connection.

	// Sets 0 to CERR if success
	// Sets 1 to CERR if port already opened
	// 0x92                 OPEN_PORT                 OPEN_PORT
	void                    op_open_port();           // Open a port for new connections.

	// Sets 0 to CERR if success
	// Sets 1 to CERR if connections isn't closed
	// 0x93                 CLOSE_PORT                CLOSE_PORT
	void                    op_close_port();           // Closes a port.
	
	// Writes to CERR port state
	// 0 - closed
	// 1 - opened
	// 0x94                 PORT_STATE                PORT_STATE
	void                    op_port_get_state();           // Sets in CERR port state.

	/* 0xA0 - 0xAF */ // Filesystem opcodes
	// Filename addr has saved in FNA




	// Places a file size in FS
	// FERR:
	// 0 - file exists
	// 1 - file doesn't exists
	// 2 - wrong filename (wrong chars)
	// 0xA0                 GFI                       GFI
	void                    op_getfileinfo();         // Places info about file in FERR and in FS

	// FERR
	// 0 - success
	// 1 - wrong part
	// 2 - wrong filename (wrong chars)
	// 0xA1                 LOAD_PART                 LOAD_PART #REG
	void                    op_load_part_reg();       // Loads a part of file in 0x300-0x3FF

	// FERR
	// 0 - success
	// 1 - wrong part
	// 2 - wrong filename (wrong chars)
	// 0xA2                 LOAD_PART                 LOAD_PART $LL
	void                    op_load_part();           // Loads a part of file in 0x300-0x3FF

	// FERR
	// 0 - success
	// 1 - wrong part
	// 2 - wrong filename (wrong chars)
	// 0xA3                 SAVE_PART                 SAVE_PART $LL
	void                    op_save_file_part();      // Saves a part of file from 0x300-0x3FF


	// FERR
	// 0 - success
	// 1 - wrong part
	// 2 - wrong filename (wrong chars)
	// 0xA4                 SAVE_PART                 SAVE_PART #REG
	void                    op_save_file_part_reg();  // Saves a part of file from 0x300-0x3FF


// Register interaction funcs

	// Write 2 bytes to register 
	void                    WriteToReg2							(BYTE, unsigned short);
	
	// Read 2 bytes from register 
	unsigned short			ReadFromReg2						(BYTE);
	
	// Write byte to register 
	void                    WriteToReg							(BYTE, BYTE);
	
	// Read byte from register
	BYTE                    ReadFromReg							(BYTE);
	
	// Get register addr in ProcData
	BYTE                    GetRegAddr							(BYTE);
	
	// Register type
	// 1 - 1 byte
	// 0 - 2 byte
	bool                    RegType								(BYTE);
	
	// is register
	// 1 - true
	// 0 - false
	bool                    IsReg                               (BYTE);
	
	// is flag
	// 1 - true
	// 0 - false
	bool                    IsFlag								(BYTE);
public: 
	void keyPressed(BYTE key);
	void NextOp();
	std::function<BYTE(unsigned long long int)> IoConnectToNewDevice;
	std::function<bool(unsigned long long int)> IoIsPortOpened;
	std::function<void(unsigned long long int)> IoCloseConnection;
	std::function<void(void)> OpenPort;
	std::function<void(void)> ClosePort;
	std::function<bool(void)> GetPortState;
public: // structors
	CPU();
};
#include "cpu.cpp"

#undef DEBUG_MODE
#undef WriteReg
#undef SwitchReg

#undef ADDR
#undef SIZE
#undef BYTE

#undef MATRIX_X_MAX_SIZE
#undef MATRIX_Y_MAX_SIZE

#undef IP
#undef SP
#undef ZF 
#undef SF 
#undef OF 
#undef IR
#undef IV
#undef IF
#undef FS
#undef MY
#undef MX
#undef FERR
#undef CRL
#undef CRH

#undef READ
#undef READ_FS
#undef READ_FERR
#undef READ_IP  
#undef READ_SP
#undef READ_ZF   
#undef READ_SF     
#undef READ_OF  
#undef READ_IR
#undef READ_IV
#undef READ_IF
#undef READ_MX
#undef READ_MY
#undef READ_CRL
#undef READ_CRH

#undef WRITE
#undef WRITE_FS
#undef WRITE_FERR
#undef WRITE_IR
#undef WRITE_IV
#undef WRITE_IF
#undef WRITE_SF   
#undef WRITE_OF  
#undef WRITE_ZF
#undef WRITE_SP 
#undef WRITE_IP
#undef WRITE_MX
#undef WRITE_MY
#undef WRITE_CRH
#undef WRITE_CRL

#undef IsReg2Bytes