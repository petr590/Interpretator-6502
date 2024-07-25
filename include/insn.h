#ifndef JIT6502_INSN_H
#define JIT6502_INSN_H

#include <functional>
#include <string>
#include <vector>
#include <map>

namespace jit6502 {
	
	static const uint16_t
			STACK_POS = 0x100,
			GPU_POS   = 0x200,
			CODE_POS  = 0x600,
			RND_POS   = 0xFE,
			INPUT_POS = 0xFF;

	enum Opcode {
		NULL_OPR  = 0x00,
		
		BRK       = 0x00,
		BPL       = 0x10,
		JSR       = 0x20,
		BMI       = 0x30,
		RTI       = 0x40,
		BVC       = 0x50,
		RTS       = 0x60,
		BVS       = 0x70,
		////////////////
		BCC       = 0x90,
		LDY_IMM   = 0xA0,
		BCS       = 0xB0,
		CPY_IMM   = 0xC0,
		BNE       = 0xD0,
		CPX_IMM   = 0xE0,
		BEQ       = 0xF0,
		
		ORA_IND_X = 0x01,
		ORA_IND_Y = 0x11,
		AND_IND_X = 0x21,
		AND_IND_Y = 0x31,
		EOR_IND_X = 0x41,
		EOR_IND_Y = 0x51,
		ADC_IND_X = 0x61,
		ADC_IND_Y = 0x71,
		STA_IND_X = 0x81,
		STA_IND_Y = 0x91,
		LDA_IND_X = 0xA1,
		LDA_IND_Y = 0xB1,
		CMP_IND_X = 0xC1,
		CMP_IND_Y = 0xD1,
		SBC_IND_X = 0xE1,
		SBC_IND_Y = 0xF1,
		
		LDX_IMM   = 0xA2,
		
		BIT_ZP    = 0x24,
		STY_ZP    = 0x84,
		STY_ZP_X  = 0x94,
		LDY_ZP    = 0xA4,
		LDY_ZP_X  = 0xB4,
		CPY_ZP    = 0xC4,
		CPX_ZP    = 0xE4,
		
		ORA_ZP    = 0x05,
		ORA_ZP_X  = 0x15,
		AND_ZP    = 0x25,
		AND_ZP_X  = 0x35,
		EOR_ZP    = 0x45,
		EOR_ZP_X  = 0x55,
		ADC_ZP    = 0x65,
		ADC_ZP_X  = 0x75,
		STA_ZP    = 0x85,
		STA_ZP_X  = 0x95,
		LDA_ZP    = 0xA5,
		LDA_ZP_X  = 0xB5,
		CMP_ZP    = 0xC5,
		CMP_ZP_X  = 0xD5,
		SBC_ZP    = 0xE5,
		SBC_ZP_X  = 0xF5,
		
		ASL_ZP    = 0x06,
		ASL_ZP_X  = 0x16,
		ROL_ZP    = 0x26,
		ROL_ZP_X  = 0x36,
		LSR_ZP    = 0x46,
		LSR_ZP_X  = 0x56,
		ROR_ZP    = 0x66,
		ROR_ZP_X  = 0x76,
		STX_ZP    = 0x86,
		STX_ZP_Y  = 0x96,
		LDX_ZP    = 0xA6,
		LDX_ZP_Y  = 0xB6,
		DEC_ZP    = 0xC6,
		DEC_ZP_X  = 0xD6,
		INC_ZP    = 0xE6,
		INC_ZP_X  = 0xF6,
		
		PHP       = 0x08,
		CLC       = 0x18,
		PLP       = 0x28,
		SEC       = 0x38,
		PHA       = 0x48,
		CLI       = 0x58,
		PLA       = 0x68,
		SEI       = 0x78,
		DEY       = 0x88,
		TYA       = 0x98,
		TAY       = 0xA8,
		CLV       = 0xB8,
		INY       = 0xC8,
		CLD       = 0xD8,
		INX       = 0xE8,
		SED       = 0xF8,
		
