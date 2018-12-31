#pragma once
#include "../CPU/cpu.h"
#include <boost/thread.hpp>
#include <boost/filesystem.hpp>
#include <thread>
class VirtualMachine {
private: // Variables
	bool closed = 1;
	Ram *RAM;
	bool opened;
	boost::thread *thread;
public:
	CPU *core;
	const unsigned long long unique_id;
	std::wstring disketSaveFolder;
	unsigned short int Delay = 25;
private: // functions
	void PrivateUpdate();
	void PrivateThread();
	void Open();
	void Close();
public:
	bool IsOpened();
	void Update();
	void OpenThread();
	void CloseThread();
	bool GetThreadState();

	void Reboot();
	void Shutdown();
	void Boot();
	void SaveToDisket();
	bool LoadFromDisket();
	unsigned char**& GetMatrix();
	void ReceiveKey(unsigned char byte);
public:  // structors
	VirtualMachine(unsigned long long unique_id);
};
#include "core.cpp"