#include "insn.h"
#include "error_codes.h"
#include "util.h"
#include <string>
#include <map>
#include <regex>
#include <cstring>
#include <cstdarg>
#include <cassert>

namespace int6502 {

	using std::string;
	using std::vector;
	using std::map;
	
	using std::regex;
	using std::smatch;
	using std::regex_match;

	
	
	inline int numberTooLargeError(int lineNum, const char* value) {
		return syntaxError(lineNum, "Number \"%s\" is too large", value);
	}
	
	inline int invalidNumberError(int lineNum, const char* value) {
		return syntaxError(lineNum, "Invalid number \"%s\"", value);
	}
	
	inline int addressingModeNotSupported(int lineNum, const char* insn) {
		return syntaxError(lineNum, "This addressing mode is not supported by \"%s\" instruction", insn);
	}
	
	inline int labelTooFar(int lineNum, const char* label) {
		return syntaxError(lineNum, "Label \"%s\" is too far", label);
	}
	
	inline int noOperandError(int lineNum) {
		return syntaxError(lineNum, "Expected operand");
	}
	
	
	static uint8_t EMPTY_VAR;
	#define VOID &EMPTY_VAR
	
	int parseInt(const char* str, int lineNum, uint16_t* res, uint8_t* size) {
		const char* const srcStr = str;
		
		int base = 10;
		
		if (str[0] == '$') {
			str += 1;
			base = 16;
			
			switch (strlen(str)) {
				case 0: return invalidNumberError(lineNum, srcStr);
				case 1: case 2: *size = 1; break;
				case 3: case 4: *size = 2; break;
				default: return numberTooLargeError(lineNum, srcStr);
			}
		}
		
		if (str[0] == '\0') {
			return invalidNumberError(lineNum, srcStr);
		}
		
		int num = 0;
		
		for (const char* s = str; *s != '\0'; s++) {
			int c = std::tolower(*s);
			int digit;
			
			if (c >= '0' && c <= '9') {
				digit = c - '0';
				
			} else if (base == 16 && c >= 'a' && c <= 'f') {
				digit = c - 'a' + 10;
				
			} else {
				return invalidNumberError(lineNum, srcStr);
			}
			
			num = num * base + digit;
		}
		
		
		if (uint16_t(num) != num) {
			return numberTooLargeError(lineNum, srcStr);
		}
		
		if (base == 10) {
			*size = num > 0xFF ? 2 : 1;
		}
		
		*res = uint16_t(num);
		return EXIT_SUCCESS;
	}
	
	
	// ------------------------------------------------------------------- Labels -------------------------------------------------------------------
		
		
	enum class AddrMode {
		REL, // one-byte signed address
		ABS, // two-byte unsigned address
	};
	
	struct RequiredLabel {
		size_t pos;
		AddrMode mode;
		int lineNum;
		string label;
		
		RequiredLabel(size_t pos, AddrMode mode, int lineNum, const string& label):
				pos(pos), mode(mode), lineNum(lineNum), label(label) {}
	};
	
	
	static vector<RequiredLabel> requiredLabels;
	
	
	void addRequiredLabel(AddrMode mode, int lineNum, const string& label, vector<uint8_t>& code) {
		requiredLabels.emplace_back(code.size(), mode, lineNum, label);
		
		code.push_back(0x00);
		
		if (mode == AddrMode::ABS)
			code.push_back(0x00);
	}
	
	void addRequiredLabel(AddrMode mode, int lineNum, const string& label, vector<uint8_t>& code, uint8_t opcode) {
		code.push_back(opcode);
		addRequiredLabel(mode, lineNum, label, code);
	}
	
	
	int initLabels(vector<uint8_t>& code, map<string, size_t>& labels) {
		for (const RequiredLabel& req : requiredLabels) {
			auto found = labels.find(req.label);
			
			if (found != labels.end()) {
				switch (req.mode) {
					case AddrMode::REL: {
						int16_t offset = int16_t(found->second - req.pos - 1);
				
						if (int8_t(offset) != offset) {
							return labelTooFar(req.lineNum, req.label.c_str());
						}
						
						code[req.pos] = uint8_t(offset);
						break;
					}
					
					case AddrMode::ABS: {
						size_t addr = CODE_POS + found->second;
						
						if (uint16_t(addr) != addr) {
							return labelTooFar(req.lineNum, req.label.c_str());
						}
						
						code[req.pos]   = uint8_t(addr);
						code[req.pos+1] = uint8_t(addr >> 8);
						break;
					}
					
					default:
						return error(INTERNAL_ERROR, "Illegal mode: %d", req.mode);
				}
				
			} else {
				return syntaxError(req.lineNum, "Label \"%s\" not found", req.label.c_str());
			}
		}
		
		return EXIT_SUCCESS;
	}
	
	
	
