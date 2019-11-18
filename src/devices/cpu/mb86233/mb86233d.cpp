// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "emu.h"
#include "mb86233d.h"

/*

    Main register bank:
        0   1   2   3   4   5   6   7   8   9   a   b   c   d   e   f
    0  b0  b1  x0  x1  x2  i0  i1  i2  sp pag vsm dmc  c0  c1  pc   -
    1   a  ah  al   b  bh  bl   c  ch  cl   d  dh  dl   p  ph  pl sft

    Second register bank:
         0    1    2    3    4    5    6    7    8    9    a    b    c    d    e    f
    2 <register file>
    3 sio0  si1  pio pioa  rpc    -    -    -  pad  mod  ear   st mask  tim   cx   dx

    In the 86233/86234 the C, page, serial, parallel, timer registers
    don't seem to exist.  The register file is turned into i/o ports.
    Of the second bank part '3', only "mask" is actually used.


    Address encoding (adx uses x0/b0, ady uses x1/b1):
      0 0aaa aaaa  $a
      0 1aaa aaaa  $a(xn)
      1 0aaa aaaa  $a(xn+)
      1 100b bbbb  (bxn+$b)
      1 101b bbbb  (xn+$b)
      1 110b bbbb  [bxn+$b]
      1 111b bbbb  [xn+$b]



    3322 2222 2222 1111 1111 1100 0000 0000
    1098 7654 3210 9876 5432 1098 7654 3210

    0000 00aa aaa0 0?yy yyyy yyyx xxxx xxxx lab adx, ady (e)
    0000 00aa aaa0 11yy yyyy yyyx xxxx xxxx lab adx, ady + 0x200
    0000 00aa aaa1 00yy yyyy yyyx xxxx xxxx lab adx + 0x200, ady

    0001 11aa aaa0 0?yy yyyy yyyx xxxx xxxx mov adx, ady (e)  (the ? is not a + 0x200 select, cf. VR copro code)
    0001 11aa aaa0 10yy yyyy yyyx xxxx xxxx mov adx (e), ady
    0001 11aa aaa0 11yy yyyy yyyx xxxx xxxx mov adx, ady + 0x200
    0001 11aa aaa1 00yy yyyy yyyx xxxx xxxx mov adx + 0x200, ady
    0001 11aa aaa1 01yy yyyy yyyx xxxx xxxx mov adx (o), ady

    0001 11aa aaa1 1100 0rrr rrry yyyy yyyy mov r, ady
    0001 11aa aaa1 1100 1rrr rrry yyyy yyyy mov r, ady (e)
    0001 11aa aaa1 1101 0rrr rrry yyyy yyyy mov ady + 0x200, r
    0001 11aa aaa1 1101 1rrr rrry yyyy yyyy mov ady, r
    0001 11aa aaa1 1110 0rrr rrry yyyy yyyy mov ady (e), r
    0001 11aa aaa1 1110 1rrr rrrx xxxx xxxx mov adx (o), r
    0001 11aa aaa1 1111 0rrr rrr. ..ss ssss mov s, r

    0011 011. .... 101. vvvv vvvv vvvv vvvv stmh (bit0 = fp, bit 1-2 = rounding mode, rest unused)

    0011 10rr vvvv vvvv vvvv vvvv vvvv vvvv lipl/lia/lib/lid #v

    0011 11.a aaaa 000. rrrr rrrr rrrr rrrr clr0 <reglist>
    0011 11.a aaaa 001. ffff ffff ffff ffff clr1 flags
    0011 11.a aaaa 010. 0... .... nnnn nnnn rep #n (0 = 0x100)
    0011 11.a aaaa 010. 1... .... ..11 0100 rep rpc
    0011 11.a aaaa 011. ffff ffff ffff ffff set flags

    01rr rrrr vvvv vvvv vvvv vvvv vvvv vvvv ldi #v, r (actual v size varies 3-24 depending on register)

    1i11 11.c cccc 0000 aaaa aaaa aaaa aaaa brif cond #a
    1i11 11.c cccc 0010 .0.. ...x xxxx xxxx brul cond (adx)
    1i11 11.c cccc 0010 .1.. .... ...r rrrr brul cond r
    1i11 11.c cccc 0100 aaaa aaaa aaaa aaaa bsif cond #a
    1i11 11.c cccc 0110 .0.. ...x xxxx xxxx bsul cond (adx)
    1i11 11.c cccc 0110 .1.. .... ...r rrrr bsul cond r
    1i11 11.c cccc 1010 .... .... .... .... rtif cond
    1i11 11.c cccc 1100 .rrr rrrx xxxx xxxx ldif cond adx, r
    1011 11.1 0110 1110 .... .... .... .... iret

    Top 3 bits = instruction group, except when it isn't.


    Conditions are 5 bits, but only a subset is known, see condition()

    Alu can theorically be on C or D registers, but since C doesn't
    exist, we don't know how C access is encoded.  The operation
    itself is 5-bits for the "full" alu range.

 */



