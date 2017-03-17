// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
#include "emu.h"
#include "debugger.h"
#include "mb86233.h"

/*

    Main register bank:
        0   1   2   3   4   5   6   7   8   9   a   b   c   d   e   f
    0  b0  b1  x0  x1  x2  i0  i1  i2  sp pag vsm dmc  c0  c1  pc   -
    1   a  ah  al   b  bh  bl   c  ch  cl   d  dh  dl   p  ph  pl sft
    
    Second register bank:
         0    1    2    3    4    5    6    7    8    9    a    b    c    d    e    f
    2 <register file>
    3 sio0  si1  pio pioa  rpc    -    -    -  pad  mod  ear   st mask  tim   cx   dx



    Address encoding:
      0 0aaa aaaa  $a
      0 1aaa aaaa  $a(xn)
      1 0aaa aaaa  $a(xn+)
      1 100b bbbb  (xn+$b)
      1 101b bbbb  (bxn+$b)
      1 110b bbbb  [xn+$b]
      1 111b bbbb  [bxn+$b]



    3322 2222 2222 1111 1111 1100 0000 0000
    1098 7654 3210 9876 5432 1098 7654 3210

    0000 00aa aaao ooyy yyyy yyyx xxxx xxxx lab

    0000 00aa aaa0 11yy yyyy yyyx xxxx xxxx lab adx (e), ady
    0000 00aa aaa1 00yy yyyy yyyx xxxx xxxx lab adx, ady (e)

    0001 11aa aaa0 10yy yyyy yyyx xxxx xxxx mov adx, ady
    0001 11aa aaa0 11yy yyyy yyyx xxxx xxxx mov adx (e), ady
    0001 11aa aaa1 00yy yyyy yyyx xxxx xxxx mov adx, ady (e)
    0001 11aa aaa1 01yy yyyy yyyx xxxx xxxx mov adx (o), ady
    0001 11aa aaa1 1100 0rrr rrry yyyy yyyy mov r, ady (e)
    0001 11aa aaa1 1100 1rrr rrry yyyy yyyy mov r, ady
    0001 11aa aaa1 1101 1rrr rrrx xxxx xxxx mov adx (e), r
    0001 11aa aaa1 1110 0rrr rrrx xxxx xxxx mov adx, r
    0001 11aa aaa1 1110 1rrr rrrx xxxx xxxx mov adx (o), r
    0001 11aa aaa1 1111 0rrr rrr. ..ss ssss mov s, r

    0011 10rr vvvv vvvv vvvv vvvv vvvv vvvv lipl/lia/lib/lid #v

    0011 11aa aaa. 000. rrrr rrrr rrrr rrrr clr0 <reglist>
    0011 11aa aaa. 001. ffff ffff ffff ffff clr1 flags
    0011 11aa aaa. 010. 0... .... nnnn nnnn rep #n
    0011 11aa aaa. 010. 1... .... ..11 0100 rep rpc
    0011 11aa aaa. 011. ffff ffff ffff ffff set flags

    010r rrrr .... .... vvvv vvvv vvvv vvvv ldi #v, b0/.../- (bank 0/1) (h/l is special and targets exponent/mantissa size 8/24)

    1i11 11.c cccc 0000 aaaa aaaa aaaa aaaa brif cond #a
    1i11 11.c cccc 0010 .0.. ...x xxxx xxxx brul cond (adx)
    1i11 11.c cccc 0010 .1.. .... ...r rrrr brul cond r
    1i11 11.c cccc 0100 aaaa aaaa aaaa aaaa bsif cond #a
    1i11 11.c cccc 0110 .0.. ...x xxxx xxxx bsul cond (adx)
    1i11 11.c cccc 0110 .1.. .... ...r rrrr bsul cond r
    1i11 11.c cccc 1010 .... .... .... .... rtif cond
    1i11 11.c cccc 1100 .rrr rrrx xxxx xxxx ldif cond adx, r
    1011 11.c 0110 1110 .... .... .... .... iret

    Top 3 bits = instruction group, except when it isn't.
    
 */



static const char *const regnames[0x40] = {
	"b0", "b1", "x0", "x1", "x2", "i0", "i1", "i2", "sp", "pag", "vsm", "dmc", "c0", "c1", "pc", "-",
	"a", "ah", "al", "b", "bh", "bl", "c", "ch", "cl", "d", "dh", "dl", "p", "ph", "pl", "sft",
	"rf0", "rf1", "rf2", "rf3", "rf4", "rf5", "rf6", "rf7", "rf8", "rf9", "rfa", "rfb", "rfc", "rfd", "rfe", "rff",
	"sio0", "si1", "pio", "pioa", "rpc", "r?35", "r?36", "r?37", "pad", "mod", "ear", "st", "mask", "tim", "cx", "dx"
};

