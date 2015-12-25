// license:BSD-3-Clause
// copyright-holders:Karl Stenerud
/* ======================================================================== */
/* =============================== COPYRIGHT ============================== */
/* ======================================================================== */
/*

G65C816 CPU Emulator V0.90

Copyright Karl Stenerud
All rights reserved.

*/


#include "emu.h"
#include "g65816ds.h"

#ifdef SEC
#undef SEC
#endif

#define ADDRESS_65816(A) ((A)&0xffffff)


struct g65816_opcode_struct
{
	unsigned char name;
	unsigned char flag;
	unsigned char ea;
};

enum
{
	IMP , ACC , RELB, RELW, IMM , A   , AI  , AL  , ALX , AX  , AXI ,
	AY  , D   , DI  , DIY , DLI , DLIY, DX  , DXI , DY  , S   , SIY ,
	SIG /*, MVN , MVP , PEA , PEI , PER */
};

enum
{
	I, /* ignore */
	M, /* check m bit */
	X  /* check x bit */
};

enum
{
	ADC ,  AND ,  ASL ,  BCC ,  BCS ,  BEQ ,  BIT ,  BMI ,  BNE ,  BPL ,  BRA ,
	BRK ,  BRL ,  BVC ,  BVS ,  CLC ,  CLD ,  CLI ,  CLV ,  CMP ,  COP ,  CPX ,
	CPY ,  DEA ,  DEC ,  DEX ,  DEY ,  EOR ,  INA ,  INC ,  INX ,  INY ,  JML ,
	JMP ,  JSL ,  JSR ,  LDA ,  LDX ,  LDY ,  LSR ,  MVN ,  MVP ,  NOP ,  ORA ,
	PEA ,  PEI ,  PER ,  PHA ,  PHB ,  PHD ,  PHK ,  PHP ,  PHX ,  PHY ,  PLA ,
	PLB ,  PLD ,  PLP ,  PLX ,  PLY ,  REP ,  ROL ,  ROR ,  RTI ,  RTL ,  RTS ,
	SBC ,  SEC ,  SED ,  SEI ,  SEP ,  STA ,  STP ,  STX ,  STY ,  STZ ,  TAX ,
	TAY ,  TCS ,  TCD ,  TDC ,  TRB ,  TSB ,  TSC ,  TSX ,  TXA ,  TXS ,  TXY ,
	TYA ,  TYX ,  WAI ,  WDM ,  XBA ,  XCE
};

static const char *const g_opnames[] =
{
	"ADC", "AND", "ASL", "BCC", "BCS", "BEQ", "BIT", "BMI", "BNE", "BPL", "BRA",
	"BRK", "BRL", "BVC", "BVS", "CLC", "CLD", "CLI", "CLV", "CMP", "COP", "CPX",
	"CPY", "DEA", "DEC", "DEX", "DEY", "EOR", "INA", "INC", "INX", "INY", "JML",
	"JMP", "JSL", "JSR", "LDA", "LDX", "LDY", "LSR", "MVN", "MVP", "NOP", "ORA",
	"PEA", "PEI", "PER", "PHA", "PHB", "PHD", "PHK", "PHP", "PHX", "PHY", "PLA",
	"PLB", "PLD", "PLP", "PLX", "PLY", "REP", "ROL", "ROR", "RTI", "RTL", "RTS",
	"SBC", "SEC", "SED", "SEI", "SEP", "STA", "STP", "STX", "STY", "STZ", "TAX",
	"TAY", "TCS", "TCD", "TDC", "TRB", "TSB", "TSC", "TSX", "TXA", "TXS", "TXY",
	"TYA", "TYX", "WAI", "WDM", "XBA", "XCE"
};

