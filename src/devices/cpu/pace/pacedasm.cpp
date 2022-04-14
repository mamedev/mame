// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    National Semiconductor PACE (IPC-16A, INS8900) disassembler

    This uses the same basic instruction set as IMP-16, but the binary
    encoding is not compatible.

***************************************************************************/

#include "emu.h"
#include "pacedasm.h"


//**************************************************************************
//  PACE DISASSEMBLER
//**************************************************************************

pace_disassembler::pace_disassembler()
	: util::disasm_interface()
{
}

u32 pace_disassembler::opcode_alignment() const
{
	return 1;
}

const char *const pace_disassembler::s_cc[16] = {
	"STFL",     // stack full
	"REQ0",     // AC0 equals zero
	"PSIGN",    // AC0 has positive sign
	"BIT0",     // AC0 has bit 0 set
	"BIT1",     // AC0 has bit 1 set
	"NREQ0",    // AC0 does not equal zero
	"BIT2",     // AC0 has bit 2 set
	"CONTIN",   // CONTIN input is high
	"LINK",     // LINK is set
	"IEN",      // IEN is set
	"CARRY",    // CARRY is set
	"NSIGN",    // AC0 has negative sign
	"OV",       // OV is set
	"JC13",     // JC13 is high
	"JC14",     // JC14 is high
	"JC15"      // JC15 is high
};

const char *const pace_disassembler::s_flags[16] = {
	"F0", // does not exist in FR
	"IE1",
	"IE2",
	"IE3",
	"IE4",
	"IE5",
	"OVF",
	"CRY",
	"LINK",
	"IEN",
	"BYTE",
	"F11",
	"F12",
	"F13",
	"F14",
	"F15" // does not exist in FR, but used to reset IE0 after level 0 interrupt
};

void pace_disassembler::format_addr(std::ostream &stream, u16 addr)
{
	if (addr >= 0x1000)
		stream << "0";
	util::stream_format(stream, "%04X", addr);
}

void pace_disassembler::format_disp(std::ostream &stream, u8 disp)
{
	if (disp >= 0x80)
	{
		stream << "-";
		disp = 0x100 - disp;
	}
	if (disp < 0x0a)
		util::stream_format(stream, "%d", disp);
	else
		util::stream_format(stream, "0%X", disp);
}

void pace_disassembler::format_ea(std::ostream &stream, u16 pc, u16 inst)
{
	if ((inst & 0x0300) == 0x0000)
	{
		// 0000–00FF or 0000–007F, FF80–FFFF depending on BPS input
		if ((inst & 0x00ff) >= 0x10)
			stream << "0";
		util::stream_format(stream, "%02X", inst & 0x00ff);
	}
	else if ((inst & 0x0300) == 0x0100)
		format_addr(stream, pc + 1 + s8(inst & 0x00ff));
	else
	{
		format_disp(stream, inst & 0x00ff);
		util::stream_format(stream, "(AC%d)", (inst & 0x0300) >> 8);
	}
}

