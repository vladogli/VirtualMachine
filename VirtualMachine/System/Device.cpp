#include "Device.h"
void System::Work() {
	for (uint8_t i = 0; i < size; i++) {
		devices[i]->Work();
	}
}
CPU::CPU() {
procMemory = new uint8_t[0x30];
memset(procMemory, 0, 0x30);
WriteToRegister(procMemory, 0x08, 0x20);
typeOfDevice = 1;
	if (functions == nullptr) {
		functions = new ::std::function<void(MemoryService *RAM, uint8_t *procMemory)>*[0x200];
		for (size_t i = 0; i < 0x200; i++) {
			functions[i] = nullptr;
		}
		setReg2Value(procMemory, 0, 0x190);
#define init_func(x,f) functions[x] = new \
::std::function<void(MemoryService *RAM, uint8_t *procMemory)>(f);
		init_func(0, nop);
		init_func(1, op_jump_if);
		init_func(2, op_jump_ifnt);
		init_func(3, jmp);
		init_func(4, call);
		init_func(5, ret);
		init_func(6, push);
		init_func(7, pop);
		init_func(8, set_flag);
		init_func(9, clc_flag);
		
		init_func(0x10, xor_reg_reg);
		init_func(0x11, or_reg_reg);
		init_func(0x12, and_reg_reg);
		init_func(0x13, add_reg_reg);
		init_func(0x14, sub_reg_reg);
		init_func(0x15, cmp_reg_reg);
		init_func(0x16, mul_reg_reg);
		init_func(0x17, div_reg_reg);
		init_func(0x18, mod_reg_reg);
		init_func(0x19, xor_flag_flag);
		init_func(0x1A, or_flag_flag);
		init_func(0x1B, and_flag_flag);
		init_func(0x1C, inc_reg);
		init_func(0x1D, dec_reg);
		init_func(0x1E, not_reg);
		init_func(0x1F, not_flag);

		init_func(0x20, xor_reg_value);
		init_func(0x21, or_reg_value);
		init_func(0x22, and_reg_value);
		init_func(0x23, add_reg_value);
		init_func(0x24, sub_reg_value);
		init_func(0x25, cmp_reg_value);
		init_func(0x26, mul_reg_value);
		init_func(0x27, div_reg_value);
		init_func(0x28, mod_reg_value);
		init_func(0x29, xor_flag_value);
		init_func(0x2A, or_flag_value);
		init_func(0x2B, and_flag_value);

		init_func(0x30, mov_reg_value);
		init_func(0x31, mov_reg_reg);
		init_func(0x32, mov_reg_MEM);
		init_func(0x33, mov_MEM_reg);
		init_func(0x34, mov_MEMreg_value);
		init_func(0x35, mov_MEMreg_reg);
		init_func(0x36, mov_MEM_value);
#undef init_func
	}
}
void CPU::Work() {
	uint16_t value = getReg2Value(procMemory, 0);
	uint8_t ir = uint8_t(getReg2Value(procMemory, 0xA));
	if (ir != 0 && !ReadFromRegister(procMemory, 0x16)) {
	//	WriteValueToStack(value, memory, procMemory);
		setReg2Value(procMemory, 0, memory->Read<uint16_t>((ir - 1)*2));
		setReg2Value(procMemory, 0xA, 0);
	}
	value = memory->Read<uint8_t>(value);
	if (functions[value] != nullptr) {
		functions[value]->operator()(memory, procMemory);
	}
	else functions[0]->operator()(memory, procMemory);
}
bool CPU::CallInterrupt(uint8_t type, uint16_t value) {
	if (getRegValue(procMemory, 0x16)) {
		return 0;
	}
	setReg2Value(procMemory, 0x09, value);
	setReg2Value(procMemory, 0x0A, type+1);
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
	devices[0]->memory = &memService;
	devices[1] = new Monitor();
	devices[1]->memory = &memService;
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
	while (1) {
		if (threadState == 0) {
			::std::cout << "BLYAT.";
		}
		while (threadState == 2 || computers.size() == 0) {
			::std::this_thread::sleep_for(::std::chrono::milliseconds(50));
		}
		for (size_t i = 0; i < computers.size(); i++) {
			while (threadState == 2 || computers.size() == 0) {
				::std::this_thread::sleep_for(::std::chrono::milliseconds(50));
			}
			computers[i].first->Work();
		}
	}
}
void  SystemService::InitCard(CIDevice*) {}
SystemService::SystemService(){
	func = ::std::bind(&SystemService::InitCard, this, ::std::placeholders::_1);
	thread = new ::std::thread(::std::bind(&SystemService::WorkThread, this));
}
Computer*  SystemService::GetPointerToComputer(uint16_t id) {
	for (auto ptr = computers.begin(); ptr != computers.end(); ptr++) {
		if ((*ptr).second == id) {
			return (*ptr).first;
		}
	}
	return nullptr;
}
#include <fstream>
uint16_t SystemService::CreateNewComputer(::std::string load_path/*, ::std::string filepath // will be realized later*/) {
	threadState = 2;
	::std::ifstream file(load_path + "load_iso"), file2(load_path + "interrupt_data");
	::std::string str1, str2;
	while (getline(file, str2)) {
		str1 += str2;
	}
	file.close();
	computers.push_back(
		ComputerWithIP(
			new Computer(func),
			computers.size()));
	memcpy(&computers[computers.size() - 1].first->memService.memory[0x190], str1.c_str(), str1.size());
	str1 = "";
	while (getline(file2, str2)) {
		str1 += str2;
	}
	memcpy(computers[computers.size() - 1].first->memService.memory, str1.c_str(), str1.size());
	file2.close();
	threadState = 1;
	return uint16_t(computers.size() - 1);
}
SystemService::~SystemService() {
	threadState = 0;
	thread->join();
	delete thread;
	for (size_t i = 0; i < computers.size(); i++) {
		delete computers[i].first;
	}
}