char const *const mb86233_disassembler::regnames[0x40] = {
	"b0", "b1", "x0", "x1", "x2", "i0", "i1", "i2", "sp", "pag", "vsm", "dmc", "c0", "c1", "pc", "-",
	"a", "ah", "al", "b", "bh", "bl", "c", "ch", "cl", "d", "dh", "dl", "p", "ph", "pl", "sft",
	"rf0", "rf1", "rf2", "rf3", "rf4", "rf5", "rf6", "rf7", "rf8", "rf9", "rfa", "rfb", "rfc", "rfd", "rfe", "rff",
	"sio0", "si1", "pio", "pioa", "rpc", "r?35", "r?36", "r?37", "pad", "mod", "ear", "st", "mask", "tim", "cx", "dx"
};

std::string mb86233_disassembler::condition(unsigned int cond, bool invert)
{
	std::ostringstream stream;

	if(invert)
		stream << '!';

	switch(cond) {
		case 0x00: util::stream_format(stream, "zrd"); return stream.str();
		case 0x01: util::stream_format(stream, "ged"); return stream.str();
		case 0x02: util::stream_format(stream, "led"); return stream.str();
		case 0x0a: util::stream_format(stream, "gpio0"); return stream.str();
		case 0x0b: util::stream_format(stream, "gpio1"); return stream.str();
		case 0x0c: util::stream_format(stream, "gpio2"); return stream.str();
		case 0x10: util::stream_format(stream, "zc0"); return stream.str();
		case 0x11: util::stream_format(stream, "zc1"); return stream.str();
		case 0x12: util::stream_format(stream, "gpio3"); return stream.str();
		case 0x16: util::stream_format(stream, "alw"); return stream.str();
	}

	util::stream_format(stream, "cond(%02x)", cond);
	return stream.str();
}

std::string mb86233_disassembler::regs(u32 reg)
{
	return regnames[reg & 0x3f];
}

