// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#include "emu.h"
#include "dsp56000d.h"

static char const* const CCCC[] =
{
	"cc", "ge", "ne", "pl", "nn", "ec", "lc", "gt",
	"cs", "lt", "eq", "mi", "nr", "es", "ls", "le",
};

static char const* const JJD[] =
{
	"x0,a", "x0,b", "y0,a", "y0,b", "x1,a", "x1,b", "y1,a", "y1,b",
};

static char const* const JJJD[] =
{
	"b,a", "a,b", "?", "?", "?", "?", "?", "?",
	"x0,a", "x0,b", "y0,a", "y0,b", "x1,a", "x1,b", "y1,a", "y1,b",
};

static char const* const EE[] =
{
	"mr", "ccr", "omr", nullptr,
};

static char const* const DD[] = { "x0", "x1", "y0", "y1" };
static char const* const ee[] = { "x0", "x1", "a", "b" };
static char const* const ff[] = { "y0", "y1", "a", "b" };

static char const* const DDD[] = { "a0", "b0", "a2", "b2", "a1", "b1", "a", "b" };
static char const* const LLL[] = { "a10", "b10", "x", "y", "a", "b", "ab", "ba" };
static char const* const FFF[] = { "m0", "m1", "m2", "m3", "m4", "m5", "m6", "m7" };
static char const* const NNN[] = { "n0", "n1", "n2", "n3", "n4", "n5", "n6", "n7" };
static char const* const TTT[] = { "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7" };
static char const* const GGG[] = { "?", "sr", "omr", "sp", "ssh", "ssl", "la", "lc" };

static char const* const QQQ[] = { "x0,x0", "y0,y0", "x1,x0", "y1,y0", "x0,y1", "y0,x0", "x1,y0", "y1,x1" };

static char const* const NIE[][8] =
{
	{ "move", "tfr",  "addr", "tst",  "*",    "cmp",  "subr", "cmpm" },
	{ "add",  "rnd",  "addl", "clr",  "sub",  "*",    "subl", "not"  },
	{ "add",  "adc",  "asr",  "lsr",  "sub",  "sbc",  "abs",  "ror"  },
	{ "add",  "adc",  "asl",  "lsl",  "sub",  "sbc",  "neg",  "rol"  },
	{ "add",  "tfr",  "or",   "eor",  "sub",  "cmp",  "and",  "cmpm" },
	{ "add",  "tfr",  "or",   "eor",  "sub",  "cmp",  "and",  "cmpm" },
	{ "add",  "tfr",  "or",   "eor",  "sub",  "cmp",  "and",  "cmpm" },
	{ "add",  "tfr",  "or",   "eor",  "sub",  "cmp",  "and",  "cmpm" },
};

