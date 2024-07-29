#ifndef INT6502_UTIL_H
#define INT6502_UTIL_H

#include <cstdarg>
#include <string>
#include <algorithm>

namespace int6502 {
	
	inline void ltrim(std::string& s) {
    	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [] (char ch) { return !std::isspace(ch); }));
	}
	
	inline void rtrim(std::string& s) {
		s.erase(std::find_if(s.rbegin(), s.rend(), [] (char ch) { return !std::isspace(ch); }).base(), s.end());
	}
	
	inline void trim(std::string& s) {
		rtrim(s);
		ltrim(s);
	}
	
	inline void tolower(std::string& str) {
		std::transform(str.begin(), str.end(), str.begin(), [] (char c) { return std::tolower(c); });
	}
	
	// Выводит форматированное сообщение об ошибке в консоль и возвращает code.
	// В конце выводит "\r\n".
	inline int error(int code, const char* fmt, ...) {
		va_list args;
		va_start(args, fmt);
		va_end(args);
		
		vfprintf(stderr, fmt, args);
		fprintf(stderr, "\r\n");
		return code;
	}
	
	// Выводит форматированное сообщение об ошибке в консоль и возвращает INVALID_SYNTAX_ERROR.
	// Автоматически выводит номер строки в начале и перенос в конце.
	inline int syntaxError(int lineNum, const char* fmt, ...) {
		va_list args;
		va_start(args, fmt);
		va_end(args);
		
		fprintf(stderr, "Error at line %d: ", lineNum);
		vfprintf(stderr, fmt, args);
		fprintf(stderr, "\r\n");
		return INVALID_SYNTAX_ERROR;
	}
	
	
	inline int invalidSyntaxError(int lineNum) {
		return syntaxError(lineNum, "Invalid syntax");
	}
	
	
	inline bool isValidLabel(const std::string& label) {
		return !label.empty() &&
				(std::isalpha(label[0]) || label[0] == '_') &&
				std::all_of(label.cbegin() + 1, label.cend(), [] (char c) { return std::isalpha(c) || std::isdigit(c) || c == '_'; });
	}
}

#endif /* INT6502_UTIL_H */