	// ---------------------------------------------------------------- Instructions ----------------------------------------------------------------	
	
	struct Insn {
		uint8_t opcode;
		uint8_t len;
		
		Insn(uint8_t opcode, uint8_t len):
				opcode(opcode), len(len) {}
		
		bool isNull() const {
			return opcode == NULL_OPR;
		}
	};
	
	
	// Типы адресации: IMM, ZP, ZP+X, ZP+Y, ABS, ABS+X, ABS+Y, IND X, IND Y
	int insn(
			const string& operation, const string& operand, const DefineTable& defineTable, int lineNum, vector<uint8_t>& code,
			uint8_t imm, uint8_t zp, uint8_t zpX, uint8_t zpY, uint8_t abs, uint8_t absX, uint8_t absY, uint8_t indX, uint8_t indY, uint8_t regA
	) {
		if (operand.empty()) {
			return noOperandError(lineNum);
		}
		
		const string& str = defineTable[operand];
		
		if (str == "a" || str == "A") {
			Insn insn(regA, 1);
			
			if (insn.isNull()) {
				return addressingModeNotSupported(lineNum, operation.c_str());
			}
			
			code.push_back(insn.opcode);
			return EXIT_SUCCESS;
		}
		
		
		#define WORD "([$\\w_]+)"
		#define SPACE "\\s*"
		static const regex immediateRegex ("#" WORD);
		static const regex commonRegex    (WORD "(?:" SPACE "," SPACE "([xyXY])" SPACE ")?");
		static const regex indirectXRegex ("\\(" SPACE WORD SPACE "," SPACE "[xX]" SPACE "\\)");
		static const regex indirectYRegex ("\\(" SPACE WORD SPACE "\\)" SPACE "," SPACE "[yY]");
		#undef SPACE
		#undef WORD
		
		Insn insn1(NULL_OPR, 0),
			 insn2(NULL_OPR, 0);
		
		string value;
		
		smatch match;
		
		if (regex_match(operand, match, immediateRegex)) {
			insn1 = Insn(imm, 2);
			value = match[1].str();
			
		} else if (regex_match(operand, match, commonRegex)) {
			value = match[1].str();
			string xy = match[2].str();
			tolower(xy);
			
			insn1 = Insn(xy == "x" ? zpX  : xy == "y" ? zpY  : zp,  2);
			insn2 = Insn(xy == "x" ? absX : xy == "y" ? absY : abs, 3);
		
		} else if (regex_match(operand, match, indirectXRegex)) {
			insn1 = Insn(indX, 2);
			value = match[1];
			
		} else if (regex_match(operand, match, indirectYRegex)) {
			insn1 = Insn(indY, 2);
			value = match[1];
			
		} else {
			return invalidSyntaxError(lineNum);
		}
		
		
		const string& defined = defineTable[value];
		
		if (isValidLabel(defined) && !insn2.isNull()) {
			addRequiredLabel(AddrMode::ABS, lineNum, defined, code, insn2.opcode);
			return EXIT_SUCCESS;
		}
		
		
		uint16_t num;
		uint8_t size;
		
		int res = parseInt(defined.c_str(), lineNum, &num, &size);
		if (res != EXIT_SUCCESS) return res;
		
		Insn& insn = size == 1 ? insn1 : insn2;
		
		if (insn.isNull()) {
			return addressingModeNotSupported(lineNum, operation.c_str());
		}
		
		
		code.push_back(insn.opcode);
		
		for (uint8_t i = 1; i < insn.len; i++, num >>= 8) {
			code.push_back(uint8_t(num));
		}
		
		return EXIT_SUCCESS;
	}
	
	
	
