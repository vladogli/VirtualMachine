#include "memory.h"
#define ADDR unsigned int
#define SIZE unsigned short
#define BYTE unsigned char
memory::memory(SIZE size) : memSize(size) {
	mem = new BYTE[size];
	Fill(0, size, 0);
}
memory::~memory() {
	delete[] mem;
}
void memory::Fill(ADDR addr, SIZE size, BYTE byte) {
#ifndef memory_NOEXCEPT
	if (addr + size > memSize) {
		throw WRONG_ADDR;
	}
#endif
	memset(mem, byte, size);
}
void memory::Copy(ADDR addr1, ADDR addr2, SIZE size) {
#ifndef memory_NOEXCEPT
	if (addr1 + size > memSize || addr2 + size > memSize) {
		throw WRONG_ADDR;
	}
#endif
	memcpy(&mem[addr1], &mem[addr2], size);
}
bool memory::Compare(ADDR addr1, ADDR addr2, SIZE size) {
#ifndef memory_NOEXCEPT
	if (addr1 + size > memSize || addr2 + size > memSize) {
		throw WRONG_ADDR;
	}
#endif
	return (memcmp(&mem[addr1], &mem[addr2], size) == 0);
}
void memory::Move(ADDR source, ADDR dest, SIZE size) {
#ifndef memory_NOEXCEPT
	if (source + size > memSize || dest + size > memSize) {
		throw WRONG_ADDR;
	}
#endif
	memmove(&mem[dest], &mem[source], size);
}
BYTE memory::Read(ADDR addr) {
#ifndef memory_NOEXCEPT
	if (addr > memSize) {
		throw WRONG_ADDR;
	}
#endif
	return mem[addr];
}
unsigned short memory::Read2Bytes(ADDR addr) {
#ifndef memory_NOEXCEPT
	if (addr+1 > memSize) {
		throw WRONG_ADDR;
	}
#endif
	return mem[addr] + mem[addr + 1] * 256;
}
void memory::Read(ADDR addr, BYTE*& dest, SIZE size) {
#ifndef memory_NOEXCEPT
	if (addr + size > memSize) {
		throw WRONG_ADDR;
	}
#endif
	memcpy(dest, &mem[addr], size);
}
void memory::Write(ADDR addr, BYTE byte) {
#ifndef memory_NOEXCEPT
	if (addr > memSize) {
		throw WRONG_ADDR;
	}
#endif
	mem[addr] = byte;
}
void memory::Write2Bytes(ADDR addr, unsigned short _Value) {
#ifndef memory_NOEXCEPT
	if (addr+1 > memSize) {
		throw WRONG_ADDR;
	}
#endif
	mem[addr] = _Value % 256;
	mem[addr+1] = _Value / 256;
}
void memory::Write(ADDR dest, BYTE*& source, SIZE size) {
#ifndef memory_NOEXCEPT
	if (dest + size > memSize) {
		throw WRONG_ADDR;
	}
#endif
	memcpy(&mem[dest], source, size);
}
#undef ADDR
#undef SIZE
#undef BYTE