offs_t pace_disassembler::disassemble(std::ostream &stream, offs_t pc, const pace_disassembler::data_buffer &opcodes, const pace_disassembler::data_buffer &params)
{
	u16 inst = opcodes.r16(pc);

	switch (inst & 0xfc00)
	{
	case 0x0000:
		stream << "HALT";
		return 1 | SUPPORTED;

	case 0x0400:
		util::stream_format(stream, "%-8sAC%d", "CFR", (inst & 0x0300) >> 8);
		return 1 | SUPPORTED;

	case 0x0800:
		util::stream_format(stream, "%-8sAC%d", "CRF", (inst & 0x0300) >> 8);
		return 1 | SUPPORTED;

	case 0x0c00:
		stream << "PUSHF";
		return 1 | SUPPORTED;

	case 0x1000:
		stream << "PULLF";
		return 1 | SUPPORTED;

	case 0x1400:
		util::stream_format(stream, "%-8s", "JSR");
		format_ea(stream, pc, inst);
		return 1 | STEP_OVER | SUPPORTED;

	case 0x1800:
		util::stream_format(stream, "%-8s", "JMP");
		format_ea(stream, pc, inst);
		return 1 | SUPPORTED;

	case 0x1c00:
		util::stream_format(stream, "%-8sAC%d", "XCHRS", (inst & 0x0300) >> 8);
		return 1 | SUPPORTED;

	case 0x2000:
		util::stream_format(stream, "%-8sAC%d,%d,%d", "ROL", (inst & 0x0300) >> 8, (inst & 0x00fe) >> 1, inst & 0x0001);
		return 1 | SUPPORTED;

	case 0x2400:
		util::stream_format(stream, "%-8sAC%d,%d,%d", "ROR", (inst & 0x0300) >> 8, (inst & 0x00fe) >> 1, inst & 0x0001);
		return 1 | SUPPORTED;

	case 0x2800:
		util::stream_format(stream, "%-8sAC%d,%d,%d", "SHL", (inst & 0x0300) >> 8, (inst & 0x00fe) >> 1, inst & 0x0001);
		return 1 | SUPPORTED;

	case 0x2c00:
		util::stream_format(stream, "%-8sAC%d,%d,%d", "SHR", (inst & 0x0300) >> 8, (inst & 0x00fe) >> 1, inst & 0x0001);
		return 1 | SUPPORTED;

	case 0x3000: case 0x3400: case 0x3800: case 0x3c00:
		util::stream_format(stream, "%-8s%s", BIT(inst, 7) ? "SFLG" : "PFLG", s_flags[(inst & 0x0f00) >> 8]);
		return 1 | SUPPORTED;

	case 0x4000: case 0x4400: case 0x4800: case 0x4c00:
		util::stream_format(stream, "%-8s%s,", "BOC", s_cc[(inst & 0x0f00) >> 8]);
		format_addr(stream, pc + 1 + s8(inst & 0x00ff));
		return 1 | STEP_COND | SUPPORTED;

	case 0x5000:
		util::stream_format(stream, "%-8sAC%d,", "LI", (inst & 0x0300) >> 8);
		format_disp(stream, inst & 0x00ff);
		return 1 | SUPPORTED;

	case 0x5400:
		util::stream_format(stream, "%-8sAC%d,AC%d", "RAND", (inst & 0x00c0) >> 6, (inst & 0x0300) >> 8);
		return 1 | SUPPORTED;

	case 0x5800:
		util::stream_format(stream, "%-8sAC%d,AC%d", "RXOR", (inst & 0x00c0) >> 6, (inst & 0x0300) >> 8);
		return 1 | SUPPORTED;

	case 0x5c00:
		if ((inst & 0x0300) >> 2 == (inst & 0x00c0))
			stream << "NOP";
		else
			util::stream_format(stream, "%-8sAC%d,AC%d", "RCPY", (inst & 0x00c0) >> 6, (inst & 0x0300) >> 8);
		return 1 | SUPPORTED;

	case 0x6000:
		util::stream_format(stream, "%-8sAC%d", "PUSH", (inst & 0x0300) >> 8);
		return 1 | SUPPORTED;

	case 0x6400:
		util::stream_format(stream, "%-8sAC%d", "PULL", (inst & 0x0300) >> 8);
		return 1 | SUPPORTED;

	case 0x6800:
		util::stream_format(stream, "%-8sAC%d,AC%d", "RADD", (inst & 0x00c0) >> 6, (inst & 0x0300) >> 8);
		return 1 | SUPPORTED;

	case 0x6c00:
		util::stream_format(stream, "%-8sAC%d,AC%d", "RXCH", (inst & 0x00c0) >> 6, (inst & 0x0300) >> 8);
		return 1 | SUPPORTED;

	case 0x7000:
		util::stream_format(stream, "%-8sAC%d,", "CAI", (inst & 0x0300) >> 8);
		format_disp(stream, inst & 0x00ff);
		return 1 | SUPPORTED;

	case 0x7400:
		util::stream_format(stream, "%-8sAC%d,AC%d", "RADC", (inst & 0x00c0) >> 6, (inst & 0x0300) >> 8);
		return 1 | SUPPORTED;

	case 0x7800:
		util::stream_format(stream, "%-8sAC%d,", "AISZ", (inst & 0x0300) >> 8);
		format_disp(stream, inst & 0x00ff);
		return 1 | STEP_COND | SUPPORTED;

	case 0x7c00:
		util::stream_format(stream, "%-8s", "RTI");
		format_disp(stream, inst & 0x00ff);
		return 1 | STEP_OUT | SUPPORTED;

	case 0x8000:
		util::stream_format(stream, "%-8s", "RTS");
		format_disp(stream, inst & 0x00ff);
		return 1 | STEP_OUT | SUPPORTED;

	case 0x8800:
		util::stream_format(stream, "%-8sAC0,", "DECA");
		format_ea(stream, pc, inst);
		return 1 | SUPPORTED;

	case 0x8c00:
		util::stream_format(stream, "%-8s", "ISZ");
		format_ea(stream, pc, inst);
		return 1 | STEP_COND | SUPPORTED;

	case 0x9000:
		util::stream_format(stream, "%-8sAC0,", "SUBB");
		format_ea(stream, pc, inst);
		return 1 | SUPPORTED;

	case 0x9400:
		util::stream_format(stream, "%-8s@", "JSR");
		format_ea(stream, pc, inst);
		return 1 | STEP_OVER | SUPPORTED;

	case 0x9800:
		util::stream_format(stream, "%-8s@", "JMP");
		format_ea(stream, pc, inst);
		return 1 | SUPPORTED;

	case 0x9c00:
		util::stream_format(stream, "%-8sAC0,", "SKG");
		format_ea(stream, pc, inst);
		return 1 | STEP_COND | SUPPORTED;

	case 0xa000:
		util::stream_format(stream, "%-8sAC0,@", "LD");
		format_ea(stream, pc, inst);
		return 1 | SUPPORTED;

	case 0xa400:
		util::stream_format(stream, "%-8sAC0,", "OR");
		format_ea(stream, pc, inst);
		return 1 | SUPPORTED;

	case 0xa800:
		util::stream_format(stream, "%-8sAC0,", "AND");
		format_ea(stream, pc, inst);
		return 1 | SUPPORTED;

	case 0xac00:
		util::stream_format(stream, "%-8s", "DSZ");
		format_ea(stream, pc, inst);
		return 1 | STEP_COND | SUPPORTED;

	case 0xb000:
		util::stream_format(stream, "%-8sAC0,@", "ST");
		format_ea(stream, pc, inst);
		return 1 | SUPPORTED;

	case 0xb800:
		util::stream_format(stream, "%-8sAC0,", "SKAZ");
		format_ea(stream, pc, inst);
		return 1 | STEP_COND | SUPPORTED;

	case 0xbc00:
		util::stream_format(stream, "%-8sAC0,", "LSEX");
		format_ea(stream, pc, inst);
		return 1 | SUPPORTED;

	case 0xc000: case 0xc400: case 0xc800: case 0xcc00:
		util::stream_format(stream, "%-8sAC%d,", "LD", (inst & 0x0c00) >> 10);
		format_ea(stream, pc, inst);
		return 1 | SUPPORTED;

	case 0xd000: case 0xd400: case 0xd800: case 0xdc00:
		util::stream_format(stream, "%-8sAC%d,", "ST", (inst & 0x0c00) >> 10);
		format_ea(stream, pc, inst);
		return 1 | SUPPORTED;

	case 0xe000: case 0xe400: case 0xe800: case 0xec00:
		util::stream_format(stream, "%-8sAC%d,", "ADD", (inst & 0x0c00) >> 10);
		format_ea(stream, pc, inst);
		return 1 | SUPPORTED;

	case 0xf000: case 0xf400: case 0xf800: case 0xfc00:
		util::stream_format(stream, "%-8sAC%d,", "SKNE", (inst & 0x0c00) >> 10);
		format_ea(stream, pc, inst);
		return 1 | STEP_COND | SUPPORTED;

	default:
		util::stream_format(stream, "%-8s", ".WORD");
		format_addr(stream, inst);
		return 1 | SUPPORTED;
	}
}
