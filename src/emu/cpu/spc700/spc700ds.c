/* ======================================================================== */
/* =============================== COPYRIGHT ============================== */
/* ======================================================================== */
/*

SPC700 CPU Emulator V0.90

Copyright (c) 2000 Karl Stenerud
All rights reserved.

Permission is granted to use this source code for non-commercial purposes.
To use this code for commercial purposes, you must get permission from the
author (Karl Stenerud) at karl@higashiyama-unet.ocn.ne.jp.


*/


#include "spc700ds.h"


#ifndef INLINE
#define INLINE static
#endif


typedef struct
{
	unsigned char name;
	unsigned char args[2];
} opcode_struct;

enum
{
	IMP , A   , X   , Y   , YA  , SP  , PSW , C   , REL , UPAG, IMM , XI  ,
	XII , YI  , DP  , DPX , DPY , DPI , DXI , DIY , ABS , ABX , ABY , AXI , N0  ,
	N1  , N2  , N3  , N4  , N5  , N6  , N7  , N8  , N9  , N10 , N11 , N12 ,
	N13 , N14 , N15 , DP0 , DP1 , DP2 , DP3 , DP4 , DP5 , DP6 , DP7 , MEMN,
	MEMI
};


enum
{
	ADC   ,  ADDW  ,  AND   ,  AND1  ,  ASL   ,  BBC   ,  BBS   ,  BCC   ,
	BCS   ,  BEQ   ,  BMI   ,  BNE   ,  BPL   ,  BRA   ,  BRK   ,  BVC   ,
	BVS   ,  CALL  ,  CBNE  ,  CLR1  ,  CLRC  ,  CLRP  ,  CLRV  ,  CMP   ,
	CMPW  ,  DAA   ,  DAS   ,  DBNZ  ,  DEC   ,  DECW  ,  DI    ,  DIV   ,
	EI    ,  EOR   ,  EOR1  ,  INC   ,  INCW  ,  JMP   ,  LSR   ,  MOV   ,
	MOV1  ,  MOVW  ,  MUL   ,  NOP   ,  NOT1  ,  NOTQ  ,  NOTC  ,  OR    ,
	OR1   ,  PCALL ,  POP   ,  PUSH  ,  RET   ,  RETI  ,  ROL   ,  ROR   ,
	SBC   ,  SET1  ,  SETC  ,  SETP  ,  SLEEP ,  STOP  ,  SUBW  ,  TCALL ,
	TCLR1 ,  TSET1 ,  XCN
};


static const char* g_opnames[] =
{
	"ADC  ", "ADDW ", "AND  ", "AND1 ", "ASL  ", "BBC  ", "BBS  ", "BCC  ",
	"BCS  ", "BEQ  ", "BMI  ", "BNE  ", "BPL  ", "BRA  ", "BRK  ", "BVC  ",
	"BVS  ", "CALL ", "CBNE ", "CLR1 ", "CLRC ", "CLRP ", "CLRV ", "CMP  ",
	"CMPW ", "DAA  ", "DAS  ", "DBNZ ", "DEC  ", "DECW ", "DI   ", "DIV  ",
	"EI   ", "EOR  ", "EOR1 ", "INC  ", "INCW ", "JMP  ", "LSR  ", "MOV  ",
	"MOV1 ", "MOVW ", "MUL  ", "NOP  ", "NOT1 ", "NOTQ ", "NOTC ", "OR   ",
	"OR1  ", "PCALL", "POP  ", "PUSH ", "RET  ", "RETI ", "ROL  ", "ROR  ",
	"SBC  ", "SET1 ", "SETC ", "SETP ", "SLEEP", "STOP ", "SUBW ", "TCALL",
	"TCLR1", "TSET1", "XCN  "
};

