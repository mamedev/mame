// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Disassembler for AT&T's WE32100 processor.

    Operands are formatted similarly to AT&T's m32dis. Note that operand
    encoding is little-endian even though memory accesses are big-endian
    (and must also be aligned).

***************************************************************************/

#include "emu.h"
#include "we32100d.h"


//**************************************************************************
//  WE32100 DISASSEMBLER
//**************************************************************************

we32100_disassembler::we32100_disassembler()
	: util::disasm_interface()
{
}

u32 we32100_disassembler::opcode_alignment() const
{
	return 1;
}

const char *const we32100_disassembler::s_rnames[16] = {
	"r0",
	"r1",
	"r2",
	"r3",
	"r4",
	"r5",
	"r6",
	"r7",
	"r8",
	"fp",   // Frame Pointer
	"ap",   // Argument Pointer
	"psw",  // Processor Status Word (privileged)
	"sp",   // Stack Pointer
	"pcbp", // Process Control Block Pointer (privileged)
	"isp",  // Interrupt Stack Pointer (privileged)
	"pc"    // Program Counter
};

void we32100_disassembler::format_signed(std::ostream &stream, s32 x)
{
	if (x == 0)
		stream << "0";
	else if (x > 0)
		util::stream_format(stream, "0x%x", x);
	else
		util::stream_format(stream, "-0x%x", u32(-x));
}

void we32100_disassembler::dasm_am(std::ostream &stream, offs_t &pc, const we32100_disassembler::data_buffer &opcodes, u8 n, bool dst, bool spectype)
{
	switch (n & 0xf0)
	{
	case 0x00: case 0x10: case 0x20: case 0x30: case 0xf0:
		if (dst)
			util::stream_format(stream, "invalid(0x%02x)", n);
		else
		{
			// Positive or negative literal
			stream << "&";
			format_signed(stream, s8(n));
		}
		break;

	case 0x40:
		if (n != 0x4f)
		{
			// Register
			util::stream_format(stream, "%%%s", s_rnames[n & 0x0f]);
		}
		else if (dst)
			util::stream_format(stream, "invalid(0x%02x)", n);
		else
		{
			// Word immediate
			util::stream_format(stream, "&0x%08x", swapendian_int32(opcodes.r32(pc)));
			pc += 4;
		}
		break;

	case 0x50:
		if (n != 0x5b && n != 0x5f)
		{
			// Register deferred
			util::stream_format(stream, "(%%%s)", s_rnames[n & 0x0f]);
		}
		else if (n == 0x5b || dst)
			util::stream_format(stream, "invalid(0x%02x)", n);
		else
		{
			// Halfword immediate
			util::stream_format(stream, "&0x%04x", swapendian_int16(opcodes.r16(pc)));
			pc += 2;
		}
		break;

	case 0x60:
		if (n != 0x6f)
		{
			// FP short offset
			format_signed(stream, n & 0x0f);
			stream << "(%fp)";
		}
		else if (dst)
			util::stream_format(stream, "invalid(0x%02x)", n);
		else
		{
			// Byte immediate
			util::stream_format(stream, "&0x%02x", opcodes.r8(pc++));
		}
		break;

	case 0x70:
		if (n != 0x7f)
		{
			// AP short offset
			format_signed(stream, n & 0x0f);
			stream << "(%ap)";
		}
		else
		{
			// Absolute
			util::stream_format(stream, "$0x%08x", swapendian_int32(opcodes.r32(pc)));
			pc += 4;
		}
		break;

	case 0x80: case 0x90:
		if ((n & 0x0f) == 0x0b)
			util::stream_format(stream, "invalid(0x%02x)", n);
		else
		{
			// Word displacement or displacement deferred
			if (BIT(n, 4))
				stream << "*";
			format_signed(stream, s32(swapendian_int32(opcodes.r32(pc))));
			pc += 4;
			util::stream_format(stream, "(%%%s)", s_rnames[n & 0x0f]);
		}
		break;

	case 0xa0: case 0xb0:
		if ((n & 0x0f) == 0x0b)
			util::stream_format(stream, "invalid(0x%02x)", n);
		else
		{
			// Halfword displacement or displacement deferred
			if (BIT(n, 4))
				stream << "*";
			format_signed(stream, s16(swapendian_int16(opcodes.r16(pc))));
			pc += 2;
			util::stream_format(stream, "(%%%s)", s_rnames[n & 0x0f]);
		}
		break;

	case 0xc0: case 0xd0:
		if ((n & 0x0f) == 0x0b)
			util::stream_format(stream, "invalid(0x%02x)", n);
		else
		{
			// Byte displacement or displacement deferred
			if (BIT(n, 4))
				stream << "*";
			format_signed(stream, s8(opcodes.r8(pc++)));
			util::stream_format(stream, "(%%%s)", s_rnames[n & 0x0f]);
		}
		break;

	case 0xe0:
		if (n == 0xef)
		{
			// Absolute deferred
			util::stream_format(stream, "*$0x%08x", swapendian_int32(opcodes.r32(pc)));
			pc += 4;
		}
		else if (spectype)
			util::stream_format(stream, "invalid(0x%02x)", n);
		else if (BIT(n, 3) || (n & 0x03) == 0x01)
			util::stream_format(stream, "reserved(0x%02x)", n);
		else
		{
			// Expanded operand type
			if (!BIT(n, 1))
				util::stream_format(stream, "{%cword}", BIT(n, 2) ? 's' : 'u');
			else if (!BIT(n, 0))
				util::stream_format(stream, "{%chalf}", BIT(n, 2) ? 's' : 'u');
			else
				util::stream_format(stream, "{%cbyte}", BIT(n, 2) ? 's' : 'u');
			u8 m = opcodes.r8(pc++);
			dasm_am(stream, pc, opcodes, m, dst, true);
		}
		break;
	}
}

