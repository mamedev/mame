// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * Disassembler for Panafacom MN1610 and MN1613.
 *
 * Source:
 *  - http://www.st.rim.or.jp/~nkomatsu/panafacom/MN1610.html
 *
 * TODO:
 *  - mn1617
 */
#include "emu.h"

#include "mn1610d.h"

namespace {

char const *const reg[] = { "r0", "r1", "r2", "r3", "r4", "sp", "str", "ic" };

enum operand_type : unsigned
{
	NONE = 0,

	// mn1610
	EA, // effective address
	Rd, // register at bits 8-10
	Rs, // register at bits 0-2
	I2, // 2 bit immediate
	I4, // 4 bit immediate
	Ib, // 8 bit immediate
	EE, // shift carry operation
	SK, // skip

	// mn1613
	R0, // register 0
	D0, // double-width register 0
	Ri, // index register at bits 0-1
	Mi, // index register at bits 0-1 with optional pre-decrement/post-increment
	Iw, // 16 bit immediate
	BB, // segment register
	C,  // include carry
	BR, // base register
	SR, // special register
	HR, // hardware register
};

struct instruction
{
	u16 value;
	u16 mask;

	char const *const opcode;
	operand_type operands[4];

	u32 flags;
};

const struct instruction mn1610_table[] =
{
	// memory
	{ 0xc700, 0xc700, "b",    { EA } },            // 11mm m111 nnnn nnnn
	{ 0x8700, 0xc700, "bal",  { EA },              // 10mm m111 nnnn nnnn
		util::disasm_interface::STEP_OVER },
	{ 0xc600, 0xc700, "ims",  { EA },              // 11mm m110 nnnn nnnn
		util::disasm_interface::STEP_COND },
	{ 0x8600, 0xc700, "dms",  { EA },              // 10mm m110 nnnn nnnn
		util::disasm_interface::STEP_COND },
	{ 0xc000, 0xc000, "l",    { Rd, EA } },        // 11mm mrrr nnnn nnnn
	{ 0x8000, 0xc000, "st",   { Rd, EA } },        // 10mm mrrr nnnn nnnn

	// arithmetic
	{ 0x5000, 0xf808, "cb",   { Rd, Rs, SK } },    // 0101 0ddd kkkk 0sss
	{ 0x5008, 0xf808, "c",    { Rd, Rs, SK } },    // 0101 0ddd kkkk 1sss
	{ 0x5800, 0xf808, "s",    { Rd, Rs, SK } },    // 0101 1ddd kkkk 0sss
	{ 0x5808, 0xf808, "a",    { Rd, Rs, SK } },    // 0101 1ddd kkkk 1sss
	{ 0x6000, 0xf808, "eor",  { Rd, Rs, SK } },    // 0110 0ddd kkkk 0sss
	{ 0x6008, 0xf808, "or",   { Rd, Rs, SK } },    // 0110 0ddd kkkk 1sss
	{ 0x6808, 0xf808, "and",  { Rd, Rs, SK } },    // 0110 1ddd kkkk 1sss
	{ 0x6800, 0xf808, "lad",  { Rd, Rs, SK } },    // 0110 1ddd kkkk 0sss
	{ 0x7000, 0xf808, "dswp", { Rd, Rs, SK } },    // 0111 0ddd kkkk 0sss
	{ 0x7008, 0xf808, "bswp", { Rd, Rs, SK } },    // 0111 0ddd kkkk 1sss
	{ 0x7800, 0xf808, "mvb",  { Rd, Rs, SK } },    // 0111 1ddd kkkk 0sss
	{ 0x7808, 0xf808, "mv",   { Rd, Rs, SK } },    // 0111 1ddd kkkk 1sss

	// shift
	{ 0x200c, 0xf80c, "sl",   { Rd, EE, SK } },    // 0010 0rrr kkkk 11ee
	{ 0x2008, 0xf80c, "sr",   { Rd, EE, SK } },    // 0010 0rrr kkkk 10ee

	// immediate
	{ 0x3800, 0xf800, "sbit", { Rd, I4, SK } },    // 0011 1rrr kkkk nnnn
	{ 0x3000, 0xf800, "rbit", { Rd, I4, SK } },    // 0011 0rrr kkkk nnnn
	{ 0x2800, 0xf800, "tbit", { Rd, I4, SK } },    // 0010 1rrr kkkk nnnn
	{ 0x4800, 0xf800, "ai",   { Rd, I4, SK } },    // 0100 1rrr kkkk nnnn
	{ 0x4000, 0xf800, "si",   { Rd, I4, SK } },    // 0100 0rrr kkkk nnnn

	// i/o
	{ 0x1800, 0xf800, "rd",   { Rd, Ib } },        // 0001 1rrr nnnn nnnn
	{ 0x1000, 0xf800, "wt",   { Rd, Ib } },        // 0001 0rrr nnnn nnnn
	{ 0x0800, 0xf800, "mvi",  { Rd, Ib } },        // 0000 1rrr nnnn nnnn

	// other
	{ 0x2000, 0xffff, "h",    { },                 // 0010 0000 0000 0000
		util::disasm_interface::STEP_OVER },
	{ 0x2001, 0xf8ff, "push", { Rd } },            // 0010 0rrr 0000 0001
	{ 0x2002, 0xf8ff, "pop",  { Rd } },            // 0010 0rrr 0000 0010
	{ 0x2003, 0xffff, "ret",  { },                 // 0010 0000 0000 0011
		util::disasm_interface::STEP_OUT },
	{ 0x2004, 0xfffc, "lpsw", { I2 },              // 0010 0000 0000 01nn
		util::disasm_interface::STEP_OUT },
};

// opcodes are sorted in descending order of number of bits in mask to ensure correct decoding
const struct instruction mn1613_table[] =
{
	{ 0x1707, 0xffff, "popm", { } },               // 0001 0111 0000 0111
	{ 0x170f, 0xffff, "pshm", { } },               // 0001 0111 0000 1111
	{ 0x2607, 0xffff, "bd",   { Iw } },            // 0010 0110 0000 0111  a16
	{ 0x2617, 0xffff, "bald", { Iw },              // 0010 0110 0001 0111  a16
		util::disasm_interface::STEP_OVER },
	{ 0x270f, 0xffff, "bl",   { Iw } },            // 0010 0111 0000 1111  a16
	{ 0x271f, 0xffff, "ball", { Iw },              // 0010 0111 0001 1111  a16
		util::disasm_interface::STEP_OVER },
	{ 0x3f07, 0xffff, "retl", { },                 // 0011 1111 0000 0111
		util::disasm_interface::STEP_OUT },
	{ 0x3f17, 0xffff, "blk",  { } },               // 0011 1111 0001 0111

	{ 0x2704, 0xfffc, "br",   { Ri } },            // 0010 0111 0000 01ii
	{ 0x2714, 0xfffc, "balr", { Ri },              // 0010 0111 0001 01ii
		util::disasm_interface::STEP_OVER },

	{ 0x0f07, 0xff8f, "lb",   { BR, Iw } },        // 0000 1111 0bbb 0111  a16
	{ 0x0f0f, 0xff8f, "ls",   { SR, Iw } },        // 0000 1111 0ppp 1111  a16
	{ 0x0f87, 0xff8f, "stb",  { BR, Iw } },        // 0000 1111 1bbb 0111  a16
	{ 0x0f8f, 0xff8f, "sts",  { SR, Iw } },        // 0000 1111 1ppp 1111  a16

	{ 0x1f0f, 0xff0f, "fix",  { R0, D0, SK } },    // 0001 1111 kkkk 1111
	{ 0x1f07, 0xff0f, "flt",  { D0, R0, SK } },    // 0001 1111 kkkk 0111

	{ 0x2010, 0xf8fc, "wtr",  { Rd, Ri } },        // 0010 0rrr 0001 00ii
	{ 0x2014, 0xf8fc, "rdr",  { Rd, Ri } },        // 0010 0rrr 0001 01ii
	{ 0x2708, 0xffc8, "ld",   { Rs, BB, Iw } },    // 0010 0111 00bb 1rrr  a16
	{ 0x2748, 0xffc8, "std",  { Rs, BB, Iw } },    // 0010 0111 01bb 1rrr  a16

	{ 0x0f00, 0xff88, "setb", { Rs, BR } },        // 0000 1111 0bbb 0sss
	{ 0x0f08, 0xff88, "sets", { Rs, SR } },        // 0000 1111 0ppp 1sss
	{ 0x0f80, 0xff88, "cpyb", { Rs, BR } },        // 0000 1111 1bbb 0ddd
	{ 0x0f88, 0xff88, "cpys", { Rs, SR } },        // 0000 1111 1ppp 1ddd
	{ 0x3f00, 0xff88, "seth", { Rs, HR } },        // 0011 1111 0hhh 0sss
	{ 0x3f70, 0xff88, "srbt", { R0, Rs } },        // 0011 1111 0111 0sss
	{ 0x3f80, 0xff88, "cpyh", { Rs, HR } },        // 0011 1111 1hhh 0ddd
	{ 0x3ff0, 0xff88, "debp", { Rs, R0 } },        // 0011 1111 1111 0ddd
	{ 0x5700, 0xff0c, "cbr",  { R0, Ri, SK } },    // 0101 0111 kkkk 00ii
	{ 0x5708, 0xff0c, "cwr",  { R0, Ri, SK } },    // 0101 0111 kkkk 10ii
	{ 0x5f00, 0xff0c, "swr",  { R0, Ri, SK } },    // 0101 1111 kkkk 00ii
	{ 0x5f08, 0xff0c, "awr",  { R0, Ri, SK } },    // 0101 1111 kkkk 10ii
	{ 0x6700, 0xff0c, "eorr", { R0, Ri, SK } },    // 0110 0111 kkkk 00ii
	{ 0x6704, 0xff0c, "fd",   { D0, Ri, SK } },    // 0110 0111 kkkk 01ii
	{ 0x6708, 0xff0c, "orr",  { R0, Ri, SK } },    // 0110 0111 kkkk 10ii
	{ 0x670c, 0xff0c, "fm",   { D0, Ri, SK } },    // 0110 0111 kkkk 11ii
	{ 0x6f00, 0xff0c, "ladr", { R0, Ri, SK } },    // 0110 1111 kkkk 00ii
	{ 0x6f04, 0xff0c, "fs",   { D0, Ri, SK } },    // 0110 1111 kkkk 01ii
	{ 0x6f08, 0xff0c, "andr", { R0, Ri, SK } },    // 0110 1111 kkkk 10ii
	{ 0x6f0c, 0xff0c, "fa",   { D0, Ri, SK } },    // 0110 1111 kkkk 11ii
	{ 0x7700, 0xff0c, "dswr", { R0, Ri, SK } },    // 0111 0111 kkkk 00ii
	{ 0x7708, 0xff0c, "bswr", { R0, Ri, SK } },    // 0111 0111 kkkk 10ii
	{ 0x770c, 0xff0c, "d",    { R0, Ri, SK } },    // 0111 0111 kkkk 11ii
	{ 0x7f00, 0xff0c, "mvbr", { R0, Ri, SK } },    // 0111 1111 kkkk 00ii
	{ 0x7f08, 0xff0c, "mvwr", { R0, Ri, SK } },    // 0111 1111 kkkk 10ii
	{ 0x7f0c, 0xff0c, "m",    { D0, Ri, SK } },    // 0111 1111 kkkk 11ii

	{ 0x1700, 0xff08, "trst", { Rs, Iw, SK } },    // 0001 0111 kkkk 0sss  a16
	{ 0x1708, 0xff08, "tset", { Rs, Iw, SK } },    // 0001 0111 kkkk 1sss  a16
	{ 0x4704, 0xff04, "sd",   { D0, Ri, C, SK } }, // 0100 0111 kkkk c1ii
	{ 0x4f04, 0xff04, "ad",   { D0, Ri, C, SK } }, // 0100 1111 kkkk c1ii
	{ 0x5704, 0xff04, "das",  { R0, Ri, C, SK } }, // 0101 0111 kkkk c1ii
	{ 0x5f04, 0xff04, "daa",  { R0, Ri, C, SK } }, // 0101 1111 kkkk c1ii

	{ 0x5007, 0xf80f, "cbi",  { Rd, Iw, SK } },    // 0101 0ddd kkkk 0111  i16
	{ 0x500f, 0xf80f, "cwi",  { Rd, Iw, SK } },    // 0101 0ddd kkkk 1111  i16
	{ 0x5807, 0xf80f, "swi",  { Rd, Iw, SK } },    // 0101 1ddd kkkk 0111  i16
	{ 0x580f, 0xf80f, "awi",  { Rd, Iw, SK } },    // 0101 1ddd kkkk 1111  i16
	{ 0x6007, 0xf80f, "eori", { Rd, Iw, SK } },    // 0110 0ddd kkkk 0111  i16
	{ 0x600f, 0xf80f, "ori",  { Rd, Iw, SK } },    // 0110 0ddd kkkk 1111  i16
	{ 0x6807, 0xf80f, "ladi", { Rd, Iw, SK } },    // 0110 1ddd kkkk 0111  i16
	{ 0x680f, 0xf80f, "andi", { Rd, Iw, SK } },    // 0110 1ddd kkkk 1111  i16
	{ 0x780f, 0xf80f, "mvwi", { Rd, Iw, SK } },    // 0111 1ddd kkkk 1111  i16

	// these two instructions require mm != 00
	{ 0x2040, 0xf8cc, "lr",   { Rd, BB, Mi } },    // 0010 0rrr mmbb 00ii
	{ 0x2080, 0xf8cc, "lr",   { Rd, BB, Mi } },    // 0010 0rrr mmbb 00ii
	{ 0x20c0, 0xf8cc, "lr",   { Rd, BB, Mi } },    // 0010 0rrr mmbb 00ii
	{ 0x2044, 0xf8cc, "str",  { Rd, BB, Mi } },    // 0010 0rrr mmbb 01ii
	{ 0x2084, 0xf8cc, "str",  { Rd, BB, Mi } },    // 0010 0rrr mmbb 01ii
	{ 0x20c4, 0xf8cc, "str",  { Rd, BB, Mi } },    // 0010 0rrr mmbb 01ii

	{ 0x1f00, 0xff00, "neg",  { Rs, C, SK } },     // 0001 1111 kkkk cddd
};

} // anonymous namespace


