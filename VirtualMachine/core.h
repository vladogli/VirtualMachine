#pragma once
#include "CPU/cpu.h"
#include <boost/thread.hpp>
#include <boost/filesystem.hpp>
#include <thread>
class VirtualMachine {
private: // Variables
	CPU *core;
	bool closed = 0;
	memory *RAM;
	memory *outPort;
	boost::thread *thread;
	const unsigned long long unique_id;
public:
	std::wstring disketSaveFolder;
	unsigned short int Delay = 25;
private: // functions
	void PrivateUpdate();
	void PrivateThread();
public:
	void Update();
	void OpenThread();
	void CloseThread();
	bool GetThreadState();

	void Reboot();
	void Shutdown();
	void Boot();
	void SaveToDisket();
	bool LoadFromDisket();
	void SaveLive();
	bool LoadLive();

	unsigned char**& GetMatrix();
public:  // structors
	VirtualMachine(unsigned long long unique_id, std::function<void(void)>, bool = 0);
};
#include "core.cpp"