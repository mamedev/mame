// license:BSD-3-Clause
// copyright-holders:Karl Stenerud
/* ======================================================================== */
/* =============================== COPYRIGHT ============================== */
/* ======================================================================== */
/*

G65C816 CPU Emulator V0.90

Copyright Karl Stenerud

*/

#include "emu.h"
#include "g65816ds.h"

enum class g65816_disassembler::op : unsigned
{
	ADC,  AND,  ASL,  BCC,  BCS,  BEQ,  BIT,  BMI,  BNE,  BPL,  BRA,
	BRK,  BRL,  BVC,  BVS,  CLC,  CLD,  CLI,  CLV,  CMP,  COP,  CPX,
	CPY,  DEA,  DEC,  DEX,  DEY,  EOR,  INA,  INC,  INX,  INY,  JML,
	JMP,  JSL,  JSR,  LDA,  LDX,  LDY,  LSR,  MVN,  MVP,  NOP,  ORA,
	PEA,  PEI,  PER,  PHA,  PHB,  PHD,  PHK,  PHP,  PHX,  PHY,  PLA,
	PLB,  PLD,  PLP,  PLX,  PLY,  REP,  ROL,  ROR,  RTI,  RTL,  RTS,
	SBC,  SEC,  SED,  SEI,  SEP,  STA,  STP,  STX,  STY,  STZ,  TAX,
	TAY,  TCS,  TCD,  TDC,  TRB,  TSB,  TSC,  TSX,  TXA,  TXS,  TXY,
	TYA,  TYX,  WAI,  WDM,  XBA,  XCE
};

class g65816_disassembler::opcode_struct {
	public:
		op m_name;
		u8 flag;
		u8 ea;

		opcode_struct(op n, u8 f, u8 e);
		const char *name() const;
		bool is_call() const;
		bool is_return() const;
		bool is_cond() const;
	};

g65816_disassembler::g65816_disassembler(const config *conf) : m_config(conf)
{
}

u32 g65816_disassembler::opcode_alignment() const
{
	return 1;
}

u32 g65816_disassembler::interface_flags() const
{
	return PAGED;
}

u32 g65816_disassembler::page_address_bits() const
{
	return 16;
}

const char *g65816_disassembler::opcode_struct::name() const
{
	return g65816_disassembler::s_opnames[unsigned(m_name)];
}

bool g65816_disassembler::opcode_struct::is_call() const
{
	switch (m_name)
	{
	case op::JSR:
	case op::JSL:
		return true;
	default:
		return false;
	}
}

bool g65816_disassembler::opcode_struct::is_return() const
{
	switch (m_name)
	{
	case op::RTI:
	case op::RTL:
	case op::RTS:
		return true;
	default:
		return false;
	}
}

bool g65816_disassembler::opcode_struct::is_cond() const
{
	return ea == RELB && m_name != op::BRA;
}

g65816_disassembler::opcode_struct::opcode_struct(op n, u8 f, u8 e) : m_name(n), flag(f), ea(e)
{
}



const char *const g65816_disassembler::s_opnames[] =
{
	"adc", "and", "asl", "bcc", "bcs", "beq", "bit", "bmi", "bne", "bpl", "bra",
	"brk", "brl", "bvc", "bvs", "clc", "cld", "cli", "clv", "cmp", "cop", "cpx",
	"cpy", "dea", "dec", "dex", "dey", "eor", "ina", "inc", "inx", "iny", "jml",
	"jmp", "jsl", "jsr", "lda", "ldx", "ldy", "lsr", "mvn", "mvp", "nop", "ora",
	"pea", "pei", "per", "pha", "phb", "phd", "phk", "php", "phx", "phy", "pla",
	"plb", "pld", "plp", "plx", "ply", "rep", "rol", "ror", "rti", "rtl", "rts",
	"sbc", "sec", "sed", "sei", "sep", "sta", "stp", "stx", "sty", "stz", "tax",
	"tay", "tcs", "tcd", "tdc", "trb", "tsb", "tsc", "tsx", "txa", "txs", "txy",
	"tya", "tyx", "wai", "wdm", "xba", "xce"
};

