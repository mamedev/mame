// license:BSD-3-Clause
// copyright-holders:Andrea Bogazzi

// Jaleco "FPU" math coprocessor disassembler

#include "emu.h"
#include "jalfpu_dasm.h"

namespace {

constexpr u8 LOAD_SEL[6] = { 0x3, 0x7, 0xb, 0xd, 0xe, 0xf };

constexpr u16 CTL_HALT = 0x4080;

std::string reg(unsigned r)
{
	return util::string_format("s%x", r);
}

std::string cond_name(unsigned code)
{
	switch (code)
	{
	case 0x0: return "t";
	case 0x2: return "c6";
	case 0x3: return "djnz6";
	case 0x4: return "c7";
	case 0x5: return "djnz7";
	case 0x8: return "z";
	case 0x9: return "lt";
	case 0xa: return "ge";
	case 0xb: return "gt";
	case 0xd: return "c";
	case 0xe: return "n";
	case 0xf: return "v";
	default: return util::string_format("cc%x", code);
	}
}

std::string mem_mode(unsigned mode)
{
	return util::string_format("%d%s", (mode >> 2) & 7, BIT(mode, 1) ? (BIT(mode, 0) ? "-" : "+") : "");
}

} // anonymous namespace

offs_t jaleco_fpu_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	u32 const op = opcodes.r32(pc) & 0xfffff;
	unsigned const opc = op >> 16;
	u16 const arg = op & 0xffff;
	unsigned const fn = arg >> 10;
	unsigned const a = (arg >> 6) & 0xf;
	unsigned const b = arg & 0xf;
	offs_t flags = SUPPORTED;

	switch (opc)
	{
	case 0x0: case 0x1: case 0x2: case 0x3: case 0x4: case 0x5:
		util::stream_format(stream, "ld    %s,#$%04x", reg(LOAD_SEL[opc]), arg);
		break;

	case 0x6: case 0x7:
		util::stream_format(stream, "ld    c%d,#$%04x", opc, arg);
		break;

	case 0x8:
		switch (fn)
		{
		case 0x07: util::stream_format(stream, "add   %s,%s", reg(b), reg(a)); break;
		case 0x0f: util::stream_format(stream, "adc   %s,%s", reg(b), reg(a)); break;
		case 0x17: util::stream_format(stream, "sub   %s,%s", reg(b), reg(a)); break;
		case 0x1f: util::stream_format(stream, "sbc   %s,%s", reg(b), reg(a)); break;
		case 0x27: util::stream_format(stream, "cmp   %s,%s", reg(b), reg(a)); break;
		case 0x2f: util::stream_format(stream, "and   %s,%s", reg(b), reg(a)); break;
		case 0x37: util::stream_format(stream, "or    %s,%s", reg(b), reg(a)); break;
		case 0x3f: util::stream_format(stream, "xor   %s,%s", reg(b), reg(a)); break;
		default: util::stream_format(stream, "alu%02x %s,%s", fn, reg(b), reg(a)); break;
		}
		break;

	case 0x9:
		switch (fn)
		{
		case 0x17: case 0x1c: case 0x1d: case 0x1e:
			util::stream_format(stream, "mul   %s:sd,%s", reg(b), reg(a));
			if (fn != 0x17)
				util::stream_format(stream, " ; %02x", fn);
			break;
		case 0x07: util::stream_format(stream, "tst   %s", reg(b)); break;
		case 0x27: util::stream_format(stream, "divu  %s:sd,%s", reg(b), reg(a)); break;
		case 0x2f: util::stream_format(stream, "div   %s:sd,%s", reg(b), reg(a)); break;
		default: util::stream_format(stream, "md%02x  %s,%s", fn, reg(b), reg(a)); break;
		}
		break;

	case 0xa:
		switch (fn)
		{
		case 0x3f: case 0x3c: case 0x3d: case 0x3e:
			util::stream_format(stream, "mov   %s,%s", reg(b), reg(a));
			if (fn != 0x3f)
				util::stream_format(stream, " ; %02x", fn);
			break;
		case 0x07: util::stream_format(stream, "inc   %s,%s", reg(b), reg(a)); break;
		case 0x0f: util::stream_format(stream, "dec   %s,%s", reg(b), reg(a)); break;
		case 0x17: util::stream_format(stream, "neg   %s,%s", reg(b), reg(a)); break;
		case 0x1f: util::stream_format(stream, "abs   %s,%s", reg(b), reg(a)); break;
		case 0x27: util::stream_format(stream, "not   %s,%s", reg(b), reg(a)); break;
		default: util::stream_format(stream, "un%02x  %s,%s", fn, reg(b), reg(a)); break;
		}
		break;

	case 0xb:
		switch (fn)
		{
		case 0x07: util::stream_format(stream, "shl   %s,%s", reg(b), reg(a)); break;
		case 0x0f: util::stream_format(stream, "shr   %s,%s", reg(b), reg(a)); break;
		case 0x1f: util::stream_format(stream, "sar   %s,%s", reg(b), reg(a)); break;
		case 0x37: util::stream_format(stream, "rcl   %s,%s", reg(b), reg(a)); break;
		case 0x3f: util::stream_format(stream, "rcr   %s,%s", reg(b), reg(a)); break;
		default: util::stream_format(stream, "sh%02x  %s,%s", fn, reg(b), reg(a)); break;
		}
		break;

	case 0xc:
		util::stream_format(stream, "%s    %s,[%s:%s]", BIT(fn, 5) ? "rd" : "wr", reg(b), reg(a), mem_mode(fn & 0x1f));
		break;

	case 0xd:
		switch (fn)
		{
		case 0x1e: case 0x1f: util::stream_format(stream, "sclr%x #$%02x", fn & 1, (arg >> 4) & 0x3f); break;
		case 0x2d: util::stream_format(stream, "slat  #$%02x", (arg >> 4) & 0x3f); break;
		case 0x3a: util::stream_format(stream, "ssat  #$%02x", (arg >> 4) & 0x3f); break;
		default: util::stream_format(stream, "grp%02x #$%02x", fn, (arg >> 4) & 0x3f); break;
		}
		break;

	case 0xe:
		if (arg == CTL_HALT)
			util::stream_format(stream, "halt");
		else
			util::stream_format(stream, "ctl   #$%04x", arg);
		break;

	case 0xf:
	{
		unsigned const code = fn & 0xf;
		bool const sense = BIT(fn, 4);
		const char *const delay = BIT(fn, 5) ? "d" : "";
		u16 const target = arg & 0x3ff;
		if (code == 0x7)
		{
			if (sense)
			{
				util::stream_format(stream, "%-5s %03x", util::string_format("call%s", delay), target);
				flags |= STEP_OVER | step_over_extra(BIT(fn, 5) ? 1 : 0);
			}
			else
			{
				util::stream_format(stream, "ret%s", delay);
				flags |= STEP_OUT | step_over_extra(BIT(fn, 5) ? 1 : 0);
			}
		}
		else if (code == 0 && sense)
			util::stream_format(stream, "%-5s %03x", util::string_format("jmp%s", delay), target);
		else
		{
			util::stream_format(stream, "%-5s %03x", util::string_format("b%s%s%s", sense ? "" : "n", cond_name(code), delay), target);
			flags |= STEP_COND | step_over_extra(BIT(fn, 5) ? 1 : 0);
		}
		break;
	}
	}

	return 1 | flags;
}
