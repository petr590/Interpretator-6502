#ifndef INT6502_EXECUTOR_H
#define INT6502_EXECUTOR_H

#include <vector>
#include <cstdint>

namespace int6502 {
	// Выполняет переданный код. Возвращает 0 в случае успеха, иначе код ошибки.
	int executeCode(const std::vector<uint8_t>& code);
}

#endif /* INT6502_EXECUTOR_H */
