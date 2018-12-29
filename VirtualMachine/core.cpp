#include "core.h"


VirtualMachine::VirtualMachine(unsigned long long unique_id1, std::function<void(void)>, bool x) : unique_id(unique_id1){
	core = new CPU();
	RAM = new memory(0xFFFF);
	outPort = new memory(0xFFFF);
	core->RAM = RAM;
	core->outPort = outPort;
	if (x) {
		LoadLive();
	}
	else {
		LoadFromDisket();
	}
}
void VirtualMachine::PrivateUpdate() {
	core->NextOp();
}
void VirtualMachine::PrivateThread() {
	while (!closed) {
		PrivateUpdate();
		std::this_thread::sleep_for(std::chrono::milliseconds(Delay));
	}
	closed = 0;
}
void VirtualMachine::Update() {
	if (closed) {
		PrivateUpdate();
	}
}
void VirtualMachine::OpenThread() {
	if (closed) {
		closed = 0;
		thread = new boost::thread(std::bind(&VirtualMachine::PrivateThread, this));
	}
}
void VirtualMachine::CloseThread() {
	if (!closed) {
		closed = 1;
		while (closed != 0) {
			std::this_thread::sleep_for(std::chrono::milliseconds(Delay));
		}
		closed = 1;
		delete thread;
	}
}
bool VirtualMachine::GetThreadState() {
	return !closed;
}

void VirtualMachine::Reboot() {

}
void VirtualMachine::Shutdown() {

}
void VirtualMachine::Boot() {

}

void VirtualMachine::SaveToDisket() {
	boost::filesystem::ofstream File(disketSaveFolder + std::to_wstring(unique_id) + L".iso", std::ios_base::binary);
	for (size_t i = 0; i < RAM->memSize; i++) {
		File << RAM->mem[i];
	}
	File.close();
}
bool VirtualMachine::LoadFromDisket() {
	if (!boost::filesystem::exists(disketSaveFolder + std::to_wstring(unique_id) + L".iso")) {
		return 0;
	}
	boost::filesystem::ifstream File(disketSaveFolder + std::to_wstring(unique_id) + L".iso");
	std::string _Val, _Val1;
	while (getline(File, _Val1)) {
		_Val += _Val1;
	}
	File.close();
	if (_Val.size() > 0) {
		const unsigned char* _v = reinterpret_cast<const unsigned char*>(_Val.c_str());
		RAM->Write(0, _v, 0xFFFF);
	}
	return 1;
}
void VirtualMachine::SaveLive() {
	boost::filesystem::ofstream File(disketSaveFolder + std::to_wstring(unique_id) + L".LRAM", std::ios_base::binary);
	for (size_t i = 0; i < RAM->memSize; i++) {
		File << RAM->mem[i];
	}
	File.close();
	File.open(disketSaveFolder + std::to_wstring(unique_id) + L".LSTACK", std::ios_base::binary);
	for (size_t i = 0; i < core->RAM->memSize; i++) {
		File << core->stack->mem[i];
	}
	File.close();
}
bool VirtualMachine::LoadLive() {
	if (!boost::filesystem::exists(disketSaveFolder + std::to_wstring(unique_id) + L".LRAM") ||
		!boost::filesystem::exists(disketSaveFolder + std::to_wstring(unique_id) + L".LSTACK")) {
		return 0;
	}
	boost::filesystem::ifstream File(disketSaveFolder + std::to_wstring(unique_id) + L".LRAM");
	std::string _Val, _Val1;
	while (getline(File, _Val1)) {
		_Val += _Val1;
	}
	File.close();
	if (_Val.size() > 0) {
		const unsigned char* _v = reinterpret_cast<const unsigned char*>(_Val.c_str());
		RAM->Write(0, _v, 0xFFFF);
	}

	File.open(disketSaveFolder + std::to_wstring(unique_id) + L".LSTACK");
	_Val = "";
	while (getline(File, _Val1)) {
		_Val += _Val1;
	}
	File.close();
	if (_Val.size() > 0) {
		const unsigned char* _v = reinterpret_cast<const unsigned char*>(_Val.c_str());
		core->stack->Write(0, _v, 0xFFFF);
	}
	return 1;
}
unsigned char**& VirtualMachine::GetMatrix() {
	return core->matrix;
}
void VirtualMachine::ReceiveKey(unsigned char byte) {
	core->keyPressed(byte);
}