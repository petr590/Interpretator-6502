#ifndef INT6502_ERROR_CODES_H
#define INT6502_ERROR_CODES_H

namespace int6502 {
	static const int
			ARGUMENTS_ERROR           = 1,
			INVALID_SYNTAX_ERROR      = 2,
			COLOR_NOT_SUPPORTED_ERROR = 3,
			OPEN_FILE_ERROR           = 4,
			INTERNAL_ERROR            = 5,
			UNKNOWN_INSTRUCTION_ERROR = 6;
}

#endif /* INT6502_ERROR_CODES_H */
