#ifndef JIT6502_EXECUTOR_H
#define JIT6502_EXECUTOR_H

#include <vector>
#include <cstdint>

namespace jit6502 {
	// Выполняет переданный код. Возвращает 0 в случае успеха, иначе код ошибки.
	int executeCode(const std::vector<uint8_t>& code);
}

#endif /* JIT6502_EXECUTOR_H */
