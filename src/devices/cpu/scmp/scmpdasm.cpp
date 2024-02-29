// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    National Semiconductor SC/MP disassembler

    The pseudo-instruction "JS Pn, dest" (not handled by this disassembler)
    awkwardly effects a subroutine call by the following sequence of five
    instructions:

        LDI     H(dest-1)
        XPAH    Pn
        LDI     L(dest-1)
        XPAL    Pn
        XPPC    Pn

    The Programming and Assembler manual also mentions stacks, which are
    only supported as a software convention. The .DBYTE directive stores
    16-bit data in big-endian format, though no single instruction loads
    more than 8 bits.

    All address arithmetic operates within 4 KB pages, passing bits 15-12
    of pointer registers unchanged.

***************************************************************************/

#include "emu.h"
#include "scmpdasm.h"

scmp_disassembler::scmp_disassembler()
	: util::disasm_interface()
{
}

u32 scmp_disassembler::opcode_alignment() const
{
	return 1;
}

u32 scmp_disassembler::interface_flags() const
{
	return PAGED;
}

u32 scmp_disassembler::page_address_bits() const
{
	return 12;
}

namespace {

const char *const s_mref_insts[8] =
{
	"LD", "ST", "AND", "OR", "XOR", "DAD", "ADD", "CAD"
};

const char *const s_alu_ops[8] =
{
	"LD", "ST", "AN", "OR", "XR", "DA", "AD", "CA"
};

const char *const s_xfer_insts[4] =
{
	"JMP", "JP", "JZ", "JNZ"
};

const char *const s_ptr_regs[4] =
{
	"PC", "P1", "P2", "P3"
};

} // anonymous namespace

void scmp_disassembler::format_disp(std::ostream &stream, offs_t pc, u8 op, u8 disp)
{
	if (disp == 0x80)
		util::stream_format(stream, "E(%s)", s_ptr_regs[BIT(op, 0, 2)]);
	else if (BIT(op, 0, 2) == 0)
		util::stream_format(stream, "X'%04X", (pc & 0xf000) | ((pc + ((op & 0xf0) == 0x90 ? 2 : 1) + s8(disp)) & 0x0fff));
	else
	{
		if (s8(disp) < 0)
		{
			stream << '-';
			disp = -disp;
		}
		if (disp > 9)
			stream << "X'";
		util::stream_format(stream, "%X(%s)", disp, s_ptr_regs[BIT(op, 0, 2)]);
	}
}

offs_t scmp_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	u8 op = opcodes.r8(pc);
	if (op >= 0xc0 && op != 0xcc)
	{
		if (BIT(op, 0, 3) == 4)
		{
			// Immediate instructions
			util::stream_format(stream, "%s%-6cX'%02X", s_alu_ops[BIT(op, 3, 3)], 'I', opcodes.r8(pc + 1));
		}
		else
		{
			// Memory reference instructions
			util::stream_format(stream, "%-8s", s_mref_insts[BIT(op, 3, 3)]);
			if (BIT(op, 2))
				stream << '@'; // "Auto-indexed" (post-increment or pre-decrement)
			format_disp(stream, pc, op, opcodes.r8(pc + 1));
		}
		return 2 | SUPPORTED;
	}
	else if ((op & 0xec) == 0xa8)
	{
		// Memory increment/decrement instructions
		util::stream_format(stream, "%-8s", BIT(op, 4) ? "DLD" : "ILD");
		format_disp(stream, pc, op, opcodes.r8(pc + 1));
		return 2 | SUPPORTED;
	}
	else if ((op & 0xf0) == 0x90)
	{
		// Transfer instructions (effective address is off by 1)
		util::stream_format(stream, "%-8s", s_xfer_insts[BIT(op, 2, 2)]);
		format_disp(stream, pc, op, opcodes.r8(pc + 1));
		return 2 | (BIT(op, 2, 2) != 0 ? STEP_COND : 0) | SUPPORTED;
	}
	else if (op == 0x8f)
	{
		util::stream_format(stream, "%-8s%d", "DLY", opcodes.r8(pc + 1));
		return 2 | SUPPORTED;
	}
	else
	{
		// Single-byte instructions
		switch (op)
		{
		case 0x00:
			stream << "HALT";
			break;

		case 0x01:
			stream << "XAE";
			break;

		case 0x02: case 0x03:
			util::stream_format(stream, "%cCL", BIT(op, 0) ? 'S' : 'C');
			break;

		case 0x04:
			stream << "DINT";
			break;

		case 0x05:
			stream << "IEN";
			break;

		case 0x06:
			stream << "CSA";
			break;

		case 0x07:
			stream << "CAS";
			break;

		case 0x08:
			stream << "NOP";
			break;

		case 0x19:
			stream << "SIO";
			break;

		case 0x1c: case 0x1d: case 0x1e: case 0x1f:
			// Shift and rotate instructions
			util::stream_format(stream, "%cR", BIT(op, 1) ? 'R' : 'S');
			if (BIT(op, 0))
				stream << 'L';
			break;

		case 0x30: case 0x31: case 0x32: case 0x33:
		case 0x34: case 0x35: case 0x36: case 0x37:
			// Exchange pointer with AC
			util::stream_format(stream, "%-8s%s", BIT(op, 2) ? "XPAH" : "XPAL", s_ptr_regs[BIT(op, 0, 2)]);
			break;

		case 0x3c: case 0x3d: case 0x3e: case 0x3f:
			// Exchange pointer with PC
			util::stream_format(stream, "%-8s%s", "XPPC", s_ptr_regs[BIT(op, 0, 2)]);
			return 1 | STEP_OUT | SUPPORTED;

		case 0x40: case 0x50: case 0x58: case 0x60: case 0x68: case 0x70: case 0x78:
			// Extension register instructions
			util::stream_format(stream, "%sE", s_alu_ops[BIT(op, 3, 3)]);
			break;

		default:
			util::stream_format(stream, "%-8sX'%02X", ".BYTE", op);
			break;
		}
		return 1 | SUPPORTED;
	}
}