std::string mb86233_disassembler::memory(u32 reg, bool x1, bool bank)
{
	std::ostringstream stream;

	switch(reg & 0x180) {
	case 0x000:
		if(bank)
			util::stream_format(stream, "$0x%x", 0x200 | (reg & 0x7f));
		else if((reg & 0x7f) < 10)
			util::stream_format(stream, "$%d", reg & 0x7f);
		else
			util::stream_format(stream, "$0x%x", reg & 0x7f);
		break;

	case 0x080:
		if(bank || (reg & 0x7f)) {
			if(bank)
				util::stream_format(stream, "$0x%x", 0x200 | (reg & 0x7f));
			else if((reg & 0x7f) < 10)
				util::stream_format(stream, "$%d", reg & 0x7f);
			else
				util::stream_format(stream, "$0x%x", reg & 0x7f);
		}

		stream << '(';

		if(x1)
				util::stream_format(stream, "x1");
		else
				util::stream_format(stream, "x0");
		stream << ')';
		break;

	case 0x100:
		if(bank || (reg & 0x7f)) {
			if(bank)
				util::stream_format(stream, "$0x%x", 0x200 | (reg & 0x7f));
			else if((reg & 0x7f) < 10)
				util::stream_format(stream, "$%d", reg & 0x7f);
			else
				util::stream_format(stream, "$0x%x", reg & 0x7f);
		}

		stream << '(';

		if ( x1 )
				util::stream_format(stream, "x1");
		else
				util::stream_format(stream, "x0");
		stream << "+)";
		break;

	case 0x180:
		stream << (reg & 0x40 ? "[" : "(");
		if(x1) {
			if(!(reg & 0x20))
				util::stream_format(stream, "bx1");
			else
				util::stream_format(stream, "x1");
		} else {
			if(!(reg & 0x20))
				util::stream_format(stream, "bx0");
			else
				util::stream_format(stream, "x0");
		}

		if(reg & 0x10) {
			if((0x10 - (reg & 0xf)) < 10)
				util::stream_format(stream, "-%d", 0x10 - (reg & 0xf));
			else
				util::stream_format(stream, "-0x%x", 0x10 - (reg & 0xf));

		} else if(reg & 0xf) {
			if((reg & 0xf) < 10)
				util::stream_format(stream, "+%d", reg & 0xf);
			else
				util::stream_format(stream, "+0x%x", reg & 0xf);
		}

		stream << (reg & 0x40 ? ']' : ')');

		if(bank)
			stream << "+0x200";

		break;
	}

	return stream.str();
}

std::string mb86233_disassembler::alu0_func(u32 alu)
{
	std::ostringstream stream;

	switch(alu) {
	case 0x00: break;
	case 0x01: util::stream_format(stream, "andd"); break;
	case 0x02: util::stream_format(stream, "orad"); break;
	case 0x03: util::stream_format(stream, "eord"); break;
	case 0x04: util::stream_format(stream, "notd"); break;
	case 0x05: util::stream_format(stream, "fcpd"); break;
	case 0x06: util::stream_format(stream, "fadd"); break;
	case 0x07: util::stream_format(stream, "fsbd"); break;
	case 0x08: util::stream_format(stream, "fml"); break;
	case 0x09: util::stream_format(stream, "fmsd"); break;
	case 0x0a: util::stream_format(stream, "fmrd"); break;
	case 0x0b: util::stream_format(stream, "fabd"); break;
	case 0x0c: util::stream_format(stream, "fsmd"); break;
	case 0x0d: util::stream_format(stream, "fspd"); break;
	case 0x0e: util::stream_format(stream, "cxfd"); break;
	case 0x0f: util::stream_format(stream, "cfxd"); break;
	case 0x10: util::stream_format(stream, "fdvd"); break;
	case 0x11: util::stream_format(stream, "fned"); break;
		// 12
	case 0x13: util::stream_format(stream, "d=b+a"); break;
	case 0x14: util::stream_format(stream, "d=b-a"); break;
		// 15
	case 0x16: util::stream_format(stream, "lsrd"); break;
	case 0x17: util::stream_format(stream, "lsld"); break;
	case 0x18: util::stream_format(stream, "asrd"); break;
	case 0x19: util::stream_format(stream, "asld"); break;
	case 0x1a: util::stream_format(stream, "addd"); break;
	case 0x1b: util::stream_format(stream, "subd"); break;
		// 1c
		// 1d
		// 1e
		// 1f
	default: util::stream_format(stream, "alu0_func(%02x)", alu); break;
	}

	return stream.str();
}

