#include "VMService.h"
BYTE VMService::ConnectTwoDevices(u64 first, u64 second) {
	for (size_t i = 0; i < connections.size(); i++) {
		if (connections[i].first == first || connections[i].second == first) {
			return 1;
		}
	}
	for (size_t i = 0; i < connections.size(); i++) {
		if (connections[i].first == second || connections[i].second == second) {
			return 5;
		}
	}
	bool _V2 = 0;
	VirtualMachine *fptr, *sptr;
	for (size_t i = 0; i < ptrs.size(); i++) {
		if (ptrs[i]->unique_id == first) {
			if (!ptrs[i]->IsOpened()) {
				return 4;
			}
			fptr = ptrs[i];
		}
		else if (ptrs[i]->unique_id == second) {
			if (!ptrs[i]->IsOpened()) {
				return 3;
			}
			sptr = ptrs[i];
			_V2 = 1;
		}
	}
	if (!_V2) {
		return 2;
	}
	delete[] fptr->core->RAM->IN;
	fptr->core->RAM->IN = sptr->core->RAM->OUT;
	delete[] sptr->core->RAM->IN;
	sptr->core->RAM->IN = fptr->core->RAM->OUT;
	connections.push_back(std::pair<u64, u64>(first, second));
	return 0;
}
void VMService::CloseConnection(u64 first, u64 second) {
	for (size_t i = 0; i < connections.size(); i++) {
		if (connections[i].first == first || connections[i].first == second) {
			connections.erase(connections.begin() + i);
			for (size_t i = 0; i < ptrs.size(); i++) {
				if (ptrs[i]->unique_id == first) {
					delete[] ptrs[i]->core->RAM->IN;
					ptrs[i]->core->RAM->IN = new BYTE[0x100];
				}
				if (ptrs[i]->unique_id == second) {
					delete[] ptrs[i]->core->RAM->IN;
					ptrs[i]->core->RAM->IN = new BYTE[0x100];
				}
			}
		}
	}
}
bool VMService::IsConnectionAlive(u64 first, u64 second) {
	for (size_t i = 0; i < connections.size(); i++) {
		if (connections[i].first == first || connections[i].first == second) {
			return connections[i].second == second;
		}
	}
	return 0;
}
void VMService::RemoveMachine(u64 id) {
	for (size_t i = 0; i < ptrs.size(); i++) {
		if (ptrs[i]->unique_id == id) {
			ptrs.erase(ptrs.begin() + i);
			return;
		}
	}
	return;
}
VirtualMachine *VMService::CreateNewMachine(u64 id) {
	VirtualMachine *ptr = new VirtualMachine(id);
	ptr->core->IoConnectToNewDevice =
		std::bind(&VMService::ConnectTwoDevices, this, id, std::placeholders::_1);
	ptr->core->IoIsPortOpened =
		std::bind(&VMService::IsConnectionAlive, this, id, std::placeholders::_1);
	ptr->core->IoCloseConnection = 
		std::bind(&VMService::CloseConnection, this, id, std::placeholders::_1);

	ptrs.push_back(ptr);
	return ptr;
}
bool VMService::OpenConsoleByID(const u64 id) {
	for (size_t i = 0; i < ptrs.size(); i++) {
		if (ptrs[i]->unique_id == id) {
			ptrs[i]->OpenThread();
			return 1;
		}
	}
	return 0;
}
void VMService::PThread() {
	while (1) {
		for (size_t i = 0; i < ptrs.size(); i++) {
			ptrs[i]->Update();
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(25));
	}
}
bool VMService::CloseConsoleByID(const u64 id) {
	for (size_t i = 0; i < ptrs.size(); i++) {
		if (ptrs[i]->unique_id == id) {
			ptrs[i]->CloseThread();
			return 1;
		}
	}
	return 0;
}
VMService::VMService() {
	UpdateConsolesThread = new boost::thread(std::bind(&VMService::PThread, this));
}