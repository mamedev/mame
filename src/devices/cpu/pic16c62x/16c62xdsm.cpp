// license:BSD-3-Clause
// copyright-holders:Tony La Porta
	/**************************************************************************\
	*                  Microchip PIC16C62X Emulator                            *
	*                                                                          *
	*                          Based On                                        *
	*                  Microchip PIC16C5X Emulator                             *
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
	* X Not used                                                               *
	*                                                                          *
	\**************************************************************************/

#include "emu.h"
#include "16c62xdsm.h"

#include <ctype.h>
#include <stdexcept>


/* Registers bank 0/1 */
const char *const pic16c62x_disassembler::regfile[32] = { "Reg$00 (INDF)",    "Reg$01 (TMR0/OPTION)",    "Reg$02 (PCL)",  "Reg$03 (STATUS)", "Reg$04 (FSR)", "Reg$05 (PORTA/TRISA)", "Reg$06 (PORTB/TRISB)", "Reg$07",
									"Reg$08", "Reg$09", "Reg$0A (PCLATH)", "Reg$0B (INTCON)", "Reg$0C (PIR1/PIE1)", "Reg$0D", "Reg$0E (none/PCON)", "Reg$0F",
									"Reg$10", "Reg$11", "Reg$12", "Reg$13", "Reg$14", "Reg$15", "Reg$16", "Reg$17",
									"Reg$18", "Reg$19", "Reg$1A", "Reg$1B", "Reg$1C", "Reg$1D", "Reg$1E", "Reg$1F (CMCON/VRCON)" };
/* Registers bank 1 */
/*const char *const regfile1[32] = { "Reg$00 (INDF)",    "Reg$01 (OPTION)",    "Reg$02 (PCL)",  "Reg$03 (STATUS)", "Reg$04 (FSR)", "Reg$05 (TRISA)", "Reg$06 (TRISB)", "Reg$07",
                                 "Reg$08", "Reg$09", "Reg$0A (PCLATH)", "Reg$0B (INTCON)", "Reg$0C (PIE1)", "Reg$0D", "Reg$0E (PCON)", "Reg$0F",
                                 "Reg$10", "Reg$11", "Reg$12", "Reg$13", "Reg$14", "Reg$15", "Reg$16", "Reg$17",
                                 "Reg$18", "Reg$19", "Reg$1A", "Reg$1B", "Reg$1C", "Reg$1D", "Reg$1E", "Reg$1F (VRCON)" };
const char **regfile[2] = { regfile0, regfile1 };*/

const char *const pic16c62x_disassembler::dest[2] = { "W", "Reg" };

const char *const pic16c62x_disassembler::PIC16C62xFormats[] = {
	"0000000xx00000", "nop",
	"00000000001000", "return",
	"00000000001001", "retfie",
	"00000001100011", "sleep",
	"00000001100100", "clrwdt",
	"0000001fffffff", "movwf  %F",
	"00000100000011", "clrw",
	"0000011fffffff", "clrf   %F",
	"000010dfffffff", "subwf  %F,%D",
	"000011dfffffff", "decf   %F,%D",
	"000100dfffffff", "iorwf  %F,%D",
	"000101dfffffff", "andwf  %F,%D",
	"000110dfffffff", "xorwf  %F,%D",
	"000111dfffffff", "addwf  %F,%D",
	"001000dfffffff", "movf   %F,%D",
	"001001dfffffff", "comf   %F,%D",
	"001010dfffffff", "incf   %F,%D",
	"001011dfffffff", "decfsz %F,%D",
	"001100dfffffff", "rrf    %F,%D",
	"001101dfffffff", "rlf    %F,%D",
	"001110dfffffff", "swapf  %F,%D",
	"001111dfffffff", "incfsz %F,%D",
	"0100bbbfffffff", "bcf    %F,%B",
	"0101bbbfffffff", "bsf    %F,%B",
	"0110bbbfffffff", "btfsc  %F,%B",
	"0111bbbfffffff", "btfss  %F,%B",
	"1101xxkkkkkkkk", "retlw  %K",
	"100aaaaaaaaaaa", "call   %A",
	"101aaaaaaaaaaa", "goto   %A",
	"1100xxkkkkkkkk", "movlw  %K",
	"111000kkkkkkkk", "iorlw  %K",
	"111001kkkkkkkk", "andlw  %K",
	"111010kkkkkkkk", "xorlw  %K",
	"11110xkkkkkkkk", "sublw  %K",
	"11111xkkkkkkkk", "addlw  %K",
	nullptr
};

pic16c62x_disassembler::pic16c62x_disassembler()
{
	const char *p;
	const char *const *ops;
	u16 mask, bits;
	int bit;
	int i;

	ops = PIC16C62xFormats; i = 0;
	while (*ops)
	{
		p = *ops;
		mask = 0; bits = 0; bit = 13;
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
				case 'x':
					bit --;
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

offs_t pic16c62x_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
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
		util::stream_format(stream, "???? dw %04Xh", code);
		return cnt;
	}
	//buffertmp = buffer;
	if (Op[op].extcode)     /* Actually, theres no double length opcodes */
	{
		bit = 29;
		code <<= 16;
		code |= params.r16(pc+cnt);
		cnt++;
	}
	else
	{
		bit = 13;
	}

	/* shift out operands */
	cp = Op[op].parse;
	a = b = d = f = k = 0;

	while (bit >= 0)
	{
		/* osd_printf_debug("{%c/%d}",*cp,bit); */
		switch(*cp)
		{
			case 'a': a <<=1; a |= ((code & (1<<bit)) ? 1 : 0); bit--; break;
			case 'b': b <<=1; b |= ((code & (1<<bit)) ? 1 : 0); bit--; break;
			case 'd': d <<=1; d |= ((code & (1<<bit)) ? 1 : 0); bit--; break;
			case 'f': f <<=1; f |= ((code & (1<<bit)) ? 1 : 0); bit--; break;
			case 'k': k <<=1; k |= ((code & (1<<bit)) ? 1 : 0); bit--; break;
			case ' ': break;
			case '1': case '0': case 'x':  bit--; break;
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
				case 'F': if (f < 0x20) util::stream_format(stream, "%s",regfile[f]); else util::stream_format(stream, "Reg$%02X",f); break;
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

uint32_t pic16c62x_disassembler::opcode_alignment() const
{
	return 1;
}
