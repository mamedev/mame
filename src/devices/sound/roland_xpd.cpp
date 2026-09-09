// license:BSD-3-Clause
// copyright-holders:superctr, giulioz
/*
 *  Roland XP effect DSP disassembler
 */
#include "emu.h"
#include "roland_xpd.h"

namespace {

// col[5:4] names the multiply input
const char *const INPUT[4] = { "prev", "acc", "R", "latch" };

const int SHIFT[4] = { 0, 1, 2, 4 };

constexpr int SLOTS = 288;

std::string word_name(int word)
{
	return util::string_format("$%02x", word);
}

const char *condition_name(int code)
{
	switch (code)
	{
	case 0: return "acc==0";
	case 1: return "acc!=0";
	case 3: case 5: case 13: case 14: return "always";
	case 6: case 8: return "acc>=0";
	case 7: case 9: return "acc<0";
	case 10: return "acc>0";
	case 11: return "acc<=0";
	default: return "never";
	}
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
	for (offs_t slot = 0; slot < (pc % SLOTS); slot++)
		second = !second && ((opcodes.r32(slot) >> 23) & 3) != 0;
	return second;
}

// the ALU functions both encodings share
void roland_xp_disassembler::function(std::string &r, int fn, int mode, const std::string &k)
{
	switch (fn)
	{
	case 0x2: append(r, "acc += R"); break;
	case 0x3: append(r, "acc += p"); break;
	case 0x4: append(r, "acc = R"); break;
	case 0x5: append(r, "acc = p"); break;
	case 0x6: append(r, "acc = -acc"); break;
	case 0x7: append(r, "acc = R - acc"); break;
	case 0x8: append(r, "acc = p - acc"); break;
	case 0x9: append(r, "acc = R + p"); break;
	case 0xa: append(r, "acc = min(acc, R)"); break;
	case 0xb: append(r, "acc = max(acc, R)"); break;
	case 0xc:
		if (mode == 2)
			append(r, "acc += p>>13");
		else if (mode == 3)
			append(r, "acc = p>>13");
		break;
	case 0xd:
		switch (mode)
		{
		case 0: append(r, util::string_format("acc &= #%s", k)); break;
		case 1: append(r, util::string_format("acc |= #%s", k)); break;
		case 2: append(r, util::string_format("acc ^= #%s", k)); break;
		default: append(r, "?"); break;
		}
		break;
	case 0xe:
		switch (mode)
		{
		case 0: append(r, util::string_format("acc = min(acc, #%s)", k)); break;
		case 1: append(r, util::string_format("acc = max(acc, #%s)", k)); break;
		default: append(r, "?"); break;
		}
		break;
	case 0xf:
		switch (mode)
		{
		case 0: append(r, util::string_format("acc += #%s", k)); break;
		case 1: append(r, util::string_format("acc = R + #%s", k)); break;
		case 2: append(r, util::string_format("acc = p + #%s", k)); break;
		default: append(r, util::string_format("acc = #%s - acc", k)); break;
		}
		break;
	default:
		break;
	}
}

// col 0x30: the CRAM word is a second instruction
void roland_xp_disassembler::parallel(std::string &r, offs_t pc, int st, int word) const
{
	const u16 c = coefficient_word(pc);
	const int fn = c & 0xf;
	const int input = (c >> 4) & 3;
	const int factor_select = (c >> 6) & 3;
	const bool complement = BIT(c, 8);
	const bool multiply = BIT(c, 9);
	const int post = (c >> 11) & 7;

	std::string factor;
	switch (factor_select)
	{
	case 0: factor = complement ? "(1-f)" : "f"; break;
	case 1: factor = complement ? "(1-mag)" : "mag"; break;
	case 2: factor = complement ? "~acc>>8" : "acc>>8"; break;
	default: factor = complement ? "~G" : "G"; break;
	}
	// unity is a gain register at shift 1 and the other factors at shift 0
	const int unit = factor_select == 3 ? 1 : 0;
	if (SHIFT[c >> 14] > unit)
		factor += util::string_format("*%d", 1 << (SHIFT[c >> 14] - unit));
	else if (SHIFT[c >> 14] < unit)
		factor += "/2";

	if (!multiply)
	{
		if (st == 1 && word < ramp_base())
			append(r, util::string_format("P = %s", word_name(word)));
	}
	else
		append(r, util::string_format("P = %s*%s", INPUT[input], factor));

	switch (fn)
	{
	case 0x0: append(r, "acc += p + R"); break;
	case 0x1: break;
	case 0xc: append(r, "acc = R + p - acc"); break;
	case 0xd: append(r, "acc = acc + p - R"); break;
	case 0xe: append(r, "acc = p - R - acc"); break;
	case 0xf: append(r, "acc = p - R"); break;
	default: function(r, fn, input, constant(pc)); break;
	}

	switch (post)
	{
	case 1: append(r, "acc = abs(acc)"); break;
	case 2: append(r, "acc = wrap(acc)"); break;
	case 3: append(r, "acc = prng(acc)"); break;
	case 4: append(r, "acc = fold(acc)"); break;
	case 5: append(r, "acc = abs(wrap(acc))"); break;
	default: break;
	}
	if (BIT(c, 10) && post != 2)
		append(r, "acc = wrap(acc)");

	append(r, util::string_format("[%04x]", c));
}

offs_t roland_xp_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	const u32 w = opcodes.r32(pc);
	const u16 cram = coefficient_word(pc);
	const int ext = (w >> 25) & 7;
	const int st = (w >> 14) & 3;
	const int word = (w >> 6) & 0xff;
	const int fn = w & 0xf;
	const int input = (w >> 4) & 3;
	const std::string source = word_name(word);

	std::string r;

	if (st == 1)
		append(r, util::string_format("%s = %s", word >= ramp_base() ? "G" : "R", source));

	if (fn == 0 && input == 3)
	{
		if (has_coefficient())
			parallel(r, pc, st, word);
		else
			append(r, "parallel [C]");
	}
	else
	{
		if (fn == 0xc && input < 2)
			append(r, util::string_format("P = %s*%s", input ? "serB" : "ser", coefficient(pc)));
		else if (fn >= 1 && fn <= 0xc)
			append(r, util::string_format("P = %s*%s", INPUT[input], coefficient(pc)));

		switch (fn ? -1 : input)
		{
		case 1:
			if (BIT(cram, 9))
				append(r, util::string_format("branch %s pc%+d", condition_name((cram >> 10) & 0xf), s8(cram)));
			else
				append(r, util::string_format("branch %s $%02x", condition_name((cram >> 10) & 0xf), cram & 0xff));
			break;
		case 2: append(r, "eread [acc>>12]"); break;
		default: function(r, fn, input, constant(pc)); break;
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