		ORA_IMM   = 0x09,
		ORA_ABS_Y = 0x19,
		AND_IMM   = 0x29,
		AND_ABS_Y = 0x39,
		EOR_IMM   = 0x49,
		EOR_ABS_Y = 0x59,
		ADC_IMM   = 0x69,
		ADC_ABS_Y = 0x79,
		////////////////
		STA_ABS_Y = 0x99,
		LDA_IMM   = 0xA9,
		LDA_ABS_Y = 0xB9,
		CMP_IMM   = 0xC9,
		CMP_ABS_Y = 0xD9,
		SBC_IMM   = 0xE9,
		SBC_ABS_Y = 0xF9,
		
		
		ASL_A     = 0x0A,
		ROL_A     = 0x2A,
		LSR_A     = 0x4A,
		ROR_A     = 0x6A,
		
		TXA       = 0x8A,
		TXS       = 0x9A,
		TAX       = 0xAA,
		TSX       = 0xBA,
		DEX       = 0xCA,
		
		NOP       = 0xEA,
		
		
		BIT_ABS   = 0x2C,
		JMP_ABS   = 0x4C,
		JMP_IND   = 0x6C,
		STY_ABS   = 0x8C,
		LDY_ABS   = 0xAC,
		LDY_ABS_X = 0xBC,
		CPY_ABS   = 0xCC,
		CPX_ABS   = 0xEC,
		
		
		ORA_ABS   = 0x0D,
		ORA_ABS_X = 0x1D,
		AND_ABS   = 0x2D,
		AND_ABS_X = 0x3D,
		EOR_ABS   = 0x4D,
		EOR_ABS_X = 0x5D,
		ADC_ABS   = 0x6D,
		ADC_ABS_X = 0x7D,
		STA_ABS   = 0x8D,
		STA_ABS_X = 0x9D,
		LDA_ABS   = 0xAD,
		LDA_ABS_X = 0xBD,
		CMP_ABS   = 0xCD,
		CMP_ABS_X = 0xDD,
		SBC_ABS   = 0xED,
		SBC_ABS_X = 0xFD,
		
		ASL_ABS   = 0x0E,
		ASL_ABS_X = 0x1E,
		ROL_ABS   = 0x2E,
		ROL_ABS_X = 0x3E,
		LSR_ABS   = 0x4E,
		LSR_ABS_X = 0x5E,
		ROR_ABS   = 0x6E,
		ROR_ABS_X = 0x7E,
		STX_ABS   = 0x8E,
		////////////////
		LDX_ABS   = 0xAE,
		LDX_ABS_Y = 0xBE,
		DEC_ABS   = 0xCE,
		DEC_ABS_X = 0xDE,
		INC_ABS   = 0xEE,
		INC_ABS_X = 0xFE,
	};
	
	
	// Таблица define-ов
	class DefineTable {
	public:
		std::map<std::string, std::string> table;
		
	public:
		DefineTable() {}
		
		DefineTable(const DefineTable&) = delete;
		
		// Возвращает значение по ключу или сам ключ
		inline const std::string& operator[](const std::string& key) const {
			auto found = table.find(key);
			return found != table.end() ? found->second : key;
		}
		
		inline std::string& operator[](const std::string& key) {
			std::string& val = table[key];
			
			if (val.empty()) {
				val = key;
			}
			
			return val;
		}
	};
	
	
	// Параметры:
	// - Название операции
	// - Оставшиеся операнды
	// - Таблица define-ов
	// - Номер строки (начиная с 1)
	// - Результирующий код
	// Возвращает EXIT_SUCCESS, если всё норм, иначе код ошибки.
	using InsnFunction = std::function<int(const std::string&, const std::string&, DefineTable&, int, std::vector<uint8_t>&)>;
	
	// Возвращает карту, где ключ - название инструкции, значение - функция этой инструкции
	extern std::map<std::string, InsnFunction> createInsnTable();
	
	// Инициализирует лейблы
	extern int initLabels(std::vector<uint8_t>& code, std::map<std::string, size_t>& labels);
}

#endif /* JIT6502_INSN_H */