static const g65816_opcode_struct g_opcodes[256] =
{
	{BRK, I, SIG }, {ORA, M, DXI }, {COP, I, SIG }, {ORA, M, S   },
	{TSB, M, D   }, {ORA, M, D   }, {ASL, M, D   }, {ORA, M, DLI },
	{PHP, I, IMP }, {ORA, M, IMM }, {ASL, M, ACC }, {PHD, I, IMP },
	{TSB, M, A   }, {ORA, M, A   }, {ASL, M, A   }, {ORA, M, AL  },
	{BPL, I, RELB}, {ORA, M, DIY }, {ORA, M, DI  }, {ORA, M, SIY },
	{TRB, M, D   }, {ORA, M, DX  }, {ASL, M, DX  }, {ORA, M, DLIY},
	{CLC, I, IMP }, {ORA, M, AY  }, {INA, I, IMP }, {TCS, I, IMP },
	{TRB, M, A   }, {ORA, M, AX  }, {ASL, M, AX  }, {ORA, M, ALX },
	{JSR, I, A   }, {AND, M, DXI }, {JSL, I, AL  }, {AND, M, S   },
	{BIT, M, D   }, {AND, M, D   }, {ROL, M, D   }, {AND, M, DLI },
	{PLP, I, IMP }, {AND, M, IMM }, {ROL, M, ACC }, {PLD, I, IMP },
	{BIT, M, A   }, {AND, M, A   }, {ROL, M, A   }, {AND, M, AL  },
	{BMI, I, RELB}, {AND, M, DIY }, {AND, M, DI  }, {AND, M, SIY },
	{BIT, M, DX  }, {AND, M, DX  }, {ROL, M, DX  }, {AND, M, DLIY},
	{SEC, I, IMP }, {AND, M, AY  }, {DEA, I, IMP }, {TSC, I, IMP },
	{BIT, M, AX  }, {AND, M, AX  }, {ROL, M, AX  }, {AND, M, ALX },
	{RTI, I, IMP }, {EOR, M, DXI }, {WDM, I, IMP }, {EOR, M, S   },
	{MVP, I, MVP }, {EOR, M, D   }, {LSR, M, D   }, {EOR, M, DLI },
	{PHA, I, IMP }, {EOR, M, IMM }, {LSR, M, ACC }, {PHK, I, IMP },
	{JMP, I, A   }, {EOR, M, A   }, {LSR, M, A   }, {EOR, M, AL  },
	{BVC, I, RELB}, {EOR, M, DIY }, {EOR, M, DI  }, {EOR, M, SIY },
	{MVN, I, MVN }, {EOR, M, DX  }, {LSR, M, DX  }, {EOR, M, DLIY},
	{CLI, I, IMP }, {EOR, M, AY  }, {PHY, I, IMP }, {TCD, I, IMP },
	{JMP, I, AL  }, {EOR, M, AX  }, {LSR, M, AX  }, {EOR, M, ALX },
	{RTS, I, IMP }, {ADC, M, DXI }, {PER, I, PER }, {ADC, M, S   },
	{STZ, M, D   }, {ADC, M, D   }, {ROR, M, D   }, {ADC, M, DLI },
	{PLA, I, IMP }, {ADC, M, IMM }, {ROR, M, ACC }, {RTL, I, IMP },
	{JMP, I, AI  }, {ADC, M, A   }, {ROR, M, A   }, {ADC, M, AL  },
	{BVS, I, RELB}, {ADC, M, DIY }, {ADC, M, DI  }, {ADC, M, SIY },
	{STZ, M, DX  }, {ADC, M, DX  }, {ROR, M, DX  }, {ADC, M, DLIY},
	{SEI, I, IMP }, {ADC, M, AY  }, {PLY, I, IMP }, {TDC, I, IMP },
	{JMP, I, AXI }, {ADC, M, AX  }, {ROR, M, AX  }, {ADC, M, ALX },
	{BRA, I, RELB}, {STA, M, DXI }, {BRL, I, RELW}, {STA, M, S   },
	{STY, X, D   }, {STA, M, D   }, {STX, X, D   }, {STA, M, DLI },
	{DEY, I, IMP }, {BIT, M, IMM }, {TXA, I, IMP }, {PHB, I, IMP },
	{STY, X, A   }, {STA, M, A   }, {STX, X, A   }, {STA, M, AL  },
	{BCC, I, RELB}, {STA, M, DIY }, {STA, M, DI  }, {STA, M, SIY },
	{STY, X, DX  }, {STA, M, DX  }, {STX, X, DY  }, {STA, M, DLIY},
	{TYA, I, IMP }, {STA, M, AY  }, {TXS, I, IMP }, {TXY, I, IMP },
	{STZ, M, A   }, {STA, M, AX  }, {STZ, M, AX  }, {STA, M, ALX },
	{LDY, X, IMM }, {LDA, M, DXI }, {LDX, X, IMM }, {LDA, M, S   },
	{LDY, X, D   }, {LDA, M, D   }, {LDX, X, D   }, {LDA, M, DLI },
	{TAY, I, IMP }, {LDA, M, IMM }, {TAX, I, IMP }, {PLB, I, IMP },
	{LDY, X, A   }, {LDA, M, A   }, {LDX, X, A   }, {LDA, M, AL  },
	{BCS, I, RELB}, {LDA, M, DIY }, {LDA, M, DI  }, {LDA, M, SIY },
	{LDY, X, DX  }, {LDA, M, DX  }, {LDX, X, DY  }, {LDA, M, DLIY},
	{CLV, I, IMP }, {LDA, M, AY  }, {TSX, I, IMP }, {TYX, I, IMP },
	{LDY, X, AX  }, {LDA, M, AX  }, {LDX, X, AY  }, {LDA, M, ALX },
	{CPY, X, IMM }, {CMP, M, DXI }, {REP, I, IMM }, {CMP, M, S   },
	{CPY, X, D   }, {CMP, M, D   }, {DEC, M, D   }, {CMP, M, DLI },
	{INY, I, IMP }, {CMP, M, IMM }, {DEX, I, IMP }, {WAI, I, IMP },
	{CPY, X, A   }, {CMP, M, A   }, {DEC, M, A   }, {CMP, M, AL  },
	{BNE, I, RELB}, {CMP, M, DIY }, {CMP, M, DI  }, {CMP, M, SIY },
	{PEI, I, PEI }, {CMP, M, DX  }, {DEC, M, DX  }, {CMP, M, DLIY},
	{CLD, I, IMP }, {CMP, M, AY  }, {PHX, I, IMP }, {STP, I, IMP },
	{JML, I, AI  }, {CMP, M, AX  }, {DEC, M, AX  }, {CMP, M, ALX },
	{CPX, X, IMM }, {SBC, M, DXI }, {SEP, I, IMM }, {SBC, M, S   },
	{CPX, X, D   }, {SBC, M, D   }, {INC, M, D   }, {SBC, M, DLI },
	{INX, M, IMP }, {SBC, M, IMM }, {NOP, I, IMP }, {XBA, I, IMP },
	{CPX, X, A   }, {SBC, M, A   }, {INC, M, A   }, {SBC, M, AL  },
	{BEQ, I, RELB}, {SBC, M, DIY }, {SBC, M, DI  }, {SBC, M, SIY },
	{PEA, I, PEA }, {SBC, M, DX  }, {INC, M, DX  }, {SBC, M, DLIY},
	{SED, I, IMP }, {SBC, M, AY  }, {PLX, I, IMP }, {XCE, I, IMP },
	{JSR, I, AXI }, {SBC, M, AX  }, {INC, M, AX  }, {SBC, M, ALX }
};

