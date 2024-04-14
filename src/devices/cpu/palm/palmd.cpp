// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#include "emu.h"
#include "palmd.h"

static const char *const r[] = { "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15" };
static const char *const jm[] = { "le", "lo", "eq", "no", "all", "allm", "nom", "ham", "hi", "he", "hl", "sb", "sn", "snm", "sm", "hsnm" };
static const char *const im[] = { "+1", "+2", "+3", "+4", "-1", "-2", "-3", "-4" };

// helper for IBM bit numbering
template <typename T, typename U, typename V> constexpr T IBIT(T x, U n, V w)
{
	return BIT(x, sizeof(T) * 8 - n - w, w);
}

// instruction field shorthand
#define Rx  IBIT(op, 4, 4)
#define Ry  IBIT(op, 8, 4)
#define DA  IBIT(op, 4, 4)
#define IMM IBIT(op, 8, 8)
#define MOD IBIT(op, 12, 4)

offs_t palm_disassembler::disassemble(std::ostream &stream, offs_t pc, data_buffer const &opcodes, data_buffer const &params)
{
	u16 const op = opcodes.r16(pc);
	u32 flags = 0;

	pc += 2;

	switch (IBIT(op, 0, 4))
	{
	case 0x0:
		switch (MOD)
		{
		case 0x0:
			if (IBIT(op, 4, 8))
				// move minus 2
				util::stream_format(stream, "mvm2  %s,%s", r[Rx], r[Ry]);
			else
				// synthetic halt (branch to self)
				util::stream_format(stream, "halt");
			break;
		case 0x1: util::stream_format(stream, "mvm1  %s,%s", r[Rx], r[Ry]); break; // move minus 1
		case 0x2: util::stream_format(stream, "mvp1  %s,%s", r[Rx], r[Ry]); break; // move plus 1
		case 0x3: util::stream_format(stream, "mvp2  %s,%s", r[Rx], r[Ry]); break; // move plus 2
		case 0x4:
			if (Rx)
				// move register
				util::stream_format(stream, "move  %s,%s", r[Rx], r[Ry]);
			else if (Ry)
			{
				// synthetic jump to register
				util::stream_format(stream, "jump  %s", r[Ry]);
				flags |= STEP_COND;
			}
			else // synthetic no operation
				util::stream_format(stream, "noop");
			break;
		case 0x5: util::stream_format(stream, "and   %s,%s", r[Rx], r[Ry]); break; // and byte
		case 0x6: util::stream_format(stream, "orb   %s,%s", r[Rx], r[Ry]); break; // or byte
		case 0x7: util::stream_format(stream, "xor   %s,%s", r[Rx], r[Ry]); break; // xor byte
		case 0x8: util::stream_format(stream, "add   %s,%s", r[Rx], r[Ry]); break; // add
		case 0x9: util::stream_format(stream, "sub   %s,%s", r[Rx], r[Ry]); break; // subtract
		case 0xa: util::stream_format(stream, "adds1 %s,%s", r[Rx], r[Ry]); break; // add special #1
		case 0xb: util::stream_format(stream, "adds2 %s,%s", r[Rx], r[Ry]); break; // add special #2
		case 0xc: util::stream_format(stream, "htl   %s,%s", r[Rx], r[Ry]); break; // high to low
		case 0xd: util::stream_format(stream, "lth   %s,%s", r[Rx], r[Ry]); break; // low to high
		case 0xe: util::stream_format(stream, "getr  $%x,%s", DA, r[Ry]); break; // get to register
		case 0xf: util::stream_format(stream, "geta  $%x,%s", DA, r[Ry]); break; // get to register and add
		}
		break;
	case 0x1: util::stream_format(stream, "ctl   $%x,#$%x", DA, IMM); break; // control
	case 0x2: util::stream_format(stream, "ldhd  %s,$%x", r[Rx], IMM * 2); break; // load halfword direct
	case 0x3: util::stream_format(stream, "sthd  %s,$%x", r[Rx], IMM * 2); break; // store halfword direct
	case 0x4:
		// put byte
		if (MOD < 8)
			util::stream_format(stream, "putb  $%x,%s,%s", DA, r[Ry], im[MOD]);
		else
			util::stream_format(stream, "putb  $%x,%s", DA, r[Ry]);
		break;
	case 0x5:
		// store halfword indirect
		if (MOD < 8)
			util::stream_format(stream, "sthi  %s,%s,%s", r[Rx], r[Ry], im[MOD]);
		else
			util::stream_format(stream, "sthi  %s,%s", r[Rx], r[Ry]);
		break;
	case 0x6:
		// load byte indirect
		if (MOD < 8)
			util::stream_format(stream, "ldbi  %s,%s,%s", r[Rx], r[Ry], im[MOD]);
		else
			util::stream_format(stream, "ldbi  %s,%s", r[Rx], r[Ry]);
		break;
	case 0x7:
		// store byte indirect
		if (MOD < 8)
			util::stream_format(stream, "stbi  %s,%s,%s", r[Rx], r[Ry], im[MOD]);
		else
			util::stream_format(stream, "stbi  %s,%s", r[Rx], r[Ry]);
		break;
	case 0x8: util::stream_format(stream, "emit  %s,#$%x", r[Rx], IMM); break; // emit byte
	case 0x9: util::stream_format(stream, "clri  %s,#$%x", r[Rx], IMM); break; // clear immediate
	case 0xa:
		if (Rx)
			// add immediate
			util::stream_format(stream, "addi  %s,#$%x", r[Rx], IMM + 1);
		else
		{
			// synthetic jump relative (forward)
			util::stream_format(stream, "jump  $%x", pc + IMM + 1);
			flags |= STEP_COND;
		}
		break;
	case 0xb: util::stream_format(stream, "seti  %s,#$%x", r[Rx], IMM); break; // set immediate
	case 0xc:
		switch (MOD)
		{
		case 0x3: case 0x4: case 0xb: case 0xc:
			// one-operand jumps
			util::stream_format(stream, "j%-4s %s", jm[MOD], r[Rx]);
			break;
		case 0x0: case 0x1: case 0x2: case 0x5:
		case 0x6: case 0x7: case 0x8: case 0x9:
		case 0xa: case 0xd: case 0xe: case 0xf:
			// two-operand jumps
			util::stream_format(stream, "j%-4s %s,%s", jm[MOD], r[Rx], r[Ry]);
			break;
		}
		flags |= STEP_COND;
		break;
	case 0xd:
		// load halfword indirect
		if (MOD < 8)
			util::stream_format(stream, "ldhi  %s,%s,%s", r[Rx], r[Ry], im[MOD]);
		else
			util::stream_format(stream, "ldhi  %s,%s", r[Rx], r[Ry]);
		break;
	case 0xe:
		if (DA == 0 && MOD >= 0xc)
		{
			switch (MOD)
			{
			case 0xc: util::stream_format(stream, "shftr %s", r[Ry]); break; // shift right 1
			case 0xd: util::stream_format(stream, "rotr  %s", r[Ry]); break; // shift right and rotate 1
			case 0xe: util::stream_format(stream, "srr3  %s", r[Ry]); break; // shift right and rotate 3
			case 0xf: util::stream_format(stream, "srr4  %s", r[Ry]); break; // shift right and rotate 4
			}
		}
		else
		{
			if (MOD < 0x8)
				// get byte
				util::stream_format(stream, "getb  $%x,%s,%s", DA, r[Ry], im[MOD]);
			else if (MOD < 0xc)
				// get byte
				util::stream_format(stream, "getb  $%x,%s", DA, r[Ry]);
			else
				// get register byte
				util::stream_format(stream, "getrb $%x,%s", DA, r[Ry]);
		}
		break;
	case 0xf:
		if (Rx)
			// subtract immediate
			util::stream_format(stream, "subi  %s,#$%x", r[Rx], IMM + 1);
		else
		{
			// synthetic jump relative (backward)
			util::stream_format(stream, "jump  $%x", pc - IMM - 1);
			flags |= STEP_COND;
		}
		break;
	}

	return 2 | flags | SUPPORTED;
}
