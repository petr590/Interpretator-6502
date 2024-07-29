#include "scroll.h"
#include "drawer.h"
#include <vector>
#include <ncurses.h>

namespace int6502 {
	static const int START_X = WIN_WIDTH + 2;
	
	
	using std::string;
	
	std::vector<string> lines;
	size_t index = 0;
	
	
	void addLine(const char* line) {
		lines.push_back(line);
	}
	
	void addLine(string&& line) {
		lines.push_back(line);
	}
	
	char hexChar(uint64_t num) {
		num &= 0xF;
		return num >= 10 ? num + ('A' - 10) : num + '0';
	}
	
	
	
	void dump(const char* header, uint8_t* mem, size_t offset, size_t lineSize, size_t lines) {
		addLine("");
		addLine(header);
				
		for (size_t i = 0; i < lines; ++i) {
			
			string line;
			line.reserve(6 + lineSize * 3);
			
			size_t base = offset + i * lineSize;
			
			line += "0x";
			line += hexChar(base >> 12);
			line += hexChar(base >> 8);
			line += hexChar(base >> 4);
			line += hexChar(base);
			line += ':';
			
			for (size_t j = 0; j < lineSize; ++j) {
				uint8_t val = mem[base + j];
				
				line += ' ';
				line += hexChar(val >> 4);
				line += hexChar(val);
			}
			
			addLine(std::move(line));
		}
	}
	
	
	char EMPTY_LINE[MAX_LINE_LENGTH + 1] = {};
	
	bool initEmptyLine() {
		for (int i = 0; i < MAX_LINE_LENGTH; ++i) {
			EMPTY_LINE[i] = ' ';
		}
		
		return true;
	}
	
	
	void printLines() {
		{
			static bool unused = initEmptyLine();
			(void)unused;
		}
		
		size_t i = index;
		
		for (int y = 0, s = LINES - 1; y < s; y++, i++) {
			move(y, START_X);
			
			if (i < lines.size()) {
				addstr(lines[i].c_str());
				addstr(EMPTY_LINE + std::min(lines[i].size(), size_t(MAX_LINE_LENGTH)));
			} else {
				addstr(EMPTY_LINE);
			}
		}
		
		refresh();
	}
	
	uint64_t zeroIfNegative(uint64_t value) {
		return uint64_t(std::max(int64_t(value), int64_t(0)));
	}
	
	void scrollUp() {
		index = zeroIfNegative(index - 1);
		printLines();
	}
	
	void scrollDown() {
		uint64_t limit = zeroIfNegative(lines.size() - LINES + 1);
		index = std::min(index + 1, limit);
		printLines();
	}
}
