// license:BSD-3-Clause
// copyright-holders:superctr
/*
 *  Roland LSP (MB87837) disassembler
 */
#include "emu.h"
#include "roland_lspd.h"

namespace {

const char *const ACCUMULATOR[4] = { "", "accA", "accB", "accA!" };

// offsets 1 to 4 read a power of two instead of memory, so the coefficient becomes an immediate
const int IMMEDIATE[5] = { 0, 7, 12, 17, 22 };

void operand(std::ostream &stream, u8 offset)
{
	if (offset >= 1 && offset <= 4)
		util::stream_format(stream, "#%d", IMMEDIATE[offset]);
	else
		util::stream_format(stream, "$%02x", offset);
}

void product(std::ostream &stream, u8 offset, u8 coefficient, int shift)
{
	operand(stream, offset);
	util::stream_format(stream, "*%c%02x>>%d", BIT(coefficient, 7) ? '-' : '+', BIT(coefficient, 7) ? -s8(coefficient) & 0xff : coefficient, shift);
}

void slot_source(std::ostream &stream, int slot, u8 coefficient)
{
	switch (slot)
	{
	case 0x18: stream << "audio_out"; break;
	case 0x1a: case 0x1b: case 0x1c: case 0x1d: util::stream_format(stream, "eram%d", slot - 0x1a); break;
	case 0x1e: stream << "audio_in"; break;
	default:   util::stream_format(stream, "slot%02x", slot); break;
	}
}

} // anonymous namespace

void roland_lsp_disassembler::describe(std::ostream &stream, offs_t pc, u32 word)
{
	const int op = (word >> 21) & 7;
	const int store = (word >> 19) & 3;
	const int eram = (word >> 16) & 7;
	const int shift = BIT(word, 15) ? 5 : 7;
	const u8 offset = (word >> 8) & 0x7f;
	const u8 coefficient = word & 0xff;
	const int slot = offset & 0x1f;
	const bool replace = BIT(offset, 5);

	if (op < 6 && store)
		util::stream_format(stream, "[$%02x] = %s, ", offset, ACCUMULATOR[store]);

	switch (op)
	{
	case 0: case 1: case 2: case 3:
		util::stream_format(stream, "acc%c %s ", (op & 2) ? 'B' : 'A', (op & 1) ? " =" : "+=");
		product(stream, offset, coefficient, shift);
		break;

	case 4:
		util::stream_format(stream, "acc%c %s %c", BIT(coefficient, 4) ? 'B' : 'A',
			(BIT(coefficient, 3) && !BIT(coefficient, 6)) ? " =" : "+=", BIT(coefficient, 2) ? '-' : ' ');
		operand(stream, offset);
		util::stream_format(stream, "*mul%d%s>>%d", BIT(coefficient, 1), BIT(coefficient, 6) ? ".lo" : ".hi", shift);
		break;

	case 5:
		stream << "accA  = |";
		product(stream, offset, coefficient, shift);
		stream << '|';
		break;

	default:
		switch (slot)
		{
		case 0x0d: case 0x0e: case 0x0f:
			util::stream_format(stream, "%s %03x", (slot == 0x0d) ? "jmpn" : (slot == 0x0e) ? "jmpp" : "jmp ", coefficient << 1);
			if (store)
				util::stream_format(stream, ", %s", ACCUMULATOR[store]);
			break;

		case 0x10:
			if (store)
				util::stream_format(stream, "eram = %s", ACCUMULATOR[store]);
			else
			{
				util::stream_format(stream, "acc%c %s prev*%02x>>%d", (op & 1) ? 'B' : 'A', replace ? " =" : "+=", coefficient, 8 + shift);
			}
			break;

		case 0x13:
			util::stream_format(stream, "tap:mul0 = %s", ACCUMULATOR[store]);
			break;

		case 0x14: case 0x15:
			util::stream_format(stream, "mul%d = %s", slot - 0x14, ACCUMULATOR[store]);
			break;

		case 0x18:
			util::stream_format(stream, "audio_out = %s", ACCUMULATOR[store]);
			if (replace)
				util::stream_format(stream, ", acc%c = 0", (op & 1) ? 'B' : 'A');
			break;

		case 0x1a: case 0x1b: case 0x1c: case 0x1d: case 0x1e:
			util::stream_format(stream, "acc%c %s ", (op & 1) ? 'B' : 'A', replace ? " =" : "+=");
			slot_source(stream, slot, coefficient);
			util::stream_format(stream, "*%c%02x>>%d", BIT(coefficient, 7) ? '-' : '+', BIT(coefficient, 7) ? -s8(coefficient) & 0xff : coefficient, shift);
			break;

		default:
			util::stream_format(stream, "slot%02x = %s", slot, ACCUMULATOR[store]);
			break;
		}
		break;
	}

	if (eram)
		util::stream_format(stream, "  ; e%d", eram);
}

offs_t roland_lsp_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	describe(stream, pc, opcodes.r32(pc) & 0xffffff);
	return 1 | SUPPORTED;
}
