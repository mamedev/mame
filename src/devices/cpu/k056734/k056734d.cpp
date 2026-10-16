// license:BSD-3-Clause
// copyright-holders:Paul Priest, R. Belmont

// Konami 056734 (ESC) disassembler

// Instruction words are viewed as sixteen 2-bit lanes, lane k = bits 2k+1:2k
// of the descrambled word.  Opcodes are written as the lane digits
// 10, 8, (7,) 3, 0.

#include "emu.h"
#include "k056734d.h"

namespace {

std::string reg(unsigned r)
{
	return util::string_format("%c%d", (r & 0x20) ? 's' : 'r', r & 0x1f);
}

constexpr char const *const SIZE_SUFFIX[4] = { "", ".b", ".h", ".?" };

} // anonymous namespace

k056734_disassembler::descrambler::descrambler() : descrambler(0, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 })
{
}

k056734_disassembler::descrambler::descrambler(u32 xor_mask, const std::array<u8, 16> &lanes) : m_xor(xor_mask)
{
	for (unsigned b = 0; b < 4; b++)
	{
		for (unsigned v = 0; v < 256; v++)
		{
			u32 out = 0;
			for (unsigned k = 0; k < 16; k++)
			{
				if ((lanes[k] >> 2) == b)
					out |= u32(BIT(v, (lanes[k] & 3) * 2, 2)) << (k * 2);
			}
			m_table[b][v] = out;
		}
	}
}

u32 k056734_disassembler::opcode_alignment() const
{
	return 1;
}