static opcode_struct g_opcodes[256] =
{
/* 00 */ {NOP    , {IMP , IMP }},
/* 01 */ {TCALL  , {N0  , IMP }},
/* 02 */ {SET1   , {DP0 , IMP }},
/* 03 */ {BBS    , {DP0 , REL }},
/* 04 */ {OR     , {A   , DP  }},
/* 05 */ {OR     , {A   , ABS }},
/* 06 */ {OR     , {A   , XI  }},
/* 07 */ {OR     , {A   , DXI }},
/* 08 */ {OR     , {A   , IMM }},
/* 09 */ {OR     , {DP  , DP  }},
/* 0A */ {OR1    , {C   , MEMN}},
/* 0B */ {ASL    , {DP  , IMP }},
/* 0C */ {ASL    , {ABS , IMP }},
/* 0D */ {PUSH   , {PSW , IMP }},
/* 0E */ {TSET1  , {ABS , IMP }},
/* 0F */ {BRK    , {IMM , IMP }},
/* 10 */ {BPL    , {REL , IMP }},
/* 11 */ {TCALL  , {N1  , IMP }},
/* 12 */ {CLR1   , {DP0 , IMP }},
/* 13 */ {BBC    , {DP0 , REL }},
/* 14 */ {OR     , {A   , DPX }},
/* 15 */ {OR     , {A   , ABX }},
/* 16 */ {OR     , {A   , ABY }},
/* 17 */ {OR     , {A   , DIY }},
/* 18 */ {OR     , {DP  , IMM }},
/* 19 */ {OR     , {XI  , YI  }},
/* 1A */ {DECW   , {DP  , IMP }},
/* 1B */ {ASL    , {DPX , IMP }},
/* 1C */ {ASL    , {A   , IMP }},
/* 1D */ {DEC    , {X   , IMP }},
/* 1E */ {CMP    , {X   , ABS }},
/* 1F */ {JMP    , {AXI , IMP }},
/* 20 */ {CLRP   , {IMP , IMP }},
/* 21 */ {TCALL  , {N2  , IMP }},
/* 22 */ {SET1   , {DP1 , IMP }},
/* 23 */ {BBS    , {DP1 , REL }},
/* 24 */ {AND    , {A   , DP  }},
/* 25 */ {AND    , {A   , ABS }},
/* 26 */ {AND    , {A   , XI  }},
/* 27 */ {AND    , {A   , DXI }},
/* 28 */ {AND    , {A   , IMM }},
/* 29 */ {AND    , {DP  , DP  }},
/* 2A */ {OR1    , {C   , MEMI}},
/* 2B */ {ROL    , {DP  , IMP }},
/* 2C */ {ROL    , {ABS , IMP }},
/* 2D */ {PUSH   , {A   , IMP }},
/* 2E */ {CBNE   , {DP  , REL }},
/* 2F */ {BRA    , {REL , IMP }},
/* 30 */ {BMI    , {REL , IMP }},
/* 31 */ {TCALL  , {N3  , IMP }},
/* 32 */ {CLR1   , {DP1 , IMP }},
/* 33 */ {BBC    , {DP1 , REL }},
/* 34 */ {AND    , {A   , DPX }},
/* 35 */ {AND    , {A   , ABX }},
/* 36 */ {AND    , {A   , ABY }},
/* 37 */ {AND    , {A   , DIY }},
/* 38 */ {AND    , {DP  , IMM }},
/* 39 */ {AND    , {XI  , YI  }},
/* 3A */ {INCW   , {DI  , IMP }},
/* 3B */ {ROL    , {DPX , IMP }},
/* 3C */ {ROL    , {A   , IMP }},
/* 3D */ {INC    , {X   , IMP }},
/* 3E */ {CMP    , {X   , DP  }},
/* 3F */ {CALL   , {ABS , IMP }},
/* 40 */ {SETP   , {IMP , IMP }},
/* 41 */ {TCALL  , {N4  , IMP }},
/* 42 */ {SET1   , {DP2 , IMP }},
/* 43 */ {BBS    , {DP2 , REL }},
/* 44 */ {EOR    , {A   , DP  }},
/* 45 */ {EOR    , {A   , ABS }},
/* 46 */ {EOR    , {A   , XI  }},
/* 47 */ {EOR    , {A   , DXI }},
/* 48 */ {EOR    , {A   , IMM }},
/* 49 */ {EOR    , {DP  , DP  }},
/* 4A */ {AND1   , {C   , MEMN}},
/* 4B */ {LSR    , {DP  , IMP }},
/* 4C */ {LSR    , {ABS , IMP }},
/* 4D */ {PUSH   , {X   , IMP }},
/* 4E */ {TCLR1  , {ABS , IMP }},
/* 4F */ {PCALL  , {UPAG, IMP }},
/* 50 */ {BVC    , {REL , IMP }},
/* 51 */ {TCALL  , {N5  , IMP }},
/* 52 */ {CLR1   , {DP2 , IMP }},
/* 53 */ {BBC    , {DP2 , REL }},
/* 54 */ {EOR    , {A   , DPX }},
/* 55 */ {EOR    , {A   , ABX }},
/* 56 */ {EOR    , {A   , ABY }},
/* 57 */ {EOR    , {A   , DIY }},
/* 58 */ {EOR    , {DP  , IMM }},
/* 59 */ {EOR    , {XI  , YI  }},
/* 5A */ {CMPW   , {DP  , IMP }},
/* 5B */ {LSR    , {DPX , IMP }},
/* 5C */ {LSR    , {A   , IMP }},
/* 5D */ {MOV    , {X   , A   }},
/* 5E */ {CMP    , {Y   , ABS }},
/* 5F */ {JMP    , {ABS , IMP }},
/* 60 */ {CLRC   , {IMP , IMP }},
/* 61 */ {TCALL  , {N6  , IMP }},
/* 62 */ {SET1   , {DP3 , IMP }},
/* 63 */ {BBS    , {DP3 , REL }},
/* 64 */ {CMP    , {A   , DP  }},
/* 65 */ {CMP    , {A   , ABS }},
/* 66 */ {CMP    , {A   , XI  }},
/* 67 */ {CMP    , {A   , DXI }},
/* 68 */ {CMP    , {A   , IMM }},
/* 69 */ {CMP    , {DP  , DP  }},
/* 6A */ {AND1   , {C   , MEMI}},
/* 6B */ {ROR    , {DP  , IMP }},
/* 6C */ {ROR    , {ABS , IMP }},
/* 6D */ {PUSH   , {Y   , IMP }},
/* 6E */ {DBNZ   , {DP  , REL }},
/* 6F */ {RET    , {IMP , IMP }},
/* 70 */ {BVS    , {REL , IMP }},
/* 71 */ {TCALL  , {N7  , IMP }},
/* 72 */ {CLR1   , {DP3 , IMP }},
/* 73 */ {BBC    , {DP3 , REL }},
/* 74 */ {CMP    , {A   , DPX }},
/* 75 */ {CMP    , {A   , ABX }},
/* 76 */ {CMP    , {A   , ABY }},
/* 77 */ {CMP    , {A   , DIY }},
/* 78 */ {CMP    , {IMM , DP  }},
/* 79 */ {CMP    , {XI  , YI  }},
/* 7A */ {ADDW   , {DI  , IMP }},
/* 7B */ {ROR    , {DPX , IMP }},
/* 7C */ {ROR    , {A   , IMP }},
/* 7D */ {MOV    , {A   , X   }},
/* 7E */ {CMP    , {Y   , DP  }},
/* 7F */ {RETI   , {IMP , IMP }},
/* 80 */ {SETC   , {IMP , IMP }},
/* 81 */ {TCALL  , {N8  , IMP }},
/* 82 */ {SET1   , {DP4 , IMP }},
/* 83 */ {BBS    , {DP4 , REL }},
/* 84 */ {ADC    , {A   , DP  }},
/* 85 */ {ADC    , {A   , ABS }},
/* 86 */ {ADC    , {A   , XI  }},
/* 87 */ {ADC    , {A   , DXI }},
/* 88 */ {ADC    , {A   , IMM }},
/* 89 */ {ADC    , {DP  , DP  }},
/* 8A */ {EOR1   , {C   , MEMN}},
/* 8B */ {DEC    , {DP  , IMP }},
/* 8C */ {DEC    , {ABS , IMP }},
/* 8D */ {MOV    , {Y   , IMM }},
/* 8E */ {POP    , {PSW , IMP }},
/* 8F */ {MOV    , {IMM , DP  }},
/* 90 */ {BCC    , {REL , IMP }},
/* 91 */ {TCALL  , {N9  , IMP }},
/* 92 */ {CLR1   , {DP4 , IMP }},
/* 93 */ {BBC    , {DP4 , REL }},
/* 94 */ {ADC    , {A   , DPX }},
/* 95 */ {ADC    , {A   , ABX }},
/* 96 */ {ADC    , {A   , ABY }},
/* 97 */ {ADC    , {A   , DIY }},
/* 98 */ {ADC    , {DP  , IMM }},
/* 99 */ {ADC    , {XI  , YI  }},
/* 9A */ {SUBW   , {DP  , IMP }},
/* 9B */ {DEC    , {DPX , IMP }},
/* 9C */ {DEC    , {A   , IMP }},
/* 9D */ {MOV    , {X   , SP  }},
/* 9E */ {DIV    , {YA  , X   }},
/* 9F */ {XCN    , {A   , IMP }},
/* A0 */ {EI     , {IMP , IMP }},
/* A1 */ {TCALL  , {N10 , IMP }},
/* A2 */ {SET1   , {DP5 , IMP }},
/* A3 */ {BBS    , {DP5 , REL }},
/* A4 */ {SBC    , {A   , DP  }},
/* A5 */ {SBC    , {A   , ABS }},
/* A6 */ {SBC    , {A   , XI  }},
/* A7 */ {SBC    , {A   , DXI }},
/* A8 */ {SBC    , {A   , IMM }},
/* A9 */ {SBC    , {DP  , DP  }},
/* AA */ {MOV1   , {C   , MEMN}},
/* AB */ {INC    , {DP  , IMP }},
/* AC */ {INC    , {ABS , IMP }},
/* AD */ {CMP    , {Y   , IMM }},
/* AE */ {POP    , {A   , IMP }},
/* AF */ {MOV    , {XII , A   }},
/* B0 */ {BCS    , {REL , IMP }},
/* B1 */ {TCALL  , {N11 , IMP }},
/* B2 */ {CLR1   , {DP5 , IMP }},
/* B3 */ {BBC    , {DP5 , REL }},
/* B4 */ {SBC    , {A   , DPX }},
/* B5 */ {SBC    , {A   , ABX }},
/* B6 */ {SBC    , {A   , ABY }},
/* B7 */ {SBC    , {A   , DIY }},
/* B8 */ {SBC    , {DP  , IMM }},
/* B9 */ {SBC    , {XI  , YI  }},
/* BA */ {MOVW   , {YA  , DP  }},
/* BB */ {INC    , {DPX , IMP }},
/* BC */ {INC    , {A   , IMP }},
/* BD */ {MOV    , {SP  , X   }},
/* BE */ {DAS    , {A   , IMP }},
/* BF */ {MOV    , {A   , XII }},
/* C0 */ {DI     , {IMP , IMP }},
/* C1 */ {TCALL  , {N12 , IMP }},
/* C2 */ {SET1   , {DP6 , IMP }},
/* C3 */ {BBS    , {DP6 , REL }},
/* C4 */ {MOV    , {DP  , A   }},
/* C5 */ {MOV    , {ABS , A   }},
/* C6 */ {MOV    , {XI  , A   }},
/* C7 */ {MOV    , {DXI , A   }},
/* C8 */ {CMP    , {X   , IMM }},
/* C9 */ {MOV    , {ABS , X   }},
/* CA */ {MOV1   , {MEMN, C   }},
/* CB */ {MOV    , {DP  , Y   }},
/* CC */ {MOV    , {ABS , Y   }},
/* CD */ {MOV    , {X   , IMM }},
/* CE */ {POP    , {X   , IMP }},
/* CF */ {MUL    , {YA  , IMP }},
/* D0 */ {BNE    , {REL , IMP }},
/* D1 */ {TCALL  , {N13 , IMP }},
/* D2 */ {CLR1   , {DP6 , IMP }},
/* D3 */ {BBC    , {DP6 , REL }},
/* D4 */ {MOV    , {DPX , A   }},
/* D5 */ {MOV    , {ABX , A   }},
/* D6 */ {MOV    , {ABY , A   }},
/* D7 */ {MOV    , {DIY , A   }},
/* D8 */ {MOV    , {DP  , X   }},
/* D9 */ {MOV    , {DPY , X   }},
/* DA */ {MOVW   , {DP  , YA  }},
/* DB */ {MOV    , {DPX , Y   }},
/* DC */ {DEC    , {Y   , IMP }},
/* DD */ {MOV    , {A   , Y   }},
/* DE */ {CBNE   , {DPX , REL }},
/* DF */ {DAA    , {IMP , IMP }},
/* E0 */ {CLRV   , {IMP , IMP }},
/* E1 */ {TCALL  , {N14 , IMP }},
/* E2 */ {SET1   , {DP7 , IMP }},
/* E3 */ {BBS    , {DP7 , REL }},
/* E4 */ {MOV    , {A   , DP  }},
/* E5 */ {MOV    , {A   , ABS }},
/* E6 */ {MOV    , {A   , XI  }},
/* E7 */ {MOV    , {A   , DXI }},
/* E8 */ {MOV    , {A   , IMM }},
/* E9 */ {MOV    , {X   , ABS }},
/* EA */ {NOT1   , {IMP , IMP }},
/* EB */ {MOV    , {Y   , DP  }},
/* EC */ {MOV    , {Y   , ABS }},
/* ED */ {NOTC   , {IMP , IMP }},
/* EE */ {POP    , {Y   , IMP }},
/* EF */ {SLEEP  , {IMP , IMP }},
/* F0 */ {BEQ    , {REL , IMP }},
/* F1 */ {TCALL  , {N15 , IMP }},
/* F2 */ {CLR1   , {DP7 , IMP }},
/* F3 */ {BBC    , {DP7 , REL }},
/* F4 */ {MOV    , {A   , DPX }},
/* F5 */ {MOV    , {A   , ABX }},
/* F6 */ {MOV    , {A   , ABY }},
/* F7 */ {MOV    , {A   , DIY }},
/* F8 */ {MOV    , {X   , DP  }},
/* F9 */ {MOV    , {X   , DPY }},
/* FA */ {MOV    , {DP  , DP  }},
/* FB */ {MOV    , {Y   , DPX }},
/* FC */ {INC    , {Y   , IMP }},
/* FD */ {MOV    , {Y   , A   }},
/* FE */ {DBNZ   , {Y   , REL }},
/* FF */ {STOP   , {IMP , IMP }},
};