void we32100_disassembler::dasm_src(std::ostream &stream, offs_t &pc, const we32100_disassembler::data_buffer &opcodes)
{
	u8 n = opcodes.r8(pc++);
	dasm_am(stream, pc, opcodes, n, false, false);
}

void we32100_disassembler::dasm_srcw(std::ostream &stream, offs_t &pc, const we32100_disassembler::data_buffer &opcodes)
{
	u8 n = opcodes.r8(pc++);
	dasm_am(stream, pc, opcodes, n, false, true);
}

void we32100_disassembler::dasm_dst(std::ostream &stream, offs_t &pc, const we32100_disassembler::data_buffer &opcodes)
{
	u8 n = opcodes.r8(pc++);
	dasm_am(stream, pc, opcodes, n, true, false);
}

void we32100_disassembler::dasm_dstw(std::ostream &stream, offs_t &pc, const we32100_disassembler::data_buffer &opcodes)
{
	u8 n = opcodes.r8(pc++);
	dasm_am(stream, pc, opcodes, n, true, true);
}

void we32100_disassembler::dasm_ea(std::ostream &stream, offs_t &pc, offs_t ppc, const we32100_disassembler::data_buffer &opcodes)
{
	u8 n = opcodes.r8(pc++);
	if (n >= 0x50 && (n < 0xe0 || n == 0xef) && n != 0x5f && n != 0x6f)
	{
		if ((n & 0x8f) == 0x8f && n != 0xef)
		{
			if (BIT(n, 4))
				stream << "*";

			// Show calculated PC-relative displacement
			s32 disp;
			if (BIT(n, 6))
				disp = s8(opcodes.r8(pc++));
			else if (BIT(n, 5))
			{
				disp = s16(swapendian_int16(opcodes.r16(pc)));
				pc += 2;
			}
			else
			{
				disp = s32(swapendian_int32(opcodes.r32(pc)));
				pc += 4;
			}
			format_signed(stream, disp);
			util::stream_format(stream, "(%%%s) /*0x%x*/", s_rnames[15], u32(ppc + disp));
		}
		else
			dasm_am(stream, pc, opcodes, n, false, true);
	}
	else
		util::stream_format(stream, "invalid(0x%02x)", n);
}

