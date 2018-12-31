#pragma once
#include "core/core.h"
#include <vector>
#define BYTE unsigned char
#define u64 unsigned long long int
class VMService {
	std::vector<VirtualMachine*> ptrs;
	std::vector<std::pair<u64, u64>> connections;
	boost::thread *UpdateConsolesThread;
private:
	void PThread();
	BYTE ConnectTwoDevices(u64, u64);
	void CloseConnection(u64, u64);
	bool IsConnectionAlive(u64, u64);
public:
	VirtualMachine *CreateNewMachine(u64);
	void RemoveMachine(u64);
	bool OpenConsoleByID(const u64 id);  // open a thread in vm by id
	bool CloseConsoleByID(const u64  id); // close a thread in vm by id
	VMService();
};
#include "VMService.cpp"
#undef u64 
#undef BYTE