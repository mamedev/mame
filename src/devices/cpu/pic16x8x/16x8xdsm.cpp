// license:BSD-3-Clause
// copyright-holders: Grull Osgo
/************************************************************************

  Microchip PIC16X8x Emulator

  Based on MAME's PIC16C5x cpu_device dissasm developed by Tony La Porta.

A Address to jump to.
B Bit address within an 8-bit file register.
D Destination select (0 = store result in W (accumulator))
                     (1 = store result in file register)
F Register file address.
K Literal field, constant data.

Includes Flags ID's on SFR registers where possible.

************************************************************************/

#include "emu.h"
#include "16x8xdsm.h"

#include <cctype>
#include <stdexcept>

const u8 pic16x8x_disassembler::sfr_bank0[16] = { 0, 0, 0, 1, 0, 2, 3, 0, 0, 0, 0, 4, 0, 0, 0, 0};

const char *const pic16x8x_disassembler::sfregs[12] =
{
	"INDF", "TMR0", "PCL", "STATUS", "FSR", "PORTA", "PORTB", "Reg$07", "EEDATA", "EEADDR", "PCLATH", "INTCON"
};

const char *const pic16x8x_disassembler::dest[2] = { "W", "Reg" };

const char *const pic16x8x_disassembler::reg_flags[9][8] =
{
    {"0", "1", "2", "3", "4", "5", "6", "7"},                         // no flags
    {"C", "DC", "Z", "PD", "TO", "RP0", "RP1", "IRP"},                // status
    {"RA0", "RA1", "RA2", "RA3", "RA4/T0CKI", "5", "6", "7"},         // portA
    {"RB0/INT", "RB1", "RB2", "RB3", "RB4", "RB5", "RB6", "RB7"},     // portB
    {"RBIF", "INTF", "T0IF", "RBIE", "INTE", "T0IE", "EEIE", "GIE"},  // intcon
    {"PS0", "PS1", "PS2", "PSA", "T0SE", "T0CS", "INTEDG", "RBPU"},   // option
    {"RA0", "RA1", "RA2", "RA3", "RA4", "5", "6", "7"},               // trisa
    {"RB0", "RB1", "RB2", "RB3", "RB4", "RB5", "RB6", "RB7"},         // trisb
    {"RD", "WR", "WREN", "WRERR", "EEIF", "5", "6", "7"}              // eecon
};

const char *const pic16x8x_disassembler::PIC16X8xFormats[] =
{
	"00000000000000", "nop",
	"00000000001000", "return",
	"00000000001001", "retfie",	
	"00000000100000", "nop",
	"00000001000000", "nop",
	"00000001100000", "nop",
	"00000001100011", "sleep",
	"00000001100100", "clrwdt",
	"0000001fffffff", "movwf  %F",
	"00000100000000", "clrw",
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
	"110100kkkkkkkk", "retlw  %K",
	"110101kkkkkkkk", "retlw  %K",
	"110110kkkkkkkk", "retlw  %K",
	"110111kkkkkkkk", "retlw  %K",
	"100aaaaaaaaaaa", "call   %A",
	"101aaaaaaaaaaa", "goto   %A",
	"110000kkkkkkkk", "movlw  %K",
	"110001kkkkkkkk", "movlw  %K",
	"110010kkkkkkkk", "movlw  %K",
	"110011kkkkkkkk", "movlw  %K",	
	"111000kkkkkkkk", "iorlw  %K",
	"111001kkkkkkkk", "andlw  %K",
	"111010kkkkkkkk", "xorlw  %K",
	"111100kkkkkkkk", "sublw  %K",
	"111101kkkkkkkk", "sublw  %K",
	"111110kkkkkkkk", "addlw  %K",
	"111111kkkkkkkk", "addlw  %K",
	nullptr
};

pic16x8x_disassembler::pic16x8x_disassembler()
{
	const char *p;
	const char *const *ops;
	u16 mask, bits;
	int bit;

	ops = PIC16X8xFormats;
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
					bit--;
					break;
				default:
					throw std::logic_error(util::string_format("Invalid instruction encoding '%s %s'\n", ops[0],ops[1]));
			}
		}
		if (bit != -1)
		{
			throw std::logic_error(util::string_format("not enough bits in encoding '%s %s' %d\n", ops[0],ops[1],bit));
		}
		while (isspace((u8)*p)) p++;
		Op.emplace_back(mask, bits, *p, ops[0], ops[1]);

		ops += 2;
	}
}

offs_t pic16x8x_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	int a, b, d, f, k; // these can all be filled in by parsing an instruction
	int i;
	int op;
	int cnt = 1;
	int code;
	int bit;
	const char *cp; // character pointer in OpFormats
	u32 flags = 0;
	bool bit_option = false;

	op = -1; // no matching opcode
	code = opcodes.r16(pc);
	for (i = 0; i < int(Op.size()); i++)
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

	// shift out operands
	cp = Op[op].parse;
	a = b = d = f = k = 0;
	bit = 13;

	while (bit >= 0)
	{
		osd_printf_debug("{%c/%d}",*cp,bit);
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
	
	
	// now traverse format string
	cp = Op[op].fmt;
	if (!strncmp(cp, "call", 4))
		flags = STEP_OVER;
	else if (!strncmp(cp, "ret", 3))
		flags = STEP_OUT;
	else if (!strncmp(cp, "btfs", 4) || !strncmp(cp + 2, "cfsz", 4))
		flags = STEP_COND;

	// bit oriented instructions
	switch (f)
	{
		case 0x03:
		case 0x05:
		case 0x06:
		case 0x0b:
		case 0x81:
		case 0x85:
		case 0x86:
			bit_option = true;
			break;
		default:
			bit_option = false;
			break;
	}		

	
	while (*cp)
	{
		if (*cp == '%')
		{
			cp++;
			switch (*cp++)
			{
				case 'A': util::stream_format(stream, "$%03X", a); break; 
				case 'D': util::stream_format(stream, "%s", dest[d]); break;
				case 'F': (f < 0xc) ? util::stream_format(stream, "%s", sfregs[f]) : util::stream_format(stream, "byte_DATA_$%02X", f); break;
				case 'B':
				{
					if (bit_option)
						util::stream_format(stream, "%s", reg_flags[sfr_bank0[f & 0x0f]][b]);
					else
						util::stream_format(stream, "%d", b);
				}
					break; 		
				case 'K': util::stream_format(stream, "%02xh", k); break;
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

u32 pic16x8x_disassembler::opcode_alignment() const
{
	return 1;
}