static std::string condition(unsigned int cond, bool invert)
{
	std::ostringstream stream;

	if(invert)
		stream << '!';

	switch(cond) {
		case 0x00: util::stream_format(stream, "zrd"); return stream.str();
		case 0x01: util::stream_format(stream, "ged"); return stream.str();
		case 0x02: util::stream_format(stream, "led"); return stream.str();
		case 0x10: util::stream_format(stream, "zc0"); return stream.str();
		case 0x11: util::stream_format(stream, "zc1"); return stream.str();
		case 0x16: util::stream_format(stream, "alw"); return stream.str();
	}

	util::stream_format(stream, "cond(%02x)", cond);
	return stream.str();
}

static std::string regs(uint32_t reg)
{
	return regnames[reg & 0x3f];
}

static std::string memory(uint32_t reg, bool x1)
{
	std::ostringstream stream;

	switch(reg & 0x180) {
	case 0x000:
		if((reg & 0x7f) < 10)
			util::stream_format(stream, "$%d", reg & 0x7f );
		else
			util::stream_format(stream, "$0x%x", reg & 0x7f );
		break;

	case 0x080:
		if(reg & 0x7f) {
			if((reg & 0x7f) < 10)
				util::stream_format(stream, "$%d", reg & 0x7f );
			else
				util::stream_format(stream, "$0x%x", reg & 0x7f );
		}

		stream << '(';

		if(x1)
				util::stream_format(stream, "x1");
		else
				util::stream_format(stream, "x0");
		stream << ')';
		break;

	case 0x100:
		if(reg & 0x7f)
			util::stream_format(stream, "$0x%x", reg & 0x7f );

		stream << '(';

		if ( x1 )
				util::stream_format(stream, "x1");
		else
				util::stream_format(stream, "x0");
		stream << "+)";
		break;

	case 0x180:
		stream << (reg & 0x40 ? "$[" : "$(");
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
		break;
	}

	return stream.str();
}