offs_t mb86233_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	u32 opcode = opcodes.r32(pc);

	switch((opcode >> 26) & 0x3f) {
	case 0x00: { // Dual move AB
		u32 r1 = opcode & 0x1ff;
		u32 r2 = (opcode >> 9) & 0x1ff;
		u32 alu = (opcode >> 21) & 0x1f;
		u32 op = (opcode >> 18) & 0x7;

		if(alu)
			util::stream_format(stream, "%s : ", alu0_func(alu) );

		switch(op) {
		case 0: case 1:
			util::stream_format(stream, "lab %s, %s (e)", memory(r1, false, false), memory(r2, true, false));
			break;

		case 3:
			util::stream_format(stream, "lab %s, %s", memory(r1, false, false), memory(r2, true, true));
			break;

		case 4:
			util::stream_format(stream, "lab %s, %s", memory(r1, false, true), memory(r2, true, false));
			break;

		default:
			util::stream_format(stream, "lab {%d} %s, %s", op, memory(r1, false, false), memory(r2, true, false));
			break;
		}
		break;
	}

	case 0x07: { // LD/MOV
		u32 r1 = opcode & 0x1ff;
		u32 r2 = (opcode >> 9) & 0x1ff;
		u32 alu = (opcode >> 21) & 0x1f;
		u32 op = (opcode >> 18) & 0x7;

		if(alu) {
			util::stream_format(stream, "%s", alu0_func(alu));
			if((opcode & 0x001fffff) == 0x1f1e10) {
				// mov a, -  used as nop
				break;
			}
			stream << " : ";
		}

		switch(op) {
		case 0:
			util::stream_format(stream, "mov {0} %s, %s (e)", memory(r1, false, false), memory(r2, true, false));
			break;

		case 1:
			util::stream_format(stream, "mov %s, %s (e)", memory(r1, false, false), memory(r2, true, false));
			break;

		case 2:
			util::stream_format(stream, "mov %s (e), %s", memory(r1, false, false), memory(r2, true, false));
			break;

		case 3:
			util::stream_format(stream, "mov %s, %s", memory(r1, false, false), memory(r2, true, true));
			break;

		case 4:
			util::stream_format(stream, "mov %s, %s", memory(r1, false, true), memory(r2, true, false));
			break;

		case 5:
			util::stream_format(stream, "mov %s (o), %s", memory(r1, false, false), memory(r2, true, false));
			break;

		case 7: {
			switch(r2 >> 6) {
			case 0:
				util::stream_format(stream, "mov %s, %s", regs(r2 & 0x3f), memory(r1, true, false));
				break;

			case 1:
				util::stream_format(stream, "mov %s, %s (e)", regs(r2 & 0x3f), memory(r1, true, false));
				break;

			case 2:
				util::stream_format(stream, "mov %s, %s", memory(r1, true, true), regs(r2 & 0x3f));
				break;

			case 3:
				util::stream_format(stream, "mov %s, %s", memory(r1, true, false), regs(r2 & 0x3f));
				break;

			case 4:
				util::stream_format(stream, "mov %s (e), %s", memory(r1, true, false), regs(r2 & 0x3f));
				break;

			case 5:
				util::stream_format(stream, "mov %s (o), %s", memory(r1, false, false), regs(r2 & 0x3f));
				break;

			case 6:
				if(r1 >> 6)
					util::stream_format(stream, "mov {r1 %d} %s, %s", r1 >> 6, regs(r1 & 0x3f), regs(r2 & 0x3f));
				else
					util::stream_format(stream, "mov %s, %s", regs(r1 & 0x3f), regs(r2 & 0x3f));
				break;

			default:
				util::stream_format(stream, "mov {r2 %d} %s, %s", r2 >> 6, memory(r1, false, false), regs(r2 & 0x3f));
				break;
			}
			break;
		}
		default:
			util::stream_format(stream, "mov {%d} %s, %s", op, memory(r1, false, false), memory(r2, true, false));
			break;
		}
		break;
	}

	case 0x0d: { // stm/clm
		u32 sub2 = (opcode >> 17) & 7;

		switch(sub2) {
		case 5: {
			util::stream_format(stream, "stmh");
			if(opcode & 0x0001)
				util::stream_format(stream, " fp");
			static char const *const round_mode[4] = { "rn", "rp", "rm", "rz" };
			util::stream_format(stream, " %s", round_mode[(opcode >> 1) & 3]);
			break;
		}

		default:
			util::stream_format(stream, "unk %02x.%x", opcode >> 26, sub2);
			break;
		}
		break;
	}

	case 0x0e: { // Load 24 bit val
		static char const *const inst[4] = { "lipl", "lia", "lib", "lid" };
		util::stream_format(stream, "%s #0x%x", inst[(opcode >> 24) & 0x3], opcode&0xffffff);
		break;
	}

	case 0x0f: { // rep/clr0/clr1/set
		u32 alu = (opcode >> 20) & 0x1f;
		u32 sub2 = (opcode >> 17) & 7;

		if(alu)
			util::stream_format(stream, "%s : ", alu0_func(alu));

		switch(sub2) {
		case 0: {
			static char const *const rl2[16] = {
				"?0", "?1", "a", "b", "d", "?5", "?6", "?7", "?8", "?9", "?a", "?b", "?c", "?d", "?e", "?f"
			};
			util::stream_format(stream, "clr0");
			bool first = true;
			for(int i=0; i<16; i++)
				if(opcode & (1<<i)) {
					if(first) {
						first = false;
						util::stream_format(stream, " %s", rl2[i]);
					} else
						util::stream_format(stream, ", %s", rl2[i]);
				}
			break;
		}

		case 1:
			util::stream_format(stream, "clr1 #0x%04x",opcode & 0xffff);
			break;

		case 2:
			if(opcode & 0x8000)
				util::stream_format(stream, "rep %s", regs(opcode & 0x3f));
			else if(!(opcode & 0xff))
				util::stream_format(stream, "rep #0x100");
			else if((opcode & 0xff) < 10)
				util::stream_format(stream, "rep #%d", opcode & 0xff);
			else
				util::stream_format(stream, "rep #0x%x", opcode & 0xff);
			break;

		case 3:
			util::stream_format(stream, "set #0x%04x",opcode & 0xffff);
			break;

		default:
			util::stream_format(stream, "unk %02x.%x", opcode >> 26, sub2);
			break;
		}
		break;
	}

	case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
	case 0x18: case 0x19: case 0x1a: case 0x1b: case 0x1c: case 0x1d: case 0x1e: case 0x1f:
		util::stream_format(stream, "ldi #0x%x, %s", opcode & 0xffffff, regnames[(opcode >> 24) & 0x3f]); break;
		break;

	case 0x2f: case 0x3f: {
		u32 cond = ( opcode >> 20 ) & 0x1f;
		u32 subtype = ( opcode >> 17 ) & 7;
		u32 data = opcode & 0xffff;
		bool invert = opcode & 0x40000000;

		switch(subtype) {
		case 0:
			util::stream_format(stream, "brif %s #0x%x", condition(cond, invert), data);
			break;

		case 1:
			util::stream_format(stream, "brul %s ", condition(cond, invert));
			if(opcode & 0x4000)
				util::stream_format(stream, "%s", regs(opcode & 0x1f));
			else
				util::stream_format(stream, "(%s)", memory(opcode & 0x1ff, false, false));
			break;

		case 2:
			util::stream_format(stream, "bsif %s #0x%x", condition(cond, invert), data);
			break;

		case 3:
			util::stream_format(stream, "bsul %s ", condition(cond, invert));
			if(opcode & 0x4000)
				util::stream_format(stream, "%s", regs(opcode & 0x1f));
			else
				util::stream_format(stream, "(%s)", memory(opcode & 0x1ff, false, false));
			break;

		case 5:
			util::stream_format(stream, "rtif %s", condition(cond, invert));
			break;

		case 6:
			util::stream_format(stream, "ldif %s %s, %s", condition(cond, invert), memory(data & 0x1ff, false, false), regs((data >> 9) & 0x3f));
			break;

		case 7:
			util::stream_format(stream, "iret");
			break;

		default:
			util::stream_format(stream, "unk %02x.%d", opcode >> 26, subtype);
			break;
		}
		break;
	}

	default:
		util::stream_format(stream, "unk %02x", opcode >> 26);
		break;
	}

	return 1 | SUPPORTED;
}

u32 mb86233_disassembler::opcode_alignment() const
{
	return 1;
}
