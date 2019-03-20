#define COMPILER
#ifdef COMPILER
#include <iostream>
#include <string>
#include "Compiler/Compiler.h"
int main(int argc, char** argv) {
	if (argc <= 2) {
		::std::string filename, output_filename;
		uint16_t offset;
		::std::cin >> filename >> output_filename >> offset;
		try {
			compile(filename, output_filename, offset);
		}
		catch (std::string v) {
			::std::cout << "Unknown token: " << v;
		}
	}
	else {
		try {
			compile(argv[1], argv[2], ::std::stoi(argv[3]));
		}
		catch (std::string v) {
			::std::cout << "Unknown token: " << v;
		}
	}
	system("pause");
}
#else
#include <iostream>
#include <Windows.h>
#include <string>
#include "System/Device.h"
#include <conio.h>
void gotoxy(short x, short y)
{
	COORD coord;
	coord.X = x;
	coord.Y = y;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}
void function(Monitor *m) {
	while (1) {
		m->ProcessKey(_getch());
		std::this_thread::sleep_for(std::chrono::milliseconds(25));
	}
}
void renderScreen(Monitor* monitor) {
	auto matr = monitor->getPtrtoPtr();
	unsigned char* saved = new unsigned char[4080];
	memset(saved, 0, 4080);
	memset((*matr), 0x20, 4080);
	while (1) {
		std::this_thread::sleep_for(std::chrono::milliseconds(25));
		for (short int i = 0; i < 4080; i++) {
			if (saved[i] != (*matr)[i]) {
				gotoxy(i % 80 , (i / 80));
				::std::cout << (*matr)[i];
				saved[i] = (*matr)[i];
			}
		}
	}
}
#include <fstream>
int main(int argc, char ** argv) {
	
	SystemService ss;
	uint16_t id = ss.CreateNewComputer();
	::std::thread keys(function, (Monitor*)ss.GetPointerToComputer(id)->devices[1]), render(renderScreen, (Monitor*)ss.GetPointerToComputer(id)->devices[1]);
	render.join();
	keys.join();
}
#endif