const g65816_disassembler::opcode_struct g65816_disassembler::s_opcodes[256] =
{
	{op::BRK, I, SIG }, {op::ORA, M, DXI }, {op::COP, I, SIG }, {op::ORA, M, S   },
	{op::TSB, M, D   }, {op::ORA, M, D   }, {op::ASL, M, D   }, {op::ORA, M, DLI },
	{op::PHP, I, IMP }, {op::ORA, M, IMM }, {op::ASL, M, ACC }, {op::PHD, I, IMP },
	{op::TSB, M, A   }, {op::ORA, M, A   }, {op::ASL, M, A   }, {op::ORA, M, AL  },
	{op::BPL, I, RELB}, {op::ORA, M, DIY }, {op::ORA, M, DI  }, {op::ORA, M, SIY },
	{op::TRB, M, D   }, {op::ORA, M, DX  }, {op::ASL, M, DX  }, {op::ORA, M, DLIY},
	{op::CLC, I, IMP }, {op::ORA, M, AY  }, {op::INA, I, IMP }, {op::TCS, I, IMP },
	{op::TRB, M, A   }, {op::ORA, M, AX  }, {op::ASL, M, AX  }, {op::ORA, M, ALX },
	{op::JSR, I, A   }, {op::AND, M, DXI }, {op::JSL, I, AL  }, {op::AND, M, S   },
	{op::BIT, M, D   }, {op::AND, M, D   }, {op::ROL, M, D   }, {op::AND, M, DLI },
	{op::PLP, I, IMP }, {op::AND, M, IMM }, {op::ROL, M, ACC }, {op::PLD, I, IMP },
	{op::BIT, M, A   }, {op::AND, M, A   }, {op::ROL, M, A   }, {op::AND, M, AL  },
	{op::BMI, I, RELB}, {op::AND, M, DIY }, {op::AND, M, DI  }, {op::AND, M, SIY },
	{op::BIT, M, DX  }, {op::AND, M, DX  }, {op::ROL, M, DX  }, {op::AND, M, DLIY},
	{op::SEC, I, IMP }, {op::AND, M, AY  }, {op::DEA, I, IMP }, {op::TSC, I, IMP },
	{op::BIT, M, AX  }, {op::AND, M, AX  }, {op::ROL, M, AX  }, {op::AND, M, ALX },
	{op::RTI, I, IMP }, {op::EOR, M, DXI }, {op::WDM, I, SIG }, {op::EOR, M, S   },
	{op::MVP, I, MVP }, {op::EOR, M, D   }, {op::LSR, M, D   }, {op::EOR, M, DLI },
	{op::PHA, I, IMP }, {op::EOR, M, IMM }, {op::LSR, M, ACC }, {op::PHK, I, IMP },
	{op::JMP, I, A   }, {op::EOR, M, A   }, {op::LSR, M, A   }, {op::EOR, M, AL  },
	{op::BVC, I, RELB}, {op::EOR, M, DIY }, {op::EOR, M, DI  }, {op::EOR, M, SIY },
	{op::MVN, I, MVN }, {op::EOR, M, DX  }, {op::LSR, M, DX  }, {op::EOR, M, DLIY},
	{op::CLI, I, IMP }, {op::EOR, M, AY  }, {op::PHY, I, IMP }, {op::TCD, I, IMP },
	{op::JMP, I, AL  }, {op::EOR, M, AX  }, {op::LSR, M, AX  }, {op::EOR, M, ALX },
	{op::RTS, I, IMP }, {op::ADC, M, DXI }, {op::PER, I, PER }, {op::ADC, M, S   },
	{op::STZ, M, D   }, {op::ADC, M, D   }, {op::ROR, M, D   }, {op::ADC, M, DLI },
	{op::PLA, I, IMP }, {op::ADC, M, IMM }, {op::ROR, M, ACC }, {op::RTL, I, IMP },
	{op::JMP, I, AI  }, {op::ADC, M, A   }, {op::ROR, M, A   }, {op::ADC, M, AL  },
	{op::BVS, I, RELB}, {op::ADC, M, DIY }, {op::ADC, M, DI  }, {op::ADC, M, SIY },
	{op::STZ, M, DX  }, {op::ADC, M, DX  }, {op::ROR, M, DX  }, {op::ADC, M, DLIY},
	{op::SEI, I, IMP }, {op::ADC, M, AY  }, {op::PLY, I, IMP }, {op::TDC, I, IMP },
	{op::JMP, I, AXI }, {op::ADC, M, AX  }, {op::ROR, M, AX  }, {op::ADC, M, ALX },
	{op::BRA, I, RELB}, {op::STA, M, DXI }, {op::BRL, I, RELW}, {op::STA, M, S   },
	{op::STY, X, D   }, {op::STA, M, D   }, {op::STX, X, D   }, {op::STA, M, DLI },
	{op::DEY, I, IMP }, {op::BIT, M, IMM }, {op::TXA, I, IMP }, {op::PHB, I, IMP },
	{op::STY, X, A   }, {op::STA, M, A   }, {op::STX, X, A   }, {op::STA, M, AL  },
	{op::BCC, I, RELB}, {op::STA, M, DIY }, {op::STA, M, DI  }, {op::STA, M, SIY },
	{op::STY, X, DX  }, {op::STA, M, DX  }, {op::STX, X, DY  }, {op::STA, M, DLIY},
	{op::TYA, I, IMP }, {op::STA, M, AY  }, {op::TXS, I, IMP }, {op::TXY, I, IMP },
	{op::STZ, M, A   }, {op::STA, M, AX  }, {op::STZ, M, AX  }, {op::STA, M, ALX },
	{op::LDY, X, IMM }, {op::LDA, M, DXI }, {op::LDX, X, IMM }, {op::LDA, M, S   },
	{op::LDY, X, D   }, {op::LDA, M, D   }, {op::LDX, X, D   }, {op::LDA, M, DLI },
	{op::TAY, I, IMP }, {op::LDA, M, IMM }, {op::TAX, I, IMP }, {op::PLB, I, IMP },
	{op::LDY, X, A   }, {op::LDA, M, A   }, {op::LDX, X, A   }, {op::LDA, M, AL  },
	{op::BCS, I, RELB}, {op::LDA, M, DIY }, {op::LDA, M, DI  }, {op::LDA, M, SIY },
	{op::LDY, X, DX  }, {op::LDA, M, DX  }, {op::LDX, X, DY  }, {op::LDA, M, DLIY},
	{op::CLV, I, IMP }, {op::LDA, M, AY  }, {op::TSX, I, IMP }, {op::TYX, I, IMP },
	{op::LDY, X, AX  }, {op::LDA, M, AX  }, {op::LDX, X, AY  }, {op::LDA, M, ALX },
	{op::CPY, X, IMM }, {op::CMP, M, DXI }, {op::REP, I, IMM }, {op::CMP, M, S   },
	{op::CPY, X, D   }, {op::CMP, M, D   }, {op::DEC, M, D   }, {op::CMP, M, DLI },
	{op::INY, I, IMP }, {op::CMP, M, IMM }, {op::DEX, I, IMP }, {op::WAI, I, IMP },
	{op::CPY, X, A   }, {op::CMP, M, A   }, {op::DEC, M, A   }, {op::CMP, M, AL  },
	{op::BNE, I, RELB}, {op::CMP, M, DIY }, {op::CMP, M, DI  }, {op::CMP, M, SIY },
	{op::PEI, I, PEI }, {op::CMP, M, DX  }, {op::DEC, M, DX  }, {op::CMP, M, DLIY},
	{op::CLD, I, IMP }, {op::CMP, M, AY  }, {op::PHX, I, IMP }, {op::STP, I, IMP },
	{op::JML, I, AI  }, {op::CMP, M, AX  }, {op::DEC, M, AX  }, {op::CMP, M, ALX },
	{op::CPX, X, IMM }, {op::SBC, M, DXI }, {op::SEP, I, IMM }, {op::SBC, M, S   },
	{op::CPX, X, D   }, {op::SBC, M, D   }, {op::INC, M, D   }, {op::SBC, M, DLI },
	{op::INX, M, IMP }, {op::SBC, M, IMM }, {op::NOP, I, IMP }, {op::XBA, I, IMP },
	{op::CPX, X, A   }, {op::SBC, M, A   }, {op::INC, M, A   }, {op::SBC, M, AL  },
	{op::BEQ, I, RELB}, {op::SBC, M, DIY }, {op::SBC, M, DI  }, {op::SBC, M, SIY },
	{op::PEA, I, PEA }, {op::SBC, M, DX  }, {op::INC, M, DX  }, {op::SBC, M, DLIY},
	{op::SED, I, IMP }, {op::SBC, M, AY  }, {op::PLX, I, IMP }, {op::XCE, I, IMP },
	{op::JSR, I, AXI }, {op::SBC, M, AX  }, {op::INC, M, AX  }, {op::SBC, M, ALX }
};