std::optional<std::string> mn1610_disassembler::operand(unsigned t, u16 pc, u16 data)
{
	char const *const ee[] = { nullptr, "re", "se", "ce" };
	char const *const sk[] = { nullptr, "skp", "m", "pz", "z", "nz", "mz", "p", "ez", "enz", "oz", "onz", "lmz", "lp", "lpz", "lm" };

	switch (t)
	{
	case EA:
		switch (BIT(data, 11, 3))
		{
		case 0: return util::string_format("0x%02x", BIT(data, 0, 8));
		case 1: return util::string_format("0x%04x", pc + s8(BIT(data, 0, 8)));
		case 2: return util::string_format("[0x%02x]", BIT(data, 0, 8));
		case 3: return util::string_format("[0x%04x]", pc + s8(BIT(data, 0, 8)));
		case 4: return util::string_format("0x%02x(%s)", BIT(data, 0, 8), reg[3]);
		case 5: return util::string_format("0x%02x(%s)", BIT(data, 0, 8), reg[4]);
		case 6: return util::string_format("[0x%02x](%s)", BIT(data, 0, 8), reg[3]);
		case 7: return util::string_format("[0x%02x](%s)", BIT(data, 0, 8), reg[4]);
		}
		break;
	case Rd: return util::string_format("%s", reg[BIT(data, 8, 3)]);
	case Rs: return util::string_format("%s", reg[BIT(data, 0, 3)]);
	case I2: return util::string_format("0x%x", BIT(data, 0, 2));
	case I4: return util::string_format("0x%x", BIT(data, 0, 4));
	case Ib: return util::string_format("0x%02x", BIT(data, 0, 8));
	case EE:
		if (BIT(data, 0, 2))
			return util::string_format("%s", ee[BIT(data, 0, 2)]);
		break;
	case SK:
		if (BIT(data, 4, 4))
			return util::string_format("%s", sk[BIT(data, 4, 4)]);
		break;
	}

	return std::nullopt;
}

