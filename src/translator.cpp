#include "translator.h"
#include "error_codes.h"
#include "util.h"
#include "insn.h"
#include <fstream>
#include <string>
#include <vector>

namespace int6502 {
	using std::string;
	using std::vector;
	using std::map;
	
	
	int processLine(string line, const map<string, InsnFunction>& insnTable,
	                map<string, size_t>& labels, DefineTable& defineTable,
	                int lineNum, vector<uint8_t>& code) {
		
		size_t index = line.find(';');
		
		if (index != string::npos)
			line = line.substr(0, index);
		
		
		index = line.find(':');
		
		if (index != string::npos) {
			string label = line.substr(0, index);
			line = line.substr(index + 1);
			
			trim(label);
			
			if (!std::all_of(label.cbegin(), label.cend(), [] (char c) { return std::isalpha(c) || std::isdigit(c) || c == '_'; })) {
				return invalidSyntaxError(lineNum);
			}
			
			labels[std::move(label)] = code.size();
		}
		
		trim(line);
		
		
		string operation, operand;
		
		index = line.find_first_of(" \v\f\t\r");
		
		if (index != string::npos) {
			operation = line.substr(0, index);
			operand = line.substr(index);
			ltrim(operand);
		} else {
			operation = std::move(line);
		}
		
		if (operation.empty() && operand.empty())
			return EXIT_SUCCESS;
		
		tolower(operation);
		
		
		auto found = insnTable.find(operation);
		
		if (found == insnTable.end()) {
			return syntaxError(lineNum, "Unknown instruction \"%s\"", operation.c_str());
		}
		
		return found->second(operation, operand, defineTable, lineNum, code);
	}
	
	
	int translate(const char* filename, vector<uint8_t>& code) {
		std::ifstream file(filename);
		
		if (!file.good()) {
			return error(OPEN_FILE_ERROR, "Cannot open file \"%s\"", filename);
		}
		
		static const map<string, InsnFunction> insnTable = createInsnTable();
		
		map<string, size_t> labels;
		DefineTable defineTable;
		int lineNum = 1;
		
		for (string line; std::getline(file, line); ++lineNum) {
			if (line.empty()) continue;
			
			int res = processLine(line, insnTable, labels, defineTable, lineNum, code);
			if (res != EXIT_SUCCESS) return res;
		}
		
		return initLabels(code, labels);
	}
}
