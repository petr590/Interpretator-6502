#include "executor.h"
#include "insn.h"
#include "drawer.h"
#include "scroll.h"
#include "error_codes.h"
#include <cstring>
#include <vector>
#include <thread>
#include <ncurses.h>

namespace int6502 {
	
	
	using std::vector;
	
	static const size_t MEM_SIZE = 0x10000;
	
	static uint8_t SIZES[0x100] = {};
	
	static bool initSizes() {
		SIZES[LDA_IMM]   = 2;
		SIZES[LDA_ZP]    = 2;
		SIZES[LDA_ZP_X]  = 2;
		SIZES[LDA_ABS]   = 3;
		SIZES[LDA_ABS_X] = 3;
		SIZES[LDA_ABS_Y] = 3;
		SIZES[LDA_IND_X] = 2;
		SIZES[LDA_IND_Y] = 2;
		
		SIZES[LDX_IMM]   = 2;
		SIZES[LDX_ZP]    = 2;
		SIZES[LDX_ZP_Y]  = 2;
		SIZES[LDX_ABS]   = 3;
		SIZES[LDX_ABS_Y] = 3;
		
		SIZES[LDY_IMM]   = 2;
		SIZES[LDY_ZP]    = 2;
		SIZES[LDY_ZP_X]  = 2;
		SIZES[LDY_ABS]   = 3;
		SIZES[LDY_ABS_X] = 3;
		
		SIZES[STA_ZP]    = 2;
		SIZES[STA_ZP_X]  = 2;
		SIZES[STA_ABS]   = 3;
		SIZES[STA_ABS_X] = 3;
		SIZES[STA_ABS_Y] = 3;
		SIZES[STA_IND_X] = 2;
		SIZES[STA_IND_Y] = 2;
		
		SIZES[STX_ZP]    = 2;
		SIZES[STX_ZP_Y]  = 2;
		SIZES[STX_ABS]   = 3;
		
		SIZES[STY_ZP]    = 2;
		SIZES[STY_ZP_X]  = 2;
		SIZES[STY_ABS]   = 3;
		
		SIZES[CMP_IMM]   = 2;
		SIZES[CMP_ZP]    = 2;
		SIZES[CMP_ZP_X]  = 2;
		SIZES[CMP_ABS]   = 3;
		SIZES[CMP_ABS_X] = 3;
		SIZES[CMP_ABS_Y] = 3;
		SIZES[CMP_IND_X] = 2;
		SIZES[CMP_IND_Y] = 2;
		
		SIZES[CPX_IMM]   = 2;
		SIZES[CPX_ZP]    = 2;
		SIZES[CPX_ABS]   = 3;
		
		SIZES[CPY_IMM]   = 2;
		SIZES[CPY_ZP]    = 2;
		SIZES[CPY_ABS]   = 3;
		
		SIZES[BIT_ZP]    = 2;
		SIZES[BIT_ABS]   = 3;
		
		SIZES[AND_IMM]   = 2;
		SIZES[AND_ZP]    = 2;
		SIZES[AND_ZP_X]  = 2;
		SIZES[AND_ABS]   = 3;
		SIZES[AND_ABS_X] = 3;
		SIZES[AND_ABS_Y] = 3;
		SIZES[AND_IND_X] = 2;
		SIZES[AND_IND_Y] = 2;

		SIZES[ORA_IMM]   = 2;
		SIZES[ORA_ZP]    = 2;
		SIZES[ORA_ZP_X]  = 2;
		SIZES[ORA_ABS]   = 3;
		SIZES[ORA_ABS_X] = 3;
		SIZES[ORA_ABS_Y] = 3;
		SIZES[ORA_IND_X] = 2;
		SIZES[ORA_IND_Y] = 2;
		
		SIZES[EOR_IMM]   = 2;
		SIZES[EOR_ZP]    = 2;
		SIZES[EOR_ZP_X]  = 2;
		SIZES[EOR_ABS]   = 3;
		SIZES[EOR_ABS_X] = 3;
		SIZES[EOR_ABS_Y] = 3;
		SIZES[EOR_IND_X] = 2;
		SIZES[EOR_IND_Y] = 2;

		SIZES[ADC_IMM]   = 2;		
		SIZES[ADC_ZP]    = 2;
		SIZES[ADC_ZP_X]  = 2;
		SIZES[ADC_ABS]   = 3;
		SIZES[ADC_ABS_X] = 3;
		SIZES[ADC_ABS_Y] = 3;
		SIZES[ADC_IND_X] = 2;
		SIZES[ADC_IND_Y] = 2;
		
		SIZES[SBC_IMM]   = 2;
		SIZES[SBC_ZP]    = 2;
		SIZES[SBC_ZP_X]  = 2;
		SIZES[SBC_ABS]   = 3;
		SIZES[SBC_ABS_X] = 3;
		SIZES[SBC_ABS_Y] = 3;
		SIZES[SBC_IND_X] = 2;
		SIZES[SBC_IND_Y] = 2;

		SIZES[ASL_A]     = 1;
		SIZES[ASL_ZP]    = 2;
		SIZES[ASL_ZP_X]  = 2;
		SIZES[ASL_ABS]   = 3;
		SIZES[ASL_ABS_X] = 3;
		
		SIZES[LSR_A]     = 1;
		SIZES[LSR_ZP]    = 2;
		SIZES[LSR_ZP_X]  = 2;
		SIZES[LSR_ABS]   = 3;
		SIZES[LSR_ABS_X] = 3;
		
		SIZES[ROL_A]     = 1;
		SIZES[ROL_ZP]    = 2;
		SIZES[ROL_ZP_X]  = 2;
		SIZES[ROL_ABS]   = 3;
		SIZES[ROL_ABS_X] = 3;
		
		SIZES[ROR_A]     = 1;
		SIZES[ROR_ZP]    = 2;
		SIZES[ROR_ZP_X]  = 2;
		SIZES[ROR_ABS]   = 3;
		SIZES[ROR_ABS_X] = 3;
		
		SIZES[INC_ZP]    = 2;
		SIZES[INC_ZP_X]  = 2;
		SIZES[INC_ABS]   = 3;
		SIZES[INC_ABS_X] = 3;
		
		SIZES[DEC_ZP]    = 2;
		SIZES[DEC_ZP_X]  = 2;
		SIZES[DEC_ABS]   = 3;
		SIZES[DEC_ABS_X] = 3;
		
		SIZES[INX]       = 1;
		SIZES[INY]       = 1;
		SIZES[DEX]       = 1;
		SIZES[DEY]       = 1;

		SIZES[CLC]       = 1;
		SIZES[CLI]       = 1;
		SIZES[CLD]       = 1;
		SIZES[CLV]       = 1;
		SIZES[SEC]       = 1;
		SIZES[SEI]       = 1;
		SIZES[SED]       = 1;
		
		SIZES[TAX]       = 1;
		SIZES[TXA]       = 1;
		SIZES[TAY]       = 1;
		SIZES[TYA]       = 1;
		SIZES[TSX]       = 1;
		SIZES[TXS]       = 1;
		
		SIZES[PHA]       = 1;
		SIZES[PHP]       = 1;
		SIZES[PLA]       = 1;
		SIZES[PLP]       = 1;

		SIZES[NOP]       = 1;
		
		SIZES[BEQ]       = 2;
		SIZES[BNE]       = 2;
		SIZES[BMI]       = 2;
		SIZES[BPL]       = 2;
		SIZES[BCS]       = 2;
		SIZES[BCC]       = 2;
		SIZES[BVS]       = 2;
		SIZES[BVC]       = 2;

		SIZES[JMP_ABS]   = 3;
		SIZES[JMP_IND]   = 2;
		
		SIZES[JSR]       = 3;
		SIZES[RTS]       = 1;
		
		SIZES[BRK]       = 1;
		SIZES[RTI]       = 1;
		
		return true;
	}
	
	
	struct processor_state {
		uint8_t a, x, y, sp;
		uint16_t pc;
		uint8_t flags;
	};
	
