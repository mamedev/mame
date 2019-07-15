// license:BSD-3-Clause
// copyright-holders:Tony La Porta
	/**************************************************************************\
	*                Texas Instruments TMS32010 DSP Disassembler               *
	*                                                                          *
	*                  Copyright Tony La Porta                                 *
	*               To be used with TMS32010 DSP Emulator engine.              *
	*                                                                          *
	*         Many thanks to those involved in the i8039 Disassembler          *
	*                        as this was based on it.                          *
	*                                                                          *
	*                                                                          *
	*                                                                          *
	* A Memory address                                                         *
	* B Branch Address for Branch instructions (Requires next opcode read)     *
	* D Immediate byte load                                                    *
	* K Immediate bit  load                                                    *
	* W Immediate word load (Actually 13 bit)                                  *
	* M AR[x] register modification type (for indirect addressing)             *
	* N ARP register to change ARP pointer to (for indirect addressing)        *
	* P I/O port address number                                                *
	* R AR[R] register to use                                                  *
	* S Shift ALU left                                                         *
	*                                                                          *
	\**************************************************************************/

#include "emu.h"
#include "32010dsm.h"

#include <ctype.h>
#include <stdexcept>


const char *const tms32010_disassembler::arith[4] = { "*" , "*-" , "*+" , "??" } ;
const char *const tms32010_disassembler::nextar[4] = { ",AR0" , ",AR1" , "" , "" } ;