static std::string alu0_func( uint32_t alu)
{
	std::ostringstream stream;

	switch(alu) {
	case 0x00: break;
	case 0x01: util::stream_format(stream, "andd"); break;
	case 0x02: util::stream_format(stream, "orad"); break;
	case 0x03: util::stream_format(stream, "eord"); break;
		// 04
	case 0x05: util::stream_format(stream, "fcpd"); break;
	case 0x06: util::stream_format(stream, "fadd"); break;
	case 0x07: util::stream_format(stream, "fsbd"); break;
	case 0x08: util::stream_format(stream, "fml"); break;
	case 0x09: util::stream_format(stream, "fmsd"); break;
	case 0x0a: util::stream_format(stream, "fmrd"); break;
	case 0x0b: util::stream_format(stream, "fabd"); break;
	case 0x0c: util::stream_format(stream, "fsmd"); break;
	case 0x0d: util::stream_format(stream, "fspd"); break;
	case 0x0e: util::stream_format(stream, "cifd"); break;
	case 0x0f: util::stream_format(stream, "cfid"); break;
	case 0x10: util::stream_format(stream, "fdvd"); break;
	case 0x11: util::stream_format(stream, "fned"); break;
		// 12
	case 0x13: util::stream_format(stream, "d=b+a"); break;
	case 0x14: util::stream_format(stream, "d=b-a"); break;
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

static unsigned dasm_mb86233(std::ostream &stream, uint32_t opcode )
{
	switch((opcode >> 26) & 0x3f) {
	case 0x00: { // Dual move AB
		uint32_t r1 = opcode & 0x1ff;
		uint32_t r2 = (opcode >> 9) & 0x1ff;
		uint32_t alu = (opcode >> 21) & 0x1f;
		uint32_t op = (opcode >> 18) & 0x7;

		if(alu)
			util::stream_format(stream, "%s : ", alu0_func(alu) );

		switch(op) {
		case 3:
			util::stream_format(stream, "lab %s (e), %s", memory(r1, false), memory(r2, true));
			break;

		case 4:
			util::stream_format(stream, "lab %s, %s (e)", memory(r1, false), memory(r2, true));
			break;

		default:
			util::stream_format(stream, "lab {%d} %s, %s", op, memory(r1, false), memory(r2, true));
			break;
		}
		break;
	}

	case 0x07: { // LD/MOV
		uint32_t r1 = opcode & 0x1ff;
		uint32_t r2 = (opcode >> 9) & 0x1ff;
		uint32_t alu = (opcode >> 21) & 0x1f;
		uint32_t op = (opcode >> 18) & 0x7;

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
			util::stream_format(stream, "mov %s, %s", memory(r1, false), memory(r2, true));
			break;

		case 1:
			util::stream_format(stream, "mov %s, %s (io)", memory(r1, false), memory(r2, true));
			break;

		case 2:
			util::stream_format(stream, "mov %s (io), %s", memory(r1, false), memory(r2, true));
			break;

		case 3:
			util::stream_format(stream, "mov %s (e), %s", memory(r1, false), memory(r2, true));
			break;

		case 4:
			util::stream_format(stream, "mov %s, %s (e)", memory(r1, false), memory(r2, true));
			break;

		case 5:
			util::stream_format(stream, "mov %s (o), %s", memory(r1, false), memory(r2, true));
			break;

		case 7: {
			switch(r2 >> 6) {
			case 0:
				util::stream_format(stream, "mov %s, %s (e)", regs(r2 & 0x3f), memory(r1, true));
				break;

			case 1:
				util::stream_format(stream, "mov %s, %s (io)", regs(r2 & 0x3f), memory(r1, true));
				break;

			case 2:
				util::stream_format(stream, "mov %s, %s", memory(r1, false), regs(r2 & 0x3f));
				break;

			case 3:
				util::stream_format(stream, "mov %s (e), %s", memory(r1, false), regs(r2 & 0x3f));
				break;

			case 4:
				util::stream_format(stream, "mov %s (io), %s", memory(r1, false), regs(r2 & 0x3f));
				break;

			case 5:
				util::stream_format(stream, "mov %s (o), %s", memory(r1, false), regs(r2 & 0x3f));
				break;

			case 6:
				if(r1 >> 6)
					util::stream_format(stream, "mov {r1 %d} %s, %s", r1 >> 6, regs(r1 & 0x3f), regs(r2 & 0x3f));
				else
					util::stream_format(stream, "mov %s, %s", regs(r1 & 0x3f), regs(r2 & 0x3f));
				break;

			default:
				util::stream_format(stream, "mov {r2 %d} %s, %s", r2 >> 6, memory(r1, true), regs(r2 & 0x3f));
				break;
			}
			break;
		}
		default:
			util::stream_format(stream, "mov {%d} %s, %s", op, memory(r1, false), memory(r2, true));
			break;
		}				
		break;
	}

	case 0x0e: { // Load 24 bit val
		static const char *const inst[4] = { "lipl", "lia", "lib", "lid" };
		util::stream_format(stream, "%s #0x%x", inst[(opcode >> 24) & 0x3], opcode&0xffffff);
		break;
	}

	case 0x0f: { // rep/clr0/clr1/set
		uint32_t alu = (opcode >> 21) & 0x1f;
		uint32_t sub2 = (opcode >> 17) & 7;

		if(alu)
			util::stream_format(stream, "%s : ", alu0_func(alu));

		switch(sub2) {
		case 0: {
			static const char *rl2[16] = {
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
		util::stream_format(stream, "ldi #0x%x, %s", opcode & 0xffffff, regnames[(opcode >> 24) & 0x3f]); break;
		break;
		
	case 0x2f: case 0x3f: {
		uint32_t cond = ( opcode >> 20 ) & 0x1f;
		uint32_t subtype = ( opcode >> 17 ) & 7;
		uint32_t data = opcode & 0xffff;
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
				util::stream_format(stream, "(%s)", memory(opcode & 0x1ff, false));
			break;

		case 2:
			util::stream_format(stream, "bsif %s #0x%x", condition(cond, invert), data);
			break;

		case 3:
			util::stream_format(stream, "bsul %s ", condition(cond, invert));
			if(opcode & 0x4000)
				util::stream_format(stream, "%s", regs(opcode & 0x1f));
			else
				util::stream_format(stream, "(%s)", memory(opcode & 0x1ff, false));
			break;

		case 5:
			util::stream_format(stream, "rtif %s", condition(cond, invert));
			break;

		case 6:
			util::stream_format(stream, "ldif %s %s, %s", condition(cond, invert), memory(data & 0x1ff, false), regs((data >> 9) & 0x3f));
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

	return (1 | DASMFLAG_SUPPORTED);
}

CPU_DISASSEMBLE(mb86233)
{
	uint32_t op = *(uint32_t *)oprom;
	op = little_endianize_int32(op);
	return dasm_mb86233(stream, op);
}
