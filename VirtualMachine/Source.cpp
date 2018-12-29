

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
	unsigned char** saved = new (unsigned char*[51]);
	for (size_t j = 0; j < 51; j++) {
		saved[j] = new unsigned char[80];
		for (int i = 0; i < 80; i++) {
			saved[j][i] = 0;
		}
	}
	while (1) {
		std::this_thread::sleep_for(std::chrono::milliseconds(25));
		for (short i = 0; i < 80; i++) {
			for (short j = 0; j < 50; j++) {
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
#include <vector>
struct word {
	std::string    Name;
	unsigned short Addr;
	bool _v = 0;
	enum PARAMS {
		NOTHING,
		REG,
		REG_REG,
		REG_REG_REG,
		REG_V2,
		V2_REG,
		V2_V2_V2,
		V2, // value with 2 bytes
		V1, // value with 1 byte

	} Param;
	word(PARAMS param, std::string name, unsigned short addr, bool _v = 0) {
		Name = name;
		Addr = addr;
		Param = param;
		this->_v = _v;
	}
	word() {}
};
void AllToHighChars(std::string &str){
	for (size_t i = 0; i < str.size(); i++) {
		if (str[i] >= 'a' && str[i] <= 'z') str[i] = str[i] - 'a' + 'A';
	}
}
bool Function(std::string _Value, char _Val) {
	for (size_t i = 0; i < _Value.size(); i++) {
		if (_Value[i] == _Val) return 1;
	}
	return 0;
}
bool CheckVector(std::vector<word>& words, std::string _Value) {
	for (size_t i = 0; i < words.size(); i++) {
		if (words[i].Name == _Value) {
			return 0;
		}
	}
	return 1;
}
word find(std::vector<word>& words, std::string _Value) {
	for (size_t i = 0; i < words.size(); i++) {
		if (words[i].Name == _Value) {
			return words[i];
		}
	}
	return word();
}
int main(void) {
	std::string source, dest;
	unsigned short offset;
	std::cout << "Enter source filename: ";
	std::cin >> source;
	std::cout << "Enter dest filename: ";
	std::cin >> dest;
	std::cout << "Enter offset: ";
	std::cin >> offset;
	
	std::ifstream src(source);
	std::ofstream out(source, std::ios_base::binary);
	std::vector<word> words;
#define WORDS_ADD(x,y,z) words.push_back(word(word::x,y,z))
	WORDS_ADD(NOTHING, "EXT", 0x00);
	WORDS_ADD(REG, "ITOS", 0x01);

	WORDS_ADD(V2, "JMP", 0x10);
	WORDS_ADD(V2, "JZ", 0x11);
	WORDS_ADD(V2, "JNZ", 0x12);

	WORDS_ADD(REG_REG, "XOR", 0x20);
	WORDS_ADD(REG_REG, "OR",  0x21);
	WORDS_ADD(REG_REG, "AND", 0x22);
	WORDS_ADD(REG_REG, "ADD", 0x23);
	WORDS_ADD(REG_REG, "SUB", 0x24);
	WORDS_ADD(REG_REG, "MUL", 0x25);
	WORDS_ADD(REG_REG, "DIV", 0x26);
	WORDS_ADD(REG_REG, "INC", 0x27);
	WORDS_ADD(REG_REG, "DEC", 0x28);

	WORDS_ADD(REG_REG_REG, "CONCAT", 0x30);
	WORDS_ADD(REG_REG, "TOI", 0x31);
	WORDS_ADD(REG_REG, "SLN", 0x32);

	WORDS_ADD(REG_REG, "CMP", 0x40);
	WORDS_ADD(REG_V2, "CMP", 0x41);
	WORDS_ADD(REG_REG, "CMPS", 0x42);

	WORDS_ADD(NOTHING, "NOP", 0x50);
	WORDS_ADD(REG_REG, "MOV", 0x51);


	WORDS_ADD(REG_V2, "MOV", 0x60);
	WORDS_ADD(V2_REG, "MOV", 0x61);
	WORDS_ADD(V2_V2_V2, "CPY", 0x62);
	WORDS_ADD(REG_REG_REG, "CPY", 0x62);
	WORDS_ADD(NOTHING, "CPY", 0x62);

	WORDS_ADD(V2, "PUSH", 0x70);
	WORDS_ADD(REG, "PUSH", 0x71);
	WORDS_ADD(REG, "POP", 0x72);
	WORDS_ADD(NOTHING, "RET", 0x73);
	WORDS_ADD(NOTHING, "CALL", 0x74);

	WORDS_ADD(V2, "SWSTR", 0x80);
	WORDS_ADD(REG, "SWSTR", 0x81);
	WORDS_ADD(V2, "SWINT", 0x82);
	WORDS_ADD(REG, "SWINT", 0x83);
	WORDS_ADD(V2, "SWCHR", 0x84);
	WORDS_ADD(REG, "SWCHR", 0x85);
	WORDS_ADD(NOTHING, "SCR", 0x86);
	WORDS_ADD(NOTHING, "ENDL", 0x87);
	WORDS_ADD(NOTHING, "PAGE", 0x88);

	std::string _Value;
	std::vector<std::string> errors;
	while (getline(src,_Value)) {
		AllToHighChars(_Value);
		std::string Word;
		for (size_t i = 0; i < _Value.size(); i++) {
			if (_Value[i] == ' ') {
				if (Word.size() > 0) {
					break;
				}
				else {
					continue;
				}
			}
			Word += _Value[i];
		}
		if (Function(_Value,':')) {
			words.push_back(word(word::NOTHING, Word, offset,1));
		}
		else  if (CheckVector(words, Word)) {
			errors.push_back("Unknown element: " + Word);
		}
		else {
			auto _V = find(words, Word);
			switch (_V.Param) {
				case word::NOTHING: {
					if (_V._v) {
						out << (unsigned char)(_V.Addr % 256);
						out << (unsigned char)(_V.Addr / 256);
					}
					else {
						out << (unsigned char)(_V.Addr);
					}
					break;
				}
				case word::REG: {
					out << (unsigned char)(_V.Addr);
					Word = "";
					for (size_t i = 0; i < _Value.size(); i++) {
						if (_Value[i] == ' ') {
							if (Word.size() > 0) {
								break;
							}
							else {
								continue;
							}
						}
						Word += _Value[i];
					}
					if (CheckVector(words, Word)) {
						errors.push_back("Unknown element: " + Word);
					}
					auto _X = find(words, Word);
					out << (unsigned char)_X.Addr;
					break;
				}
				case word::REG_REG: {
					out << (unsigned char)(_V.Addr);
				
				}
			}
		}
	}
	for (int i = 0; i < errors.size(); i++) {
		std::cout << errors[i] << std::endl;
	}
}
#endif