std::string g65816_disassembler::int_8_str(u8 val)
{
	if(val & 0x80)
		return util::string_format("-$%x", (0-val) & 0x7f);
	else
		return util::string_format("$%x", val & 0x7f);
}

std::string g65816_disassembler::int_16_str(u16 val)
{
	if(val & 0x8000)
		return util::string_format("-$%x", (0-val) & 0x7fff);
	else
		return util::string_format("$%x", val & 0x7fff);
}


offs_t g65816_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	int var;
	int length = 1;
	offs_t dasm_flags;

	u8 instruction = opcodes.r8(pc & 0xffffff);
	const opcode_struct *opcode = &s_opcodes[instruction];

	stream << opcode->name();

	if (opcode->is_call())
		dasm_flags = STEP_OVER;
	else if (opcode->is_return())
		dasm_flags = STEP_OUT;
	else if (opcode->is_cond())
		dasm_flags = STEP_COND;
	else
		dasm_flags = 0;

	switch(opcode->ea)
	{
		case IMP :
			break;
		case ACC :
			util::stream_format(stream, "a");
			break;
		case RELB:
			var = (int8_t) opcodes.r8((pc+1) & 0xffffff);
			length++;
			util::stream_format(stream, " %06x (%s)", (pc & 0xff0000) | ((pc + length + var)&0xffff), int_8_str(var));
			break;
		case RELW:
		case PER :
			var = opcodes.r16((pc+1) & 0xffffff);
			length += 2;
			util::stream_format(stream, " %06x (%s)", (pc & 0xff0000) | ((pc + length + var)&0xffff), int_16_str(var));
			break;
		case IMM :
			if((opcode->flag == M && !m_config->get_m_flag()) || (opcode->flag == X && !m_config->get_x_flag()))
			{
				util::stream_format(stream, " #$%04x", opcodes.r16((pc+1) & 0xffffff));
				length += 2;
			}
			else
			{
				util::stream_format(stream, " #$%02x", opcodes.r8((pc+1) & 0xffffff));
				length++;
			}
			break;
		case A   :
		case PEA :
			util::stream_format(stream, " $%04x", opcodes.r16((pc+1) & 0xffffff));
			length += 2;
			break;
		case AI  :
			util::stream_format(stream, " ($%04x)", opcodes.r16((pc+1) & 0xffffff));
			length += 2;
			break;
		case AL  :
			util::stream_format(stream, " $%06x", opcodes.r32((pc+1) & 0xffffff) & 0xffffff);
			length += 3;
			break;
		case ALX :
			util::stream_format(stream, " $%06x,x", opcodes.r32((pc+1) & 0xffffff) & 0xffffff);
			length += 3;
			break;
		case AX  :
			util::stream_format(stream, " $%04x,x", opcodes.r16((pc+1) & 0xffffff));
			length += 2;
			break;
		case AXI :
			util::stream_format(stream, " ($%04x,x)", opcodes.r16((pc+1) & 0xffffff));
			length += 2;
			break;
		case AY  :
			util::stream_format(stream, " $%04x,y", opcodes.r16((pc+1) & 0xffffff));
			length += 2;
			break;
		case D   :
			util::stream_format(stream, " $%02x", opcodes.r8((pc+1) & 0xffffff));
			length++;
			break;
		case DI  :
		case PEI :
			util::stream_format(stream, " ($%02x)", opcodes.r8((pc+1) & 0xffffff));
			length++;
			break;
		case DIY :
			util::stream_format(stream, " ($%02x),y", opcodes.r8((pc+1) & 0xffffff));
			length++;
			break;
		case DLI :
			util::stream_format(stream, " [$%02x]", opcodes.r8((pc+1) & 0xffffff));
			length++;
			break;
		case DLIY:
			util::stream_format(stream, " [$%02x],y", opcodes.r8((pc+1) & 0xffffff));
			length++;
			break;
		case DX  :
			util::stream_format(stream, " $%02x,x", opcodes.r8((pc+1) & 0xffffff));
			length++;
			break;
		case DXI :
			util::stream_format(stream, " ($%02x,x)", opcodes.r8((pc+1) & 0xffffff));
			length++;
			break;
		case DY  :
			util::stream_format(stream, " $%02x,y", opcodes.r8((pc+1) & 0xffffff));
			length++;
			break;
		case S   :
			util::stream_format(stream, " %s,s", int_8_str(opcodes.r8((pc+1) & 0xffffff)));
			length++;
			break;
		case SIY :
			util::stream_format(stream, " (%s,s),y", int_8_str(opcodes.r8((pc+1) & 0xffffff)));
			length++;
			break;
		case SIG :
			util::stream_format(stream, " #$%02x", opcodes.r8((pc+1) & 0xffffff));
			length++;
			break;
		case MVN :
		case MVP :
			util::stream_format(stream, " $%02x, $%02x", opcodes.r8((pc+2) & 0xffffff), opcodes.r8((pc+1) & 0xffffff));
			length += 2;
			break;
	}

	return length | SUPPORTED | dasm_flags;
}
