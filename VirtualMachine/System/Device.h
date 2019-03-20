#pragma once
#include <cstdlib>
#include "../Core/MemoryService.h"
// TODO
// network
class Device {
public:
	MemoryService *memory;
	virtual void Work () {}
	uint16_t typeOfDevice = 0;
};
#include "../Core/OPCodes.h"
::std::function<void(MemoryService *RAM, uint8_t *procMemory)> **functions = nullptr;
// 0x00-0x9F
// for interrupts and stack
// 0x00-0x1F - interrupts,
// 0x20-0x9F - stack (0x80(128) bytes, 64 numbers)
class CPU : public Device {
private:
	/*
		Registers
		ASM(name)  Addr           Desc                   BYTE-CODE
		IP         0x00-0x01      Instruction Pointer    0x00
		AX         0x02-0x03      Accumulator            0x01, AL - 0x20, AH - 0x21
		BX         0x04-0x05      Base        Register   0x02, BL - 0x22, BH - 0x23
		CX         0x06-0x07      Count       Register   0x03, CL - 0x24, CH - 0x25
		DX         0x08-0x09      Base        Register   0x04, DL - 0x26, DH - 0x27
		SI         0x0A-0x0B      Source      Index      0x05
		DI         0x0C-0x0D      Destination Index      0x06
		BP         0x0E-0x0F      Base        Pointer    0x07
		SP         0x10-0x11      Stack       Pointer    0x08
		IV         0x12-0x13      Interrupt value        0x09
		IR         0x14-0x15      Interrupt Register     0x0A
		FLAGS
		ASM(name)  Addr           Desc                   BYTE-CODE
		CF         0x20           Carry     Flag         0x10
		PF         0x21           Parity    Flag         0x11
		AF         0x22           Auxiliary Carry  Flag  0x12
		ZF         0x23           Zero      Flag         0x13
		SF         0x24           Sign      Flag         0x14
		TF         0x25           Trap      Flag         0x15
		IF         0x26           Interrupt Enable Flag  0x16
		DF         0x27           Direction Flag         0x17
		OF         0x28           Overflow  Flag         0x18

	*/
	uint8_t *procMemory;
public:
	bool CallInterrupt(uint8_t type, uint16_t value);
	CPU();
	void Work();
	~CPU();
};
// call interrupt device
class CIDevice : public Device {
public:
	::std::function<bool(uint8_t, uint16_t)> callInterrupt;
};
// 0xA0-0x100
class Monitor : public CIDevice {
private:
	uint8_t *local_memory;
public:
	uint8_t **getPtrtoPtr() ;
	// Size of the monitor - 80 x 51 (4080 bytes) 
/* in RAM
* 0x00 - line number(0...50)
* 0x01 - color (0xBF), B - background, F - foreground.
* 0x02 - apply changes
0 = Black  8 = Gray
1 = Blue   9 = Light Blue
2 = Green  A = Light green
3 = Blue   B = Light Blue
4 = Red    C = Light Red
5 = Lilac  D = Light purple
6 = Yellow E = Light Yellow
7 = White  F = Bright White
* 0x10-0x60 - data
*/
	void Work();
	void ProcessKey(uint16_t key) {
		callInterrupt(0, key);
	}
	Monitor();
	~Monitor();
};
// 0x100-0x190
// 0x100-0x101 - dest address
// 0x102       - port 
// 0x110-0x190 - data || 128 bytes
class NetCard : public CIDevice {
public:
	void Work() {}
	NetCard() { typeOfDevice = 3; }
	~NetCard() {}
};
class System {
public:
	Device **devices;
	MemoryService memService;
	uint8_t size;
	System(uint8_t size) : size(size), memService(0xFFFF) {}
	void Work();
	~System();
};
class Computer : public System {
public:
	Computer(::std::function<void(CIDevice*)>& initNetCard);
	~Computer() {  }
};
#include <vector>
class SystemService {
typedef ::std::pair<Computer*, uint16_t> ComputerWithIP;
	::std::vector<ComputerWithIP> computers;
	::std::function<void(CIDevice*)> func;
	::std::thread thread;
	// 0 - close
	// 1 - work
	// 2 - pause
	uint8_t threadState;
	void WorkThread();
public:
	void InitCard(CIDevice*);
	SystemService();
	Computer* GetPointerToComputer(uint16_t id);
	uint16_t CreateNewComputer();
	~SystemService();
};
#include "Device.cpp"