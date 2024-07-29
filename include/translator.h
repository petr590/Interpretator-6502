#ifndef INT6502_TRANSLATOR_H
#define INT6502_TRANSLATOR_H

#include <vector>
#include <cstdint>

namespace int6502 {
	// Транслирует код из файла в машинный код. Результат записывается в 
	// переменную code. Возвращает 0 в случае успеха, иначе код ошибки.
	extern int translate(const char* filename, std::vector<uint8_t>& code);
}

#endif /* INT6502_TRANSLATOR_H */
