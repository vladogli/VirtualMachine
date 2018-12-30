

#include "core.h"
#include <Windows.h>
#include <iostream>
#include <conio.h>
#define CORE_TEST
#ifdef CORE_TEST
void gotoxy(short x, short y)
{
	COORD coord;
	coord.X = x;
	coord.Y = y;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}
void function(VirtualMachine& VM) {
	while (1) {
		VM.ReceiveKey((unsigned char)_getch());
		std::this_thread::sleep_for(std::chrono::milliseconds(25));
	}
}
void func() {

}
int main(void)
{
	VirtualMachine VM(0xabcdef, std::function<void(void)>(func));
	VM.OpenThread();
	boost::thread thread(function, std::ref(VM));
	auto matr = VM.GetMatrix();
	unsigned char** saved = new (unsigned char*[60]);
	for (size_t j = 0; j < 60; j++) {
		saved[j] = new unsigned char[150];
		for (int i = 0; i < 150; i++) {
			saved[j][i] = 0;
		}
	}
	while (1) {
		std::this_thread::sleep_for(std::chrono::milliseconds(25));
		for (short i = 0; i < 150; i++) {
			for (short j = 0; j < 60; j++) {
				if (matr[j][i] != saved[j][i])
				{
					saved[j][i] = matr[j][i];
					gotoxy(i, j);
					std::cout << matr[j][i];
				}
			}
		}
	}
	std::this_thread::sleep_for(std::chrono::hours(60));
	return 0;
}
#else
// TODO:
// Compiler
#endif