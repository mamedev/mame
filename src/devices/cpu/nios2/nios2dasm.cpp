// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Altera Nios II disassembler

    This FPGA-embedded 32-bit RISC processor architecture differs vastly
    from the original Nios ISA, which uses 16-bit instruction words and
    existed in both 16-bit and 32-bit flavors.

    Hardware support for multiply and divide instructions is optional. The
    mulxss, mulxsu and mulxuu instructions require the fast 32 x 32-bit
    multiplier available on Stratix and Stratix II FPGAs.

    This disassembler does not strictly enforce that "0" fields are clear,
    especially for the R-Type instructions.

***************************************************************************/

#include "emu.h"
#include "nios2dasm.h"

nios2_disassembler::nios2_disassembler()
	: util::disasm_interface()
{
}

u32 nios2_disassembler::opcode_alignment() const
{
	return 4;
}

namespace {

const char *const s_reg_names[32] =
{
	"zero", // r0 = 0x00000000
	"at",   // r1 = assembler temporary
	"r2", "r3", "r4", "r5", "r6", "r7",
	"r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
	"r16", "r17", "r18", "r19", "r20", "r21", "r22", "r23",
	"et",   // r24 = exception temporary
	"bt",   // r25 = breakpoint temporary
	"gp",   // r26 = global pointer
	"sp",   // r27 = stack pointer
	"fp",   // r28 = frame pointer
	"ea",   // r29 = exception return address (trap/eret)
	"ba",   // r30 = breakpoint return address (break/bret)
	"ra"    // r31 = return address (call/ret)
};

const char *const s_ctlreg_names[16] =
{
	"status", "estatus", "bstatus", "ienable", "ipending", "cpuid", "ctl6", "exception",
	"pteaddr", "tlbacc", "tlbmisc", "eccinj", "badaddr", "config", "mpubase", "mpuacc"
};

} // anonymous namespace

void nios2_disassembler::format_simm16(std::ostream &stream, u16 imm16) const
{
	if (s16(imm16) < 0)
	{
		stream << '-';
		imm16 = -imm16;
	}
	if (imm16 > 9)
		stream << "0x";
	util::stream_format(stream, "%x", imm16);
}

void nios2_disassembler::format_ctlreg(std::ostream &stream, u8 n) const
{
	if (n < std::size(s_ctlreg_names))
		stream << s_ctlreg_names[n];
	else
		util::stream_format(stream, "ctl%d", n);
}

