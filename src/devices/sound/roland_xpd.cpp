// license:BSD-3-Clause
// copyright-holders:superctr
/*
 *  Roland XP (MBCS30109) effect DSP disassembler
 */
#include "emu.h"
#include "roland_xpd.h"

namespace {

// col[5:4] names the operand the slot multiplies
const char *const OPERAND[4] = { "R", "acc", "R", "now" };

std::string word_name(int word)
{
	return util::string_format("$%02x", word);
}

} // anonymous namespace

void roland_xp_disassembler::append(std::string &r, const std::string &e)
{
	if (!r.empty())
		r += ", ";
	r += e;
}

std::string roland_xp_disassembler::coefficient(offs_t pc) const
{
	if (!has_coefficient())
		return "C";

	const u16 c = coefficient_word(pc);
	static const int SHIFT[4] = { 0, 1, 2, 4 };
	const s32 mantissa = s32(s16(c << 2)) >> 2;
	return util::string_format("%g", double(mantissa << SHIFT[c >> 14]) / 8192.0);
}

std::string roland_xp_disassembler::constant(offs_t pc) const
{
	if (!has_coefficient())
		return "K";

	const u16 c = coefficient_word(pc);
	return util::string_format("%d", BIT(c, 15) ? s32(c & 0x3fff) << 13 : s32(s16(c << 2)) >> 2);
}

// an external RAM access spans two slots, so the second slot's field is address, not command
bool roland_xp_disassembler::continuation(offs_t pc, const data_buffer &opcodes)
{
	bool second = false;
	for (offs_t slot = 0; slot < (pc & 0xff); slot++)
		second = !second && ((opcodes.r32(slot) >> 23) & 3) != 0;
	return second;
}

offs_t roland_xp_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	const u32 w = opcodes.r32(pc);
	const u16 cram = coefficient_word(pc);
	const int ext = (w >> 25) & 7;
	const int st = (w >> 14) & 3;
	const int word = (w >> 6) & 0xff;
	const int col = w & 0x3f;
	const int nibble = col & 0xf;
	const std::string source = word_name(word);

	std::string r;

	const bool ramp = col == 0x30 && (cram & 0x3e80) == 0x0280 && has_coefficient();

	if (st == 1)
		append(r, util::string_format("%s = %s", word >= 0xf0 ? "G" : (ramp && (cram & 0x3f) == 0x21) ? "now" : "R", source));

	if (col == 0x30 && cram <= 0x000f && has_coefficient())
	{
		append(r, util::string_format("P = %s", source));
		switch (cram & 0xf)
		{
		case 0: case 2: append(r, "acc += R"); break;
		case 4:         append(r, "acc = R"); break;
		case 5:         append(r, "acc = p"); break;
		case 9:         append(r, "acc = R + p"); break;
		default:        append(r, "acc += p"); break;
		}
		append(r, util::string_format("[%04x]", cram));
	}
	else if (ramp)
	{
		const int code = cram & 0xf;
		const int select = (cram >> 4) & 3;

		if (code == 5)
		{
			append(r, util::string_format("P = %s*G", select == 1 ? (st == 3 ? "stored" : "acc''") : select == 2 ? "R" : "now"));
			append(r, "acc = p");
		}
		else
		{
			if (code == 1 || code == 2 || code == 3)
				append(r, util::string_format("P = %s*G", (select == 2 && code == 3) ? "R" : (select == 2 && code == 1) ? ((st == 1 && word < 0xf0) ? "now" : "R") : (select == 3 && code != 2) ? "now" : "acc"));
			else
				append(r, util::string_format("P = %s", OPERAND[col >> 4]));
			append(r, (code == 2 || code == 3) ? "acc += p" : "acc = acc*G + p");
		}
		append(r, util::string_format("[%04x]", cram));
	}
	else if (col == 0x30 && has_coefficient() && (cram == 0x1001 || cram == 0x2801 || cram == 0x0231 || cram == 0x0325))
	{
		switch (cram)
		{
		case 0x1001: append(r, "acc = wrap(acc)"); break;
		case 0x2801: append(r, "acc = abs(wrap(acc))"); break;
		case 0x0231: append(r, util::string_format("acc = lerp(%s, now, f)", source)); break;
		default:     append(r, "nop"); break;
		}
		append(r, util::string_format("[%04x]", cram));
	}
	else if (col == 0x0c)
		append(r, util::string_format("P = ser*%s", coefficient(pc)));
	else
	{
		const std::string operand = OPERAND[col >> 4];
		append(r, (nibble == 0 && col != 0x30) ? util::string_format("P = %s", operand)
				: util::string_format("P = %s*%s", operand, coefficient(pc)));

		switch (col)
		{
		case 0x11: append(r, util::string_format("acc = %s", source)); break;
		case 0x14: append(r, "acc = R"); break;
		case 0x0f: append(r, util::string_format("acc += #%s", constant(pc))); break;
		case 0x1f: append(r, util::string_format("acc = R + #%s", constant(pc))); break;
		case 0x2f: append(r, util::string_format("acc = p + #%s", constant(pc))); break;
		case 0x20: append(r, "eread [acc>>12], f = acc"); break;
		case 0x21: break;
		case 0x32: append(r, "acc += p"); break;
		default:
			switch (nibble)
			{
			case 0: case 3: append(r, "acc += p"); break;
			case 5:         append(r, "acc = p"); break;
			case 9:         append(r, "acc = R + p"); break;
			default:        break;
			}
			break;
		}
	}

	if (st == 2)
		append(r, util::string_format("%s = latch", source));
	else if (st == 3)
		append(r, util::string_format("%s = acc", source));

	if (!continuation(pc, opcodes))
	{
		const u16 offset = u16(((w >> 16) & 0x7f) << 9) | u16((opcodes.r32(pc + 1) >> 16) & 0x1ff);
		switch ((w >> 23) & 3)
		{
		case 1: append(r, util::string_format("latch = eram[+%04x]", offset)); break;
		case 2: append(r, util::string_format("eram[+%04x] = R", offset)); break;
		case 3: append(r, util::string_format("eram[+%04x] = acc", offset)); break;
		default: break;
		}
	}

	if (ext)
		append(r, util::string_format("ext %d", ext));

	if (r.empty())
		r = "nop";

	stream << r;
	return 1 | SUPPORTED;
}
