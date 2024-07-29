#ifndef INT6502_SCROLL_H
#define INT6502_SCROLL_H

#include <string>

namespace int6502 {
	static const int MAX_LINE_LENGTH = 56;
	
	// Добавляет строку в конец страницы
	extern void addLine(const char* line);
	
	// Добавляет строку в конец страницы
	extern void addLine(std::string&& line);
	
	// Добавляет отформатированную строку в конец страницы
	template<typename... Args>
	void addLine(size_t bufSize, const char* fmt, Args... args) {
		std::string buffer(bufSize, '\0');
		
		int n = snprintf(&buffer[0], bufSize + 1, fmt, args...);
		
		if (n >= 0 && static_cast<size_t>(n) <= bufSize) {
			addLine(std::move(buffer));
		} else {
			addLine("<error>");
		}
	}
	
	
	// Добавляет дамп указанной памяти конец страницы
	extern void dump(const char* header, uint8_t* mem, size_t offset, size_t lineSize, size_t lines);
	
	// Выводит все видимые строки на экран
	extern void printLines();
	
	// Прокручивает страницу вверх
	extern void scrollUp();
	
	// Прокручивает страницу вниз
	extern void scrollDown();
}

#endif /* INT6502_SCROLL_H */
