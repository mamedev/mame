// license:BSD-3-Clause
// copyright-holders:Peter Trauner
/*****************************************************************************
 *
 *   scdasm.c
 *   portable sharp 61860 emulator interface
 *   (sharp pocket computers)
 *
 *   Copyright Peter Trauner, all rights reserved.
 *
 *****************************************************************************/

#include "emu.h"
#include "debugger.h"

#include "sc61860.h"

/*
  new:
  clra 0x23
  nopt 0x33
  nopt 0x68
  nopt 0x6a
  rz n 0x72, 0x73, 0x76, 0x77
  tsma 0xc6
  nopw 0xcd
  nopw 0xd3
  sz n 0xd7
  nopw 0xda

!  writ 211 nopw?
*/

/* explanations for the sharp mnemonics
   d data: external memory
   m memory: internal memory (address in p register)
   p register (internal memory address)
   q register (internal memory address), internally used in several opcodes!?
   r stack pointer (internal memory)
   c carry flag
   z zero flag

   the following are special internal memory registers
   a akkumulator: (2)
   b b register: (3)
   i,j,k,l,v,w counter
   x external memory address
   y external memory address
   ia input/output (92)
   ib input/output (93)
   f0 output (94)
   c output(95)

   li load immediate
   ld load accu
   st store accu
   or
   an and
   inc
   dec
   ad add
   sb sub
   cp compare
   jr jump relativ
   jp jump absolut
   mv move
   ex exchange
*/


enum Adr
{
	Ill,
	Imp,
	Imm, ImmW,
	RelP, RelM,
	Abs,
	Ptc,
	Etc,
	Cal,
	Lp
};

static const struct { const char *mnemonic; Adr adr; } table[]={
	{ "LII",    Imm }, { "LIJ",     Imm }, { "LIA",     Imm }, { "LIB",     Imm },
	{ "IX",     Imp }, { "DX",      Imp }, { "IY",      Imp }, { "DY",      Imp },
	{ "MVW",    Imp }, { "EXW",     Imp }, { "MVB",     Imp }, { "EXB",     Imp },
	{ "ADN",    Imp }, { "SBN",     Imp }, { "ADW",     Imp }, { "SBW",     Imp },

	{ "LIDP",   ImmW}, { "LIDL",    Imm }, { "LIP",     Imm }, { "LIQ",     Imm },
	{ "ADB",    Imp }, { "SBB",     Imp }, { "LIDP",    ImmW}, { "LIDL",    Imm },
	{ "MVWD",   Imp }, { "EXWD",    Imp }, { "MVBD",    Imp }, { "EXBD",    Imp },
	{ "SRW",    Imp }, { "SLW",     Imp }, { "FILM",    Imp }, { "FILD",    Imp },

	{ "LDP",    Imp }, { "LPQ",     Imp }, { "LPR",     Imp }, { nullptr,         Ill },
	{ "IXL",    Imp }, { "DXL",     Imp }, { "IYS",     Imp }, { "DYS",     Imp },
	{ "JRNZP",  RelP}, { "JRNZM",   RelM}, { "JRNCP",   RelP}, { "JRNCM",   RelM},
	{ "JRP",    RelP}, { "JRM",     RelM}, { nullptr,         Ill }, { "LOOP",    RelM},

	{ "STP",    Imp }, { "STQ",     Imp }, { "STR",     Imp }, { nullptr,         Ill },
	{ "PUSH",   Imp }, { "DATA",    Imp }, { nullptr,         Ill }, { "RTN",     Imp },
	{ "JRZP",   RelP}, { "JRZM",    RelM}, { "JRCP",    RelP}, { "JRCM",    RelM},
	{ nullptr,        Ill }, { nullptr,         Ill }, { nullptr,         Ill }, { nullptr,         Ill },

	{ "INCI",   Imp }, { "DECI",    Imp }, { "INCA",    Imp }, { "DECA",    Imp },
	{ "ADM",    Imp }, { "SBM",     Imp }, { "ANMA",    Imp }, { "ORMA",    Imp },
	{ "INCK",   Imp }, { "DECK",    Imp }, { "INCV",    Imp }, { "DECV",    Imp },
	{ "INA",    Imp }, { "NOPW",    Imp }, { "WAIT",    Imm }, { "IPXL"/*CDN, lxn*/,            Imp },

	{ "INCP",   Imp }, { "DECP",    Imp }, { "STD",     Imp }, { "MVDM",    Imp },
	{ "READM",/*mvmp*/  Imp }, { "MVMD",    Imp }, { "READ"/*ldpc*/,    Imp }, { "LDD",     Imp },
	{ "SWP",    Imp }, { "LDM",     Imp }, { "SL",      Imp }, { "POP",     Imp },
	{ nullptr,        Ill }, { "OUTA",    Imp }, { nullptr,         Ill }, { "OUTF",    Imp },

	{ "ANIM",   Imm }, { "ORIM",    Imm }, { "TSIM",    Imm }, { "CPIM",    Imm },
	{ "ANIA",   Imm }, { "ORIA",    Imm }, { "TSIA",    Imm }, { "CPIA",    Imm },
	{ nullptr,        Ill }, { "ETC",     Etc }, { nullptr,         Ill }, { "TEST",    Imm },
	{ nullptr,        Ill }, { nullptr,         Ill }, { nullptr,         Ill }, { "IPXH"/*CDN,lxp*/, Imp },

	{ "ADIM",   Imm }, { "SBIM",    Imm }, { nullptr,         Ill }, { nullptr,         Ill },
	{ "ADIA",   Imm }, { "SBIA",    Imm }, { nullptr,         Ill }, { nullptr,         Ill },
	{ "CALL",   Abs }, { "JP",      Abs }, { "PTC",     Ptc }, { nullptr,         Ill },
	{ "JPNZ",   Abs }, { "JPNC",    Abs }, { "JPZ",     Abs }, { "JPC",     Abs },


	{ "LP", Lp  }, { nullptr }, { nullptr }, { nullptr },  { nullptr }, { nullptr }, { nullptr }, { nullptr },
	{ nullptr }, { nullptr }, { nullptr }, { nullptr },  { nullptr }, { nullptr }, { nullptr }, { nullptr },

	{ nullptr }, { nullptr }, { nullptr }, { nullptr },  { nullptr }, { nullptr }, { nullptr }, { nullptr },
	{ nullptr }, { nullptr }, { nullptr }, { nullptr },  { nullptr }, { nullptr }, { nullptr }, { nullptr },

	{ nullptr }, { nullptr }, { nullptr }, { nullptr },  { nullptr }, { nullptr }, { nullptr }, { nullptr },
	{ nullptr }, { nullptr }, { nullptr }, { nullptr },  { nullptr }, { nullptr }, { nullptr }, { nullptr },

	{ nullptr }, { nullptr }, { nullptr }, { nullptr },  { nullptr }, { nullptr }, { nullptr }, { nullptr },
	{ nullptr }, { nullptr }, { nullptr }, { nullptr },  { nullptr }, { nullptr }, { nullptr }, { nullptr },

	{ "INCJ",   Imp }, { "DECJ",    Imp }, { "INCB",    Imp }, { "DECB",    Imp },
	{ "ACDM",   Imp }, { "SBCM",    Imp }, { nullptr,         Ill }, { "CPMA",    Imp },
	{ "INCL",   Imp }, { "DECL",    Imp }, { "INCW",    Imp }, { "DECW",    Imp },
	{ "INB",    Imp }, { nullptr,         Ill }, { "NOPT",    Imp }, { nullptr,         Ill },

	{ "SC",     Imp }, { "RC",      Imp }, { "SR",      Imp }, { nullptr,         Ill },
	{ "ANID",   Imm }, { "ORID",    Imm }, { "TSID",    Imm }, { nullptr,         Ill },
	{ "LEAVE",  Imp }, { nullptr,         Ill }, { "EXAB",    Imp }, { "EXAM",    Imp },
	{ nullptr,        Ill }, { "OUTB",    Imp }, { nullptr,         Ill }, { "OUTC",    Imp },

	{ "CAL", Imp }, { nullptr }, { nullptr }, { nullptr },  { nullptr }, { nullptr }, { nullptr }, { nullptr },
	{ nullptr }, { nullptr }, { nullptr }, { nullptr },  { nullptr }, { nullptr }, { nullptr }, { nullptr },

	{ nullptr }, { nullptr }, { nullptr }, { nullptr },  { nullptr }, { nullptr }, { nullptr }, { nullptr },
	{ nullptr }, { nullptr }, { nullptr }, { nullptr },  { nullptr }, { nullptr }, { nullptr }, { nullptr },
};