void we32100_disassembler::dasm_sr(std::ostream &stream, offs_t &pc, const we32100_disassembler::data_buffer &opcodes)
{
	u8 n = opcodes.r8(pc++);
	if (n >= 0x40 && n <= 0x49)
		util::stream_format(stream, "%%%s", s_rnames[n & 0x0f]);
	else
		util::stream_format(stream, "invalid(0x%02x)", n);
}

void we32100_disassembler::dasm_bdisp(std::ostream &stream, offs_t &pc, const we32100_disassembler::data_buffer &opcodes, bool byte)
{
	u32 ppc = pc - 1;
	int disp;
	if (byte)
	{
		disp = s8(opcodes.r8(pc++));
	}
	else
	{
		disp = s16(swapendian_int16(opcodes.r16(pc)));
		pc += 2;
	}
	format_signed(stream, disp);
	util::stream_format(stream, " /*0x%x*/", u32(ppc + disp));
}

offs_t we32100_disassembler::dasm_30xx(std::ostream &stream, offs_t &pc, const we32100_disassembler::data_buffer &opcodes)
{
	u32 flags = SUPPORTED;
	u8 op = opcodes.r8(pc++);

	switch (op)
	{
	case 0x09:
		stream << "MVERNO";
		break;

	case 0x0d:
		stream << "ENBVJMP"; // (privileged)
		break;

	case 0x13:
		stream << "DISVJMP"; // (privileged)
		break;

	case 0x19:
		stream << "MOVBLW";
		break;

	case 0x1f:
		stream << "STREND";
		break;

	case 0x2f:
		stream << "INTACK"; // (privileged)
		break;

	case 0x35:
		stream << "STRCPY";
		break;

	case 0x45:
		stream << "RETG";
		flags |= STEP_OUT;
		break;

	case 0x61:
		stream << "GATE";
		flags |= STEP_OVER;
		break;

	case 0xac:
		stream << "CALLPS"; // (privileged)
		flags |= STEP_OVER;
		break;

	case 0xc8:
		stream << "RETPS"; // (privileged)
		flags |= STEP_OUT;
		break;

	default:
		stream << "reserved";
		break;
	}

	return flags;
}