static const UINT8 *base_oprom;
static UINT32 base_pc;

static inline unsigned int read_8(unsigned int address)
{
	address = ADDRESS_65816(address);
	return base_oprom[address - base_pc];
}

static inline unsigned int read_16(unsigned int address)
{
	unsigned int val = read_8(address);
	return val | (read_8(address+1)<<8);
}

static inline unsigned int read_24(unsigned int address)
{
	unsigned int val = read_8(address);
	val |= (read_8(address+1)<<8);
	return val | (read_8(address+2)<<16);
}

static inline char* int_8_str(unsigned int val)
{
	static char str[20];

	val &= 0xff;

	if(val & 0x80)
		sprintf(str, "-$%x", (0-val) & 0x7f);
	else
		sprintf(str, "$%x", val & 0x7f);

	return str;
}

static inline char* int_16_str(unsigned int val)
{
	static char str[20];

	val &= 0xffff;

	if(val & 0x8000)
		sprintf(str, "-$%x", (0-val) & 0x7fff);
	else
		sprintf(str, "$%x", val & 0x7fff);

	return str;
}


unsigned g65816_disassemble(char* buff, unsigned int pc, unsigned int pb, const UINT8 *oprom, int m_flag, int x_flag)
{
	unsigned int instruction;
	const g65816_opcode_struct* opcode;
	char* ptr;
	int var;
	int length = 1;
	unsigned int address;
	unsigned dasm_flags;

	pb <<= 16;
	address = pc | pb;

	base_oprom = oprom;
	base_pc = address;

	instruction = read_8(address);
	opcode = g_opcodes + instruction;

	strcpy(buff, g_opnames[opcode->name]);
	ptr = buff + strlen(buff);

	switch(opcode->name)
	{
		case JSR:
		case JSL:
			dasm_flags = DASMFLAG_STEP_OVER;
			break;

		case RTI:
		case RTL:
		case RTS:
			dasm_flags = DASMFLAG_STEP_OUT;
			break;

		default:
			dasm_flags = 0;
			break;
	}

	switch(opcode->ea)
	{
		case IMP :
			break;
		case ACC :
			sprintf(ptr, "A");
			break;
		case RELB:
			var = (INT8) read_8(address+1);
			length++;
			sprintf(ptr, " %06x (%s)", pb | ((pc + length + var)&0xffff), int_8_str(var));
			break;
		case RELW:
		case PER :
			var = read_16(address+1);
			length += 2;
			sprintf(ptr, " %06x (%s)", pb | ((pc + length + var)&0xffff), int_16_str(var));
			break;
		case IMM :
			if((opcode->flag == M && !m_flag) || (opcode->flag == X && !x_flag))
			{
				sprintf(ptr, " #$%04x", read_16(address+1));
				length += 2;
			}
			else
			{
				sprintf(ptr, " #$%02x", read_8(address+1));
				length++;
			}
			break;
		case A   :
		case PEA :
			sprintf(ptr, " $%04x", read_16(address+1));
			length += 2;
			break;
		case AI  :
			sprintf(ptr, " ($%04x)", read_16(address+1));
			length += 2;
			break;
		case AL  :
			sprintf(ptr, " $%06x", read_24(address+1));
			length += 3;
			break;
		case ALX :
			sprintf(ptr, " $%06x,X", read_24(address+1));
			length += 3;
			break;
		case AX  :
			sprintf(ptr, " $%04x,X", read_16(address+1));
			length += 2;
			break;
		case AXI :
			sprintf(ptr, " ($%04x,X)", read_16(address+1));
			length += 2;
			break;
		case AY  :
			sprintf(ptr, " $%04x,Y", read_16(address+1));
			length += 2;
			break;
		case D   :
			sprintf(ptr, " $%02x", read_8(address+1));
			length++;
			break;
		case DI  :
		case PEI :
			sprintf(ptr, " ($%02x)", read_8(address+1));
			length++;
			break;
		case DIY :
			sprintf(ptr, " ($%02x),Y", read_8(address+1));
			length++;
			break;
		case DLI :
			sprintf(ptr, " [$%02x]", read_8(address+1));
			length++;
			break;
		case DLIY:
			sprintf(ptr, " [$%02x],Y", read_8(address+1));
			length++;
			break;
		case DX  :
			sprintf(ptr, " $%02x,X", read_8(address+1));
			length++;
			break;
		case DXI :
			sprintf(ptr, " ($%02x,X)", read_8(address+1));
			length++;
			break;
		case DY  :
			sprintf(ptr, " $%02x,Y", read_8(address+1));
			length++;
			break;
		case S   :
			sprintf(ptr, " %s,S", int_8_str(read_8(address+1)));
			length++;
			break;
		case SIY :
			sprintf(ptr, " (%s,S),Y", int_8_str(read_8(address+1)));
			length++;
			break;
		case SIG :
			sprintf(ptr, " #$%02x", read_8(address+1));
			length++;
			break;
		case MVN :
		case MVP :
			sprintf(ptr, " $%02x, $%02x", read_8(address+2), read_8(address+1));
			length += 2;
			break;
	}

	return length | DASMFLAG_SUPPORTED | dasm_flags;
}

CPU_DISASSEMBLE( g65816_generic )
{
	return g65816_disassemble(buffer, (pc & 0x00ffff), (pc & 0xff0000) >> 16, oprom, 0, 0);
}
