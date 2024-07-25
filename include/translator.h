#ifndef JIT6502_TRANSLATOR_H
#define JIT6502_TRANSLATOR_H

#include <vector>
#include <cstdint>

namespace jit6502 {
	// Транслирует код из файла в машинный код. Результат записывается в 
	// переменную code. Возвращает 0 в случае успеха, иначе код ошибки.
	extern int translate(const char* filename, std::vector<uint8_t>& code);
}

#endif /* JIT6502_TRANSLATOR_H */