	int noOpsInsn(const string& operation, const string& operand, int lineNum, vector<uint8_t>& code, uint8_t opcode) {
		if (!operand.empty()) {
			return syntaxError(lineNum, "Too many operands for \"%s\" instruction", operation.c_str());
		}
		
		code.push_back(opcode);
		return EXIT_SUCCESS;
	}
	
	
	int labelInsn(const string& operand, const DefineTable& defineTable, int lineNum, vector<uint8_t>& code, uint8_t opcode, AddrMode mode) {
		if (operand.empty()) {
			return noOperandError(lineNum);
		}
		
		const string& label = defineTable[operand];
		
		if (!isValidLabel(label)) {
			return syntaxError(lineNum, "Invalid label name: \"%s\"", label.c_str());
		}
		
		addRequiredLabel(mode, lineNum, label, code, opcode);
		
		return EXIT_SUCCESS;
	}
	
	
	int jmpInsn(const string& rawOperand, const DefineTable& defineTable, int lineNum, vector<uint8_t>& code, uint8_t abs, uint8_t ind) {
		if (rawOperand.empty()) {
			return noOperandError(lineNum);
		}
		
		const string& operand = defineTable[rawOperand];
		
		if (isValidLabel(operand)) {
			addRequiredLabel(AddrMode::ABS, lineNum, operand, code, abs);
			
			return EXIT_SUCCESS;
		}
		
		string value;
		uint8_t opcode;
		
		size_t last = operand.size() - 1;
		
		if (operand[0] == '(' && operand[last] == ')') {
			value = operand.substr(1, last - 1);
			opcode = ind;
		} else {
			value = operand;
			opcode = abs;
		}
		
		uint16_t num;
		
		int res = parseInt(value.c_str(), lineNum, &num, VOID);
		if (res != EXIT_SUCCESS) return res;
		
		code.push_back(opcode);
		code.push_back(uint8_t(num));
		code.push_back(uint8_t(num >> 8));
		
		return EXIT_SUCCESS;
	}
	
	
	int dcbInsn(const string& operand, const DefineTable& defineTable, int lineNum, vector<uint8_t>& code) {
		if (operand.empty())
			return noOperandError(lineNum);
		
		
		for (size_t i = 0, next = 0; next != string::npos; i = next + 1) {
			next = operand.find(',', i);
			
			string val = operand.substr(i, next == string::npos ? string::npos : next - i);
			trim(val);
			
			const string& defined = defineTable[val];
			
			if (isValidLabel(defined)) {
				addRequiredLabel(AddrMode::ABS, lineNum, defined, code);
				continue;
			}
			
			uint16_t num;
			uint8_t size;
			
			int res = parseInt(defined.c_str(), lineNum, &num, &size);
			if (res != EXIT_SUCCESS)
				return res;
			
			code.push_back(uint8_t(num));
			
			if (size == 2)
				code.push_back(uint8_t(num >> 8));
		}
		
		return EXIT_SUCCESS;
	}
	
	
	int defineInsn(const string& operand, DefineTable& defineTable, int lineNum) {
		if (operand.empty())
			return noOperandError(lineNum);
		
		
		static const regex rgx("([a-zA-Z_][\\w_]*)\\s+([$\\w_]+)");
		
		smatch match;
		
		if (regex_match(operand, match, rgx)) {
			defineTable[match[1].str()] = match[2].str();
			return EXIT_SUCCESS;
			
		} else {
			return invalidSyntaxError(lineNum);
		}
	}
	
	
	
	static InsnFunction getInsnFunction(
			uint8_t imm,
			uint8_t zp  = NULL_OPR, uint8_t zpX  = NULL_OPR, uint8_t zpY  = NULL_OPR,
			uint8_t abs = NULL_OPR, uint8_t absX = NULL_OPR, uint8_t absY = NULL_OPR,
			uint8_t indX = NULL_OPR, uint8_t indY = NULL_OPR,
			uint8_t regA = NULL_OPR
	) {
		return [=] (const string& operation, const string& operand, DefineTable& defineTable, int lineNum, auto& code) {
			return insn(operation, operand, defineTable, lineNum, code, imm, zp, zpX, zpY, abs, absX, absY, indX, indY, regA);
		};
	}
	
