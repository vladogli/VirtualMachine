#include "core.h"


VirtualMachine::VirtualMachine(unsigned long long unique_id1) : unique_id(unique_id1){
	core = new CPU();
	RAM = new Ram();
	core->RAM = RAM;
	core->OpenPort = std::bind(&VirtualMachine::Open, this);
	core->ClosePort = std::bind(&VirtualMachine::Close, this);
	core->GetPortState = std::bind(&VirtualMachine::IsOpened, this);
	LoadFromDisket();
}
void VirtualMachine::PrivateUpdate() {
	core->NextOp();
}
void VirtualMachine::PrivateThread() {
	while (!closed) {
		PrivateUpdate();
		std::this_thread::sleep_for(Delay);
	}
	closed = 0;
}
bool VirtualMachine::IsOpened() {
	return opened;
}
void VirtualMachine::Open() {
	opened = 1;
}
void VirtualMachine::Close() {
	opened = 0;
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
	unsigned char *_value = new unsigned char[0x10000];
	RAM->ReadToDisket(_value);
	for (size_t i = 0; i < 0xFFFF; i++) {
		File << _value[i];
	}
	File.close();
	delete[] _value;
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
		RAM->WriteFromDisket(_v);
	}
	return 1;
}
unsigned char**& VirtualMachine::GetMatrix() {
	return core->matrix;
}
void VirtualMachine::ReceiveKey(unsigned char byte) {
	core->keyPressed(byte);
}