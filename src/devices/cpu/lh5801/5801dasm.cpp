// license:BSD-3-Clause
// copyright-holders:Peter Trauner
/*****************************************************************************
 *
 *   disasm.c
 *   portable lh5801 emulator interface
 *
 *
 *****************************************************************************/

#include "emu.h"
#include "5801dasm.h"

#if defined(SEC)
#undef SEC
#endif


const char *const lh5801_disassembler::ins_names[]={
	"ILL", "ILL", nullptr, "NOP",
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

const char *const lh5801_disassembler::reg_names[]= {
	nullptr, "A", "XL", "XH", "X", "YL", "YH", "Y", "UL", "UH", "U", "P", "S"
};

const lh5801_disassembler::Entry lh5801_disassembler::table[0x100]={
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

const lh5801_disassembler::Entry lh5801_disassembler::table_fd[0x100]={
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


u32 lh5801_disassembler::opcode_alignment() const
{
	return 1;
}

offs_t lh5801_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	offs_t pos = pc;
	int oper;
	uint16_t absolut;
	const Entry *entry;
	int temp;

	oper=opcodes.r8(pos++);
	entry=table+oper;

	if (table[oper].ins==PREFD) {
		oper=opcodes.r8(pos++);
		entry=table_fd+oper;
	}
	switch (entry->ins) {
	case ILL:
		util::stream_format(stream, "%s %02x", entry->ins_name(), oper);break;
	case ILL2:
		util::stream_format(stream, "%s fd%02x", entry->ins_name(), oper);break;
	default:
		switch(entry->adr) {
		case Imp:
			util::stream_format(stream, "%s", entry->ins_name());break;
		case Reg:
			util::stream_format(stream, "%s %s", entry->ins_name(),entry->reg_name());break;
		case RegImm:
			util::stream_format(stream, "%s %s,%02x", entry->ins_name(),
					entry->reg_name(), opcodes.r8(pos++));
			break;
		case RegImm16:
			absolut=opcodes.r8(pos++)<<8;
			absolut|=opcodes.r8(pos++);
			util::stream_format(stream, "%s %s,%04x", entry->ins_name(),entry->reg_name(),absolut );
			break;
		case Vec:
			util::stream_format(stream, "%s (ff%02x)", entry->ins_name(),opcodes.r8(pos++));break;
		case Vej:
			util::stream_format(stream, "%s (ff%02x)", entry->ins_name(), oper);break;
		case Imm:
			util::stream_format(stream, "%s %02x", entry->ins_name(),opcodes.r8(pos++));break;
		case Imm16:
			absolut=opcodes.r8(pos++)<<8;
			absolut|=opcodes.r8(pos++);
			util::stream_format(stream, "%s %04x", entry->ins_name(),absolut);break;
		case RelP:
			temp=opcodes.r8(pos++);
			util::stream_format(stream, "%s %04x", entry->ins_name(),pc+pos+temp);break;
		case RelM:
			temp=opcodes.r8(pos++);
			util::stream_format(stream, "%s %04x", entry->ins_name(),pc+pos-temp);break;
		case Abs:
			absolut=opcodes.r8(pos++)<<8;
			absolut|=opcodes.r8(pos++);
			util::stream_format(stream, "%s (%04x)", entry->ins_name(),absolut);break;
		case ME1Abs:
			absolut=opcodes.r8(pos++)<<8;
			absolut|=opcodes.r8(pos++);
			util::stream_format(stream, "%s #(%04x)", entry->ins_name(),absolut);break;
		case AbsImm:
			absolut=opcodes.r8(pos++)<<8;
			absolut|=opcodes.r8(pos++);
			util::stream_format(stream, "%s (%04x),%02x", entry->ins_name(),absolut,
					opcodes.r8(pos++));break;
		case ME1AbsImm:
			absolut=opcodes.r8(pos++)<<8;
			absolut|=opcodes.r8(pos++);
			util::stream_format(stream, "%s #(%04x),%02x", entry->ins_name(),absolut,
					opcodes.r8(pos++));break;
		case ME0:
			util::stream_format(stream, "%s (%s)", entry->ins_name(),entry->reg_name());break;
		case ME0Imm:
			util::stream_format(stream, "%s (%s),%02x", entry->ins_name(),entry->reg_name(),opcodes.r8(pos++));
			break;
		case ME1:
			util::stream_format(stream, "%s #(%s)", entry->ins_name(),entry->reg_name());break;
		case ME1Imm:
			util::stream_format(stream, "%s #(%s),%02x", entry->ins_name(),entry->reg_name(),opcodes.r8(pos++));
			break;
		}
	}

	return pos - pc;
}