offs_t k056734_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	u32 const op = m_fetch(opcodes.r32(pc));
	unsigned const ra = reg_a(op);
	unsigned const rb = reg_b(op);
	unsigned const off = off10(op);
	int const disp = util::sext(off, 10);
	unsigned const imm = imm16(op);
	unsigned const rc = BIT(off, 3, 6);
	char const *const sz = SIZE_SUFFIX[lane(op, 7)];

	auto const mem = [rb, disp] () { return disp ? util::string_format("[%s%+d]", reg(rb), disp) : util::string_format("[%s]", reg(rb)); };

	if (!op)
	{
		util::stream_format(stream, "nop");
		return 1 | SUPPORTED;
	}

	switch (lane(op, 0))
	{
	case 0:
		if (is_branch(op))
		{
			char const *name = nullptr;
			u32 flags = STEP_COND;
			switch (op4(op))
			{
			case op4(1, 0, 0, 0): name = "jmp"; flags = 0; break;
			case op4(1, 1, 0, 0): name = "call"; flags = STEP_OVER; break;
			case op4(2, 0, 0, 0): name = "beq"; break;
			case op4(3, 0, 0, 0): name = "bne"; break;
			case op4(0, 0, 1, 0): name = "bmi"; break;
			case op4(1, 0, 1, 0): name = "bpl"; break;
			case op4(2, 0, 1, 0): name = "bcs"; break;
			case op4(3, 0, 1, 0): name = "bcc"; break;
			case op4(1, 0, 2, 0): name = "bgt"; break;
			case op4(3, 0, 2, 0): name = "bhi"; break;
			case op4(0, 0, 3, 0): name = "blt"; break;
			}
			offs_t const target = (pc + 1 + s16(imm)) & 0x1fff;
			if (name)
				util::stream_format(stream, "%-6s %04x", name, target);
			else
				util::stream_format(stream, "b?%d%d%d   %04x", lane(op, 10), lane(op, 8), lane(op, 3), target);
			return 1 | flags | SUPPORTED;
		}
		if (op5(op) == op5(1, 0, 2, 0, 0))
		{
			if (disp)
				util::stream_format(stream, "jr %s%+d", reg(ra), disp);
			else
				util::stream_format(stream, "jr %s", reg(ra));
			return 1 | SUPPORTED;
		}
		if (op5(op) == op5(1, 1, 2, 0, 0))
		{
			util::stream_format(stream, "jsr %s", reg(ra));
			return 1 | STEP_OVER | SUPPORTED;
		}
		util::stream_format(stream, "li %s, #0x%X", reg(reg_d(op)), imm24(op));
		return 1 | SUPPORTED;

	case 1:
		util::stream_format(stream, "%s %s, [0x%06X]", BIT(lane(op, 8), 1) ? "st.abs" : "ld.abs", reg(reg_d(op)), imm24(op));
		return 1 | SUPPORTED;

	case 2:
		switch (op5(op))
		{
		case op5(1, 1, 0, 0, 2): util::stream_format(stream, "push %s", reg(ra)); return 1 | SUPPORTED;
		case op5(3, 1, 0, 0, 2): util::stream_format(stream, "pop %s", reg(ra)); return 1 | SUPPORTED;
		case op5(1, 1, 0, 1, 2): util::stream_format(stream, "ret %s", reg(ra)); return 1 | STEP_OUT | SUPPORTED;
		case op5(3, 1, 0, 1, 2): util::stream_format(stream, "ret* %s", reg(ra)); return 1 | STEP_OUT | SUPPORTED;
		case op5(1, 0, 0, 3, 2): util::stream_format(stream, "lih %s, #0x%X", reg(ra), imm); return 1 | SUPPORTED;
		case op5(3, 0, 0, 1, 2): util::stream_format(stream, "swap16 %s, %s", reg(ra), reg(rb)); return 1 | SUPPORTED;
		case op5(1, 0, 2, 1, 2): util::stream_format(stream, "ext.h %s, %s", reg(ra), reg(rb)); return 1 | SUPPORTED;
		case op5(1, 0, 1, 1, 2): util::stream_format(stream, "ext.b %s, %s", reg(ra), reg(rb)); return 1 | SUPPORTED;
		case op5(1, 0, 0, 2, 2): util::stream_format(stream, "mul %s, %s", reg(ra), reg(rb)); return 1 | SUPPORTED;
		case op5(1, 0, 2, 2, 2): util::stream_format(stream, "mul.h %s, %s", reg(ra), reg(rb)); return 1 | SUPPORTED;
		case op5(3, 0, 2, 2, 2): util::stream_format(stream, "div.h %s, %s", reg(ra), reg(rb)); return 1 | SUPPORTED;
		case op5(3, 0, 0, 0, 2): util::stream_format(stream, "neg %s, %s", reg(ra), reg(rb)); return 1 | SUPPORTED;
		}
		switch (op4(op))
		{
		case op4(1, 2, 0, 2): util::stream_format(stream, "shl%s %s, %s", sz, reg(ra), reg(rb)); return 1 | SUPPORTED;
		case op4(1, 2, 1, 2): util::stream_format(stream, "asl%s %s, %s", sz, reg(ra), reg(rb)); return 1 | SUPPORTED;
		case op4(1, 2, 2, 2): util::stream_format(stream, "rol%s %s, %s", sz, reg(ra), reg(rb)); return 1 | SUPPORTED;
		case op4(3, 2, 0, 2): util::stream_format(stream, "shr%s %s, %s", sz, reg(ra), reg(rb)); return 1 | SUPPORTED;
		case op4(3, 2, 1, 2): util::stream_format(stream, "asr%s %s, %s", sz, reg(ra), reg(rb)); return 1 | SUPPORTED;
		case op4(3, 2, 2, 2): util::stream_format(stream, "ror%s %s, %s", sz, reg(ra), reg(rb)); return 1 | SUPPORTED;
		}
		break;

	case 3:
		switch (op4(op))
		{
		case op4(1, 2, 0, 3): util::stream_format(stream, "add%s %s, %s, %s", sz, reg(ra), reg(rb), reg(rc)); return 1 | SUPPORTED;
		case op4(1, 2, 1, 3): util::stream_format(stream, "sub%s %s, %s, %s", sz, reg(ra), reg(rb), reg(rc)); return 1 | SUPPORTED;
		case op4(1, 2, 2, 3): util::stream_format(stream, "and%s %s, %s, %s", sz, reg(ra), reg(rb), reg(rc)); return 1 | SUPPORTED;
		case op4(1, 2, 3, 3): util::stream_format(stream, "xor%s %s, %s, %s", sz, reg(ra), reg(rb), reg(rc)); return 1 | SUPPORTED;
		case op4(3, 2, 1, 3):
			if (rc)
				util::stream_format(stream, "or%s %s, %s, %s", sz, reg(ra), reg(rb), reg(rc));
			else
				util::stream_format(stream, "mov%s %s, %s", sz, reg(ra), reg(rb));
			return 1 | SUPPORTED;
		case op4(3, 2, 3, 3): util::stream_format(stream, "nor%s %s, %s, %s", sz, reg(ra), reg(rb), reg(rc)); return 1 | SUPPORTED;

		case op4(1, 3, 0, 3): util::stream_format(stream, "addi%s %s, #0x%X", sz, reg(ra), imm); return 1 | SUPPORTED;
		case op4(1, 3, 1, 3): util::stream_format(stream, "subi%s %s, #0x%X", sz, reg(ra), imm); return 1 | SUPPORTED;
		case op4(1, 3, 2, 3): util::stream_format(stream, "andi%s %s, #0x%X", sz, reg(ra), imm); return 1 | SUPPORTED;
		case op4(1, 3, 3, 3): util::stream_format(stream, "xori%s %s, #0x%X", sz, reg(ra), imm); return 1 | SUPPORTED;
		case op4(3, 3, 1, 3): util::stream_format(stream, "ori%s %s, #0x%X", sz, reg(ra), imm); return 1 | SUPPORTED;

		case op4(2, 0, 3, 3): util::stream_format(stream, "ld%s %s, %s", sz, reg(ra), mem()); return 1 | SUPPORTED;
		case op4(3, 0, 3, 3): util::stream_format(stream, "st%s %s, %s", sz, reg(ra), mem()); return 1 | SUPPORTED;
		case op4(2, 1, 3, 3): util::stream_format(stream, "ld(l)%s %s, %s", sz, reg(ra), mem()); return 1 | SUPPORTED;
		case op4(3, 1, 3, 3): util::stream_format(stream, "st(l)%s %s, %s", sz, reg(ra), mem()); return 1 | SUPPORTED;

		case op4(0, 1, 0, 3): util::stream_format(stream, "add(m)%s %s, %s", sz, reg(ra), mem()); return 1 | SUPPORTED;
		case op4(0, 1, 1, 3): util::stream_format(stream, "cmp(m)%s %s, %s", sz, reg(ra), mem()); return 1 | SUPPORTED;
		case op4(0, 1, 2, 3): util::stream_format(stream, "and(m)%s %s, %s", sz, reg(ra), mem()); return 1 | SUPPORTED;
		case op4(0, 1, 3, 3): util::stream_format(stream, "xor(m)%s %s, %s", sz, reg(ra), mem()); return 1 | SUPPORTED;
		case op4(2, 1, 1, 3): util::stream_format(stream, "or(m)%s %s, %s", sz, reg(ra), mem()); return 1 | SUPPORTED;
		case op4(1, 1, 0, 3): util::stream_format(stream, "(m)add%s %s, %s", sz, reg(ra), mem()); return 1 | SUPPORTED;
		case op4(1, 1, 1, 3): util::stream_format(stream, "(m)sub%s %s, %s", sz, reg(ra), mem()); return 1 | SUPPORTED;
		case op4(1, 1, 2, 3): util::stream_format(stream, "(m)and%s %s, %s", sz, reg(ra), mem()); return 1 | SUPPORTED;
		case op4(1, 1, 3, 3): util::stream_format(stream, "(m)xor%s %s, %s", sz, reg(ra), mem()); return 1 | SUPPORTED;
		case op4(0, 0, 1, 3): util::stream_format(stream, "cmp.m%s %s, %s", sz, reg(ra), mem()); return 1 | SUPPORTED;
		case op4(1, 0, 1, 3): util::stream_format(stream, "cmpm%s %s, %s", sz, mem(), reg(ra)); return 1 | SUPPORTED;
		}
		break;
	}

	util::stream_format(stream, "dc.l $%08x", op);
	return 1 | SUPPORTED;
}
