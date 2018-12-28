#pragma once
#include <vcruntime_string.h>
//Memory class for  virtual machine
#define ADDR unsigned int
#define SIZE unsigned short
#define BYTE unsigned char
//#define RAM_NOEXCEPT
class memory {
public: // structs
#ifndef RAM_NOEXCEPT
	enum Exceptions {
		WRONG_ADDR
	};
#endif
public: // Variables
	BYTE* mem;
	const SIZE memSize;
public: // Functions
	// Fill from ADDR to ADDR+SIZE BYTE
	void Fill(ADDR, SIZE, BYTE);

	// Copy from ADDR1 to ADDR2 SIZE bytes
	void Copy(ADDR, ADDR, SIZE);

	// Compares 2 blocks of memory
	bool Compare(ADDR, ADDR, SIZE);

	// Moves from ADDR to ADDR SIZE bytes
	void Move(ADDR, ADDR, SIZE);

	// Read 1 byte on addr
	BYTE Read(ADDR);

	// Read 2 bytes on addr
	unsigned short Read2Bytes(ADDR);

	// Read SIZE bytes to dest
	void Read(ADDR, BYTE*&, SIZE);

	// Write 1 byte on addr
	void Write(ADDR, BYTE);

	// Write 2 bytes on addr
	void Write2Bytes(ADDR, unsigned short);

	// Write SIZE bytes to dest
	void Write(ADDR, BYTE*&, SIZE);


public: // Structors
	// Constructor, parameter is alloc size.
	memory(SIZE);
	// Destructor
	~memory();
};
#include "memory.cpp"
#undef ADDR
#undef SIZE
#undef BYTE