const char *const tms32010_disassembler::TMS32010Formats[] = {
	"0000ssss0aaaaaaa", "add  %A%S",
	"0000ssss10mmn00n", "add  %M%S%N",
	"0001ssss0aaaaaaa", "sub  %A%S",
	"0001ssss10mmn00n", "sub  %M%S%N",
	"0010ssss0aaaaaaa", "lac  %A%S",
	"0010ssss10mmn00n", "lac  %M%S%N",
	"0011000r0aaaaaaa", "sar  %R,%A",
	"0011000r10mmn00n", "sar  %R%M%N",
	"0011100r0aaaaaaa", "lar  %R,%A",
	"0011100r10mmn00n", "lar  %R%M%N",
	"01000ppp0aaaaaaa", "in   %A,%P",
	"01000ppp10mmn00n", "in   %M,%P%N",
	"01001ppp0aaaaaaa", "out  %A,%P",
	"01001ppp10mmn00n", "out  %M,%P%N",
	"01010sss0aaaaaaa", "sacl %A",     /* This instruction has a shift but */
	"01010sss10mmn00n", "sacl %M%N",   /* is documented as not performed */
	"01011sss0aaaaaaa", "sach %A%S",
	"01011sss10mmn00n", "sach %M%S%N",
	"011000000aaaaaaa", "addh %A",
	"0110000010mmn00n", "addh %M%N",
	"011000010aaaaaaa", "adds %A",
	"0110000110mmn00n", "adds %M%N",
	"011000100aaaaaaa", "subh %A",
	"0110001010mmn00n", "subh %M%N",
	"011000110aaaaaaa", "subs %A",
	"0110001110mmn00n", "subs %M%N",
	"011001000aaaaaaa", "subc %A",
	"0110010010mmn00n", "subc %M%N",
	"011001010aaaaaaa", "zalh %A",
	"0110010110mmn00n", "zalh %M%N",
	"011001100aaaaaaa", "zals %A",
	"0110011010mmn00n", "zals %M%N",
	"011001110aaaaaaa", "tblr %A",
	"0110011110mmn00n", "tblr %M%N",
	"011010001000000k", "larp %K",
	"011010000aaaaaaa", "mar  %A",     /* Actually this is executed as a NOP */
/*  "0110100010mmn00n", "mar  %M%N",   */
/*  MAR indirect has been expanded out to all its variations because one of */
/*  its opcodes is the same as LARP (actually performs the same function) */

	"0110100010001000", "mar  *",
	"0110100010001001", "mar  *",
	"0110100010010000", "mar  *-,AR0",
	"0110100010010001", "mar  *-,AR1",
	"0110100010011000", "mar  *-",
	"0110100010011001", "mar  *-",
	"0110100010100000", "mar  *+,AR0",
	"0110100010100001", "mar  *+,AR1",
	"0110100010101000", "mar  *+",
	"0110100010101001", "mar  *+",
	"0110100010110000", "mar  ??,AR0",
	"0110100010110001", "mar  ??,AR1",
	"0110100010111000", "mar  ??",
	"0110100010111001", "mar  ??",

	"011010010aaaaaaa", "dmov %A",
	"0110100110mmn00n", "dmov %M%N",
	"011010100aaaaaaa", "lt   %A",
	"0110101010mmn00n", "lt   %M%N",
	"011010110aaaaaaa", "ltd  %A",
	"0110101110mmn00n", "ltd  %M%N",
	"011011000aaaaaaa", "lta  %A",
	"0110110010mmn00n", "lta  %M%N",
	"011011010aaaaaaa", "mpy  %A",
	"0110110110mmn00n", "mpy  %M%N",
	"011011100000000k", "ldpk %K",
	"011011110aaaaaaa", "ldp  %A",
	"0110111110mmn00n", "ldp  %M%N",
	"0111000rdddddddd", "lark %R,%D",
	"011110000aaaaaaa", "xor  %A",
	"0111100010mmn00n", "xor  %M%N",
	"011110010aaaaaaa", "and  %A",
	"0111100110mmn00n", "and  %M%N",
	"011110100aaaaaaa", "or   %A",
	"0111101010mmn00n", "or   %M%N",
	"011110110aaaaaaa", "lst  %A",
	"0111101110mmn00n", "lst  %M%N",
	"011111000aaaaaaa", "sst  %A",
	"0111110010mmn00n", "sst  %M%N",
	"011111010aaaaaaa", "tblw %A",
	"0111110110mmn00n", "tblw %M%N",
	"01111110dddddddd", "lack %D",
	"0111111110000000", "nop",         /* 7F80 */
	"0111111110000001", "dint",
	"0111111110000010", "eint",
	"0111111110001000", "abs",         /* 7F88 */
	"0111111110001001", "zac",
	"0111111110001010", "rovm",
	"0111111110001011", "sovm",
	"0111111110001100", "cala",
	"0111111110001101", "ret",
	"0111111110001110", "pac",
	"0111111110001111", "apac",
	"0111111110010000", "spac",
	"0111111110011100", "push",
	"0111111110011101", "pop",         /* 7F9D */
	"100wwwwwwwwwwwww", "mpyk %W",
	"1111010000000000bbbbbbbbbbbbbbbb", "banz %B",
	"1111010100000000bbbbbbbbbbbbbbbb", "bv   %B",
	"1111011000000000bbbbbbbbbbbbbbbb", "bioz %B",
	"1111100000000000bbbbbbbbbbbbbbbb", "call %B",
	"1111100100000000bbbbbbbbbbbbbbbb", "b    %B",
	"1111101000000000bbbbbbbbbbbbbbbb", "blz  %B",
	"1111101100000000bbbbbbbbbbbbbbbb", "blez %B",
	"1111110000000000bbbbbbbbbbbbbbbb", "bgz  %B",
	"1111110100000000bbbbbbbbbbbbbbbb", "bgez %B",
	"1111111000000000bbbbbbbbbbbbbbbb", "bnz  %B",
	"1111111100000000bbbbbbbbbbbbbbbb", "bz   %B",
	nullptr
};

tms32010_disassembler::tms32010_disassembler()
{
	const char *p;
	const char *const *ops;
	u16 mask, bits;
	int bit;
	int i;

	ops = TMS32010Formats; i = 0;
	while (*ops)
	{
		p = *ops;
		mask = 0; bits = 0; bit = 15;
		while (*p && bit >= 0)
		{
			switch (*p++)
			{
				case '1': mask |= 1<<bit; bits |= 1<<bit; bit--; break;
				case '0': mask |= 1<<bit; bit--; break;
				case ' ': break;
				case 'a':
				case 'b':
				case 'd':
				case 'k':
				case 'm':
				case 'n':
				case 'p':
				case 'r':
				case 's':
				case 'w':
					bit--;
					break;
				default:
					throw std::logic_error(util::string_format("Invalid instruction encoding '%s %s'\n", ops[0],ops[1]));
			}
		}
		if (bit != -1 )
		{
			throw std::logic_error(util::string_format("not enough bits in encoding '%s %s' %d\n", ops[0],ops[1],bit));
		}
		while (isspace((uint8_t)*p)) p++;
		Op.emplace_back(mask, bits, *p, ops[0], ops[1]);

		ops += 2;
		i++;
	}
}