static unsigned int g_pc;
static const UINT8 *rombase;

INLINE unsigned int read_8_immediate(void)
{
	g_pc++;
	return *rombase++;
}

INLINE unsigned int read_16_immediate(void)
{
	unsigned int result;
	g_pc += 2;
	result = *rombase++;
	return result | (*rombase++ << 8);
}

offs_t spc700_dasm(char *buff, offs_t pc, const UINT8 *oprom, const UINT8 *opram)
{
	opcode_struct* opcode;
	UINT32 flags = 0;
	char* ptr;
	int var;
	int i;

	g_pc = pc;
	rombase = oprom;
	opcode = g_opcodes + read_8_immediate();

	sprintf(buff, "%s ", g_opnames[opcode->name]);
	ptr = buff + strlen(buff);

	if (opcode->name == CALL)
		flags = DASMFLAG_STEP_OVER;
	else if (opcode->name == RET || opcode->name == RETI)
		flags = DASMFLAG_STEP_OUT;

	for(i=0;i<2;i++)
	{
		if(i == 1 && opcode->args[0] != IMP && opcode->args[1] != IMP)
		{
			sprintf(ptr, ",");
			ptr += strlen(ptr);
		}

		switch(opcode->args[i])
		{
			case IMP:  break;
			case A:    sprintf(ptr, "A"); break;
			case X:    sprintf(ptr, "X"); break;
			case Y:    sprintf(ptr, "Y"); break;
			case YA:   sprintf(ptr, "YA"); break;
			case SP:   sprintf(ptr, "SP"); break;
			case PSW:  sprintf(ptr, "PSW"); break;
			case C:    sprintf(ptr, "C"); break;
			case REL:  sprintf(ptr, "%04x", ((g_pc + (char)read_8_immediate())&0xffff)); break;
			case UPAG: sprintf(ptr, "$%02x", read_8_immediate()); break;
			case IMM:  sprintf(ptr, "#$%02x", read_8_immediate()); break;
			case XI:   sprintf(ptr, "(X)"); break;
			case XII:  sprintf(ptr, "(X)+"); break;
			case YI:   sprintf(ptr, "(Y)"); break;
			case DP:   sprintf(ptr, "$%02x", read_8_immediate()); break;
			case DPX:  sprintf(ptr, "$%02x+X", read_8_immediate()); break;
			case DPY:  sprintf(ptr, "$%02x+Y", read_8_immediate()); break;
			case DPI:  sprintf(ptr, "($%02x)", read_8_immediate()); break;
			case DXI:  sprintf(ptr, "($%02x+X)", read_8_immediate()); break;
			case DIY:  sprintf(ptr, "($%02x)+Y", read_8_immediate()); break;
			case ABS:  sprintf(ptr, "$%04x", read_16_immediate()); break;
			case ABX:  sprintf(ptr, "$%04x+X", read_16_immediate()); break;
			case ABY:  sprintf(ptr, "$%04x+Y", read_16_immediate()); break;
			case AXI:  sprintf(ptr, "($%04x+X)", read_16_immediate()); break;
			case N0:   sprintf(ptr, "0"); break;
			case N1:   sprintf(ptr, "1"); break;
			case N2:   sprintf(ptr, "2"); break;
			case N3:   sprintf(ptr, "3"); break;
			case N4:   sprintf(ptr, "4"); break;
			case N5:   sprintf(ptr, "5"); break;
			case N6:   sprintf(ptr, "6"); break;
			case N7:   sprintf(ptr, "7"); break;
			case N8:   sprintf(ptr, "8"); break;
			case N9:   sprintf(ptr, "9"); break;
			case N10:  sprintf(ptr, "10"); break;
			case N11:  sprintf(ptr, "11"); break;
			case N12:  sprintf(ptr, "12"); break;
			case N13:  sprintf(ptr, "13"); break;
			case N14:  sprintf(ptr, "14"); break;
			case N15:  sprintf(ptr, "15"); break;
			case DP0:  sprintf(ptr, "$%02x.0", read_8_immediate()); break;
			case DP1:  sprintf(ptr, "$%02x.1", read_8_immediate()); break;
			case DP2:  sprintf(ptr, "$%02x.2", read_8_immediate()); break;
			case DP3:  sprintf(ptr, "$%02x.3", read_8_immediate()); break;
			case DP4:  sprintf(ptr, "$%02x.4", read_8_immediate()); break;
			case DP5:  sprintf(ptr, "$%02x.5", read_8_immediate()); break;
			case DP6:  sprintf(ptr, "$%02x.6", read_8_immediate()); break;
			case DP7:  sprintf(ptr, "$%02x.7", read_8_immediate()); break;
			case MEMN:
				var = read_16_immediate();
				sprintf(ptr, "%04x.%d", var&0x1fff, var>>13);
				break;
			case MEMI:
				var = read_16_immediate();
				sprintf(ptr, "/%04x.%d", var&0x1fff, var>>13);
				break;
		}
		ptr += strlen(ptr);
	}
	return (g_pc - pc) | flags | DASMFLAG_SUPPORTED;
}
