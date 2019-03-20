#pragma once
#include <memory>
#include <functional>
#include <stdint.h>


#define MEMORY_DEBUG

#ifdef MEMORY_DEBUG
#include <ios>
#include <sstream>
#include <iostream>
#include <iomanip>
#endif
#include <mutex>
class MemoryService {
public:
	uint8_t* memory;
	uint16_t size;
	::std::mutex write_mutex;
	enum exceptions {
		WRONG_ADDR = 0xFFFF
	};
#ifdef MEMORY_DEBUG
	void DUMP();
#endif
	template<typename T>
	T Read(uint16_t addr);
	template<typename T>
	void Write(uint16_t addr, T value);

	MemoryService(uint16_t size) : size(size) {
		memory = new uint8_t[size];
		memset(memory, 0, size);
	}

	~MemoryService() {
		delete[] memory;
	}
};
#include "MemoryService.cpp"

#ifdef MEMORY_DEBUG
#undef MEMORY_DEBUG
#endif