/*****************************************************************************
 *
 *   disasm.c
 *   portable lh5801 emulator interface
 *
 *   Copyright (c) 2000 Peter Trauner, all rights reserved.
 *
 *   - This source code is released as freeware for non-commercial purposes.
 *   - You are free to use and redistribute this code in modified or
 *     unmodified form, provided you list me in the credits.
 *   - If you modify this source code, you must add a notice to each modified
 *     source file that it has been changed.  If you're a nice person, you
 *     will clearly mark each change too.  :)
 *   - If you wish to use this for commercial purposes, please contact me at
 *     peter.trauner@jk.uni-linz.ac.at
 *   - The author of this copywritten work reserves the right to change the
 *     terms of its usage and license at any time, including retroactively
 *   - This entire notice must remain in the source code.
 *
 *****************************************************************************/

#include "cpuintrf.h"
#include "debugger.h"

#include "lh5801.h"

typedef enum {
	Imp,
	Reg,
	Vec, // imm byte (vector at 0xffxx)
	Vej,
	Imm,
	RegImm,
	Imm16,
	RegImm16,
	ME0,
	ME0Imm,
	Abs,
	AbsImm,
	ME1,
	ME1Imm,
	ME1Abs,
	ME1AbsImm,
	RelP,
	RelM
} Adr;

typedef enum {
	RegNone,
	A,
	XL, XH, X,
	YL, YH, Y,
	UL, UH, U,
	P, S
} Regs;

static const char *RegNames[]= {
	0, "A", "XL", "XH", "X", "YL", "YH", "Y", "UL", "UH", "U", "P", "S"
};

typedef enum {
	ILL, ILL2, PREFD, NOP,

	LDA, STA, LDI, LDX, STX,
	LDE, SDE, LIN, SIN,
	TIN, // (x++)->(y++)
	ADC, ADI, ADR, SBC, SBI,
	DCA, DCS, // bcd add and sub
	CPA, CPI, CIN, // A compared with (x++)
	AND, ANI, ORA, ORI, EOR, EAI, BIT, BII,
	INC, DEC,
	DRL, DRR, // digit rotates
	ROL, ROR,
	SHL, SHR,
	AEX, // A nibble swap

	BCR, BCS, BHR, BHS, BZR, BZS, BVR, BVS,
	BCH, LOP, // loop with ul
	JMP, SJP, RTN, RTI, HLT,
	VCR, VCS, VHR, VHS, VVS, VZR, VZS,
	VMJ, VEJ,
	PSH, POP, ATT, TTA,
	REC, SEC, RIE, SIE,

	AM0, AM1, // load timer reg
	ITA, // reads input port
	ATP, // akku send to data bus
	CDV, // clears internal divider
	OFF, // clears bf flip flop
	RDP, SDP,// reset display flip flop
	RPU, SPU,// flip flop pu off
	RPV, SPV,// flip flop pv off
} Ins;

static const char *InsNames[]={
	"ILL", "ILL", 0, "NOP",
	"LDA", "STA", "LDI", "LDX", "STX",
	"LDE", "SDE", "LIN", "SIN",
	"TIN",
	"ADC", "ADI", "ADR", "SBC", "SBI",
	"DCA", "DCS",
	"CPA", "CPI", "CIN",
	"AND", "ANI", "ORA", "ORI", "EOR", "EAI", "BIT", "BII",
	"INC", "DEC",
	"DRL", "DRR",
	"ROL", "ROR",
	"SHL", "SHR",
	"AEX",
	"BCR", "BCS", "BHR", "BHS", "BZR", "BZS", "BVR", "BVS",
	"BCH", "LOP",
	"JMP", "SJP", "RTN", "RTI", "HLT",
	"VCR", "VCS", "VHR", "VHS", "VVS", "VZR", "VZS",
	"VMJ", "VEJ",
	"PSH", "POP", "ATT", "TTA",
	"REC", "SEC", "RIE", "SIE",

	"AM0", "AM1",
	"ITA",
	"ATP",
	"CDV",
	"OFF",
	"RDP", "SDP",
	"RPU", "SPU",
	"RPV", "SPV",
};