offs_t nios2_disassembler::dasm_rtype(std::ostream &stream, u32 inst) const
{
	switch (BIT(inst, 11, 6)) // OPX field
	{
	case 0x01:
		if (BIT(inst, 27, 5) == 0x1d)
		{
			stream << "eret";
			return 4 | STEP_OUT | SUPPORTED;
		}
		else
			util::stream_format(stream, "%-8s0x%08x", ".dword", inst);
		break;

	case 0x02:
		util::stream_format(stream, "%-8s%s, %s, %d", "roli", s_reg_names[BIT(inst, 17, 5)], s_reg_names[BIT(inst, 27, 5)], BIT(inst, 6, 5));
		break;

	case 0x03: case 0x0b:
		util::stream_format(stream, "%-8s%s, %s, %s", BIT(inst, 14) ? "ror" : "rol", s_reg_names[BIT(inst, 17, 5)], s_reg_names[BIT(inst, 27, 5)], s_reg_names[BIT(inst, 22, 5)]);
		break;

	case 0x04:
		stream << "flushp";
		break;

	case 0x05:
		if (BIT(inst, 27, 5) == 0x1f)
		{
			stream << "ret";
			return 4 | STEP_OUT | SUPPORTED;
		}
		else
			util::stream_format(stream, "%-8s0x%08x", ".dword", inst);
		break;

	case 0x06: case 0x16:
		util::stream_format(stream, "%-8s%s, %s, %s", BIT(inst, 15) ? "or" : "nor", s_reg_names[BIT(inst, 17, 5)], s_reg_names[BIT(inst, 27, 5)], s_reg_names[BIT(inst, 22, 5)]);
		break;

	case 0x07: case 0x17: case 0x1f:
		util::stream_format(stream, "%-8s%s, %s, %s", BIT(inst, 14) ? "mulxss" : BIT(inst, 15) ? "mulxsu" : "mulxuu", s_reg_names[BIT(inst, 17, 5)], s_reg_names[BIT(inst, 27, 5)], s_reg_names[BIT(inst, 22, 5)]);
		break;

	case 0x08: case 0x28:
		util::stream_format(stream, "%-8s%s, %s, %s", BIT(inst, 16) ? "cmpgeu" : "cmpge", s_reg_names[BIT(inst, 17, 5)], s_reg_names[BIT(inst, 27, 5)], s_reg_names[BIT(inst, 22, 5)]);
		break;

	case 0x09:
		if (BIT(inst, 27, 5) == 0x1e)
		{
			stream << "bret";
			return 4 | STEP_OUT | SUPPORTED;
		}
		else
			util::stream_format(stream, "%-8s0x%08x", ".dword", inst);
		break;

	case 0x0c:
		util::stream_format(stream, "%-8s%s", "flushi", s_reg_names[BIT(inst, 27, 5)]);
		break;

	case 0x0d:
		util::stream_format(stream, "%-8s%s", "jmp", s_reg_names[BIT(inst, 27, 5)]);
		break;

	case 0x0e:
		util::stream_format(stream, "%-8s%s, %s, %s", "and", s_reg_names[BIT(inst, 17, 5)], s_reg_names[BIT(inst, 27, 5)], s_reg_names[BIT(inst, 22, 5)]);
		break;

	case 0x10: case 0x30:
		util::stream_format(stream, "%-8s%s, %s, %s", BIT(inst, 16) ? "cmpltu" : "cmplt", s_reg_names[BIT(inst, 17, 5)], s_reg_names[BIT(inst, 27, 5)], s_reg_names[BIT(inst, 22, 5)]);
		break;

	case 0x12: case 0x1a: case 0x3a:
		util::stream_format(stream, "%-8s%s, %s, %d", BIT(inst, 16) ? "srai" : BIT(inst, 14) ? "srli" : "slli", s_reg_names[BIT(inst, 17, 5)], s_reg_names[BIT(inst, 27, 5)], BIT(inst, 6, 5));
		break;

	case 0x13: case 0x1b: case 0x3b:
		util::stream_format(stream, "%-8s%s, %s, %s", BIT(inst, 16) ? "sra" : BIT(inst, 14) ? "srl" : "sll", s_reg_names[BIT(inst, 17, 5)], s_reg_names[BIT(inst, 27, 5)], s_reg_names[BIT(inst, 22, 5)]);
		break;

	case 0x18:
		util::stream_format(stream, "%-8s%s, %s, %s", "cmpne", s_reg_names[BIT(inst, 17, 5)], s_reg_names[BIT(inst, 27, 5)], s_reg_names[BIT(inst, 22, 5)]);
		break;

	case 0x1c:
		util::stream_format(stream, "%-8s%s", "nextpc", s_reg_names[BIT(inst, 17, 5)]);
		break;

	case 0x1d:
		if (BIT(inst, 17, 5) == 0x1f)
		{
			util::stream_format(stream, "%-8s%s", "callr", s_reg_names[BIT(inst, 27, 5)]);
			return 4 | STEP_OVER | SUPPORTED;
		}
		else
			util::stream_format(stream, "%-8s0x%08x", ".dword", inst);
		break;

	case 0x1e:
		util::stream_format(stream, "%-8s%s, %s, %s", "xor", s_reg_names[BIT(inst, 17, 5)], s_reg_names[BIT(inst, 27, 5)], s_reg_names[BIT(inst, 22, 5)]);
		break;

	case 0x20:
		util::stream_format(stream, "%-8s%s, %s, %s", "cmpeq", s_reg_names[BIT(inst, 17, 5)], s_reg_names[BIT(inst, 27, 5)], s_reg_names[BIT(inst, 22, 5)]);
		break;

	case 0x24: case 0x25:
		util::stream_format(stream, "%-8s%s, %s, %s", BIT(inst, 11) ? "div" : "divu", s_reg_names[BIT(inst, 17, 5)], s_reg_names[BIT(inst, 27, 5)], s_reg_names[BIT(inst, 22, 5)]);
		break;

	case 0x26:
		util::stream_format(stream, "%-8s%s, ", "rdctl", s_reg_names[BIT(inst, 17, 5)]);
		format_ctlreg(stream, BIT(inst, 6, 5));
		break;

	case 0x27:
		util::stream_format(stream, "%-8s%s, %s, %s", "mul", s_reg_names[BIT(inst, 17, 5)], s_reg_names[BIT(inst, 27, 5)], s_reg_names[BIT(inst, 22, 5)]);
		break;

	case 0x29:
		util::stream_format(stream, "%-8s%s", "initi", s_reg_names[BIT(inst, 27, 5)]);
		break;

	case 0x2d:
		if (BIT(inst, 17, 5) == 0x1d)
		{
			stream << "trap";
			return 4 | STEP_OVER | SUPPORTED;
		}
		else
			util::stream_format(stream, "%-8s0x%08x", ".dword", inst);
		break;

	case 0x2e:
		util::stream_format(stream, "%-8s", "wrctl");
		format_ctlreg(stream, BIT(inst, 6, 5));
		util::stream_format(stream, ", %s", s_reg_names[BIT(inst, 27, 5)]);
		break;

	case 0x31:
		if (BIT(inst, 17, 15) == 0)
			stream << "nop";
		else if (BIT(inst, 22, 5) == 0)
			util::stream_format(stream, "%-8s%s, %s", "mov", s_reg_names[BIT(inst, 17, 5)], s_reg_names[BIT(inst, 27, 5)]);
		else
			util::stream_format(stream, "%-8s%s, %s, %s", "add", s_reg_names[BIT(inst, 17, 5)], s_reg_names[BIT(inst, 27, 5)], s_reg_names[BIT(inst, 22, 5)]);
		break;

	case 0x34:
		if (BIT(inst, 17, 5) == 0x1e)
		{
			u8 imm5 = BIT(inst, 6, 5);
			if (imm5 == 0)
				stream << "break";
			else
				util::stream_format(stream, "%-8s%s%x", "break", imm5 > 9 ? "0x" : "", imm5);
			return 4 | STEP_OVER | SUPPORTED;
		}
		else
			util::stream_format(stream, "%-8s0x%08x", ".dword", inst);
		break;

	case 0x36:
		stream << "sync";
		break;

	case 0x39:
		util::stream_format(stream, "%-8s%s, %s, %s", "sub", s_reg_names[BIT(inst, 17, 5)], s_reg_names[BIT(inst, 27, 5)], s_reg_names[BIT(inst, 22, 5)]);
		break;

	default:
		util::stream_format(stream, "%-8s0x%08x", ".dword", inst);
		break;
	}
	return 4 | SUPPORTED;
}

