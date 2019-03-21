#include "Device.h"
void System::Work() {
	for (uint8_t i = 0; i < size; i++) {
		devices[i]->Work();
	}
}
CPU::CPU() {
procMemory = new uint8_t[0x30];
WriteToRegister(procMemory, 0x08, 0x20);
typeOfDevice = 1;
	if (functions == nullptr) {
		functions = new ::std::function<void(MemoryService *RAM, uint8_t *procMemory)>*[0x100];
		for (size_t i = 0; i < 0x100; i++) {
			functions[i] = nullptr;
		}
#define init_func(x,f) functions[x] = new \
::std::function<void(MemoryService *RAM, uint8_t *procMemory)>(f);
		init_func(0, nop);
		init_func(1, op_jump_if);
		init_func(2, op_jump_ifnt);
		init_func(3, ret);
		init_func(4, push);
		init_func(5, pop);
		init_func(9, set_flag);
		init_func(10,clc_flag);
		init_func(11,jmp);
		init_func(12,call);
		
		init_func(0x10, xor_reg);
		init_func(0x11, or_reg);
		init_func(0x12, and_reg);
		init_func(0x13, add_reg);
		init_func(0x14, sub_reg);
		init_func(0x15, mul_reg);
		init_func(0x16, div_reg);
		init_func(0x17, mod_reg);
		init_func(0x18, xor_flag);
		init_func(0x19, or_flag);
		init_func(0x1A, and_flag);

		init_func(0x1B, inc_reg);
		init_func(0x1C, dec_reg);
		init_func(0x1D, not_reg);
		init_func(0x1E, not_flag);
		init_func(0x20, mov_reg_value);
		init_func(0x21, mov_reg_reg);
		init_func(0x22, mov_reg_MEM);
		init_func(0x23, mov_MEM_REG);
		init_func(0x24, mov_MEMreg_value);
		init_func(0x25, mov_MEMreg_reg);
#undef init_func
	}
}
void CPU::Work() {
	uint16_t value = getReg2Value(procMemory, 0);
	uint8_t ir = uint8_t(getReg2Value(procMemory, 0xA));
	if (ir != 0 && !ReadFromRegister(procMemory, 0x16)) {
		WriteValueToStack(value, memory, procMemory);
		setReg2Value(procMemory, 0, memory->Read<uint16_t>(ir*2));
	}
}
bool CPU::CallInterrupt(uint8_t type, uint16_t value) {
	if (getRegValue(procMemory, 0x16)) {
		return 0;
	}
	setReg2Value(procMemory, 0x09, value);
	setReg2Value(procMemory, 0x0A, type);
	return 1;
}
CPU::~CPU() {
	delete[] procMemory;
}
Monitor::Monitor() {
	typeOfDevice = 2;
	local_memory = new uint8_t[4081]; //51*80 + 2
}
uint8_t **Monitor::getPtrtoPtr() { 
	return &local_memory; 
}
void Monitor::Work() {
	if (memory->Read<uint8_t>(0xA2) == 1) {
		memcpy(&local_memory[(memory->Read<uint8_t>(0xA0) * 51) % 4001], &memory->memory[0xB0], 80);
		memory->Write<uint8_t>(0xA2, 0);
	}
	uint8_t color = memory->Read<uint8_t>(0xA1);
	if (color != local_memory[4080]) {
		local_memory[4080] = color;
	}
}
Monitor::~Monitor() {
	delete[] local_memory;
}
System::~System() {
	for (size_t i = 0; i < size; i++) {
		switch (devices[i]->typeOfDevice) {
		case 1:
			delete ((CPU*)devices[i]);
			break;
		case 2:
			delete ((Monitor*)devices[i]);
			break;
		case 3:
			delete ((NetCard*)devices[i]);
			break;
		default:delete devices[i]; break;
		}
	}
	delete[] devices;
}	
Computer::Computer(::std::function<void(CIDevice*)>& initNetCard) : System(3) {
	devices = new Device*[3];
	devices[0] = new CPU();
	devices[1] = new Monitor();
	((CIDevice*)devices[1])->callInterrupt = ::std::function<bool(uint8_t, uint16_t)>(
		std::bind(&CPU::CallInterrupt, (CPU*)devices[0],
			::std::placeholders::_1, ::std::placeholders::_2));

	devices[2] = new NetCard();
	((CIDevice*)devices[2])->callInterrupt = ::std::function<bool(uint8_t, uint16_t)>(
		std::bind(&CPU::CallInterrupt, (CPU*)devices[0],
			::std::placeholders::_1, ::std::placeholders::_2));
	initNetCard((CIDevice*)devices[2]);
}
void SystemService::WorkThread() {
	while (threadState != 0) {
		while (threadState == 2 || computers.size() == 0) {
			::std::this_thread::sleep_for(::std::chrono::milliseconds(50));
		}
		for (size_t i = 0; i < computers.size(); i++) {
			computers[i].first->Work();
		}
	}
}
void  SystemService::InitCard(CIDevice*) {}
SystemService::SystemService() :
	thread(::std::bind(&SystemService::WorkThread, this)) {
	func = ::std::bind(&SystemService::InitCard, this, ::std::placeholders::_1);

}
Computer*  SystemService::GetPointerToComputer(uint16_t id) {
	for (auto ptr = computers.begin(); ptr != computers.end(); ptr++) {
		if ((*ptr).second == id) {
			return (*ptr).first;
		}
	}
	return nullptr;
}
uint16_t SystemService::CreateNewComputer() {
	computers.push_back(
		ComputerWithIP(
			new Computer(func),
			computers.size()));
	return uint16_t(computers.size() - 1);
}
SystemService::~SystemService() {
	threadState = 0;
	thread.join();
	for (size_t i = 0; i < computers.size(); i++) {
		delete computers[i].first;
	}
}