offs_t tms32010_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	uint32_t flags = 0;
	int a, b, d, k, m, n, p, r, s, w;   /* these can all be filled in by parsing an instruction */
	int i;
	int op;
	int cnt = 1;
	int code;
	int bit;
	//char *buffertmp;
	const char *cp;             /* character pointer in OpFormats */

	op = -1;                /* no matching opcode */
	code = opcodes.r16(pc);
	for ( i = 0; i < int(Op.size()); i++)
	{
		if ((code & Op[i].mask) == Op[i].bits)
		{
			if (op != -1)
			{
				osd_printf_debug("Error: opcode %04Xh matches %d (%s) and %d (%s)\n",
					code,i,Op[i].fmt,op,Op[op].fmt);
			}
			op = i;
		}
	}
	if (op == -1)
	{
		util::stream_format(stream, "dw   %04Xh *(invalid op)", code);
		return cnt | SUPPORTED;
	}
	//buffertmp = buffer;
	if (Op[op].extcode)
	{
		bit = 31;
		code <<= 16;
		code |= opcodes.r16(pc+1);
		cnt++;
	}
	else
	{
		bit = 15;
	}

	/* shift out operands */
	cp = Op[op].parse;
	a = b = d = k = m = n = p = r = s = w = 0;

	while (bit >= 0)
	{
		/* osd_printf_debug("{%c/%d}",*cp,bit); */
		switch(*cp)
		{
			case 'a': a <<=1; a |= ((code & (1<<bit)) ? 1 : 0); bit--; break;
			case 'b': b <<=1; b |= ((code & (1<<bit)) ? 1 : 0); bit--; break;
			case 'd': d <<=1; d |= ((code & (1<<bit)) ? 1 : 0); bit--; break;
			case 'k': k <<=1; k |= ((code & (1<<bit)) ? 1 : 0); bit--; break;
			case 'm': m <<=1; m |= ((code & (1<<bit)) ? 1 : 0); bit--; break;
			case 'n': n <<=1; n |= ((code & (1<<bit)) ? 1 : 0); bit--; break;
			case 'p': p <<=1; p |= ((code & (1<<bit)) ? 1 : 0); bit--; break;
			case 'r': r <<=1; r |= ((code & (1<<bit)) ? 1 : 0); bit--; break;
			case 's': s <<=1; s |= ((code & (1<<bit)) ? 1 : 0); bit--; break;
			case 'w': w <<=1; w |= ((code & (1<<bit)) ? 1 : 0); bit--; break;
			case ' ': break;
			case '1': case '0':  bit--; break;
			case '\0': throw std::logic_error(util::string_format("premature end of parse string, opcode %x, bit = %d\n",code,bit));
		}
		cp++;
	}

	/* now traverse format string */
	cp = Op[op].fmt;

	if (!strncmp(cp, "cal", 3))
		flags = STEP_OVER;
	else if (!strncmp(cp, "ret", 3))
		flags = STEP_OUT;

	while (*cp)
	{
		if (*cp == '%')
		{
			char num[20];
			cp++;
			switch (*cp++)
			{
				case 'A': sprintf(num,"%02Xh",a); break; // was $%02X
				case 'B': sprintf(num,"%04Xh",b); break; // was $%04X
				case 'D': sprintf(num,"%02Xh",d); break;
				case 'K': sprintf(num,"%d",k); break;
				case 'N': sprintf(num,"%s",nextar[n]); break;
				case 'M': sprintf(num,"%s",arith[m]); break;
				case 'P': sprintf(num,"PA%d",p); break;
				case 'R': sprintf(num,"AR%d",r); break;
				case 'S': sprintf(num,",%d",s); break;
				case 'W': sprintf(num,"%04Xh",w); break;
				default:
					throw std::logic_error(util::string_format("illegal escape character in format '%s'\n",Op[op].fmt));
			}
			stream << num;
		}
		else
		{
			stream << *cp++;
		}
	}
	return cnt | flags | SUPPORTED;
}

u32 tms32010_disassembler::opcode_alignment() const
{
	return 1;
}