offs_t mn1610_disassembler::disassemble(std::ostream &stream, offs_t pc, data_buffer const &opcodes, data_buffer const &params)
{
	u32 flags = SUPPORTED;
	u32 words = 1;

	u16 const op = opcodes.r16(pc);

	for (instruction const &i : mn1610_table)
	{
		if ((op & i.mask) == i.value)
		{
			util::stream_format(stream, "%-4s", i.opcode);
			flags |= i.flags;

			char delimeter = ' ';
			for (operand_type const t : i.operands)
			{
				std::optional<std::string> const o = operand(t, pc, op);
				if (o)
				{
					util::stream_format(stream, "%c%s", delimeter, *o);

					if (t == SK)
						flags |= STEP_COND;

					delimeter = ',';
				}
			}

			return words | flags;
		}
	}

	return words | flags;
}

std::optional<std::string> mn1613_disassembler::operand(unsigned t, u16 pc, u16 data)
{
	char const *const br[] = { "csbr", "ssbr", "tsr0", "tsr1", "osr0", "osr1", "osr2", "osr3" };
	char const *const sr[] = { "sbrb", "icb",  "npp",  "",     "",     "",     "",     "" };
	char const *const hr[] = { "tcr",  "tir",  "tsr",  "scr",  "ssr",  "sor",  "iisr", "" };

	switch (t)
	{
	case R0: return util::string_format("%s", reg[0]);
	case D0: return util::string_format("d%s", reg[0]);
	case Ri: return util::string_format("(%s)", reg[BIT(data, 0, 2)]);
	case Mi:
		switch (BIT(data, 6, 2))
		{
		case 1: return util::string_format("(%s)", reg[BIT(data, 0, 2)]);
		case 2: return util::string_format("-(%s)", reg[BIT(data, 0, 2)]);
		case 3: return util::string_format("(%s)+", reg[BIT(data, 0, 2)]);
		}
		break;
	case Iw: return util::string_format("0x%04x", data);
	case BB:
		if (BIT(data, 4, 2))
			return util::string_format("%s", br[BIT(data, 4, 2)]);
		break;
	case C: return util::string_format("c");
	case BR: return util::string_format("%s", br[BIT(data, 4, 3)]);
	case SR: return util::string_format("%s", sr[BIT(data, 4, 3)]);
	case HR: return util::string_format("%s", hr[BIT(data, 4, 3)]);

	default:
		return mn1610_disassembler::operand(t, pc, data);
	}

	return std::nullopt;
}

offs_t mn1613_disassembler::disassemble(std::ostream &stream, offs_t pc, data_buffer const &opcodes, data_buffer const &params)
{
	u32 flags = SUPPORTED;
	u32 words = 1;

	u16 const op = opcodes.r16(pc);

	for (instruction const &i : mn1613_table)
	{
		if ((op & i.mask) == i.value)
		{
			util::stream_format(stream, "%-4s", i.opcode);
			flags |= i.flags;

			char delimeter = ' ';
			for (operand_type const t : i.operands)
			{
				u16 const data = (t == Iw) ? opcodes.r16(pc + words++) : op;
				std::optional<std::string> const o = operand(t, pc, data);
				if (o)
				{
					util::stream_format(stream, "%c%s", delimeter, *o);

					if (t == SK)
						flags |= STEP_COND;

					delimeter = ',';
				}
			}

			return words | flags;
		}
	}

	return mn1610_disassembler::disassemble(stream, pc, opcodes, params);
}
