#include <fstream>
void getFile(::std::string& returnValue) {
	::std::ifstream file(returnValue);
	returnValue = "";
	::std::string buffer;
	while (getline(file, buffer)) {
		returnValue += buffer + '\n';
	}
	file.close();
}
void removeComments(::std::string& string) {
	for (size_t i = 0, size = string.size(); i < size; i++) {
		if (string[i] == 0x09) {
			string[i] = ' '; i--;
		} else 
		if (string[i] == ',') {
			bool x = 0;
			// If ',' is contained in the "...", then we do not remove it.
			for (size_t j = 0, abc = -1; j < size; j++) {
				if (string[j] == '"') {
					if (abc == -1) {
						if (j > i) {
							break;
						}
						else {
							abc = j;
						}
					}
					else {
						x = 1; break;
					}
				}
			}
			if (x) break;
			string[i] = ' '; i--;
		} else
		if (string[i] == '\n'&& i + 1 < size) {
			if (string[i + 1] == ' ' || string[i + 1] == '\n') {
				string.erase(string.begin() + i + 1); size--; i--;
			} 
		} else
		if (string[i] == ';') {
			for (; i < size && string[i] != '\n'; string.erase(string.begin() + i), size--);
		} else 
		if (string[i] == ' ' && i + 1 < size ) {
			if (string[i + 1] == ' ' || string[i + 1] == '\n' || string[i + 1] == 0x09) {
				string.erase(string.begin() + i); size--;  i--;
			} 
		}
	}
}
struct keyword {
	::std::string string;
	uint16_t bytecode;
	bool reference = 0, addr =0;
};
#include <vector>
void findKeywords(::std::string& input, ::std::vector<keyword>& keywords) {
	::std::string line;
	uint16_t line_number = 0;
	for (size_t i = 0, size = input.size();; i++) {
		if(input[i]!='\n' && i < size ) {
			line += input[i];
		}
		else {
			if (line.size() == 0) {
				if (i >= size) {
					break;
				}
				continue;
			}
			if (line[line.size() - 1] == ':') {
				line.erase(line.begin() + line.size() - 1);
				keywords.push_back({ line,line_number, 1 });
				input.erase(input.begin() + (i - line.size() - 1), input.begin() + i + 1);
				i -= line.size() + 1;
				size = input.size();
			}
			else line_number++;
			line = "";
			if (i >= size) {
				break;
			}
		}
	}
}
void loadKeywords(::std::vector<keyword> & keywords) {
	::std::ifstream keywordsF("keywords.txt");
	::std::string buffer, name;
	while (getline(keywordsF, buffer)) {
		if (buffer.size() == 0) continue;
		size_t i = 0;
		for (size_t size = buffer.size(); i < size && buffer[i] != ' '; i++);
		for (size_t j = 0; j < i; j++) {
			name += buffer[j];
		}
		buffer.erase(buffer.begin(), buffer.begin() + i);
		keyword k;
		k.string = name;
		k.bytecode = stoi(buffer);
		keywords.push_back(k);
		buffer = "";
		name = "";
	}
	keywordsF.close();
}
// 0 not
// 1 dec 1 byte
// 2 hex 1 byte
uint8_t isInt(std::string str) {
	uint8_t returnValue = (str[str.size() - 1] == 'h');
	if (returnValue) {
		str.erase(str.begin() + str.size() - 1);
	}
	for (size_t i = 0; i < str.size(); i++) {
		if (!((str[i] >= '0' && str[i] <= '9'))) {
			if ((str[i] >= 'a' && str[i] <= 'f')) {
				if (!returnValue) {
					return 0;
				}
				else continue;
			}
			return 0;
		}
	}
	return returnValue + 1;
}
#define toDec(x) \
(((x) >= '0' && (x) <= '9') ? (x - '0') : ((x) - 'a' + 10))
uint16_t getIntFromHex(::std::string value) {
	value.erase(value.begin() + value.size() - 1);
	uint16_t returnValue = 0;
	for (size_t i = 0; i < value.size(); i++) {
		returnValue += uint16_t(toDec(value[value.size() - i - 1])) * uint16_t(::std::pow(16, i));
	}
	return returnValue;
}
void compileLine(::std::string const& input, ::std::string& out, uint16_t& offset, ::std::vector<keyword> const& keywords) {
	::std::string word;
	for (size_t i = 0, size = input.size();; i++) {
		if (i < size && input[i] != ' ') {
			word += input[i];
		}
		else if (word.size() > 0){
			if (word == "db") {
				word = input; i++;
				auto value = word.find('"');
				if (value == word.size()) return;
				word.erase(word.begin(), word.begin() + value + 1);
				value = word.find('"');
				if (value == word.size()) return;
				word.erase(word.begin() + value, word.begin() + word.size());
				offset += word.size();
				out += word;
				return;
			}
			else if (word == "dbh") {
				word = "";
				i++;
				for (;; i++) {
					if (i < size && input[i] != ' ') {
						word += input[i];
					}
					else if (word.size() > 0) {
						word += 'h';
						uint8_t state = isInt(word);
						if (state == 2) {
							uint16_t value = getIntFromHex(word);
							offset += 1;
							out += int8_t(value % 0x100);
						}
						word = "";
					}
					else if (i >= size) {
						return;
					}
				}
			}
			uint8_t state = isInt(word);
			if (state == 1) {
				uint16_t value = stoi(word);
				offset += 2;
				out += int8_t(value % 0x100);
				out += int8_t(value / 0x100);
				if (i >= size) break;
				word = "";
				continue;
			} else 
			if (state == 2) {
				uint16_t value = getIntFromHex(word);
				offset += 2;
				out += int8_t(value % 0x100);
				out += int8_t(value / 0x100);
				if (i >= size) break;
				word = "";
				continue;
			}
			auto itr = keywords.begin();
			for (; itr != keywords.end() && (*itr).string != word; itr++);
			if (itr == keywords.end()) throw word;
			offset += 1 + ((*itr).addr == 1);
			out += int8_t((*itr).bytecode % 0x100);
			if ((*itr).addr ==  1) {
				out += int8_t((*itr).bytecode / 0x100);
			}
			word = "";
			if (i >= size) break;
		}
		else break;
	}
}
::std::string completeCompile(::std::string input, uint16_t offset, ::std::vector<keyword> & keywords) {
	::std::string line;
	::std::string out;
	uint16_t line_number = 0;
	for (size_t i = 0, size = input.size(); i < size; i++) {
		if (input[i] != '\n') {
			line += input[i];
		}
		else {
			for (size_t i = 0, size = keywords.size(); i < size; i++) {
				if (keywords[i].reference) {
					if (keywords[i].bytecode == line_number) {
						keywords[i].bytecode = offset;
						keywords[i].reference = 0; keywords[i].addr = 1;
					}
				}
			}
			compileLine(line, out, offset, keywords);
			line = "";
			line_number++;
		}
	} 
	return out;
}
void Save(::std::string const& filename, ::std::string value) {
	::std::ofstream file(filename);
	file << value;
	file.close();
}
void transformToUndercase(::std::string &str) {
	for (size_t i = 0, size = str.size();i<size ; i++) {
		if (str[i] >= 'A' && str[i] <= 'Z') {
			str[i] = str[i] + 'a' - 'A';
		}
	}
}
void compile(::std::string input, ::std::string out_filename, uint16_t offset) {
	getFile(input);
	removeComments(input);
	transformToUndercase(input);
	::std::vector<keyword> keywords;
	findKeywords(input, keywords);
	loadKeywords(keywords);
	Save(out_filename, completeCompile(input, offset,keywords));
}