offs_t nios2_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	u32 inst = opcodes.r32(pc);
	switch (BIT(inst, 0, 6)) // OP field
	{
	case 0x00:
		util::stream_format(stream, "%-8s0x%08x", "call", (pc & 0xf0000000) | (inst >> 4));
		return 4 | STEP_OVER | SUPPORTED;

	case 0x01:
		util::stream_format(stream, "%-8s0x%08x", "jmpi", (pc & 0xf0000000) | (inst >> 4)); // added in 2007
		return 4 | SUPPORTED;

	case 0x03: case 0x23:
		util::stream_format(stream, "%-8s%s, ", BIT(inst, 5) ? "ldbuio" : "ldbu", s_reg_names[BIT(inst, 22, 5)]);
		format_simm16(stream, BIT(inst, 6, 16));
		util::stream_format(stream, "(%s)", s_reg_names[BIT(inst, 27, 5)]);
		break;

	case 0x04:
	{
		u16 imm16 = BIT(inst, 6, 16);
		if (BIT(inst, 27, 5) == 0)
		{
			util::stream_format(stream, "%-8s%s, ", "movi", s_reg_names[BIT(inst, 22, 5)]);
			format_simm16(stream, imm16);
			if (s16(imm16) < 0)
				util::stream_format(stream, " ; =0x%08x", u32(s32(s16(imm16))));
		}
		else
		{
			if (s16(imm16) < 0)
			{
				util::stream_format(stream, "%-8s", "subi");
				imm16 = -imm16;
			}
			else
				util::stream_format(stream, "%-8s", "addi");
			util::stream_format(stream, "%s, %s, %s%x", s_reg_names[BIT(inst, 22, 5)], s_reg_names[BIT(inst, 27, 5)], imm16 > 9 ? "0x" : "", imm16);
		}
		break;
	}

	case 0x05: case 0x25:
		util::stream_format(stream, "%-8s%s, ", BIT(inst, 5) ? "stbio" : "stb", s_reg_names[BIT(inst, 22, 5)]);
		format_simm16(stream, BIT(inst, 6, 16));
		util::stream_format(stream, "(%s)", s_reg_names[BIT(inst, 27, 5)]);
		break;

	case 0x06:
		util::stream_format(stream, "%-8s0x%08x", "br", u32(pc + 4 + s16(BIT(inst, 6, 16))));
		break;

	case 0x07: case 0x27:
		util::stream_format(stream, "%-8s%s, ", BIT(inst, 5) ? "ldbio" : "ldb", s_reg_names[BIT(inst, 22, 5)]);
		format_simm16(stream, BIT(inst, 6, 16));
		util::stream_format(stream, "(%s)", s_reg_names[BIT(inst, 27, 5)]);
		break;

	case 0x08:
		util::stream_format(stream, "%-8s%s, %s, ", "cmpgei", s_reg_names[BIT(inst, 22, 5)], s_reg_names[BIT(inst, 27, 5)]);
		format_simm16(stream, BIT(inst, 6, 16));
		break;

	case 0x0b: case 0x2b:
		util::stream_format(stream, "%-8s%s, ", BIT(inst, 5) ? "ldhuio" : "ldhu", s_reg_names[BIT(inst, 22, 5)]);
		format_simm16(stream, BIT(inst, 6, 16));
		util::stream_format(stream, "(%s)", s_reg_names[BIT(inst, 27, 5)]);
		break;

	case 0x0c: case 0x2c:
		util::stream_format(stream, "%-8s%s, %s, 0x%04x", BIT(inst, 5) ? "andhi" : "andi", s_reg_names[BIT(inst, 22, 5)], s_reg_names[BIT(inst, 27, 5)], BIT(inst, 6, 16));
		break;

	case 0x0d: case 0x2d:
		util::stream_format(stream, "%-8s%s, ", BIT(inst, 5) ? "sthio" : "sth", s_reg_names[BIT(inst, 22, 5)]);
		format_simm16(stream, BIT(inst, 6, 16));
		util::stream_format(stream, "(%s)", s_reg_names[BIT(inst, 27, 5)]);
		break;

	case 0x0e: case 0x2e:
		util::stream_format(stream, "%-8s%s, %s, 0x%08x", BIT(inst, 5) ? "bgeu" : "bge", s_reg_names[BIT(inst, 27, 5)], s_reg_names[BIT(inst, 22, 5)], u32(pc + 4 + s16(BIT(inst, 6, 16))));
		return 4 | STEP_COND | SUPPORTED;

	case 0x0f: case 0x2f:
		util::stream_format(stream, "%-8s%s, ", BIT(inst, 5) ? "ldhio" : "ldh", s_reg_names[BIT(inst, 22, 5)]);
		format_simm16(stream, BIT(inst, 6, 16));
		util::stream_format(stream, "(%s)", s_reg_names[BIT(inst, 27, 5)]);
		break;

	case 0x10:
		util::stream_format(stream, "%-8s%s, %s, ", "cmplti", s_reg_names[BIT(inst, 22, 5)], s_reg_names[BIT(inst, 27, 5)]);
		format_simm16(stream, BIT(inst, 6, 16));
		break;

	case 0x14: case 0x34:
		if (BIT(inst, 27, 5) == 0)
		{
			util::stream_format(stream, "%-8s%s, 0x%04x", BIT(inst, 5) ? "movhi" : "movui", s_reg_names[BIT(inst, 22, 5)], BIT(inst, 6, 16));
			if (BIT(inst, 5))
			{
				// Look ahead to interpret movia sequence (movhi followed by addi to and of same register) and similar movhi/ori sequence
				u32 nextinst = opcodes.r32(pc + 4);
				if ((nextinst & 0xffc0002f) == (inst & 0x07c00000) * 0x21 + 0x00000004)
				{
					u16 nextimm16 = BIT(nextinst, 6, 16);
					util::stream_format(stream, " ; =%%hi%s(0x%08x)", BIT(nextinst, 4) ? "" : "adj", u32((BIT(inst, 6, 16) << 16) + (BIT(nextinst, 4) ? nextimm16 : s32(s16(nextimm16)))));
				}
			}
		}
		else
			util::stream_format(stream, "%-8s%s, %s, 0x%04x", BIT(inst, 5) ? "orhi" : "ori", s_reg_names[BIT(inst, 22, 5)], s_reg_names[BIT(inst, 27, 5)], BIT(inst, 6, 16));
		break;

	case 0x15: case 0x35:
		util::stream_format(stream, "%-8s%s, ", BIT(inst, 5) ? "stwio" : "stw", s_reg_names[BIT(inst, 22, 5)]);
		format_simm16(stream, BIT(inst, 6, 16));
		util::stream_format(stream, "(%s)", s_reg_names[BIT(inst, 27, 5)]);
		break;

	case 0x16: case 0x36:
		util::stream_format(stream, "%-8s%s, %s, 0x%08x", BIT(inst, 5) ? "bltu" : "blt", s_reg_names[BIT(inst, 27, 5)], s_reg_names[BIT(inst, 22, 5)], u32(pc + 4 + s16(BIT(inst, 6, 16))));
		return 4 | STEP_COND | SUPPORTED;

	case 0x17: case 0x37:
		util::stream_format(stream, "%-8s%s, ", BIT(inst, 5) ? "ldwio" : "ldw", s_reg_names[BIT(inst, 22, 5)]);
		format_simm16(stream, BIT(inst, 6, 16));
		util::stream_format(stream, "(%s)", s_reg_names[BIT(inst, 27, 5)]);
		break;

	case 0x18:
		util::stream_format(stream, "%-8s%s, %s, ", "cmpnei", s_reg_names[BIT(inst, 22, 5)], s_reg_names[BIT(inst, 27, 5)]);
		format_simm16(stream, BIT(inst, 6, 16));
		break;

	case 0x1b: case 0x3b:
		util::stream_format(stream, "%-8s", BIT(inst, 5) ? "flushd" : "flushda");
		format_simm16(stream, BIT(inst, 6, 16));
		util::stream_format(stream, "(%s)", s_reg_names[BIT(inst, 27, 5)]);
		break;

	case 0x1c: case 0x3c:
		util::stream_format(stream, "%-8s%s, %s, 0x%04x", BIT(inst, 5) ? "xorhi" : "xori", s_reg_names[BIT(inst, 22, 5)], s_reg_names[BIT(inst, 27, 5)], BIT(inst, 6, 16));
		break;

	case 0x1e:
		util::stream_format(stream, "%-8s%s, %s, 0x%08x", "bne", s_reg_names[BIT(inst, 27, 5)], s_reg_names[BIT(inst, 22, 5)], u32(pc + 4 + s16(BIT(inst, 6, 16))));
		return 4 | STEP_COND | SUPPORTED;

	case 0x20:
		util::stream_format(stream, "%-8s%s, %s, ", "cmpeqi", s_reg_names[BIT(inst, 22, 5)], s_reg_names[BIT(inst, 27, 5)]);
		format_simm16(stream, BIT(inst, 6, 16));
		break;

	case 0x24:
		util::stream_format(stream, "%-8s%s, %s, ", "muli", s_reg_names[BIT(inst, 22, 5)], s_reg_names[BIT(inst, 27, 5)]);
		format_simm16(stream, BIT(inst, 6, 16));
		break;

	case 0x26:
		util::stream_format(stream, "%-8s%s, %s, 0x%08x", "beq", s_reg_names[BIT(inst, 27, 5)], s_reg_names[BIT(inst, 22, 5)], u32(pc + 4 + s16(BIT(inst, 6, 16))));
		return 4 | STEP_COND | SUPPORTED;

	case 0x28:
	{
		u16 imm16 = BIT(inst, 6, 16);
		util::stream_format(stream, "%-8s%s, %s, %s%x", "cmpgeui", s_reg_names[BIT(inst, 22, 5)], s_reg_names[BIT(inst, 27, 5)], imm16 > 9 ? "0x" : "", imm16);
		break;
	}

	case 0x30:
	{
		u16 imm16 = BIT(inst, 6, 16);
		util::stream_format(stream, "%-8s%s, %s, %s%x", "cmpltui", s_reg_names[BIT(inst, 22, 5)], s_reg_names[BIT(inst, 27, 5)], imm16 > 9 ? "0x" : "", imm16);
		break;
	}

	case 0x32:
	{
		u8 n = BIT(inst, 6, 8);
		util::stream_format(stream, "%-8s%s%x, ", "custom", s_reg_names[BIT(inst, 22, 5)], n > 9 ? "0x" : "", n);
		if (BIT(inst, 16))
			stream << s_reg_names[BIT(inst, 27, 5)];
		else
			util::stream_format(stream, "c%d", BIT(inst, 27, 5));
		stream << ", ";
		if (BIT(inst, 17))
			stream << s_reg_names[BIT(inst, 22, 5)];
		else
			util::stream_format(stream, "c%d", BIT(inst, 22, 5));
		stream << ", ";
		if (BIT(inst, 18))
			stream << s_reg_names[BIT(inst, 17, 5)];
		else
			util::stream_format(stream, "c%d", BIT(inst, 17, 5));
		break;
	}

	case 0x33:
		util::stream_format(stream, "%-8s", "initd");
		format_simm16(stream, BIT(inst, 6, 16));
		util::stream_format(stream, "(%s)", s_reg_names[BIT(inst, 27, 5)]);
		break;

	case 0x3a:
		return dasm_rtype(stream, inst);

	default:
		util::stream_format(stream, "%-8s0x%08x", ".dword", inst);
		break;
	}

	return 4 | SUPPORTED;
}
