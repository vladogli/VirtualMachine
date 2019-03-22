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
//Just memory class :/
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
	// Read sizeof(T) bytes
	template<typename T>
	T Read(uint16_t addr);
	// Write sizeof(T) bytes
	template<typename T>
	void Write(uint16_t addr, T value);

	// Initialize a memService.
	MemoryService(uint16_t size) : size(size) {
		memory = new uint8_t[size];
		memset(memory, 0, size);
	}
	
	// Destruct a memService.
	~MemoryService() {
		delete[] memory;
	}
};
#include "MemoryService.cpp"

#ifdef MEMORY_DEBUG
#undef MEMORY_DEBUG
#endif