offs_t dsp56000_disassembler::disassemble(std::ostream &stream, offs_t pc, data_buffer const &opcodes, data_buffer const &params)
{
	u32 flags = SUPPORTED;
	unsigned words = 0;

	// fetch the instruction word and consume it
	u32 const op = opcodes.r32(pc + words++) & 0x00ffffffU;

	// fetch the extension word but don't consume it yet
	u32 const ew = opcodes.r32(pc + words) & 0x00ffffffU;

	/*
	 * Special-case test for X:R and R:Y class II data moves which also have
	 * four most significant bits zero.
	 */
	if (!(op >> 20) && (op & 0x0e4000U) != 0x080000U)
	{
		/*
		 * Instructions which do not allow parallel moves:
		 *
		 * 0000 1111 CCCC aaaa aaaa aaaa  JScc xxx
		 * 0000 1110 CCCC aaaa aaaa aaaa  Jcc  xxx
		 * 0000 1101 0000 aaaa aaaa aaaa  JSR  xxx
		 * 0000 1100 0000 aaaa aaaa aaaa  JMP  xxx
		 *
		 * 0000 1011 10pp pppp 1S1b bbbb+ JSSET #n,X:pp,xxxx
		 * 0000 1011 10pp pppp 1S0b bbbb+ JSCLR #n,X:pp,xxxx
		 * 0000 1010 10pp pppp 1S1b bbbb+ JSET  #n,X:pp,xxxx
		 * 0000 1010 10pp pppp 1S0b bbbb+ JCLR  #n,X:pp,xxxx
		 * 0000 1011 10pp pppp 0S1b bbbb  BTST  #n,X:pp
		 * 0000 1011 10pp pppp 0S0b bbbb  BCHG  #n,X:pp
		 * 0000 1010 10pp pppp 0S1b bbbb  BSET  #n,X:pp
		 * 0000 1010 10pp pppp 0S0b bbbb  BCLR  #n,X:pp
		 *
		 * 0000 1011 01MM MRRR 1S1b bbbb+ JSSET #n,X:ea,xxxx
		 * 0000 1011 01MM MRRR 1S0b bbbb+ JSCLR #n,X:ea,xxxx
		 * 0000 1010 01MM MRRR 1S1b bbbb+ JSET  #n,X:ea,xxxx
		 * 0000 1010 01MM MRRR 1S0b bbbb+ JCLR  #n,X:ea,xxxx
		 * 0000 1011 01MM MRRR 0S1b bbbb  BTST  #n,X:ea
		 * 0000 1011 01MM MRRR 0S0b bbbb  BCHG  #n,X:ea
		 * 0000 1010 01MM MRRR 0S1b bbbb  BSET  #n,X:ea
		 * 0000 1010 01MM MRRR 0S0b bbbb  BCLR  #n,X:ea
		 *
		 * 0000 1011 00aa aaaa 1S1b bbbb+ JSSET #n,X:aa,xxxx
		 * 0000 1011 00aa aaaa 1S0b bbbb+ JSCLR #n,X:aa,xxxx
		 * 0000 1010 00aa aaaa 1S1b bbbb+ JSET  #n,X:aa,xxxx
		 * 0000 1010 00aa aaaa 1S0b bbbb+ JCLR  #n,X:aa,xxxx
		 * 0000 1011 00aa aaaa 0S1b bbbb  BTST  #n,X:aa
		 * 0000 1011 00aa aaaa 0S0b bbbb  BCHG  #n,X:aa
		 * 0000 1010 00aa aaaa 0S1b bbbb  BSET  #n,X:aa
		 * 0000 1010 00aa aaaa 0S0b bbbb  BCLR  #n,X:aa
		 *
		 * 0000 1011 11dd dddd 001b bbbb+ JSSET #n,S,xxxx
		 * 0000 1011 11dd dddd 000b bbbb+ JSCLR #n,S,xxxx
		 * 0000 1010 11dd dddd 001b bbbb+ JSET  #n,S,xxxx
		 * 0000 1010 11dd dddd 000b bbbb+ JCLR  #n,S,xxxx
		 * 0000 1011 11dd dddd 011b bbbb  BTST  #n,D
		 * 0000 1011 11dd dddd 010b bbbb  BCHG  #n,D
		 * 0000 1010 11dd dddd 011b bbbb  BSET  #n,D
		 * 0000 1010 11dd dddd 010b bbbb  BCLR  #n,D
		 *
		 * 0000 1011 11MM MRRR 1010 CCCC+ JScc ea
		 * 0000 1011 11MM MRRR 1000 0000+ JSR  ea
		 * 0000 1010 11MM MRRR 1010 CCCC+ Jcc  ea
		 * 0000 1010 11MM MRRR 1000 0000+ JMP  ea
		 *
		 * 0000 100s W1MM MRRR 1Spp pppp+ MOVEP
		 * 0000 100S W1MM MRRR 01pp pppp+ MOVEP
		 * 0000 100S W1dd dddd 00pp pppp  MOVEP
		 *
		 * 0000 0111 W1MM MRRR 10dd dddd+ MOVE(M)
		 * 0000 0111 W0aa aaaa 00dd dddd  MOVE(M)
		 *
		 * 0000 0110 iiii iiii 1010 hhhh  REP   #xxx
		 * 0000 0110 11dd dddd 0010 0000  REP   S
		 * 0000 0110 01MM MRRR 0s10 0000  REP   X:ea
		 * 0000 0110 00aa aaaa 0s10 0000  REP   X:aa
		 * 0000 0110 iiii iiii 1000 hhhh+ DO    #xxx,expr
		 * 0000 0110 11DD DDDD 0000 0000+ DO    S,expr
		 * 0000 0110 01MM MRRR 0S00 0000+ DO    X:ea,expr
		 * 0000 0110 00aa aaaa 0S00 0000+ DO    X:aa,expr
		 *
		 * 0000 0101 iiii iiii 101d dddd  MOVE(C) #xx,D1
		 * 0000 0101 W1MM MRRR 0s1d dddd+ MOVE(C)
		 * 0000 0101 W0aa aaaa 0s1d dddd  MOVE(C)
		 *
		 * 0000 0100 W1ee eeee 101d dddd  MOVE(C) S1,D1
		 * 0000 0100 010M MRRR 0001 dddd  LUA   ea,D
		 *
		 * 0000 0011 CCCC 0ttt 0JJJ DTTT  Tcc   S1,D1  S2,D2
		 *
		 * 0000 0010 CCCC 0000 0JJJ D000  Tcc   S1,D1
		 *
		 * 0000 0001 1101 1RRR 0001 d101  NORM  Rn,D
		 * 0000 0001 1000 0000 01JJ d000  DIV   S,D
		 *
		 * 0000 0000 iiii iiii 1111 10EE  OR(I) #xx,D
		 * 0000 0000 iiii iiii 1011 10EE  AND(I) #xx,D
		 * 0000 0000 0000 0000 1000 1100  ENDDO
		 * 0000 0000 0000 0000 1000 0111  STOP
		 * 0000 0000 0000 0000 1000 0110  WAIT
		 * 0000 0000 0000 0000 1000 0100  RESET
		 * 0000 0000 0000 0000 0000 1100  RTS
		 * 0000 0000 0000 0000 0000 0110  SWI
		 * 0000 0000 0000 0000 0000 0101  ILLEGAL
		 * 0000 0000 0000 0000 0000 0100  RTI
		 * 0000 0000 0000 0000 0000 0000  NOP
		 */
		switch ((op >> 16) & 15)
		{
		case 0x0:
			switch (op & 0xff)
			{
			case 0x00: util::stream_format(stream, "nop"); break;
			case 0x04: util::stream_format(stream, "rti"); flags |= STEP_OUT; break;
			case 0x05: util::stream_format(stream, "illegal"); break;
			case 0x06: util::stream_format(stream, "swi"); break;
			case 0x0c: util::stream_format(stream, "rts"); flags |= STEP_OUT; break;
			case 0x84: util::stream_format(stream, "reset"); break;
			case 0x86: util::stream_format(stream, "wait"); break;
			case 0x87: util::stream_format(stream, "stop"); break;
			case 0x8c: util::stream_format(stream, "enddo"); break;

			case 0xf8:
			case 0xf9:
			case 0xfa:
				util::stream_format(stream, "ori   #$%x,%s", u8(op >> 8), EE[op & 3]);
				break;

			case 0xb8:
			case 0xb9:
			case 0xba:
				util::stream_format(stream, "andi  #$%x,%s", u8(op >> 8), EE[op & 3]);
				break;
			}
			break;
		case 0x1:
			switch (op & 0x00f8c7U)
			{
			case 0x008040U: util::stream_format(stream, "div   %s", JJD[(op >> 3) & 7]); break;
			case 0x00d805U: util::stream_format(stream, "norm  r%d,%c", (op >> 8) & 7, BIT(op, 3) ? 'B' : 'A'); break;
			}
			break;
		case 0x2: util::stream_format(stream, "t%s   %s", CCCC[(op >> 12) & 15], JJJD[(op >> 3) & 7]); break;
		case 0x3: util::stream_format(stream, "t%s   %s   r%d,r%d", CCCC[(op >> 12) & 15], JJJD[(op >> 3) & 7], (op >> 8) & 7, op & 7); break;
		case 0x4:
			switch (op & 0x0000e0U)
			{
			case 0x000000U: util::stream_format(stream, "lua   %s,%s", ea((op >> 8) & 31), reg(op & 31)); break;
			case 0x0000a0U:
				if (BIT(op, 15))
					util::stream_format(stream, "%-16s%s,%s", "movec", reg((op >> 8) & 63), reg(op & 63));
				else
					util::stream_format(stream, "%-16s%s,%s", "movec", reg(op & 63), reg((op >> 8) & 63));
				break;
			}
			break;
		case 0x5:
			if (BIT(op, 7))
			{
				// move control register (immediate short)
				util::stream_format(stream, "%-16s#<$%x,%s", "movec", u8(op >> 8), reg(op & 63));
			}
			else if (BIT(op, 14))
			{
				// move control register (effective address)
				char const S = BIT(op, 6) ? 'y' : 'x';
				unsigned const MMMRRR = (op >> 8) & 63;

				if (BIT(op, 15))
					if (MMMRRR == 0x34)
						util::stream_format(stream, "%-16s#$%x,%s", "movec", u16(ew), reg(op & 63));
					else
						util::stream_format(stream, "%-16s%c:%s,%s", "movec", S, ea(MMMRRR, ew), reg(op & 63));
				else
					util::stream_format(stream, "%-16s%s,%c:%s", "movec", reg(op & 63), S, ea(MMMRRR, ew));

				// consume immediate or absolute
				if ((MMMRRR >> 3) == 6)
					words++;
			}
			else
			{
				// move control register (absolute short)
				u16 const aa = (op >> 8) & 63;
				char const S = BIT(op, 6) ? 'y' : 'x';

				if (BIT(op, 15))
					util::stream_format(stream, "%-16s%c:<$%x,%s", "movec", S, aa, reg(op & 63));
				else
					util::stream_format(stream, "%-16s%s,%c:<$%x", "movec", reg(op & 63), S, aa);
			}
			break;
		case 0x6:
			if (BIT(op, 5))
				// repeat next instruction
				util::stream_format(stream, "rep   ");
			else
				// start hardware loop
				util::stream_format(stream, "do    ");

			if (BIT(op, 7))
			{
				// immediate short
				u16 const count = (op & 15) << 8 | u8(op >> 8);

				util::stream_format(stream, "#$%x", count);
			}
			else
			{
				char const s = BIT(op, 6) ? 'y' : 'x';
				unsigned const constant = (op >> 8) & 63;

				switch (op & 0x00c000U)
				{
				case 0x000000U: util::stream_format(stream, "%c:<$%x", s, constant); break;
				case 0x004000U: util::stream_format(stream, "%c:%s", s, ea(constant)); break;
				case 0x00c000U: util::stream_format(stream, "%s", reg(constant)); break;
				}
			}

			// append expr and consume extension word
			if (!BIT(op, 5))
			{
				util::stream_format(stream, ",$%x", u16(ew));
				words++;
			}
			break;
		case 0x7:
			if (BIT(op, 7))
			{
				// move program memory (effective address)
				unsigned const MMMRRR = (op >> 8) & 63;

				if (BIT(op, 15))
					util::stream_format(stream, "%-16sp:%s,%s", "movem", ea(MMMRRR, ew), reg(op & 63));
				else
					util::stream_format(stream, "%-16s%s,p:%s", "movem", reg(op & 63), ea(MMMRRR, ew));

				// consume absolute
				if ((MMMRRR >> 3) == 6)
					words++;
			}
			else
			{
				// move program memory (absolute short)
				u16 const aa = (op >> 8) & 63;

				if (BIT(op, 15))
					util::stream_format(stream, "%-16sp:<$%x,%s", "movem", aa, reg(op & 63));
				else
					util::stream_format(stream, "%-16s%s,p:<$%x", "movem", reg(op & 63), aa);
			}
			break;
		case 0x8:
		case 0x9:
			{
				u16 const pp = 0xffc0 | ((op >> 8) & 63);

				if (BIT(op, 7))
				{
					// move peripheral data (X or Y reference)
					char const S = BIT(op, 6) ? 'y' : 'x';
					unsigned const MMMRRR = (op >> 8) & 63;
					char const s = BIT(op, 16) ? 'y' : 'x';

					if (BIT(op, 15))
						if (MMMRRR == 0x34)
							util::stream_format(stream, "%-16s#$%x,%c:<<$%x", "movep", ew, s, pp);
						else
							util::stream_format(stream, "%-16s%c:%s,%c:<<$%x", "movep", S, ea(MMMRRR, ew), s, pp);
					else
						util::stream_format(stream, "%-16s%c:<<$%x,%c:%s", "movep", s, pp, S, ea(MMMRRR, ew));

					// consume immediate or absolute
					if ((MMMRRR >> 3) == 6)
						words++;
				}
				else if (BIT(op, 6))
				{
					// move peripheral data (P reference)
					unsigned const MMMRRR = (op >> 8) & 63;
					char const S = BIT(op, 6) ? 'y' : 'x';

					if (BIT(op, 15))
						util::stream_format(stream, "%-16sp:%s,%c:<<$%x", "movep", ea(MMMRRR, ew), S, pp);
					else
						util::stream_format(stream, "%-16s%c:<<$%x,p:%s", "movep", S, pp, ea(MMMRRR, ew));

					// consume absolute
					if ((MMMRRR >> 3) == 6)
						words++;
				}
				else
				{
					// move peripheral data (register)
					char const S = BIT(op, 16) ? 'y' : 'x';
					unsigned const dddddd = (op >> 8) & 63;

					if (BIT(op, 15))
						util::stream_format(stream, "%-16s%s,%c:<<$%x", "movep", reg(dddddd), S, pp);
					else
						util::stream_format(stream, "%-16s%c:<<$%x,%s", "movep", S, pp, reg(dddddd));
				}
			}
			break;
		case 0xa:
		case 0xb:
			if ((op & 0x00c000U) == 0x00c000U)
			{
				if (BIT(op, 7))
				{
					// branches (effective address)
					unsigned const MMMRRR = (op >> 8) & 63;

					switch (op & 0x010020U)
					{
					case 0x000000U: util::stream_format(stream, "jmp   %s", ea(MMMRRR, ew)); break;
					case 0x000020U: util::stream_format(stream, "j%s   %s", CCCC[op & 15], ea(MMMRRR, ew)); flags |= STEP_COND; break;
					case 0x010000U: util::stream_format(stream, "jsr   %s", ea(MMMRRR, ew)); flags |= STEP_OVER; break;
					case 0x010020U: util::stream_format(stream, "js%s  %s", CCCC[op & 15], ea(MMMRRR, ew)); flags |= STEP_OVER | STEP_COND; break;
					}

					// consume absolute
					if ((MMMRRR >> 3) == 6)
						words++;
				}
				else
				{
					// bit operations (register)
					unsigned const dddddd = (op >> 8) & 63;

					switch (op & 0x010060U)
					{
					case 0x000000U: util::stream_format(stream, "jclr  #%d,%s,$%x", op & 31, reg(dddddd), u16(ew)); words++; flags |= STEP_COND; break;
					case 0x000020U: util::stream_format(stream, "jset  #%d,%s,$%x", op & 31, reg(dddddd), u16(ew)); words++; flags |= STEP_COND; break;
					case 0x000040U: util::stream_format(stream, "bclr  #%d,%s", op & 31, reg(dddddd)); break;
					case 0x000060U: util::stream_format(stream, "bset  #%d,%s", op & 31, reg(dddddd)); break;
					case 0x010000U: util::stream_format(stream, "jsclr #%d,%s,$%x", op & 31, reg(dddddd), u16(ew)); words++; flags |= STEP_OVER | STEP_COND; break;
					case 0x010020U: util::stream_format(stream, "jsset #%d,%s,$%x", op & 31, reg(dddddd), u16(ew)); words++; flags |= STEP_OVER | STEP_COND; break;
					case 0x010040U: util::stream_format(stream, "bchg  #%d,%s", op & 31, reg(dddddd)); break;
					case 0x010060U: util::stream_format(stream, "btst  #%d,%s", op & 31, reg(dddddd)); break;
					}
				}
			}
			else
			{
				// bit operations (memory)

				// opcode and bit number
				switch (op & 0x0100a0U)
				{
				case 0x000000U: util::stream_format(stream, "bclr  #%d", op & 31); break;
				case 0x000020U: util::stream_format(stream, "bset  #%d", op & 31); break;
				case 0x000080U: util::stream_format(stream, "jclr  #%d", op & 31); flags |= STEP_COND; break;
				case 0x0000a0U: util::stream_format(stream, "jset  #%d", op & 31); flags |= STEP_COND; break;
				case 0x010000U: util::stream_format(stream, "bchg  #%d", op & 31); break;
				case 0x010020U: util::stream_format(stream, "btst  #%d", op & 31); break;
				case 0x010080U: util::stream_format(stream, "jsclr #%d", op & 31); flags |= STEP_OVER | STEP_COND; break;
				case 0x0100a0U: util::stream_format(stream, "jsset #%d", op & 31); flags |= STEP_OVER | STEP_COND; break;
				}

				// second operand (address)
				char const S = BIT(op, 6) ? 'y' : 'x';
				unsigned const constant = (op >> 8) & 63;
				switch (op & 0x00c000U)
				{
				case 0x000000U: util::stream_format(stream, ",%c:<$%x", S, constant); break;
				case 0x004000U: util::stream_format(stream, ",%c:%s", S, ea(constant)); break;
				case 0x008000U: util::stream_format(stream, ",%c:<<$%x", S, u16(0xffc0 | constant)); break;
				}

				// branch target
				if (BIT(op, 7))
				{
					util::stream_format(stream, ",$%x", u16(ew));
					words++;
				}
			}
			break;
		case 0xc: util::stream_format(stream, "jmp   <$%x", op & 0xfff); break;
		case 0xd: util::stream_format(stream, "jsr   <$%x", op & 0xfff); flags |= STEP_OVER; break;
		case 0xe: util::stream_format(stream, "j%s   <$%x", CCCC[(op >> 12) & 15], op & 0xfff); flags |= STEP_COND; break;
		case 0xf: util::stream_format(stream, "js%s  <$%x", CCCC[(op >> 12) & 15], op & 0xfff); flags |= STEP_OVER | STEP_COND; break;
		}
	}
	else
	{
		std::string alu;

		if (BIT(op, 7))
		{
			// multiply instructions
			// 1QQQ dkkk
			char const d = BIT(op, 3) ? 'b' : 'a';

			switch (op & 7)
			{
			case 0: alu = util::string_format("mpy   %s,%c", QQQ[(op >> 4) & 7], d); break;
			case 1: alu = util::string_format("mpyr  %s,%c", QQQ[(op >> 4) & 7], d); break;
			case 2: alu = util::string_format("mac   %s,%c", QQQ[(op >> 4) & 7], d); break;
			case 3: alu = util::string_format("macr  %s,%c", QQQ[(op >> 4) & 7], d); break;
			case 4: alu = util::string_format("mpy  -%s,%c", QQQ[(op >> 4) & 7], d); break;
			case 5: alu = util::string_format("mpyr -%s,%c", QQQ[(op >> 4) & 7], d); break;
			case 6: alu = util::string_format("mac  -%s,%c", QQQ[(op >> 4) & 7], d); break;
			case 7: alu = util::string_format("macr -%s,%c", QQQ[(op >> 4) & 7], d); break;
			}
		}
		else
		{
			// non-multiply instructions
			// 0JJJ Dkkk
			unsigned const JJJ = (op >> 4) & 7;
			char const D = BIT(op, 3) ? 'b' : 'a';

			switch (JJJ)
			{
			case 0:

				switch (op & 15)
				{
				case 0:
					alu = "move";
					break;
				case 8:
					// special case #1
					break;
				default:
					alu = util::string_format("%-5s %c", NIE[JJJ][op & 7], D);
					break;
				}
				break;
			case 1: alu = util::string_format("%-5s %c", NIE[JJJ][op & 7], D); break;
			case 2:
				// special case #2
				if (BIT(op, 1))
					alu = util::string_format("%-5s %c", NIE[JJJ][op & 7], D);
				else
					alu = util::string_format("%-5s x1x0,%c", NIE[JJJ][op & 7], D);
				break;
			case 3:
				// special case #2
				if (BIT(op, 1))
					alu = util::string_format("%-5s %c", NIE[JJJ][op & 7], D);
				else
					alu = util::string_format("%-5s y1y0,%c", NIE[JJJ][op & 7], D);
				break;
			case 4: alu = util::string_format("%-5s x0_0,%c", NIE[JJJ][op & 7], D); break;
			case 5: alu = util::string_format("%-5s y0_0,%c", NIE[JJJ][op & 7], D); break;
			case 6: alu = util::string_format("%-5s x1_0,%c", NIE[JJJ][op & 7], D); break;
			case 7: alu = util::string_format("%-5s y1_0,%c", NIE[JJJ][op & 7], D); break;
			}
		}

		util::stream_format(stream, "%-16s", alu);

		if (BIT(op, 23))
		{
			// XY memory data move
			// 1Wmm eeff WrrM MRRR OPER CODE
			unsigned const eax = (op & 0x001800U) ? (op >> 8) & 31 : ((op >> 8) & 31) | 32;
			unsigned const eay = eax ^ 4;

			std::string x;
			std::string y;

			if (BIT(op, 15))
				x = util::string_format("x:%s,%s", ea(eax), ee[(op >> 18) & 3]);
			else
				x = util::string_format("%s,x:%s", ee[(op >> 18) & 3], ea(eax));

			if (BIT(op, 22))
				y = util::string_format("y:%s,%s", ea(eay), ff[(op >> 16) & 3]);
			else
				y = util::string_format("%s,y:%s", ff[(op >> 16) & 3], ea(eay));

			util::stream_format(stream, "%-16s%s", x, y);
		}
		else if (BIT(op, 22))
		{
			bool const W = BIT(op, 15);
			unsigned const constant = (op >> 8) & 63;

			if (op & 0x340000U)
			{
				// X memory data move
				// 01dd 0ddd W1MM MRRR OPER CODE+
				// 01dd 0ddd W0aa aaaa OPER CODE

				// Y memory data move
				// 01dd 1ddd W1MM MRRR OPER CODE+
				// 01dd 1ddd W0aa aaaa OPER CODE

				char const S = BIT(op, 19) ? 'y' : 'x';
				std::string const r = reg(((op >> 17) & 0x18) | ((op >> 16) & 7));

				if (BIT(op, 14))
				{
					// effective address
					if (W)
						if (constant == 0x34)
							util::stream_format(stream, "#$%x,%s", ew, r);
						else
							util::stream_format(stream, "%c:%s,%s", S, ea(constant, ew), r);
					else
						util::stream_format(stream, "%s,%c:%s", r, S, ea(constant, ew));

					// consume immediate or absolute
					if ((constant >> 3) == 6)
						words++;
				}
				else
				{
					// absolute short
					if (W)
						util::stream_format(stream, "%c:<$%x,%s", S, constant, r);
					else
						util::stream_format(stream, "%s,%c:<$%x", r, S, constant);
				}
			}
			else
			{
				// long memory data move
				// 0100 L0LL W1MM MRRR OPER CODE+
				// 0100 L0LL W0aa aaaa OPER CODE

				char const *const r = LLL[((op >> 17) & 0x4) | ((op >> 16) & 3)];

				if (BIT(op, 14))
				{
					// effective address
					if (W)
						util::stream_format(stream, "l:%s,%s", ea(constant, ew), r);
					else
						util::stream_format(stream, "%s,l:%s", r, ea(constant, ew));

					// consume absolute
					if ((constant >> 3) == 6)
						words++;
				}
				else
				{
					// absolute short
					if (W)
						util::stream_format(stream, "l:<$%x,%s", constant, r);
					else
						util::stream_format(stream, "%s,l:<$%x", r, constant);
				}
			}
		}
		else if (BIT(op, 21))
		{
			if (op & 0x1c0000U)
			{
				// immediate short data move
				// 001d dddd iiii iiii OPER CODE

				util::stream_format(stream, "#<$%x,%s", u8(op >> 8), reg((op >> 16) & 31));
			}
			else
				switch (op & 0x03e000U)
				{
				case 0x000000U:
					// no parallel data move
					// 0010 0000 0000 0000 OPER CODE
					break;

				case 0x004000U:
					// address register update
					// 0010 0000 010M MRRR OPER CODE
					util::stream_format(stream, "%s", ea((op >> 8) & 31));
					break;

				default:
					// register to register data move
					// 0010 00ee eeed dddd OPER CODE
					util::stream_format(stream, "%s,%s", reg((op >> 13) & 31), reg((op >> 8) & 31));
					break;
				}
		}
		else if (BIT(op, 20))
		{
			unsigned const MMMRRR = (op >> 8) & 63;

			if (BIT(op, 14))
			{
				// register and Y memory data move (class I)
				// 0001 deff W1MM MRRR OPER CODE+

				// S1,D1
				std::string const op1 = util::string_format("%c,%s", BIT(op, 19) ? 'b' : 'a', BIT(op, 18) ? "x1" : "x0");

				if (BIT(op, 15))
					if (MMMRRR == 0x34)
						util::stream_format(stream, "%-16s#$%x,%s", op1, ew, ff[(op >> 16) & 3]);
					else
						util::stream_format(stream, "%-16sy:%s,%s", op1, ea(MMMRRR, ew), ff[(op >> 16) & 3]);
				else
					util::stream_format(stream, "%-16s%s,y:%s", op1, ff[(op >> 16) & 3], ea(MMMRRR, ew));
			}
			else
			{
				// X memory and register data move (class I)
				// 0001 ffdf W0MM MRRR OPER CODE+

				// TODO: documentation shows ff for S1,D1 but should be ee
				std::string op1;

				if (BIT(op, 15))
					if (MMMRRR == 0x34)
						op1 = util::string_format("#$%x,%s", ew, ee[(op >> 18) & 3]);
					else
						op1 = util::string_format("x:%s,%s", ea(MMMRRR, ew), ee[(op >> 18) & 3]);
				else
					op1 = util::string_format("%s,x:%s", ee[(op >> 18) & 3], ea(MMMRRR, ew));

				// S2,D2
				util::stream_format(stream, "%-16s%c,%s", op1, BIT(op, 17) ? 'b' : 'a', BIT(op, 16) ? "y1" : "y0");
			}

			// consume immediate or absolute
			if ((MMMRRR >> 3) == 6)
				words++;
		}
		else
		{
			// TODO: documentation states long absolute and long immediate are
			// excluded but shows optional effective address extension word and
			// includes those effective addressing modes in the list?

			char const d = BIT(op, 16) ? 'b' : 'a';
			unsigned const MMMRRR = (op >> 8) & 63;

			if (BIT(op, 15))
			{
				// register and Y memory data move (class II)
				// 0000 100d 10MM MRRR OPER CODE +

				std::string const op1 = string_format("y0,%c", d);
				util::stream_format(stream, "%-16s%c,y:%s", op1, d, ea(MMMRRR));
			}
			else
			{
				// X memory and register data move (class II)
				// 0000 100d 00MM RRRR OPER CODE +

				std::string const op1 = util::string_format("%c,x:%s", d, ea(MMMRRR));
				util::stream_format(stream, "%-16sx0,%c", op1, d);
			}
		}
	}

	return words | flags;
}

std::string dsp56000_disassembler::reg(unsigned const dddddd) const
{
	switch (dddddd >> 3)
	{
	case 0:
		if (BIT(dddddd, 2))
			return DD[dddddd & 3];
		break;
	case 1: return DDD[dddddd & 7];
	case 2: return TTT[dddddd & 7];
	case 3: return NNN[dddddd & 7];
	case 4: return FFF[dddddd & 7];
	case 7: return GGG[dddddd & 7];
	}

	return std::string("reserved");
}

std::string dsp56000_disassembler::ea(unsigned const MMMRRR, u16 const absolute) const
{
	unsigned const rrr = MMMRRR & 7;

	switch (MMMRRR >> 3)
	{
	case 0: return util::string_format("(r%d)-n%d", rrr, rrr);
	case 1: return util::string_format("(r%d)+n%d", rrr, rrr);
	case 2: return util::string_format("(r%d)-", rrr);
	case 3: return util::string_format("(r%d)+", rrr);
	case 4: return util::string_format("(r%d)", rrr);
	case 5: return util::string_format("(r%d+n%d)", rrr, rrr);
	case 6: return util::string_format(">$%x", absolute);
	case 7: return util::string_format("-(r%d)", rrr);
	}

	return std::string("invalid");
}
