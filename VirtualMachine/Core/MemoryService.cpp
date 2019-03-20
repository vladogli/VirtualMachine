#include "MemoryService.h"
void MemoryService::DUMP() {
	::std::cout << ::std::endl;
	for (size_t i = 0; i < size; i += 0x14) {
		for (size_t j = i; j < size && j < i + 0x14; j += 4) {
			for (size_t k = j; k < size && k < j + 0x4; k++) {
				::std::stringstream a;
				a << std::uppercase << std::setfill('0') << std::setw(2) << ::std::hex << uint16_t(memory[k]);
				::std::cout << a.str() << " ";
			} ::std::cout << "   ";
		} ::std::cout << ::std::endl;
	}
}
template<typename T>
T MemoryService::Read(uint16_t addr) {
	if ((sizeof(T) + addr) > size) {
		throw WRONG_ADDR;
	}
	T returnValue = 0;
	memcpy(&returnValue, memory + addr, sizeof(T));
	return returnValue;
}
template<typename T>
void MemoryService::Write(uint16_t addr, T value) {
	if ((sizeof(T) + addr) > size) {
		throw WRONG_ADDR;
	}
	write_mutex.lock();
	memcpy(memory + addr, &value, sizeof(T));
	write_mutex.unlock();
}