	static InsnFunction getRegAInsnFunction(uint8_t regA, uint8_t zp, uint8_t zpX, uint8_t abs, uint8_t absX) {
		
		return [=] (const auto& operation, const auto& operand, DefineTable& defineTable, int lineNum, auto& code) {
			return insn(operation, operand, defineTable, lineNum, code, NULL_OPR, zp, zpX, NULL_OPR, abs, absX, NULL_OPR, NULL_OPR, NULL_OPR, regA);
		};
	}
	
	static InsnFunction getNoOpsInsnFunction(uint8_t opcode) {
		return [=] (const auto& operation, const auto& operand, DefineTable&, int lineNum, auto& code) {
			return noOpsInsn(operation, operand, lineNum, code, opcode);
		};
	}
	
	static InsnFunction getLabelInsnFunction(uint8_t opcode, AddrMode mode) {
		return [=] (const auto&, const auto& operand, DefineTable& defineTable, int lineNum, auto& code) {
			return labelInsn(operand, defineTable, lineNum, code, opcode, mode);
		};
	}
	
	static InsnFunction getJmpInsnFunction(uint8_t abs, uint8_t ind) {
		return [=] (const auto&, const auto& operand, DefineTable& defineTable, int lineNum, auto& code) {
			return jmpInsn(operand, defineTable, lineNum, code, abs, ind);
		};
	}
	
	static InsnFunction getDcbInsnFunction() {
		return [=] (const auto&, const auto& operand, DefineTable& defineTable, int lineNum, auto& code) {
			return dcbInsn(operand, defineTable, lineNum, code);
		};
	}
	
