// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#include "emu.h"
#include "rompdasm.h"

#define R2 ((op >> 4) & 15)
#define R3 (op & 15)
#define R3_0(i, r3) (r3 ? util::string_format("0x%x(%s)", i, gpr[r3]) : util::string_format("0x%x", i))

// repurpose reserved bit for invalid condition
#define N (BIT(op, 7) ? R2 : 13)

static char const *const gpr[16] =
{
	"r0",  "r1",  "r2",  "r3",  "r4",  "r5",  "r6",  "r7",
	"r8",  "r9",  "r10", "r11", "r12", "r13", "r14", "r15",
};

static char const *const scr[16] =
{
	"scr0", "scr1", "scr2", "scr3", "scr4", "scr5", "cous", "cou",
	"ts",   "ecr",  "mq",   "mpcs", "irb",  "iar",  "ics",  "cs",
};

static char const *const cc[16] =
{
	"",    "ge", "ne", "le", "nc", "?", "no", "ntb",
	"nop", "lt", "eq", "gt", "c",  "?", "o",  "tb",
};

static char const *const tc[8] =
{
	"nop", "igt", "ieq", "ige", "ile", "ine", "ile", "rap"
};

offs_t romp_disassembler::disassemble(std::ostream &stream, offs_t pc, data_buffer const &opcodes, data_buffer const &params)
{
	u16 const op = opcodes.r16(pc);
	unsigned bytes = 2;
	u32 flags = 0;

	switch (op >> 12)
	{
		// JI format
	case 0x0: // jump on [not] condition bit
		util::stream_format(stream, "j%-4s  0x%x", cc[(op >> 8) & 15], pc + (s32(s8(op)) << 1));
		if (((op >> 8) & 7) != 0)
			flags = STEP_COND;
		break;

		// DS format
	case 0x1: util::stream_format(stream, "stcs   %s,%s", gpr[R2], R3_0((op >> 8) & 15, R3)); break; // store character short
	case 0x2: util::stream_format(stream, "sths   %s,%s", gpr[R2], R3_0((op >> 7) & 30, R3)); break; // store half short
	case 0x3: util::stream_format(stream, "sts    %s,%s", gpr[R2], R3_0((op >> 6) & 60, R3)); break; // store short
	case 0x4: util::stream_format(stream, "lcs    %s,%s", gpr[R2], R3_0((op >> 8) & 15, R3)); break; // load character short
	case 0x5: util::stream_format(stream, "lhas   %s,%s", gpr[R2], R3_0((op >> 7) & 30, R3)); break; // load half algebraic short
	case 0x6: // compute address short (X format)
		if (R3)
			util::stream_format(stream, "cas    %s,%s,%s", gpr[(op >> 8) & 15], gpr[R2], gpr[R3]);
		else
			util::stream_format(stream, "cas    %s,%s", gpr[(op >> 8) & 15], gpr[R2]);
		break;
	case 0x7: util::stream_format(stream, "ls     %s,%s", gpr[R2], R3_0((op >> 6) & 60, R3)); break; // load short

	case 0x8:
		// BI, BA format
		{
			u32 const b = opcodes.r32(pc);
			bytes = 4;

			switch (op >> 8)
			{
			// 80-87
			case 0x88: util::stream_format(stream, "b%-4s  0x%x", cc[N - 8], pc + (s32(b << 12) >> 11)); break; // branch on not condition bit immediate
			case 0x89: util::stream_format(stream, "b%-4s  0x%x", util::string_format("%sx", cc[N - 8]), pc + (s32(b << 12) >> 11)); break; // branch on not condition bit immediate with execute
			case 0x8a: util::stream_format(stream, "bala   0x%x", b & 0x00fffffeU); flags |= STEP_OVER; break; // branch and link absolute
			case 0x8b: util::stream_format(stream, "balax  0x%x", b & 0x00fffffeU); flags |= STEP_OVER | step_over_extra(1); break; // branch and link absolute with execute
			case 0x8c: util::stream_format(stream, "bali   %s,0x%x", gpr[R2], pc + (s32(b << 12) >> 11)); flags |= STEP_OVER; break; // branch and link immediate
			case 0x8d: util::stream_format(stream, "balix  %s,0x%x", gpr[R2], pc + (s32(b << 12) >> 11)); flags |= STEP_OVER | step_over_extra(1); break; // branch and link immediate with execute
			case 0x8e: // branch on condition bit immediate
				util::stream_format(stream, "b%-4s  0x%x", cc[N], pc + (s32(b << 12) >> 11));
				if (N != 8)
					flags |= STEP_COND;
				break;
			case 0x8f: // branch on condition bit immediate with execute
				util::stream_format(stream, "b%-4s  0x%x", util::string_format("%sx", cc[N]), pc + (s32(b << 12) >> 11));
				if (N != 8)
					flags |= STEP_COND | step_over_extra(1);
				break;
			}
		}
		break;

	case 0xc:
	case 0xd:
		// D format
		{
			u16 const i = opcodes.r16(pc + 2);
			bytes = 4;

			switch (op >> 8)
			{
			case 0xc0: util::stream_format(stream, "svc    %s", R3_0(i, R3)); flags |= STEP_OVER; break; // supervisor call
			case 0xc1: util::stream_format(stream, "ai     %s,%s,0x%x", gpr[R2], gpr[R3], s16(i)); break; // add immediate
			case 0xc2: util::stream_format(stream, "cal16  %s,%s", gpr[R2], R3_0(i, R3)); break; // compute address lower half 16-bit
			case 0xc3: util::stream_format(stream, "oiu    %s,%s,0x%x", gpr[R2], gpr[R3], i); break; // or immediate upper half
			case 0xc4: util::stream_format(stream, "oil    %s,%s,0x%x", gpr[R2], gpr[R3], i); break; // or immediate lower half
			case 0xc5: util::stream_format(stream, "nilz   %s,%s,0x%x", gpr[R2], gpr[R3], i); break; // and immediate lower half extended zeroes
			case 0xc6: util::stream_format(stream, "nilo   %s,%s,0x%x", gpr[R2], gpr[R3], i); break; // and immediate lower half extended ones
			case 0xc7: util::stream_format(stream, "xil    %s,%s,0x%x", gpr[R2], gpr[R3], i); break; // exclusive or immediate lower half
			case 0xc8: util::stream_format(stream, "cal    %s,%s", gpr[R2], R3_0(s16(i), R3)); break; // compute address lower half
			case 0xc9: util::stream_format(stream, "lm     %s,%s", gpr[R2], R3_0(s16(i), R3)); break; // load multiple
			case 0xca: util::stream_format(stream, "lha    %s,%s", gpr[R2], R3_0(s16(i), R3)); break; // load half algebraic
			case 0xcb: util::stream_format(stream, "ior    %s,%s", gpr[R2], R3_0(s16(i), R3)); break; // input/output read
			case 0xcc: // trap on condition immediate
				util::stream_format(stream, "t%-4s  %s,0x%x", tc[R2 & 7], gpr[R3], s16(i));
				if ((R2 & 7) != 0)
					flags |= STEP_OVER | STEP_COND;
				break;
			case 0xcd: util::stream_format(stream, "l      %s,%s", gpr[R2], R3_0(s16(i), R3)); break; // load
			case 0xce: util::stream_format(stream, "lc     %s,%s", gpr[R2], R3_0(s16(i), R3)); break; // load character
			case 0xcf: util::stream_format(stream, "tsh    %s,%s", gpr[R2], R3_0(s16(i), R3)); break; // test and set half

			case 0xd0: util::stream_format(stream, "lps    %d,%s", R2, R3_0(s16(i), R3)); flags |= STEP_OUT; break; // load program status
			case 0xd1: util::stream_format(stream, "aei    %s,%s,0x%x", gpr[R2], gpr[R3], s16(i)); break; // add extended immediate
			case 0xd2: util::stream_format(stream, "sfi    %s,%s,0x%x", gpr[R2], gpr[R3], s16(i)); break; // subtract from immediate
			case 0xd3: util::stream_format(stream, "cli    %s,0x%x", gpr[R3], s16(i)); break; // compare logical immediate
			case 0xd4: util::stream_format(stream, "ci     %s,0x%x", gpr[R3], s16(i)); break; // compare immediate
			case 0xd5: util::stream_format(stream, "niuz   %s,%s,0x%x", gpr[R2], gpr[R3], i); break; // and immediate upper half extended zeroes
			case 0xd6: util::stream_format(stream, "niuo   %s,%s,0x%x", gpr[R2], gpr[R3], i); break; // and immediate upper half extended ones
			case 0xd7: util::stream_format(stream, "xiu    %s,%s,0x%x", gpr[R2], gpr[R3], i); break; // exclusive or immediate upper half
			case 0xd8: util::stream_format(stream, "cau    %s,%s", gpr[R2], R3_0(i, R3)); break; // compute address upper half
			case 0xd9: util::stream_format(stream, "stm    %s,%s", gpr[R2], R3_0(s16(i), R3)); break; // store multiple
			case 0xda: util::stream_format(stream, "lh     %s,%s", gpr[R2], R3_0(s16(i), R3)); break; // load half
			case 0xdb: util::stream_format(stream, "iow    %s,%s", gpr[R2], R3_0(s16(i), R3)); break; // input/output write
			case 0xdc: util::stream_format(stream, "sth    %s,%s", gpr[R2], R3_0(s16(i), R3)); break; // store half
			case 0xdd: util::stream_format(stream, "st     %s,%s", gpr[R2], R3_0(s16(i), R3)); break; // store
			case 0xde: util::stream_format(stream, "stc    %s,%s", gpr[R2], R3_0(s16(i), R3)); break; // store character
			// df
			}
		}
		break;

	case 0x9:
	case 0xa:
	case 0xb:
	case 0xe:
	case 0xf:
		// R format
		switch (op >> 8)
		{
		case 0x90: util::stream_format(stream, "ais    %s,%d", gpr[R2], R3); break; // add immediate short
		case 0x91: util::stream_format(stream, "inc    %s,%d", gpr[R2], R3); break; // increment
		case 0x92: util::stream_format(stream, "sis    %s,%d", gpr[R2], R3); break; // subtract immediate short
		case 0x93: util::stream_format(stream, "dec    %s,%d", gpr[R2], R3); break; // decrement
		case 0x94: util::stream_format(stream, "cis    %s,%d", gpr[R2], R3); break; // compare immediate short
		case 0x95: util::stream_format(stream, "clrsb  %s,%d", scr[R2], R3); break; // clear scr bit
		case 0x96: util::stream_format(stream, "mfs    %s,%s", scr[R2], gpr[R3]); break; // move from scr
		case 0x97: util::stream_format(stream, "setsb  %s,%d", scr[R2], R3); break; // set scr bit
		case 0x98: util::stream_format(stream, "clrb   %s,%d", gpr[R2], R3); break; // clear bit upper half
		case 0x99: util::stream_format(stream, "clrb   %s,%d", gpr[R2], R3 + 16); break; // clear bit lower half
		case 0x9a: util::stream_format(stream, "setb   %s,%d", gpr[R2], R3); break; // set bit upper half
		case 0x9b: util::stream_format(stream, "setb   %s,%d", gpr[R2], R3 + 16); break; // set bit lower half
		case 0x9c: util::stream_format(stream, "mftbi  %s,%d", gpr[R2], R3); break; // move from test bit immediate upper half
		case 0x9d: util::stream_format(stream, "mftbi  %s,%d", gpr[R2], R3 + 16); break; // move from test bit immediate lower half
		case 0x9e: util::stream_format(stream, "mttbi  %s,%d", gpr[R2], R3); break; // move to test bit immediate upper half
		case 0x9f: util::stream_format(stream, "mttbi  %s,%d", gpr[R2], R3 + 16); break; // move to test bit immediate lower half

		case 0xa0: util::stream_format(stream, "sari   %s,%d", gpr[R2], R3); break; // shift algebraic right immediate
		case 0xa1: util::stream_format(stream, "sari   %s,%d", gpr[R2], R3 + 16); break; // shift algebraic right immediate plus sixteen
		// a2, a3
		case 0xa4: util::stream_format(stream, "lis    %s,%d", gpr[R2], R3); break; // load immediate short
		// a5, a6, a7
		case 0xa8: util::stream_format(stream, "sri    %s,%d", gpr[R2], R3); break; // shift right immediate
		case 0xa9: util::stream_format(stream, "sri    %s,%d", gpr[R2], R3 + 16); break; // shift right immediate plus sixteen
		case 0xaa: util::stream_format(stream, "sli    %s,%d", gpr[R2], R3); break; // shift left immediate
		case 0xab: util::stream_format(stream, "sli    %s,%d", gpr[R2], R3 + 16); break; // shift left immediate plus sixteen
		case 0xac: util::stream_format(stream, "srpi   %s,%d", gpr[R2], R3); break; // shift right paired immediate
		case 0xad: util::stream_format(stream, "srpi   %s,%d", gpr[R2], R3 + 16); break; // shift right paired immediate plus sixteen
		case 0xae: util::stream_format(stream, "slpi   %s,%d", gpr[R2], R3); break; // shift left paired immediate
		case 0xaf: util::stream_format(stream, "slpi   %s,%d", gpr[R2], R3 + 16); break; // shift left paired immediate plus sixteen

		case 0xb0: util::stream_format(stream, "sar    %s,%s", gpr[R2], gpr[R3]); break; // shift algebraic right
		case 0xb1: util::stream_format(stream, "exts   %s,%s", gpr[R2], gpr[R3]); break; // extend sign
		case 0xb2: util::stream_format(stream, "sf     %s,%s", gpr[R2], gpr[R3]); break; // subtract from
		case 0xb3: util::stream_format(stream, "cl     %s,%s", gpr[R2], gpr[R3]); break; // compare logical
		case 0xb4: util::stream_format(stream, "c      %s,%s", gpr[R2], gpr[R3]); break; // compare
		case 0xb5: util::stream_format(stream, "mts    %s,%s", scr[R2], gpr[R3]); break; // move to scr
		case 0xb6: util::stream_format(stream, "d      %s,%s", gpr[R2], gpr[R3]); break; // divide step
		// b7
		case 0xb8: util::stream_format(stream, "sr     %s,%s", gpr[R2], gpr[R3]); break; // shift right
		case 0xb9: util::stream_format(stream, "srp    %s,%s", gpr[R2], gpr[R3]); break; // shift right paired
		case 0xba: util::stream_format(stream, "sl     %s,%s", gpr[R2], gpr[R3]); break; // shift left
		case 0xbb: util::stream_format(stream, "slp    %s,%s", gpr[R2], gpr[R3]); break; // shift left paired
		case 0xbc: util::stream_format(stream, "mftb   %s,%s", gpr[R2], gpr[R3]); break; // move from test bit
		case 0xbd: util::stream_format(stream, "tgte   %s,%s", gpr[R2], gpr[R3]); break; // trap if register greater than or equal
		case 0xbe: util::stream_format(stream, "tlt    %s,%s", gpr[R2], gpr[R3]); break; // trap if register less than
		case 0xbf: util::stream_format(stream, "mttb   %s,%s", gpr[R2], gpr[R3]); break; // move to test bit

		case 0xe0: util::stream_format(stream, "abs    %s,%s", gpr[R2], gpr[R3]); break; // absolute
		case 0xe1: util::stream_format(stream, "a      %s,%s", gpr[R2], gpr[R3]); break; // add
		case 0xe2: util::stream_format(stream, "s      %s,%s", gpr[R2], gpr[R3]); break; // subtract
		case 0xe3: util::stream_format(stream, "o      %s,%s", gpr[R2], gpr[R3]); break; // or
		case 0xe4: util::stream_format(stream, "twoc   %s,%s", gpr[R2], gpr[R3]); break; // twos complement
		case 0xe5: util::stream_format(stream, "n      %s,%s", gpr[R2], gpr[R3]); break; // and
		case 0xe6: util::stream_format(stream, "m      %s,%s", gpr[R2], gpr[R3]); break; // multiply step
		case 0xe7: util::stream_format(stream, "x      %s,%s", gpr[R2], gpr[R3]); break; // exclusive or
		case 0xe8: // branch on not condition bit
			util::stream_format(stream, "b%-5s %s", util::string_format("%sr",  cc[N - 8]), gpr[R3]);
			if (N != 8)
				flags |= STEP_COND;
			if (R3 == 15)
				flags |= STEP_OUT;
			break;
		case 0xe9: // branch on not condition bit with execute
			util::stream_format(stream, "b%-5s %s", util::string_format("%srx", cc[N - 8]), gpr[R3]);
			if (N != 8)
				flags |= STEP_COND | step_over_extra(1);
			if (R3 == 15)
				flags |= STEP_OUT | step_over_extra(1);
			break;
		// ea
		case 0xeb: util::stream_format(stream, "lhs    %s,0(%s)", gpr[R2], gpr[R3]); break; // load half short
		case 0xec: util::stream_format(stream, "balr   %s,%s", gpr[R2], gpr[R3]); flags |= STEP_OVER; break; // branch and link
		case 0xed: util::stream_format(stream, "balrx  %s,%s", gpr[R2], gpr[R3]); flags |= STEP_OVER | step_over_extra(1); break; // branch and link with execute
		case 0xee: // branch on condition bit
			util::stream_format(stream, "b%-5s %s", util::string_format("%sr",  cc[N]), gpr[R3]); break;
			if (N != 8)
				flags |= STEP_COND;
			if (R3 == 15)
				flags |= STEP_OUT;
			break;
		case 0xef: // branch on condition bit with execute
			util::stream_format(stream, "b%-5s %s", util::string_format("%srx", cc[N]), gpr[R3]);
			if (N != 8)
				flags |= STEP_COND | step_over_extra(1);
			if (R3 == 15)
				flags |= STEP_OUT | step_over_extra(1);
			break;

		case 0xf0: util::stream_format(stream, "wait"); break; // wait
		case 0xf1: util::stream_format(stream, "ae     %s,%s", gpr[R2], gpr[R3]); break; // add extended
		case 0xf2: util::stream_format(stream, "se     %s,%s", gpr[R2], gpr[R3]); break; // subtract extended
		case 0xf3: util::stream_format(stream, "ca16   %s,%s", gpr[R2], gpr[R3]); break; // compute address 16-bit
		case 0xf4: util::stream_format(stream, "onec   %s,%s", gpr[R2], gpr[R3]); break; // ones complement
		case 0xf5: util::stream_format(stream, "clz    %s,%s", gpr[R2], gpr[R3]); break; // count leading zeros
		// f6, f7, f8
		case 0xf9: util::stream_format(stream, "mc03   %s,%s", gpr[R2], gpr[R3]); break; // move character zero from three
		case 0xfa: util::stream_format(stream, "mc13   %s,%s", gpr[R2], gpr[R3]); break; // move character one from three
		case 0xfb: util::stream_format(stream, "mc23   %s,%s", gpr[R2], gpr[R3]); break; // move character two from three
		case 0xfc: util::stream_format(stream, "mc33   %s,%s", gpr[R2], gpr[R3]); break; // move character three from three
		case 0xfd: util::stream_format(stream, "mc30   %s,%s", gpr[R2], gpr[R3]); break; // move character three from zero
		case 0xfe: util::stream_format(stream, "mc31   %s,%s", gpr[R2], gpr[R3]); break; // move character three from one
		case 0xff: util::stream_format(stream, "mc32   %s,%s", gpr[R2], gpr[R3]); break; // move character three from two
		}
		break;
	}

	return bytes | flags | SUPPORTED;
}
