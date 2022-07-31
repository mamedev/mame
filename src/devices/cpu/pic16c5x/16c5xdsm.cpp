// license:BSD-3-Clause
// copyright-holders:Tony La Porta
	/**************************************************************************\
	*                      Microchip PIC16C5x Emulator                         *
	*                                                                          *
	*                    Copyright Tony La Porta                               *
	*                 Originally written for the MAME project.                 *
	*                                                                          *
	*                                                                          *
	*      Addressing architecture is based on the Harvard addressing scheme.  *
	*                                                                          *
	*         Many thanks to those involved in the i8039 Disassembler          *
	*                        as this was based on it.                          *
	*                                                                          *
	*                                                                          *
	*                                                                          *
	* A Address to jump to.                                                    *
	* B Bit address within an 8-bit file register.                             *
	* D Destination select (0 = store result in W (accumulator))               *
	*                      (1 = store result in file register)                 *
	* F Register file address (00-1F).                                         *
	* K Literal field, constant data.                                          *
	*                                                                          *
	\**************************************************************************/

#include "emu.h"
#include "16c5xdsm.h"

#include <cctype>
#include <stdexcept>


const char *const pic16c5x_disassembler::regfile[32] = { "Reg$00 (IND)",    "Reg$01 (TMR)",    "Reg$02 (PCL)",  "Reg$03 (ST)", "Reg$04 (FSR)", "Reg$05 (PTA)", "Reg$06 (PTB)", "Reg$07 (PTC)",
									"Reg$08", "Reg$09", "Reg$0A", "Reg$0B", "Reg$0C", "Reg$0D", "Reg$0E", "Reg$0F",
									"Reg$10", "Reg$11", "Reg$12", "Reg$13", "Reg$14", "Reg$15", "Reg$16", "Reg$17",
									"Reg$18", "Reg$19", "Reg$1A", "Reg$1B", "Reg$1C", "Reg$1D", "Reg$1E", "Reg$1F" };

const char *const pic16c5x_disassembler::dest[2] = { "W", "Reg" };

const char *const pic16c5x_disassembler::PIC16C5xFormats[] = {
	"000000000000", "nop",
	"000000000010", "option",
	"000000000011", "sleep",
	"000000000100", "clrwdt",
	"000000000101", "tris   Port A",
	"000000000110", "tris   Port B",
	"000000000111", "tris   Port C",
	"0000001fffff", "movwf  %F",
	"000001000000", "clrw",
	"0000011fffff", "clrf   %F",
	"000010dfffff", "subwf  %F,%D",
	"000011dfffff", "decf   %F,%D",
	"000100dfffff", "iorwf  %F,%D",
	"000101dfffff", "andwf  %F,%D",
	"000110dfffff", "xorwf  %F,%D",
	"000111dfffff", "addwf  %F,%D",
	"001000dfffff", "movf   %F,%D",
	"001001dfffff", "comf   %F,%D",
	"001010dfffff", "incf   %F,%D",
	"001011dfffff", "decfsz %F,%D",
	"001100dfffff", "rrf    %F,%D",
	"001101dfffff", "rlf    %F,%D",
	"001110dfffff", "swapf  %F,%D",
	"001111dfffff", "incfsz %F,%D",
	"0100bbbfffff", "bcf    %F,%B",
	"0101bbbfffff", "bsf    %F,%B",
	"0110bbbfffff", "btfsc  %F,%B",
	"0111bbbfffff", "btfss  %F,%B",
	"1000kkkkkkkk", "retlw  %K",
	"1001aaaaaaaa", "call   %A",
	"101aaaaaaaaa", "goto   %A",
	"1100kkkkkkkk", "movlw  %K",
	"1101kkkkkkkk", "iorlw  %K",
	"1110kkkkkkkk", "andlw  %K",
	"1111kkkkkkkk", "xorlw  %K",
	nullptr
};

pic16c5x_disassembler::pic16c5x_disassembler()
{
	const char *p;
	const char *const *ops;
	u16 mask, bits;
	int bit;
	int i;

	ops = PIC16C5xFormats; i = 0;
	while (*ops)
	{
		p = *ops;
		mask = 0; bits = 0; bit = 11;
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
				case 'f':
				case 'k':
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

offs_t pic16c5x_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	int a, b, d, f, k;  /* these can all be filled in by parsing an instruction */
	int i;
	int op;
	int cnt = 1;
	int code;
	int bit;
	//char *buffertmp;
	const char *cp;             /* character pointer in OpFormats */
	uint32_t flags = 0;

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
		util::stream_format(stream, "???? dw %04Xh",code);
		return cnt | SUPPORTED;
	}
	//buffertmp = buffer;
	if (Op[op].extcode)     /* Actually, theres no double length opcodes */
	{
		bit = 27;
		code <<= 16;
		code |= params.r16(pc+cnt);
		cnt++;
	}
	else
	{
		bit = 11;
	}

	/* shift out operands */
	cp = Op[op].parse;
	a = b = d = f = k = 0;

	while (bit >= 0)
	{
		/* osd_printf_debug("{%c/%d}",*cp,bit); */
		switch (*cp)
		{
		case 'a': a <<=1; a |= ((code & (1<<bit)) ? 1 : 0); bit--; break;
		case 'b': b <<=1; b |= ((code & (1<<bit)) ? 1 : 0); bit--; break;
		case 'd': d <<=1; d |= ((code & (1<<bit)) ? 1 : 0); bit--; break;
		case 'f': f <<=1; f |= ((code & (1<<bit)) ? 1 : 0); bit--; break;
		case 'k': k <<=1; k |= ((code & (1<<bit)) ? 1 : 0); bit--; break;
		case ' ': break;
		case '1': case '0':  bit--; break;
		case '\0': throw std::logic_error(util::string_format("premature end of parse string, opcode %x, bit = %d\n",code,bit));
		}
		cp++;
	}

	/* now traverse format string */
	cp = Op[op].fmt;
	if (!strncmp(cp, "call", 4))
		flags = STEP_OVER;
	else if (!strncmp(cp, "ret", 3))
		flags = STEP_OUT;
	else if (!strncmp(cp, "btfs", 4) || !strncmp(cp + 2, "cfsz", 4))
		flags = STEP_COND;

	while (*cp)
	{
		if (*cp == '%')
		{
			cp++;
			switch (*cp++)
			{
				case 'A': util::stream_format(stream, "$%03X", a); break;
				case 'B': util::stream_format(stream, "%d", b); break;
				case 'D': util::stream_format(stream, "%s", dest[d]); break;
				case 'F': util::stream_format(stream, "%s", regfile[f]); break;
				case 'K': util::stream_format(stream, "%02Xh", k); break;
				default:
					throw std::logic_error(util::string_format("illegal escape character in format '%s'\n",Op[op].fmt));
			}
		}
		else
		{
			stream << *cp++;
		}
	}
	return cnt | flags | SUPPORTED;
}

uint32_t pic16c5x_disassembler::opcode_alignment() const
{
	return 1;
}