	static InsnFunction getDefineInsnFunction() {
		return [=] (const auto&, const auto& operand, DefineTable& defineTable, int lineNum, auto&) {
			return defineInsn(operand, defineTable, lineNum);
		};
	}
	
	
	map<string, InsnFunction> createInsnTable() {
		map<string, InsnFunction> table;
		
		table["lda"] = getInsnFunction(LDA_IMM,   LDA_ZP, LDA_ZP_X, NULL_OPR,  LDA_ABS, LDA_ABS_X, LDA_ABS_Y,  LDA_IND_X, LDA_IND_Y);
		table["sta"] = getInsnFunction(NULL_OPR,  STA_ZP, STA_ZP_X, NULL_OPR,  STA_ABS, STA_ABS_X, STA_ABS_Y,  STA_IND_X, STA_IND_Y);
		table["cmp"] = getInsnFunction(CMP_IMM,   CMP_ZP, CMP_ZP_X, NULL_OPR,  CMP_ABS, CMP_ABS_X, CMP_ABS_Y,  CMP_IND_X, CMP_IND_Y);
		table["and"] = getInsnFunction(AND_IMM,   AND_ZP, AND_ZP_X, NULL_OPR,  AND_ABS, AND_ABS_X, AND_ABS_Y,  AND_IND_X, AND_IND_Y);
		table["ora"] = getInsnFunction(ORA_IMM,   ORA_ZP, ORA_ZP_X, NULL_OPR,  ORA_ABS, ORA_ABS_X, ORA_ABS_Y,  ORA_IND_X, ORA_IND_Y);
		table["eor"] = getInsnFunction(EOR_IMM,   EOR_ZP, EOR_ZP_X, NULL_OPR,  EOR_ABS, EOR_ABS_X, EOR_ABS_Y,  EOR_IND_X, EOR_IND_Y);
		table["adc"] = getInsnFunction(ADC_IMM,   ADC_ZP, ADC_ZP_X, NULL_OPR,  ADC_ABS, ADC_ABS_X, ADC_ABS_Y,  ADC_IND_X, ADC_IND_Y);
		table["sbc"] = getInsnFunction(SBC_IMM,   SBC_ZP, SBC_ZP_X, NULL_OPR,  SBC_ABS, SBC_ABS_X, SBC_ABS_Y,  SBC_IND_X, SBC_IND_Y);
		
		table["ldx"] = getInsnFunction(LDX_IMM,   LDX_ZP, NULL_OPR, LDX_ZP_Y,  LDX_ABS, NULL_OPR, LDX_ABS_Y);
		table["ldy"] = getInsnFunction(LDY_IMM,   LDY_ZP, LDY_ZP_X, NULL_OPR,  LDY_ABS, LDY_ABS_X, NULL_OPR);
		
		table["stx"] = getInsnFunction(NULL_OPR,  STX_ZP, NULL_OPR, STX_ZP_Y,  STX_ABS);
		table["sty"] = getInsnFunction(NULL_OPR,  STY_ZP, STY_ZP_X, NULL_OPR,  STY_ABS);
		
		table["cpx"] = getInsnFunction(CPX_IMM,   CPX_ZP, NULL_OPR,  NULL_OPR, CPX_ABS);
		table["cpy"] = getInsnFunction(CPY_IMM,   CPY_ZP, NULL_OPR,  NULL_OPR, CPY_ABS);
		table["bit"] = getInsnFunction(NULL_OPR,  BIT_ZP, NULL_OPR,  NULL_OPR, BIT_ABS);
		
		table["asl"] = getRegAInsnFunction(ASL_A,     ASL_ZP, ASL_ZP_X,  ASL_ABS, ASL_ABS_X);
		table["lsr"] = getRegAInsnFunction(LSR_A,     LSR_ZP, LSR_ZP_X,  LSR_ABS, LSR_ABS_X);
		table["rol"] = getRegAInsnFunction(ROL_A,     ROL_ZP, ROL_ZP_X,  ROL_ABS, ROL_ABS_X);
		table["ror"] = getRegAInsnFunction(ROR_A,     ROR_ZP, ROR_ZP_X,  ROR_ABS, ROR_ABS_X);
		table["inc"] = getRegAInsnFunction(NULL_OPR,  INC_ZP, INC_ZP_X,  INC_ABS, INC_ABS_X);
		table["dec"] = getRegAInsnFunction(NULL_OPR,  DEC_ZP, DEC_ZP_X,  DEC_ABS, DEC_ABS_X);
		
		table["inx"] = getNoOpsInsnFunction(INX);
		table["iny"] = getNoOpsInsnFunction(INY);
		table["dex"] = getNoOpsInsnFunction(DEX);
		table["dey"] = getNoOpsInsnFunction(DEY);
		table["clc"] = getNoOpsInsnFunction(CLC);
		table["cli"] = getNoOpsInsnFunction(CLI);
		table["cld"] = getNoOpsInsnFunction(CLD);
		table["clv"] = getNoOpsInsnFunction(CLV);
		table["sec"] = getNoOpsInsnFunction(SEC);
		table["sei"] = getNoOpsInsnFunction(SEI);
		table["sed"] = getNoOpsInsnFunction(SED);
		table["tax"] = getNoOpsInsnFunction(TAX);
		table["txa"] = getNoOpsInsnFunction(TXA);
		table["tay"] = getNoOpsInsnFunction(TAY);
		table["tya"] = getNoOpsInsnFunction(TYA);
		table["tsx"] = getNoOpsInsnFunction(TSX);
		table["txs"] = getNoOpsInsnFunction(TXS);
		table["pha"] = getNoOpsInsnFunction(PHA);
		table["pla"] = getNoOpsInsnFunction(PLA);
		table["php"] = getNoOpsInsnFunction(PHP);
		table["plp"] = getNoOpsInsnFunction(PLP);
		table["nop"] = getNoOpsInsnFunction(NOP);
		
		table["beq"] = getLabelInsnFunction(BEQ, AddrMode::REL);
		table["bne"] = getLabelInsnFunction(BNE, AddrMode::REL);
		table["bmi"] = getLabelInsnFunction(BMI, AddrMode::REL);
		table["bpl"] = getLabelInsnFunction(BPL, AddrMode::REL);
		table["bcs"] = getLabelInsnFunction(BCS, AddrMode::REL);
		table["bcc"] = getLabelInsnFunction(BCC, AddrMode::REL);
		table["bvs"] = getLabelInsnFunction(BVS, AddrMode::REL);
		table["bvc"] = getLabelInsnFunction(BVC, AddrMode::REL);
		
		table["jmp"] = getJmpInsnFunction(JMP_ABS, JMP_IND);
		
		table["jsr"] = getLabelInsnFunction(JSR, AddrMode::ABS);
		table["rts"] = getNoOpsInsnFunction(RTS);
		
		table["brk"] = getNoOpsInsnFunction(BRK);
		table["rti"] = getNoOpsInsnFunction(RTI);
		
		table["dcb"] = getDcbInsnFunction();
		table["define"] = getDefineInsnFunction();
		
		return table;
	}
}