offs_t we32100_disassembler::disassemble(std::ostream &stream, offs_t pc, const we32100_disassembler::data_buffer &opcodes, const we32100_disassembler::data_buffer &params)
{
	offs_t ppc = pc;
	u32 flags = SUPPORTED;
	u8 op = opcodes.r8(pc++);

	if ((op & 0x03) == 0x01
		|| op == 0x00
		|| op == 0x26
		|| (op & 0xfa) == 0x0a
		|| (op & 0xfb) == 0x12
		|| (op & 0xfe) == 0x1a
		|| (op & 0xfc) == 0x98
		|| (op & 0xde) == 0xc2
		|| (op & 0xfe) == 0xd6
		|| (op & 0xfe) == 0xda)
	{
		stream << "reserved";
	}
	else switch (op & 0xfc)
	{
	case 0x00: case 0x04:
		if (!BIT(op, 1))
		{
			util::stream_format(stream, "%-8s", "MOVAW");
			dasm_ea(stream, pc, ppc, opcodes);
			stream << ",";
			dasm_dst(stream, pc, opcodes);
		}
		else if (!BIT(op, 0))
		{
			util::stream_format(stream, "%-8s0x%08x,", BIT(op, 4) ? "SPOPRT" : "SPOPRD", swapendian_int32(opcodes.r32(pc)));
			pc += 4;
			dasm_src(stream, pc, opcodes);
		}
		else
		{
			util::stream_format(stream, "%-8s0x%08x,", BIT(op, 4) ? "SPOPT2" : "SPOPD2", swapendian_int32(opcodes.r32(pc)));
			pc += 4;
			dasm_src(stream, pc, opcodes);
			stream << ",";
			dasm_dst(stream, pc, opcodes);
		}
		break;

	case 0x08:
		stream << "RET";
		flags |= STEP_OUT;
		break;

	case 0x0c:
		util::stream_format(stream, "%-8s", "MOVTRW"); // (privileged)
		dasm_src(stream, pc, opcodes); // or effective address?
		stream << ",";
		dasm_dst(stream, pc, opcodes);
		break;

	case 0x10:
		if (!BIT(op, 1))
		{
			util::stream_format(stream, "%-8s", "SAVE");
			dasm_sr(stream, pc, opcodes);
		}
		else
		{
			util::stream_format(stream, "%-8s0x%08x,", "SPOPWD", swapendian_int32(opcodes.r32(pc)));
			pc += 4;
			dasm_dst(stream, pc, opcodes);
		}
		break;

	case 0x14:
		if (!BIT(op, 1))
		{
			util::stream_format(stream, "%-8s0x%02x", "EXTOP", opcodes.r8(pc++));
			flags |= STEP_OVER;
		}
		else
		{
			util::stream_format(stream, "%-8s0x%08x,", "SPOPWT", swapendian_int32(opcodes.r32(pc)));
			pc += 4;
			dasm_dst(stream, pc, opcodes);
		}
		break;

	case 0x18:
		util::stream_format(stream, "%-8s", "RESTORE");
		dasm_sr(stream, pc, opcodes);
		break;

	case 0x1c:
		util::stream_format(stream, "%-8s", util::string_format("SWAP%cI", "W?HB"[op & 0x03]));
		dasm_ea(stream, pc, ppc, opcodes);
		break;

	case 0x20:
		if (!BIT(op, 1))
		{
			util::stream_format(stream, "%-8s", "POPW");
			dasm_dstw(stream, pc, opcodes);
		}
		else if (!BIT(op, 0))
		{
			util::stream_format(stream, "%-8s0x%08x,", "SPOPRS", swapendian_int32(opcodes.r32(pc)));
			pc += 4;
			dasm_src(stream, pc, opcodes);
		}
		else
		{
			util::stream_format(stream, "%-8s0x%08x,", "SPOPS2", swapendian_int32(opcodes.r32(pc)));
			pc += 4;
			dasm_src(stream, pc, opcodes);
			stream << ",";
			dasm_dst(stream, pc, opcodes);
		}
		break;

	case 0x24:
		if (!BIT(op, 1))
		{
			util::stream_format(stream, "%-8s", "JMP");
			dasm_ea(stream, pc, ppc, opcodes);
		}
		else
			stream << "CFLUSH";
		break;

	case 0x28:
		util::stream_format(stream, "%-8s", util::string_format("TST%c", "W?HB"[op & 0x03]));
		dasm_src(stream, pc, opcodes);
		break;

	case 0x2c:
		if (!BIT(op, 1))
		{
			util::stream_format(stream, "%-8s", "CALL");
			dasm_ea(stream, pc, ppc, opcodes);
			stream << ",";
			dasm_ea(stream, pc, ppc, opcodes);
			flags |= STEP_OVER;
		}
		else if (!BIT(op, 0))
		{
			stream << "BPT";
			flags |= STEP_OVER;
		}
		else
		{
			stream << "WAIT"; // (privileged)
		}
		break;

	case 0x30:
		if (!BIT(op, 1))
			flags = dasm_30xx(stream, pc, opcodes);
		else if (!BIT(op, 0))
		{
			util::stream_format(stream, "%-8s0x%08x", "SPOP", swapendian_int32(opcodes.r32(pc)));
			pc += 4;
		}
		else
		{
			util::stream_format(stream, "%-8s0x%08x,", "SPOPWS", swapendian_int32(opcodes.r32(pc)));
			pc += 4;
			dasm_dst(stream, pc, opcodes);
		}
		break;

	case 0x34:
		if (!BIT(op, 1))
		{
			util::stream_format(stream, "%-8s", "JSB");
			dasm_ea(stream, pc, ppc, opcodes);
		}
		else
		{
			util::stream_format(stream, "%-8s", BIT(op, 0) ? "BSBB" : "BSBH");
			dasm_bdisp(stream, pc, opcodes, BIT(op, 0));
		}
		flags |= STEP_OVER;
		break;

	case 0x38:
		util::stream_format(stream, "%-8s", util::string_format("BIT%c", "W?HB"[op & 0x03]));
		dasm_src(stream, pc, opcodes);
		stream << ",";
		dasm_src(stream, pc, opcodes);
		break;

	case 0x3c:
		util::stream_format(stream, "%-8s", util::string_format("CMP%c", "W?HB"[op & 0x03]));
		dasm_src(stream, pc, opcodes);
		stream << ",";
		dasm_src(stream, pc, opcodes);
		break;

	case 0x40:
		if (!BIT(op, 1))
		{
			stream << "RGEQ";
			flags |= STEP_OUT;
		}
		else
		{
			util::stream_format(stream, "%-8s", BIT(op, 0) ? "BGEB" : "BGEH");
			dasm_bdisp(stream, pc, opcodes, BIT(op, 0));
		}
		flags |= STEP_COND;
		break;

	case 0x44: case 0x54:
		if (!BIT(op, 1))
		{
			stream << "RGTR";
			if (BIT(op, 4))
				stream << "U";
			flags |= STEP_OUT;
		}
		else
		{
			if (BIT(op, 4))
				util::stream_format(stream, "%-8s", BIT(op, 0) ? "BGUB" : "BGUH");
			else
				util::stream_format(stream, "%-8s", BIT(op, 0) ? "BGB" : "BGH");
			dasm_bdisp(stream, pc, opcodes, BIT(op, 0));
		}
		flags |= STEP_COND;
		break;

	case 0x48:
		if (!BIT(op, 1))
		{
			stream << "RLSS";
			flags |= STEP_OUT;
		}
		else
		{
			util::stream_format(stream, "%-8s", BIT(op, 0) ? "BLB" : "BLH");
			dasm_bdisp(stream, pc, opcodes, BIT(op, 0));
		}
		flags |= STEP_COND;
		break;

	case 0x4c: case 0x5c:
		if (!BIT(op, 1))
		{
			stream << "RLEQ";
			if (BIT(op, 4))
				stream << "U";
			flags |= STEP_OUT;
		}
		else
		{
			if (BIT(op, 4))
				util::stream_format(stream, "%-8s", BIT(op, 0) ? "BLEUB" : "BLEUH");
			else
				util::stream_format(stream, "%-8s", BIT(op, 0) ? "BLEB" : "BLEH");
			dasm_bdisp(stream, pc, opcodes, BIT(op, 0));
		}
		flags |= STEP_COND;
		break;

	case 0x50:
		if (!BIT(op, 1))
		{
			stream << "RCC";
			flags |= STEP_OUT;
		}
		else
		{
			util::stream_format(stream, "%-8s", BIT(op, 0) ? "BCC" : "BCC");
			dasm_bdisp(stream, pc, opcodes, BIT(op, 0));
		}
		flags |= STEP_COND;
		break;

	case 0x58:
		if (!BIT(op, 1))
		{
			stream << "RCS";
			flags |= STEP_OUT;
		}
		else
		{
			util::stream_format(stream, "%-8s", BIT(op, 0) ? "BCSB" : "BCSH");
			dasm_bdisp(stream, pc, opcodes, BIT(op, 0));
		}
		flags |= STEP_COND;
		break;

	case 0x60:
		if (!BIT(op, 1))
		{
			stream << "RVS";
			flags |= STEP_OUT;
		}
		else
		{
			util::stream_format(stream, "%-8s", BIT(op, 0) ? "BVCB" : "BVCH");
			dasm_bdisp(stream, pc, opcodes, BIT(op, 0));
		}
		flags |= STEP_COND;
		break;

	case 0x64: case 0x74:
		if (!BIT(op, 1))
		{
			stream << "RNEQ";
			if (!BIT(op, 4))
				stream << "U";
			flags |= STEP_OUT;
		}
		else
		{
			util::stream_format(stream, "%-8s", BIT(op, 0) ? "BNEB" : "BNEH");
			dasm_bdisp(stream, pc, opcodes, BIT(op, 0));
		}
		flags |= STEP_COND;
		break;

	case 0x68:
		if (!BIT(op, 1))
		{
			stream << "RVS";
			flags |= STEP_OUT;
		}
		else
		{
			util::stream_format(stream, "%-8s", BIT(op, 0) ? "BVSB" : "BVSH");
			dasm_bdisp(stream, pc, opcodes, BIT(op, 0));
		}
		flags |= STEP_COND;
		break;

	case 0x6c: case 0x7c:
		if (!BIT(op, 1))
		{
			stream << "REQL";
			if (!BIT(op, 4))
				stream << "U";
			flags |= STEP_OUT;
		}
		else
		{
			util::stream_format(stream, "%-8s", BIT(op, 0) ? "BEB" : "BEH");
			dasm_bdisp(stream, pc, opcodes, BIT(op, 0));
		}
		flags |= STEP_COND;
		break;

	case 0x70:
		if (!BIT(op, 1))
			stream << "NOP";
		else if (BIT(op, 0))
			util::stream_format(stream, "%-8s0x%02x", "NOP2", opcodes.r8(pc++));
		else
		{
			util::stream_format(stream, "%-8s0x%04x", "NOP3", swapendian_int16(opcodes.r16(pc)));
			pc += 2;
		}
		break;

	case 0x78:
		if (!BIT(op, 1))
		{
			stream << "RSB";
			flags |= STEP_OUT;
		}
		else
		{
			util::stream_format(stream, "%-8s", BIT(op, 0) ? "BRB" : "BRH");
			dasm_bdisp(stream, pc, opcodes, BIT(op, 0));
		}
		break;

	case 0x80:
		util::stream_format(stream, "%-8s", util::string_format("CLR%c", "W?HB"[op & 0x03]));
		dasm_dst(stream, pc, opcodes);
		break;

	case 0x84:
		util::stream_format(stream, "%-8s", util::string_format("MOV%c", "W?HB"[op & 0x03]));
		dasm_src(stream, pc, opcodes);
		stream << ",";
		dasm_dst(stream, pc, opcodes);
		break;

	case 0x88:
		util::stream_format(stream, "%-8s", util::string_format("MCOM%c", "W?HB"[op & 0x03]));
		dasm_src(stream, pc, opcodes);
		stream << ",";
		dasm_dst(stream, pc, opcodes);
		break;

	case 0x8c:
		util::stream_format(stream, "%-8s", util::string_format("MNEG%c", "W?HB"[op & 0x03]));
		dasm_src(stream, pc, opcodes);
		stream << ",";
		dasm_dst(stream, pc, opcodes);
		break;

	case 0x90:
		util::stream_format(stream, "%-8s", util::string_format("INC%c", "W?HB"[op & 0x03]));
		dasm_dst(stream, pc, opcodes);
		break;

	case 0x94:
		util::stream_format(stream, "%-8s", util::string_format("DEC%c", "W?HB"[op & 0x03]));
		dasm_dst(stream, pc, opcodes);
		break;

	case 0x9c: case 0xdc:
		util::stream_format(stream, "%-8s", util::string_format("ADD%c%d", "W?HB"[op & 0x03], BIT(op, 6) ? 3 : 2));
		dasm_src(stream, pc, opcodes);
		if (BIT(op, 6))
		{
			stream << ",";
			dasm_src(stream, pc, opcodes);
		}
		stream << ",";
		dasm_dst(stream, pc, opcodes);
		break;

	case 0xa0:
		util::stream_format(stream, "%-8s", "PUSHW");
		dasm_srcw(stream, pc, opcodes);
		break;

	case 0xa4: case 0xe4:
		util::stream_format(stream, "%-8s", util::string_format("MOD%c%d", "W?HB"[op & 0x03], BIT(op, 6) ? 3 : 2));
		dasm_src(stream, pc, opcodes);
		if (BIT(op, 6))
		{
			stream << ",";
			dasm_src(stream, pc, opcodes);
		}
		stream << ",";
		dasm_dst(stream, pc, opcodes);
		break;

	case 0xa8: case 0xe8:
		util::stream_format(stream, "%-8s", util::string_format("MUL%c%d", "W?HB"[op & 0x03], BIT(op, 6) ? 3 : 2));
		dasm_src(stream, pc, opcodes);
		if (BIT(op, 6))
		{
			stream << ",";
			dasm_src(stream, pc, opcodes);
		}
		stream << ",";
		dasm_dst(stream, pc, opcodes);
		break;

	case 0xac: case 0xec:
		util::stream_format(stream, "%-8s", util::string_format("DIV%c%d", "W?HB"[op & 0x03], BIT(op, 6) ? 3 : 2));
		dasm_src(stream, pc, opcodes);
		if (BIT(op, 6))
		{
			stream << ",";
			dasm_src(stream, pc, opcodes);
		}
		stream << ",";
		dasm_dst(stream, pc, opcodes);
		break;

	case 0xb0: case 0xf0:
		util::stream_format(stream, "%-8s", util::string_format("OR%c%d", "W?HB"[op & 0x03], BIT(op, 6) ? 3 : 2));
		dasm_src(stream, pc, opcodes);
		if (BIT(op, 6))
		{
			stream << ",";
			dasm_src(stream, pc, opcodes);
		}
		stream << ",";
		dasm_dst(stream, pc, opcodes);
		break;

	case 0xb4: case 0xf4:
		util::stream_format(stream, "%-8s", util::string_format("XOR%c%d", "W?HB"[op & 0x03], BIT(op, 6) ? 3 : 2));
		dasm_src(stream, pc, opcodes);
		if (BIT(op, 6))
		{
			stream << ",";
			dasm_src(stream, pc, opcodes);
		}
		stream << ",";
		dasm_dst(stream, pc, opcodes);
		break;

	case 0xb8: case 0xf8:
		util::stream_format(stream, "%-8s", util::string_format("AND%c%d", "W?HB"[op & 0x03], BIT(op, 6) ? 3 : 2));
		dasm_src(stream, pc, opcodes);
		if (BIT(op, 6))
		{
			stream << ",";
			dasm_src(stream, pc, opcodes);
		}
		stream << ",";
		dasm_dst(stream, pc, opcodes);
		break;

	case 0xbc: case 0xfc:
		util::stream_format(stream, "%-8s", util::string_format("SUB%c%d", "W?HB"[op & 0x03], BIT(op, 6) ? 3 : 2));
		dasm_src(stream, pc, opcodes);
		if (BIT(op, 6))
		{
			stream << ",";
			dasm_src(stream, pc, opcodes);
		}
		stream << ",";
		dasm_dst(stream, pc, opcodes);
		break;

	case 0xc0: case 0xc4: case 0xd0: case 0xd4:
		util::stream_format(stream, "%-8s", util::string_format("%c%cS%c3", BIT(op, 4) ? 'L' : 'A', BIT(op, 2) ? 'R' : 'L', "W?HB"[op & 0x03]));
		dasm_src(stream, pc, opcodes);
		stream << ",";
		dasm_src(stream, pc, opcodes);
		stream << ",";
		dasm_dst(stream, pc, opcodes);
		break;

	case 0xc8:
		util::stream_format(stream, "%-8s", util::string_format("INS%c3", "W?HB"[op & 0x03]));
		dasm_src(stream, pc, opcodes);
		stream << ",";
		dasm_src(stream, pc, opcodes);
		stream << ",";
		dasm_src(stream, pc, opcodes);
		stream << ",";
		dasm_dst(stream, pc, opcodes);
		break;

	case 0xcc:
		util::stream_format(stream, "%-8s", util::string_format("EXT%c3", "W?HB"[op & 0x03]));
		dasm_src(stream, pc, opcodes);
		stream << ",";
		dasm_src(stream, pc, opcodes);
		stream << ",";
		dasm_src(stream, pc, opcodes);
		stream << ",";
		dasm_dst(stream, pc, opcodes);
		break;

	case 0xd8:
		util::stream_format(stream, "%-8s", "ROTW");
		dasm_src(stream, pc, opcodes);
		stream << ",";
		dasm_src(stream, pc, opcodes);
		stream << ",";
		dasm_dst(stream, pc, opcodes);
		break;

	case 0xe0:
		util::stream_format(stream, "%-8s", "PUSHAW");
		dasm_ea(stream, pc, ppc, opcodes);
		break;
	}

	return flags | ((pc - ppc) & LENGTHMASK);
}