typedef struct { Ins ins; Adr adr; Regs reg; } Entry;

static const Entry table[0x100]={
	{ SBC, Reg, XL }, // 0
	{ SBC, ME0, X },
	{ ADC, Reg, XL },
	{ ADC, ME0, X },
	{ LDA, Reg, XL },
	{ LDA, ME0, X },
	{ CPA, Reg, XL },
	{ CPA, ME0, X },
	{ STA, Reg, XH },
	{ AND, ME0, X },
	{ STA, Reg, XL },
	{ ORA, ME0, X },
	{ DCS, ME0, X },
	{ EOR, ME0, X },
	{ STA, ME0, X },
	{ BIT, ME0, X },
	{ SBC, Reg, YL }, // 0x10
	{ SBC, ME0, Y },
	{ ADC, Reg, YL },
	{ ADC, ME0, Y },
	{ LDA, Reg, YL },
	{ LDA, ME0, Y },
	{ CPA, Reg, YL },
	{ CPA, ME0, Y },
	{ STA, Reg, YH },
	{ AND, ME0, Y },
	{ STA, Reg, YL },
	{ ORA, ME0, Y },
	{ DCS, ME0, Y },
	{ EOR, ME0, Y },
	{ STA, ME0, Y },
	{ BIT, ME0, Y },
	{ SBC, Reg, UL }, // 0x20
	{ SBC, ME0, U },
	{ ADC, Reg, UL },
	{ ADC, ME0, U },
	{ LDA, Reg, UL },
	{ LDA, ME0, U },
	{ CPA, Reg, UL },
	{ CPA, ME0, U },
	{ STA, Reg, UH },
	{ AND, ME0, U },
	{ STA, Reg, UL },
	{ ORA, ME0, U },
	{ DCS, ME0, U },
	{ EOR, ME0, U },
	{ STA, ME0, U },
	{ BIT, ME0, U },
	{ ILL }, // 0x30
	{ ILL },
	{ ILL },
	{ ILL },
	{ ILL },
	{ ILL },
	{ ILL },
	{ ILL },
	{ NOP, Imp },
	{ ILL },
	{ ILL },
	{ ILL },
	{ ILL },
	{ ILL },
	{ ILL },
	{ ILL },
	{ INC, Reg, XL }, //0x40
	{ SIN, Reg, X },
	{ DEC, Reg, XL },
	{ SDE, Reg, X },
	{ INC, Reg, X },
	{ LIN, Reg, X },
	{ DEC, Reg, X },
	{ LDE, Reg, X },
	{ LDI, RegImm, XH },
	{ ANI, ME0Imm, X },
	{ LDI, RegImm, XL },
	{ ORI, ME0Imm, X },
	{ CPI, RegImm, XH },
	{ BII, ME0Imm, X },
	{ CPI, RegImm, XL },
	{ ADI, ME0Imm, X },
	{ INC, Reg, YL }, //0x50
	{ SIN, Reg, Y },
	{ DEC, Reg, YL },
	{ SDE, Reg, Y },
	{ INC, Reg, Y },
	{ LIN, Reg, Y },
	{ DEC, Reg, Y },
	{ LDE, Reg, Y },
	{ LDI, RegImm, YH },
	{ ANI, ME0Imm, Y },
	{ LDI, RegImm, YL },
	{ ORI, ME0Imm, Y },
	{ CPI, RegImm, YH },
	{ BII, ME0Imm, Y },
	{ CPI, RegImm, YL },
	{ ADI, ME0Imm, Y },
	{ INC, Reg, UL }, //0x60
	{ SIN, Reg, U },
	{ DEC, Reg, UL },
	{ SDE, Reg, U },
	{ INC, Reg, U },
	{ LIN, Reg, U },
	{ DEC, Reg, U },
	{ LDE, Reg, U },
	{ LDI, RegImm, UH },
	{ ANI, ME0Imm, U },
	{ LDI, RegImm, UL },
	{ ORI, ME0Imm, U },
	{ CPI, RegImm, UH },
	{ BII, ME0Imm, U },
	{ CPI, RegImm, UL },
	{ ADI, ME0Imm, U },
	{ ILL }, // 0X70
	{ ILL },
	{ ILL },
	{ ILL },
	{ ILL },
	{ ILL },
	{ ILL },
	{ ILL },
	{ ILL },
	{ ILL },
	{ ILL },
	{ ILL },
	{ ILL },
	{ ILL },
	{ ILL },
	{ ILL },
	{ SBC, Reg, XH }, // 0x80
	{ BCR, RelP },
	{ ADC, Reg, XH },
	{ BCS, RelP },
	{ LDA, Reg, XH },
	{ BHR, RelP },
	{ CPA, Reg, XH },
	{ BHS, RelP },
	{ LOP, RelM },
	{ BZR, RelP },
	{ RTI, Imp },
	{ BZS, RelP },
	{ DCA, ME0, X },
	{ BVR, RelP },
	{ BCH, RelP },
	{ BVS, RelP },
	{ SBC, Reg, YH }, // 0x90
	{ BCR, RelM },
	{ ADC, Reg, YH },
	{ BCS, RelM },
	{ LDA, Reg, YH },
	{ BHR, RelM },
	{ CPA, Reg, YH },
	{ BHS, RelM },
	{ ILL },
	{ BZR, RelM },
	{ RTN, Imp },
	{ BZS, RelM },
	{ DCA, ME0, Y },
	{ BVR, RelM },
	{ BCH, RelM },
	{ BVS, RelM },
	{ SBC, Reg, UH }, // 0xa0
	{ SBC, Abs },
	{ ADC, Reg, UH },
	{ ADC, Abs },
	{ LDA, Reg, UH },
	{ LDA, Abs },
	{ CPA, Reg, UH },
	{ CPA, Abs },
	{ SPV, Imp },
	{ AND, Abs },
	{ LDI, RegImm16, S },
	{ ORA, Abs },
	{ DCA, ME0, U },
	{ EOR, Abs },
	{ STA, Abs },
	{ BIT, Abs },
	{ ILL }, //0xb0
	{ SBI },
	{ ILL },
	{ ADI, RegImm, A },
	{ ILL },
	{ LDI, RegImm, A },
	{ ILL },
	{ CPI, RegImm, A },
	{ RPV, Imp },
	{ ANI, RegImm, A },
	{ JMP, Imm16 },
	{ ORI, RegImm, A },
	{ ILL },
	{ EAI, Imm },
	{ SJP, Imm16 },
	{ BII, RegImm, A },
	{ VEJ, Vej }, // 0xc0
	{ VCR, Vec },
	{ VEJ, Vej },
	{ VCS, Vec },
	{ VEJ, Vej },
	{ VHR, Vec },
	{ VEJ, Vej },
	{ VHS, Vec },
	{ VEJ, Vej },
	{ VZR, Vec },
	{ VEJ, Vej },
	{ VZS, Vec },
	{ VEJ, Vej },
	{ VMJ, Vec },
	{ VEJ, Vej },
	{ VVS, Vec },
	{ VEJ, Vej }, // 0xd0
	{ ROR, Imp },
	{ VEJ, Vej },
	{ DRR, Imp },
	{ VEJ, Vej },
	{ SHR, Imp },
	{ VEJ, Vej },
	{ DRL, Imp },
	{ VEJ, Vej },
	{ SHL, Imp },
	{ VEJ, Vej },
	{ ROL, Imp },
	{ VEJ, Vej },
	{ INC, Reg, A },
	{ VEJ, Vej },
	{ DEC, Reg, A },
	{ VEJ, Vej }, //0xe0
	{ SPU, Imp },
	{ VEJ, Vej },
	{ RPU, Imp },
	{ VEJ, Vej },
	{ ILL },
	{ VEJ, Vej },
	{ ILL },
	{ VEJ, Vej },
	{ ANI, AbsImm },
	{ VEJ, Vej },
	{ ORI, AbsImm },
	{ VEJ, Vej },
	{ BII, AbsImm },
	{ VEJ, Vej },
	{ ADI, AbsImm },
	{ VEJ, Vej }, //0xf0
	{ AEX, Imp },
	{ VEJ, Vej },
	{ ILL },
	{ VEJ, Vej },
	{ TIN, Imp },
	{ VEJ, Vej },
	{ CIN, Imp },
	{ ILL },
	{ REC, Imp },
	{ ILL },
	{ SEC, Imp },
	{ ILL },
	{ PREFD },
	{ ILL },
	{ ILL }
};
static const Entry table_fd[0x100]={
	{ ILL2 }, // 0x00
	{ SBC, ME1, X },
	{ ILL2 },
	{ ADC, ME1, X },
	{ ILL2 },
	{ LDA, ME1, X },
	{ ILL2 },
	{ CPA, ME1, X },
	{ LDX, Reg, X },
	{ AND, ME1, X },
	{ POP, Reg, X },
	{ ORA, ME1, X },
	{ DCS, ME1, X },
	{ EOR, ME1, X },
	{ STA, ME1, X },
	{ BIT, ME1, X },
	{ ILL2 }, // 0x10
	{ SBC, ME1, Y },
	{ ILL2 },
	{ ADC, ME1, Y },
	{ ILL2 },
	{ LDA, ME1, Y },
	{ ILL2 },
	{ CPA, ME1, Y },
	{ LDX, Reg, Y },
	{ AND, ME1, Y },
	{ POP, Reg, Y },
	{ ORA, ME1, Y },
	{ DCS, ME1, Y },
	{ EOR, ME1, Y },
	{ STA, ME1, Y },
	{ BIT, ME1, Y },
	{ ILL2 }, // 0x20
	{ SBC, ME1, U },
	{ ILL2 },
	{ ADC, ME1, U },
	{ ILL2 },
	{ LDA, ME1, U },
	{ ILL2 },
	{ CPA, ME1, U },
	{ LDX, Reg, U },
	{ AND, ME1, U },
	{ POP, Reg, U },
	{ ORA, ME1, U },
	{ DCS, ME1, U },
	{ EOR, ME1, U },
	{ STA, ME1, U },
	{ BIT, ME1, U },
	{ ILL2 }, // 0x30
	{ ILL2 },
	{ ILL2 },
	{ ILL2 },
	{ ILL2 },
	{ ILL2 },
	{ ILL2 },
	{ ILL2 },
	{ ILL2 },
	{ ILL2 },
	{ ILL2 },
	{ ILL2 },
	{ ILL2 },
	{ ILL2 },
	{ ILL2 },
	{ ILL2 },
	{ INC, Reg, XH }, // 0x40
	{ ILL2 },
	{ DEC, Reg, XH }, //46 in other part of manual
	{ ILL2 },
	{ ILL2 },
	{ ILL2 },
	{ ILL2 },
	{ ILL2 },
	{ LDX, Reg, S },
	{ ANI, ME1Imm, X },
	{ STX, Reg, X },
	{ ORI, ME1Imm, X },
	{ OFF, Imp },
	{ BII, ME1Imm, X },
	{ STX, Reg, S },
	{ ADI, ME1Imm, X },
	{ INC, Reg, YH }, // 0x50
	{ ILL2 },
	{ DEC, Reg, YH }, // 56 in other part of manual
	{ ILL2 },
	{ ILL2 },
	{ ILL2 },
	{ ILL2 },
	{ ILL2 },
	{ LDX, Reg, P },
	{ ANI, ME1Imm, Y },
	{ STX, Reg, Y },
	{ ORI, ME1Imm, Y },
	{ ILL2 },
	{ BII, ME1Imm, Y },
	{ STX, Reg, P },
	{ ADI, ME1Imm, Y },
	{ INC, Reg, UH }, // 0x60
	{ ILL2 },
	{ DEC, Reg, UH }, // 66 in other part of manual
	{ ILL2 },
	{ ILL2 },
	{ ILL2 },
	{ ILL2 },
	{ ILL2 },
	{ ILL2 },
	{ ANI, ME1Imm, U },
	{ STX, Reg, U },
	{ ORI, ME1Imm, U },
	{ ILL2 },
	{ BII, ME1Imm, U },
	{ ILL  },
	{ ADI, ME1Imm, U },
	{ ILL2 }, // 0x70
	{ ILL2 },
	{ ILL2 },
	{ ILL2 },
	{ ILL2 },
	{ ILL2 },
	{ ILL2 },
	{ ILL2 },
	{ ILL2 },
	{ ILL2 },
	{ ILL2 },
	{ ILL2 },
	{ ILL2 },
	{ ILL2 },
	{ ILL2 },
	{ ILL2 },
	{ ILL2 }, // 0x80
	{ SIE, Imp },
	{ ILL2 },
	{ ILL2 },
	{ ILL2 },
	{ ILL2 },
	{ ILL2 },
	{ ILL2 },
	{ PSH, Reg, X },
	{ ILL2 },
	{ POP, Reg, A },
	{ ILL2 },
	{ DCA, ME1, X },
	{ ILL2 },
	{ CDV, Imp },
	{ ILL2 },
	{ ILL2 }, // 0x90
	{ ILL2 },
	{ ILL2 },
	{ ILL2 },
	{ ILL2 },
	{ ILL2 },
	{ ILL2 },
	{ ILL2 },
	{ PSH, Reg, Y },
	{ ILL2 },
	{ ILL2 },
	{ ILL2 },
	{ DCA, ME1, Y },
	{ ILL2 },
	{ ILL2 },
	{ ILL2 },
	{ ILL2 }, // 0xa0
	{ SBC, ME1Abs },
	{ ILL2 },
	{ ADC, ME1Abs },
	{ ILL2 },
	{ LDA, ME1Abs },
	{ ILL2 },
	{ CPA, ME1Abs },
	{ PSH, Reg, U },
	{ AND, ME1Abs },
	{ TTA, Imp },
	{ ORA, ME1Abs },
	{ DCA, ME1, U },
	{ EOR, ME1Abs },
	{ STA, ME1Abs },
	{ BIT, ME1Abs },
	{ ILL2 }, // 0xb0
	{ HLT, Imp },
	{ ILL2 },
	{ ILL2 },
	{ ILL2 },
	{ ILL2 },
	{ ILL2 },
	{ ILL2 },
	{ ILL2 },
	{ ILL2 },
	{ ITA, Imp },
	{ ILL2 },
	{ ILL2 },
	{ ILL2 },
	{ RIE, Imp },
	{ ILL2 },
	{ RDP, Imp }, // 0xc0
	{ SDP, Imp },
	{ ILL2 },
	{ ILL2 },
	{ ILL2 },
	{ ILL2 },
	{ ILL2 },
	{ ILL2 },
	{ PSH, Reg, A },
	{ ILL2 },
	{ ADR, Reg, X },
	{ ILL2 },
	{ ATP, Imp },
	{ ILL2 },
	{ AM0, Imp },
	{ ILL2 },
	{ ILL2 }, // 0xd0
	{ ILL2 },
	{ ILL2 },
	{ DRR, ME1, X },
	{ ILL2 },
	{ ILL2 },
	{ ILL2 },
	{ DRL, ME1, X },
	{ ILL2 },
	{ ILL2 },
	{ ADR, Reg, Y },
	{ ILL2 },
	{ ILL2 },
	{ ILL2 },
	{ AM1, Imp },
	{ ILL2 },
	{ ILL2 }, // 0xe0
	{ ILL2 },
	{ ILL2 },
	{ ILL2 },
	{ ILL2 },
	{ ILL2 },
	{ ILL2 },
	{ ILL2 },
	{ ILL2 },
	{ ANI, ME1AbsImm },
	{ ADR, Reg, U },
	{ ORI, ME1AbsImm },
	{ ATT, Imp },
	{ BII, ME1AbsImm },
	{ ILL2 },
	{ ADI, ME1AbsImm },
	{ ILL2 }, // 0xf0
	{ ILL2 },
	{ ILL2 },
	{ ILL2 },
	{ ILL2 },
	{ ILL2 },
	{ ILL2 },
	{ ILL2 },
	{ ILL2 },
	{ ILL2 },
	{ ILL2 },
	{ ILL2 },
	{ ILL2 },
	{ ILL2 },
	{ ILL2 },
	{ ILL2 }
};