CPU_DISASSEMBLE( sc61860 )
{
	const UINT8 *base_oprom = oprom;
	int oper=*(oprom++);
	int t;
	UINT16 adr;

	switch(oper&0xc0) {
	case 0x80:
		sprintf(buffer,"%-6s%.2x",table[oper&0x80].mnemonic, oper&0x3f);
		break;
	default:
		switch(oper&0xe0) {
		case 0xe0:
			sprintf(buffer,"%-6s%.4x",table[oper&0xe0].mnemonic,
					*(oprom++)|((oper&0x1f)<<8));
			break;
		default:
			switch (table[oper].adr) {
			case Ill: sprintf(buffer,"?%.2x",oper);break;
			case Imp: sprintf(buffer,"%s",table[oper].mnemonic); break;
			case Imm: sprintf(buffer,"%-6s%.2x",table[oper].mnemonic, *(oprom++)); break;
			case ImmW:
				adr=(oprom[0]<<8)|oprom[1];oprom+=2;
				sprintf(buffer,"%-6s%.4x",table[oper].mnemonic, adr);
				break;
			case Abs:
				adr=(oprom[0]<<8)|oprom[1];oprom+=2;
				sprintf(buffer,"%-6s%.4x",table[oper].mnemonic, adr);
				break;
			case RelM:
				adr=pc-*(oprom++);
				sprintf(buffer,"%-6s%.4x",table[oper].mnemonic, adr&0xffff);
				break;
			case RelP:
				adr=pc+*(oprom++);
				sprintf(buffer,"%-6s%.4x",table[oper].mnemonic, adr&0xffff);
				break;
			case Ptc:
				t=*(oprom++);
				adr=(oprom[0]<<8)|oprom[1];oprom+=2;
				sprintf(buffer,"%-6s%.2x,%.4x",table[oper].mnemonic,t, adr);
				break;
			case Etc:
				sprintf(buffer,"%-6s",table[oper].mnemonic);
				/*H imm, abs */
				/* abs */
				break;
			case Cal: case Lp: break;
			}
			break;
		}
		break;
	}
	return oprom - base_oprom;
}