	// mem - память, аллоцированная для ассемблера
	// state - указатель на итоговое состояние процессора
	int run(uint8_t* mem, processor_state* state) {
		{
			static bool unused = initSizes();
			(void)unused;
		}
			
		srand(time(NULL));
		
		uint8_t a = 0, x = 0, y = 0, sp = 0xff;
		uint16_t pc = CODE_POS;
		
		bool N = 0, // sign
			 V = 0, // overflow
			 B = 0, // break
			 D = 0, // BCD mode
			 I = 0, // no interrupt
			 Z = 0, // zero
			 C = 0; // carry
		
		while (!B) {
			mem[RND_POS] = uint8_t(rand());
			
			
			#define imm mem[pc+1]
			#define get16(addr, off) uint16_t((mem[addr] | (mem[addr+1] << 8)) + off)
			
			#define zp   mem[imm]
			#define zpX  mem[uint8_t(imm+x)]
			#define zpY  mem[uint8_t(imm+y)]
			#define abs  mem[get16(pc+1,0)]
			#define absX mem[get16(pc+1,x)]
			#define absY mem[get16(pc+1,y)]
			#define ind  (u8 = uint8_t(imm),   mem[get16(u8,0)])
			#define indX (u8 = uint8_t(imm+x), mem[get16(u8,0)])
			#define indY (u8 = uint8_t(imm),   mem[get16(u8,y)])
			
			
			#define setN(val) (N = int8_t(val) < 0)
			#define setZ(val) (Z = uint8_t(val) == 0)
			#define setC(val, inv) (C = bool(val & 0x100) ^ bool(inv))
			#define setV(op1, op2, val) (V =\
					(int16_t(op1) & 0x8000) == (int16_t(op2) & 0x8000) &&\
					(int16_t(op1) & 0x8000) != (int16_t(val) & 0x8000))
			
			#define setNZ(val) (setN(val), setZ(val))
			
			#define setNZVC(op1, op2, val, invC) (setN(val), setZ(val), setC(val, invC), setV(op1, op2, val))
			
			
			#define LOAD(reg, val) reg = val; setNZ(reg);
			#define CMP(reg, val) s16 = int16_t(reg - val); setNZ(s16); C = s16 >= 0;
			#define AND(val) a &= val; setNZ(a);
			#define ORA(val) a |= val; setNZ(a);
			#define EOR(val) a ^= val; setNZ(a);
			#define ADC(val) u8 = val; u16 = uint16_t(a) + uint16_t(u8) +  C; setNZVC(a, +int16_t(u8), u16, 0); a = uint8_t(u16);
			#define SBC(val) u8 = val; s16 =  int16_t(a) -  int16_t(u8) - !C; setNZVC(a, -int16_t(u8), s16, 1); a = uint8_t(s16);
			
			#define ASL(val) p8 = &val; u8 = *p8; C = u8 & 0x80; *p8 = u8 <<= 1; setNZ(u8);
			#define LSR(val) p8 = &val; u8 = *p8; C = u8 & 0x01; *p8 = u8 >>= 1; setNZ(u8);
			
			#define ROL(val) p8 = &val; u16 = uint16_t(*p8 << 1 | C); C = u16 & 0x100; *p8 = uint8_t(u16);       setNZ(u16);
			#define ROR(val) p8 = &val; u16 = uint16_t(*p8 | C << 8); C = u16 & 0x001; *p8 = uint8_t(u16 >>= 1); setNZ(u16);
			
			#define INC(val) u8 = ++(val); setNZ(u8);
			#define DEC(val) u8 = --(val); setNZ(u8);
			
			#define PUSH(val) (mem[STACK_POS + sp--] = uint8_t(val))
			#define PULL() mem[STACK_POS + ++sp]
			
			#define PACK_FLAGS() uint8_t(N << 7 | V << 6 | 1 << 5 | B << 4 | D << 3 | I << 2 | Z << 1 | C)
			#define FLAG_N(val) (((val) >> 7) & 0x1)
			#define FLAG_V(val) (((val) >> 6) & 0x1)
			#define FLAG_B(val) (((val) >> 4) & 0x1)
			#define FLAG_D(val) (((val) >> 3) & 0x1)
			#define FLAG_I(val) (((val) >> 2) & 0x1)
			#define FLAG_Z(val) (((val) >> 1) & 0x1)
			#define FLAG_C(val) (((val) >> 0) & 0x1)
			
			// Буферные переменные
			uint8_t u8;
			int16_t s16;
			uint16_t u16;
			uint8_t* p8;
			
			uint8_t insn = mem[pc];
			
			switch (insn) {
				case LDA_IMM:   LOAD(a, imm);  break;
				case LDA_ZP:    LOAD(a, zp);   break;
				case LDA_ZP_X:  LOAD(a, zpX);  break;
				case LDA_ABS:   LOAD(a, abs);  break;
				case LDA_ABS_X: LOAD(a, absX); break;
				case LDA_ABS_Y: LOAD(a, absY); break;
				case LDA_IND_X: LOAD(a, indX); break;
				case LDA_IND_Y: LOAD(a, indY); break;
				
				case LDX_IMM:   LOAD(x, imm);  break;
				case LDX_ZP:    LOAD(x, zp);   break;
				case LDX_ZP_Y:  LOAD(x, zpY);  break;
				case LDX_ABS:   LOAD(x, abs);  break;
				case LDX_ABS_Y: LOAD(x, absY); break;
				
				case LDY_IMM:   LOAD(y, imm);  break;
				case LDY_ZP:    LOAD(y, zp);   break;
				case LDY_ZP_X:  LOAD(y, zpX);  break;
				case LDY_ABS:   LOAD(y, abs);  break;
				case LDY_ABS_X: LOAD(y, absX); break;
				
				
				case STA_ZP:    zp = a;   break;
				case STA_ZP_X:  zpX = a;  break;
				case STA_ABS:   abs = a;  break;
				case STA_ABS_X: absX = a; break;
				case STA_ABS_Y: absY = a; break;
				case STA_IND_X: indX = a; break;
				case STA_IND_Y: indY = a; break;
				
				case STX_ZP:    zp = x;   break;
				case STX_ZP_Y:  zpY = x;  break;
				case STX_ABS:   abs = x;  break;
				
				case STY_ZP:    zp = y;   break;
				case STY_ZP_X:  zpX = y;  break;
				case STY_ABS:   abs = y;  break;
				
				
				case CMP_IMM:   CMP(a, imm);  break;
				case CMP_ZP:    CMP(a, zp);   break;
				case CMP_ZP_X:  CMP(a, zpX);  break;
				case CMP_ABS:   CMP(a, abs);  break;
				case CMP_ABS_X: CMP(a, absX); break;
				case CMP_ABS_Y: CMP(a, absY); break;
				case CMP_IND_X: CMP(a, indX); break;
				case CMP_IND_Y: CMP(a, indY); break;
				
				case CPX_IMM:   CMP(x, imm); break;
				case CPX_ZP:    CMP(x, zp);  break;
				case CPX_ABS:   CMP(x, abs); break;
				
				case CPY_IMM:   CMP(y, imm); break;
				case CPY_ZP:    CMP(y, zp);  break;
				case CPY_ABS:   CMP(y, abs); break;
				
				case BIT_ZP:    u8 = zp;  N = u8 & 0x80; V = u8 & 0x40; Z = !(u8 & a); break;
				case BIT_ABS:   u8 = abs; N = u8 & 0x80; V = u8 & 0x40; Z = !(u8 & a); break;
				
				case AND_IMM:   AND(imm);  break;
				case AND_ZP:    AND(zp);   break;
				case AND_ZP_X:  AND(zpX);  break;
				case AND_ABS:   AND(abs);  break;
				case AND_ABS_X: AND(absX); break;
				case AND_ABS_Y: AND(absY); break;
				case AND_IND_X: AND(indX); break;
				case AND_IND_Y: AND(indY); break;
				
				case ORA_IMM:   ORA(imm);  break;
				case ORA_ZP:    ORA(zp);   break;
				case ORA_ZP_X:  ORA(zpX);  break;
				case ORA_ABS:   ORA(abs);  break;
				case ORA_ABS_X: ORA(absX); break;
				case ORA_ABS_Y: ORA(absY); break;
				case ORA_IND_X: ORA(indX); break;
				case ORA_IND_Y: ORA(indY); break;
				
				case EOR_IMM:   EOR(imm);  break;
				case EOR_ZP:    EOR(zp);   break;
				case EOR_ZP_X:  EOR(zpX);  break;
				case EOR_ABS:   EOR(abs);  break;
				case EOR_ABS_X: EOR(absX); break;
				case EOR_ABS_Y: EOR(absY); break;
				case EOR_IND_X: EOR(indX); break;
				case EOR_IND_Y: EOR(indY); break;
				
				case ADC_IMM:   ADC(imm);  break;
				case ADC_ZP:    ADC(zp);   break;
				case ADC_ZP_X:  ADC(zpX);  break;
				case ADC_ABS:   ADC(abs);  break;
				case ADC_ABS_X: ADC(absX); break;
				case ADC_ABS_Y: ADC(absY); break;
				case ADC_IND_X: ADC(indX); break;
				case ADC_IND_Y: ADC(indY); break;
				
				case SBC_IMM:   SBC(imm);  break;
				case SBC_ZP:    SBC(zp);   break;
				case SBC_ZP_X:  SBC(zpX);  break;
				case SBC_ABS:   SBC(abs);  break;
				case SBC_ABS_X: SBC(absX); break;
				case SBC_ABS_Y: SBC(absY); break;
				case SBC_IND_X: SBC(indX); break;
				case SBC_IND_Y: SBC(indY); break;
				
				case ASL_A:     ASL(a);    break;
				case ASL_ZP:    ASL(zp);   break;
				case ASL_ZP_X:  ASL(zpX);  break;
				case ASL_ABS:   ASL(abs);  break;
				case ASL_ABS_X: ASL(absX); break;
				
				case LSR_A:     LSR(a);    break;
				case LSR_ZP:    LSR(zp);   break;
				case LSR_ZP_X:  LSR(zpX);  break;
				case LSR_ABS:   LSR(abs);  break;
				case LSR_ABS_X: LSR(absX); break;
				
				case ROL_A:     ROL(a);    break;
				case ROL_ZP:    ROL(zp);   break;
				case ROL_ZP_X:  ROL(zpX);  break;
				case ROL_ABS:   ROL(abs);  break;
				case ROL_ABS_X: ROL(absX); break;
				
				case ROR_A:     ROR(a);    break;
				case ROR_ZP:    ROR(zp);   break;
				case ROR_ZP_X:  ROR(zpX);  break;
				case ROR_ABS:   ROR(abs);  break;
				case ROR_ABS_X: ROR(absX); break;
				
				case INC_ZP:    INC(zp);   break;
				case INC_ZP_X:  INC(zpX);  break;
				case INC_ABS:   INC(abs);  break;
				case INC_ABS_X: INC(absX); break;
				
				case DEC_ZP:    DEC(zp);   break;
				case DEC_ZP_X:  DEC(zpX);  break;
				case DEC_ABS:   DEC(abs);  break;
				case DEC_ABS_X: DEC(absX); break;
				
				case INX: x++; setNZ(x); break;
				case INY: y++; setNZ(y); break;
				case DEX: x--; setNZ(x); break;
				case DEY: y--; setNZ(y); break;
				
				case CLC: C = 0; break;
				case CLI: I = 0; break;
				case CLD: D = 0; break;
				case CLV: V = 0; break;
				case SEC: C = 1; break;
				case SEI: I = 1; break;
				case SED: D = 1; break;
				
				case TAX: LOAD(x, a);  break;
				case TXA: LOAD(a, x);  break;
				case TAY: LOAD(y, a);  break;
				case TYA: LOAD(a, y);  break;
				case TSX: LOAD(x, sp); break;
				case TXS: sp = x;      break; // Не влияет на флаги
				
				case PHA: PUSH(a); break;
				case PHP: PUSH(PACK_FLAGS()); break;
				
				case PLA: a = PULL(); setNZ(a); break;
				case PLP:
					u8 = PULL();
					N = FLAG_N(u8);
					V = FLAG_V(u8);
					B = FLAG_B(u8);
					D = FLAG_D(u8);
					I = FLAG_I(u8);
					Z = FLAG_Z(u8);
					C = FLAG_C(u8);
					break;
				
				case BEQ: if (Z == 1) pc += int8_t(imm); break;
				case BNE: if (Z == 0) pc += int8_t(imm); break;
				case BMI: if (N == 1) pc += int8_t(imm); break;
				case BPL: if (N == 0) pc += int8_t(imm); break;
				case BCS: if (C == 1) pc += int8_t(imm); break;
				case BCC: if (C == 0) pc += int8_t(imm); break;
				case BVS: if (V == 1) pc += int8_t(imm); break;
				case BVC: if (V == 0) pc += int8_t(imm); break;
				
				case JMP_ABS:
					pc = get16(pc+1, 0);
					continue;
				
				case JMP_IND:
					u16 = get16(pc+1, 0);
					pc = get16(u16, 0);
					continue;
				
				case JSR:
					u16 = uint16_t(pc + SIZES[JSR] - 1);
					PUSH(u16 >> 8);
					PUSH(u16);
					
					pc = get16(pc+1, 0);
					continue;
				
				case RTS:
					pc = PULL();
					pc |= (PULL() << 8);
					pc += 1;
					continue;
				
				
				case BRK: B = 1; break;
				case RTI: B = 0; break;
				
				case NOP: break;
				
				default:
					addLine(30, "Error: unknown instruction $%02x", insn);
					return UNKNOWN_INSTRUCTION_ERROR;
			}
			
			pc += SIZES[insn];
		}
		
		
		state->a = a;
		state->x = x;
		state->y = y;
		state->sp = sp;
		state->pc = pc;
		state->flags = PACK_FLAGS();
		return EXIT_SUCCESS;
	}
	
	
	
	
	
	int executeCode(const vector<uint8_t>& code) {
		uint8_t* mem = (uint8_t*)malloc(MEM_SIZE);
		
		if (mem == NULL) {
			return INTERNAL_ERROR;
		}
		
		memset(mem, 0, MEM_SIZE);
		memcpy(mem + CODE_POS, code.data(), code.size());
		
		
		std::thread drawThread(draw, mem + INPUT_POS, mem + GPU_POS);
		
		processor_state state;
		int res = run(mem, &state);
		
		stopped = true;
		drawThread.join();
		
		if (res != EXIT_SUCCESS) {
			free(mem);
			return res;
		}
		
		
		addLine(46, "a = $%02x, x = $%02x, y = $%02x, sp = $%02x, pc = $%03x", state.a, state.x, state.y, state.sp, state.pc);
		
		addLine("N V - B D I Z C");
		addLine(15, "%d %d 1 %d %d %d %d %d",
			FLAG_N(state.flags),
			FLAG_V(state.flags),
			FLAG_B(state.flags),
			FLAG_D(state.flags),
			FLAG_I(state.flags),
			FLAG_Z(state.flags),
			FLAG_C(state.flags)
		);
		
		dump("Zero page dump:", mem, 0,         16, 16);
		dump("Stack dump:",     mem, STACK_POS, 16, 16);
		dump("GPU dump:",       mem, GPU_POS,   16, 64);
		dump("Code dump:",      mem, CODE_POS,  16, 16);
		
		free(mem);
		mem = NULL;
		
		printLines();
		refresh();
		
		
		for (;;) {
			switch (getch()) {
				case KEY_UP:   scrollUp();   break;
				case KEY_DOWN: scrollDown(); break;
				case 'q': return EXIT_SUCCESS;
			}
		}
	}
}
