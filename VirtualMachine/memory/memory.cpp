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
#ifndef MEMORY_NOEXCEPT
	if (addr + size > memSize) {
		throw WRONG_ADDR;
	}
#endif
	memset(mem + addr, byte, size);
}
void memory::Copy(ADDR addr1, ADDR addr2, SIZE size) {
#ifndef MEMORY_NOEXCEPT
	if (addr1 + size > memSize || addr2 + size > memSize) {
		throw WRONG_ADDR;
	}
#endif
	memcpy(&mem[addr1], &mem[addr2], size);
}
bool memory::Compare(ADDR addr1, ADDR addr2, SIZE size)const {
#ifndef MEMORY_NOEXCEPT
	if (addr1 + size > memSize || addr2 + size > memSize) {
		throw WRONG_ADDR;
	}
#endif
	return (memcmp(&mem[addr1], &mem[addr2], size) == 0);
}
void memory::Move(ADDR source, ADDR dest, SIZE size) {
#ifndef MEMORY_NOEXCEPT
	if (source + size > memSize || dest + size > memSize) {
		throw WRONG_ADDR;
	}
#endif
	memmove(&mem[dest], &mem[source], size);
}
BYTE memory::Read(ADDR addr) const {
#ifndef MEMORY_NOEXCEPT
	if (addr > memSize) {
		throw WRONG_ADDR;
	}
#endif
	return mem[addr];
}
unsigned short memory::Read2Bytes(ADDR addr) const {
#ifndef MEMORY_NOEXCEPT
	if (addr+1 > memSize) {
		throw WRONG_ADDR;
	}
#endif
	return mem[addr] + mem[addr + 1] * 256;
}
void memory::Read(ADDR addr, BYTE*& dest, SIZE size) const {
#ifndef MEMORY_NOEXCEPT
	if (addr + size > memSize) {
		throw WRONG_ADDR;
	}
#endif
	memcpy(dest, &mem[addr], size);
}
void memory::Write(ADDR addr, BYTE byte) {
#ifndef MEMORY_NOEXCEPT
	if (addr > memSize) {
		throw WRONG_ADDR;
	}
#endif
	mem[addr] = byte;
}
void memory::Write2Bytes(ADDR addr, unsigned short _Value) {
#ifndef MEMORY_NOEXCEPT
	if (addr+1 > memSize) {
		throw WRONG_ADDR;
	}
#endif
	mem[addr] = _Value % 256;
	mem[addr + 1] = (unsigned char)(_Value / 256);
}
void memory::Write(ADDR dest, BYTE*& source, SIZE size) {
#ifndef MEMORY_NOEXCEPT
	if (dest + size > memSize) {
		throw WRONG_ADDR;
	}
#endif
	memcpy(&mem[dest], source, size);
}
void memory::Write(ADDR dest, const BYTE*& source, SIZE size) {

#ifndef MEMORY_NOEXCEPT
	if (dest + size > memSize) {
		throw WRONG_ADDR;
	}
#endif
	memcpy(&mem[dest], source, size);
}
void Ram::Fill(ADDR addr, SIZE size, BYTE byte) {
	if (addr < 0x100) {
		return;
	}
	if (addr < 0x200) {
		if (addr + size > 0x200) {
			memset(IN + addr, byte, 0x100);
			Fill(addr + 0x100, size - 0x100, byte);
			return;
		}
		memset(IN + addr, byte, size);
		return;
	}
	if (addr < 0x300) {
		if (addr + size > 0x300) {
			memset(OUT + addr, byte, 0x100);
			Fill(addr + 0x100, size - 0x100, byte);
			return;
		}
		memset(OUT + addr, byte, size);
		return;
	}
	if (addr + size > 0xFFFF) {
		return;
	}
	memset(RAM + addr, byte, size);
}
void Ram::Copy(ADDR dest, ADDR from, SIZE size) {
	if (dest < 0x100 || from < 0x100) {
		return;
	}
	if (dest < 0x200) {
		if (dest + size > 0x200) {
			return;
		}
		if (from < 0x200) {
			if (from + size > 0x200) {
				return;
			}
			memcpy(IN + dest, IN + from, size);
			return;
		}
		else if (from < 0x300) {
			if (from + size > 0x300) {
				return;
			}
			memcpy(IN + dest, OUT + from, size);
			return;
		}
		else {
			if (from + size > 0xFFFF) {
				return;
			}
			memcpy(IN + dest, RAM + from, size);
			return;
		}
		return;
	}
	if (dest < 0x300) {
		if (dest + size > 0x300) {
			return;
		}
		if (from < 0x200) {
			if (from + size > 0x200) {
				return;
			}
			memcpy(OUT + dest, IN + from, size);
			return;
		}
		else if (from < 0x300) {
			if (from + size > 0x300) {
				return;
			}
			memcpy(OUT + dest, OUT + from, size);
			return;
		}
		else {
			if (from + size > 0xFFFF) {
				return;
			}
			memcpy(OUT + dest, RAM + from, size);
			return;
		}
		return;
	}
	if (dest + size > 0xFFFF) {
		return;
	}
	if (from < 0x200) {
		if (from + size > 0x200) {
			return;
		}
		memcpy(RAM + dest, IN + from, size);
		return;
	}
	else if (from < 0x300) {
		if (from + size > 0x300) {
			return;
		}
		memcpy(RAM + dest, OUT + from, size);
		return;
	}
	else {
		if (from + size > 0xFFFF) {
			return;
		}
		memcpy(RAM + dest, RAM + from, size);
		return;
	}
}
bool Ram::Compare(ADDR dest, ADDR from, SIZE size) const {
	if (dest < 0x100 || from < 0x100) {
		return 0;
	}
	if (dest < 0x200) {
		if (dest + size > 0x200) {
			return 0;
		}
		if (from < 0x200) {
			if (from + size > 0x200) {
				return 0;
			}
			return memcmp(IN + dest, IN + from, size) == 0;
		}
		else if (from < 0x300) {
			if (from + size > 0x300) {
				return 0;
			}
			return memcmp(IN + dest, OUT + from, size) == 0;
		}
		else {
			if (from + size > 0xFFFF) {
				return 0;
			}
			return memcmp(IN + dest, RAM + from, size) == 0;
		}
	}
	if (dest < 0x300) {
		if (dest + size > 0x300) {
			return 0;
		}
		if (from < 0x200) {
			if (from + size > 0x200) {
				return 0;
			}
			return memcmp(OUT + dest, IN + from, size) == 0;
		}
		else if (from < 0x300) {
			if (from + size > 0x300) {
				return 0;
			}
			return memcmp(OUT + dest, OUT + from, size) == 0;
		}
		else {
			if (from + size > 0xFFFF) {
				return 0;
			}
			return memcmp(OUT + dest, RAM + from, size) == 0;
		}
	}
	if (dest + size > 0xFFFF) {
		return 0;
	}
	if (from < 0x200) {
		if (from + size > 0x200) {
			return 0;
		}
		return memcmp(RAM + dest, IN + from, size) == 0;
	}
	else if (from < 0x300) {
		if (from + size > 0x300) {
			return 0;
		}
		return memcmp(RAM + dest, OUT + from, size) == 0;
	}
	else {
		if (from + size > 0xFFFF) {
			return 0  ;
		}
		return memcmp(RAM + dest, RAM + from, size) == 0;
	}
}
void Ram::Move(ADDR dest, ADDR from, SIZE size) {
	if (dest < 0x100 || from < 0x100) {
		return;
	}
	if (dest < 0x200) {
		if (dest + size > 0x200) {
			return;
		}
		if (from < 0x200) {
			if (from + size > 0x200) {
				return;
			}
			memmove(IN + dest, IN + from, size);
			return;
		}
		else if (from < 0x300) {
			if (from + size > 0x300) {
				return;
			}
			memcpy(IN + dest, OUT + from, size);
			return;
		}
		else {
			if (from + size > 0xFFFF) {
				return;
			}
			memcpy(IN + dest, RAM + from, size);
			return;
		}
		return;
	}
	if (dest < 0x300) {
		if (dest + size > 0x300) {
			return;
		}
		if (from < 0x200) {
			if (from + size > 0x200) {
				return;
			}
			memcpy(OUT + dest, IN + from, size);
			return;
		}
		else if (from < 0x300) {
			if (from + size > 0x300) {
				return;
			}
			memcpy(OUT + dest, OUT + from, size);
			return;
		}
		else {
			if (from + size > 0xFFFF) {
				return;
			}
			memcpy(OUT + dest, RAM + from, size);
			return;
		}
		return;
	}
	if (dest + size > 0xFFFF) {
		return;
	}
	if (from < 0x200) {
		if (from + size > 0x200) {
			return;
		}
		memcpy(RAM + dest, IN + from, size);
		return;
	}
	else if (from < 0x300) {
		if (from + size > 0x300) {
			return;
		}
		memcpy(RAM + dest, OUT + from, size);
		return;
	}
	else {
		if (from + size > 0xFFFF) {
			return;
		}
		memcpy(RAM + dest, RAM + from, size);
		return;
	}
}
BYTE Ram::Read(ADDR addr) const {
	if (addr < 0x100) {
		return RAM[addr];
	}
	if (addr < 0x200) {
		return IN[addr];
	}
	if (addr < 0x300) {
		return OUT[addr];
	}
	return RAM[addr];
}
unsigned short Ram::Read2Bytes(ADDR addr) const {
	if (addr < 0x300 && (addr & 0xFF) == 0xFF) {
		return 0;
	}
	if (addr < 0x100) {
		return RAM[addr] + RAM[addr + 1] * 256;
	}
	if (addr < 0x200) {
		return IN[addr] + IN[addr + 1] * 256;
	}
	if (addr < 0x300) {
		return OUT[addr] + OUT[addr + 1] * 256;
	}
	return RAM[addr] + RAM[addr + 1] * 256;
}
void Ram::ReadToDisket(BYTE*& dest) const {
	memcpy(dest, RAM, 0x100);
	memcpy(dest + 0x100, IN, 0x100);
	memcpy(dest + 0x200, OUT, 0x100);
	memcpy(dest + 0x300, RAM + 0x300, 0xFCFF);
}
void Ram::WriteFromDisket(const BYTE*& src) {
	memcpy(RAM, src, 0x100);
	memcpy(IN, src + 0x100, 0x100);
	memcpy(OUT, src + 0x200, 0x100);
	memcpy(RAM + 0x300, src + 0x300, 0xFCFF);
}
void Ram::Write(ADDR addr, BYTE byte) {
	if (addr < 0x100) {
		RAM[addr] = byte;
		return;
	}
	if (addr < 0x200) {
		IN[addr] = byte;
		return;
	}
	if (addr < 0x300) {
		OUT[addr] = byte;
		return;
	}
	RAM[addr] = byte;
}
void Ram::Write2Bytes(ADDR addr, unsigned short _Value) {
	if (addr < 0x300 && (addr & 0xFF) == 0xFF) {
		return;
	}
	if (addr < 0x100) {
		RAM[addr] = _Value % 256;
		RAM[addr + 1] = (unsigned char)(_Value / 256);
		return;
	}
	if (addr < 0x200) {
		IN[addr] = _Value % 256;
		IN[addr + 1] = (unsigned char)(_Value / 256);
		return;
	}
	if (addr < 0x300) {
		OUT[addr] = _Value % 256;
		OUT[addr + 1] = (unsigned char)(_Value / 256);
		return;
	}
	RAM[addr] = _Value % 256;
	RAM[addr + 1] = (unsigned char)(_Value / 256);
}
Ram::Ram() {
	IN = new BYTE[0x100];
	OUT = new BYTE[0x100];
	RAM = new BYTE[0x10000];
}
Ram::~Ram() {
	delete[] IN;
	delete[] OUT;
	delete[] RAM;
}

#undef ADDR
#undef SIZE
#undef BYTE