offs_t lh5801_dasm(char *dst, offs_t pc, const UINT8 *oprom, const UINT8 *opram)
{
	int pos = 0;
	int oper;
	UINT16 absolut;
	const Entry *entry;
	int temp;

	oper=oprom[pos++];
	entry=table+oper;

	if (table[oper].ins==PREFD) {
		oper=oprom[pos++];
		entry=table_fd+oper;
	}
	switch (entry->ins) {
	case ILL:
		sprintf(dst,"%s %.2x", InsNames[entry->ins], oper);break;
	case ILL2:
		sprintf(dst,"%s fd%.2x", InsNames[entry->ins], oper);break;
	default:
		switch(entry->adr) {
		case Imp:
			sprintf(dst,"%s", InsNames[entry->ins]);break;
		case Reg:
			sprintf(dst,"%s %s", InsNames[entry->ins],RegNames[entry->reg]);break;
		case RegImm:
			sprintf(dst,"%s %s,%.2x", InsNames[entry->ins],
					RegNames[entry->reg], oprom[pos++]);
			break;
		case RegImm16:
			absolut=oprom[pos++]<<8;
			absolut|=oprom[pos++];
			sprintf(dst,"%s %s,%.4x", InsNames[entry->ins],RegNames[entry->reg],absolut );
			break;
		case Vec:
			sprintf(dst,"%s (ff%.2x)", InsNames[entry->ins],oprom[pos++]);break;
		case Vej:
			sprintf(dst,"%s (ff%.2x)", InsNames[entry->ins], oper);break;
		case Imm:
			sprintf(dst,"%s %.2x", InsNames[entry->ins],oprom[pos++]);break;
		case Imm16:
			absolut=oprom[pos++]<<8;
			absolut|=oprom[pos++];
			sprintf(dst,"%s %.4x", InsNames[entry->ins],absolut );break;
		case RelP:
			temp=oprom[pos++];
			sprintf(dst,"%s %.4x", InsNames[entry->ins],pc+pos+temp );break;
		case RelM:
			temp=oprom[pos++];
			sprintf(dst,"%s %.4x", InsNames[entry->ins],pc+pos-temp );break;
		case Abs:
			absolut=oprom[pos++]<<8;
			absolut|=oprom[pos++];
			sprintf(dst,"%s (%.4x)", InsNames[entry->ins],absolut );break;
		case ME1Abs:
			absolut=oprom[pos++]<<8;
			absolut|=oprom[pos++];
			sprintf(dst,"%s #(%.4x)", InsNames[entry->ins],absolut );break;
		case AbsImm:
			absolut=oprom[pos++]<<8;
			absolut|=oprom[pos++];
			sprintf(dst,"%s (%.4x),%.2x", InsNames[entry->ins],absolut,
					oprom[pos++]);break;
		case ME1AbsImm:
			absolut=oprom[pos++]<<8;
			absolut|=oprom[pos++];
			sprintf(dst,"%s #(%.4x),%.2x", InsNames[entry->ins],absolut,
					oprom[pos++]);break;
		case ME0:
			sprintf(dst,"%s (%s)", InsNames[entry->ins],RegNames[entry->reg] );break;
		case ME0Imm:
			sprintf(dst,"%s (%s),%.2x", InsNames[entry->ins],RegNames[entry->reg],oprom[pos++] );
			break;
		case ME1:
			sprintf(dst,"%s #(%s)", InsNames[entry->ins],RegNames[entry->reg] );break;
		case ME1Imm:
			sprintf(dst,"%s #(%s),%.2x", InsNames[entry->ins],RegNames[entry->reg],oprom[pos++] );
			break;
		}
	}

	return pos;
}
