// license:GPL-2.0+
// copyright-holders:Segher Boessenkool, David Haywood
/*****************************************************************************

    SunPlus µ'nSP disassembler

    Copyright 2008-2017  Segher Boessenkool  <segher@kernel.crashing.org>
    Licensed under the terms of the GNU GPL, version 2
    http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt

*****************************************************************************/

#include "emu.h"
#include "unspdasm.h"

offs_t unsp_disassembler::disassemble_fxxx_000_group(std::ostream& stream, offs_t pc, uint16_t op, uint16_t ximm)
{
	//                          |   | |
	// DS16     1 1 1 1   1 1 1 0   0 0 i i   i i i i
	// DS Reg   1 1 1 1   - - - 0   0 0 1 0   w r r r
	// FR Reg   1 1 1 1   - - - 0   0 0 1 1   w r r r
	uint32_t len = 1;

	if (((op & 0xffc0) == 0xfe00) && m_iso >= 12)
	{
		int imm = op & 0x003f;
		util::stream_format(stream, "ds = %02x", imm);
		return UNSP_DASM_OK;
	}
	else if (((op & 0xf1f8) == 0xf020) && m_iso >= 12)
	{
		int r = op & 0x7;
		util::stream_format(stream, "%s = ds", regs[r]);
		return UNSP_DASM_OK;
	}
	else if (((op & 0xf1f8) == 0xf028) && m_iso >= 12)
	{
		int r = op & 0x7;
		util::stream_format(stream, "ds = %s", regs[r]);
		return UNSP_DASM_OK;
	}
	else if (((op & 0xf1f8) == 0xf030) && m_iso >= 12)
	{
		int r = op & 0x7;
		util::stream_format(stream, "%s = fr", regs[r]);
		return UNSP_DASM_OK;
	}
	else if (((op & 0xf1f8) == 0xf038) && m_iso >= 12)
	{
		int r = op & 0x7;
		util::stream_format(stream, "fr = %s", regs[r]);
		return UNSP_DASM_OK;
	}

	// everything else falls through to the multiply

	// signed * unsigned
	// MUL      1 1 1 1*  r r r 0*  0 0 0 0   1 r r r     (** = sign bits, fixed here)
	print_mul(stream, op); // MUL us
	return UNSP_DASM_OK;
}

offs_t unsp_disassembler::disassemble_fxxx_001_group(std::ostream& stream, offs_t pc, uint16_t op, uint16_t ximm)
{
	//                             |   | |
	// CALL16      1 1 1 1   - - 0 0   0 1 a a   a a a a   (+imm16)

	uint32_t len = 1;

	if ((op & 0xf3c0) == 0xf040)
	{
		len = 2;
		uint8_t opimm = op & 0x3f;
		util::stream_format(stream, "call %06x", (opimm << 16) | ximm);
		return UNSP_DASM_OK;
	}
	// MEM bitop   1 1 1 1   - D 1 0   0 1 b b   o o o o   (+imm16)   ( BITOP {ds:}[A16],offset )
	else if (((op & 0xf3c0) == 0xf240) && m_iso >= 20)
	{
		len = 2;
		uint8_t bitop =  (op & 0x0030) >> 4;
		uint8_t offset = (op & 0x000f) >> 0;
		uint8_t d =      (op & 0x0400) >> 10;

		if (d)
			util::stream_format(stream, "%s ds:[%04x],%d", bitops[bitop], ximm, offset);
		else
			util::stream_format(stream, "%s [%04x],%d", bitops[bitop], ximm, offset);

		return UNSP_DASM_OK;
	}

	util::stream_format(stream, "<DUNNO>");
	return UNSP_DASM_OK;
}

offs_t unsp_disassembler::disassemble_fxxx_010_group(std::ostream& stream, offs_t pc, uint16_t op, uint16_t ximm)
{
	uint32_t len = 1;
	//                         |   | |
	// JMPF    1 1 1 1   1 1 1 0   1 0 a a   a a a a    (+imm16)

	if ((op & 0xffc0) == 0xfe80) // apparently 1.2 and above, but jak_capb needs it and otherwise seems 1.0 / 1.1?
	{
		len = 2;
		uint8_t opimm = op & 0x3f;
		util::stream_format(stream, "goto %06x", (opimm << 16) | ximm);
		return UNSP_DASM_OK;
	}

	// signed * unsigned  (size 16,1,2,3,4,5,6,7)
	// MULS    1 1 1 1*  r r r 0*  1 0*s s   s r r r    (1* = sign bit, 0* = sign bit 0* = upper size bit)

	// MULS us with upper size bit not set
	print_muls(stream, op);
	return UNSP_DASM_OK;
}

offs_t unsp_disassembler::disassemble_fxxx_011_group(std::ostream& stream, offs_t pc, uint16_t op, uint16_t ximm)
{
	uint32_t len = 1;
	//                         |   | |
	// JMPR    1 1 1 1   1 1 1 0   1 1 - -   - - - -
	if (((op & 0xffc0) == 0xfec0) && m_iso >= 12)
	{
		util::stream_format(stream, "goto mr");
		return UNSP_DASM_OK;
	}

	// signed * unsigned  (size 8,9,10,11,12,13,14,15)
	// MULS    1 1 1 1*  r r r 0*  1 1*s s   s r r r    (1* = sign bit, 0* = sign bit 1* = upper size bit)

	// MULS us with upper size bit set
	print_muls(stream, op);

	return UNSP_DASM_OK;
}

offs_t unsp_disassembler::disassemble_fxxx_100_group(std::ostream& stream, offs_t pc, uint16_t op, uint16_t ximm)
{
	uint32_t len = 1;
	//                          |   | |
	// signed * signed
	// MUL      1 1 1 1*  r r r 1*  0 0 0 0   1 r r r     (** = sign bits, fixed here)

	if ((op & 0xf1f8) == 0xf108)
	{
		print_mul(stream, op); // MUL ss
		return UNSP_DASM_OK;
	}

	util::stream_format(stream, "<DUNNO>");
	return UNSP_DASM_OK;
}


offs_t unsp_12_disassembler::disassemble_fxxx_101_group(std::ostream& stream, offs_t pc, uint16_t op, uint16_t ximm)
{
	// IRQ       1 1 1 1   - - - 1   0 1 0 0   1 0 0 I
	// FIRQ      1 1 1 1   - - - 1   0 1 0 0   1 1 F 0
	// FIR_MOV   1 1 1 1   - - - 1   0 1 0 0   0 1 0 f
	// Fraction  1 1 1 1   - - - 1   0 1 0 0   0 1 1 f
	// SECBANK   1 1 1 1   - - - 1   0 1 0 0   1 0 1 S
	// NESTMODE  1 1 1 1   - - - 1   0 1 0 0   1 1 N 1
	// CALLR     1 1 1 1   - - - 1   0 1 1 -   - 0 0 1
	// DIVS      1 1 1 1   - - - 1   0 1 1 -   - 0 1 0
	// DIVQ      1 1 1 1   - - - 1   0 1 1 -   - 0 1 1
	// EXP       1 1 1 1   - - - 1   0 1 1 -   - 1 0 0

	uint32_t len = 1;

	switch (op)
	{
	case 0xf146: case 0xf346: case 0xf546: case 0xf746: case 0xf946: case 0xfb46: case 0xfd46: case 0xff46:
		util::stream_format(stream, "fraction off");
		return UNSP_DASM_OK;

	case 0xf147: case 0xf347: case 0xf547: case 0xf747: case 0xf947: case 0xfb47: case 0xfd47: case 0xff47:
		util::stream_format(stream, "fraction on");
		return UNSP_DASM_OK;

	case 0xf14a: case 0xf34a: case 0xf54a: case 0xf74a: case 0xf94a: case 0xfb4a: case 0xfd4a: case 0xff4a:
		util::stream_format(stream, "secbank off");
		return UNSP_DASM_OK;

	case 0xf14b: case 0xf34b: case 0xf54b: case 0xf74b: case 0xf94b: case 0xfb4b: case 0xfd4b: case 0xff4b:
		util::stream_format(stream, "secbank on");
		return UNSP_DASM_OK;

	case 0xf14d: case 0xf34d: case 0xf54d: case 0xf74d: case 0xf94d: case 0xfb4d: case 0xfd4d: case 0xff4d:
		util::stream_format(stream, "irqnest off");
		return UNSP_DASM_OK;

	case 0xf14f: case 0xf34f: case 0xf54f: case 0xf74f: case 0xf94f: case 0xfb4f: case 0xfd4f: case 0xff4f:
		util::stream_format(stream, "irqnest on");
		return UNSP_DASM_OK;

	case 0xf144: case 0xf344: case 0xf544: case 0xf744: case 0xf944: case 0xfb44: case 0xfd44: case 0xff44:
		util::stream_format(stream, "fir_mov on");
		return UNSP_DASM_OK;

	case 0xf145: case 0xf345: case 0xf545: case 0xf745: case 0xf945: case 0xfb45: case 0xfd45: case 0xff45:
		util::stream_format(stream, "fir_mov off");
		return UNSP_DASM_OK;

	case 0xf161: case 0xf361: case 0xf561: case 0xf761: case 0xf961: case 0xfb61: case 0xfd61: case 0xff61:
	case 0xf169: case 0xf369: case 0xf569: case 0xf769: case 0xf969: case 0xfb69: case 0xfd69: case 0xff69:
	case 0xf171: case 0xf371: case 0xf571: case 0xf771: case 0xf971: case 0xfb71: case 0xfd71: case 0xff71:
	case 0xf179: case 0xf379: case 0xf579: case 0xf779: case 0xf979: case 0xfb79: case 0xfd79: case 0xff79:
		util::stream_format(stream, "call mr");
		return UNSP_DASM_OK;

	case 0xf162: case 0xf362: case 0xf562: case 0xf762: case 0xf962: case 0xfb62: case 0xfd62: case 0xff62:
	case 0xf16a: case 0xf36a: case 0xf56a: case 0xf76a: case 0xf96a: case 0xfb6a: case 0xfd6a: case 0xff6a:
	case 0xf172: case 0xf372: case 0xf572: case 0xf772: case 0xf972: case 0xfb72: case 0xfd72: case 0xff72:
	case 0xf17a: case 0xf37a: case 0xf57a: case 0xf77a: case 0xf97a: case 0xfb7a: case 0xfd7a: case 0xff7a:
		util::stream_format(stream, "divs mr, r2");
		return UNSP_DASM_OK;

	case 0xf163: case 0xf363: case 0xf563: case 0xf763: case 0xf963: case 0xfb63: case 0xfd63: case 0xff63:
	case 0xf16b: case 0xf36b: case 0xf56b: case 0xf76b: case 0xf96b: case 0xfb6b: case 0xfd6b: case 0xff6b:
	case 0xf173: case 0xf373: case 0xf573: case 0xf773: case 0xf973: case 0xfb73: case 0xfd73: case 0xff73:
	case 0xf17b: case 0xf37b: case 0xf57b: case 0xf77b: case 0xf97b: case 0xfb7b: case 0xfd7b: case 0xff7b:
		util::stream_format(stream, "divq mr, r2");
		return UNSP_DASM_OK;

	case 0xf164: case 0xf364: case 0xf564: case 0xf764: case 0xf964: case 0xfb64: case 0xfd64: case 0xff64:
	case 0xf16c: case 0xf36c: case 0xf56c: case 0xf76c: case 0xf96c: case 0xfb6c: case 0xfd6c: case 0xff6c:
	case 0xf174: case 0xf374: case 0xf574: case 0xf774: case 0xf974: case 0xfb74: case 0xfd74: case 0xff74:
	case 0xf17c: case 0xf37c: case 0xf57c: case 0xf77c: case 0xf97c: case 0xfb7c: case 0xfd7c: case 0xff7c:
		util::stream_format(stream, "r2 = exp r4");
		return UNSP_DASM_OK;


	default:
		return unsp_disassembler::disassemble_fxxx_101_group(stream, pc, op, ximm);
	}
}


offs_t unsp_disassembler::disassemble_fxxx_101_group(std::ostream& stream, offs_t pc, uint16_t op, uint16_t ximm)
{
	//                           |   | |
	// INT SET   1 1 1 1   - - - 1   0 1 0 0   0 0 F I
	// BREAK     1 1 1 1   - - - 1   0 1 1 -   - 0 0 0
	// NOP       1 1 1 1   - - - 1   0 1 1 -   - 1 0 1

	uint32_t len = 1;

	switch (op)
	{
	case 0xf140: case 0xf340: case 0xf540: case 0xf740: case 0xf940: case 0xfb40: case 0xfd40: case 0xff40:
		util::stream_format(stream, "int off");
		return UNSP_DASM_OK;

	case 0xf141: case 0xf341: case 0xf541: case 0xf741: case 0xf941: case 0xfb41: case 0xfd41: case 0xff41:
		util::stream_format(stream, "int irq");
		return UNSP_DASM_OK;

	case 0xf142: case 0xf342: case 0xf542: case 0xf742: case 0xf942: case 0xfb42: case 0xfd42: case 0xff42:
		util::stream_format(stream, "int fiq");
		return UNSP_DASM_OK;

	case 0xf143: case 0xf343: case 0xf543: case 0xf743: case 0xf943: case 0xfb43: case 0xfd43: case 0xff43:
		util::stream_format(stream, "int fiq,irq");
		return UNSP_DASM_OK;

	case 0xf144: case 0xf344: case 0xf544: case 0xf744: case 0xf944: case 0xfb44: case 0xfd44: case 0xff44:
		util::stream_format(stream, "fir_mov on");
		return UNSP_DASM_OK;

	case 0xf145: case 0xf345: case 0xf545: case 0xf745: case 0xf945: case 0xfb45: case 0xfd45: case 0xff45:
		util::stream_format(stream, "fir_mov off");
		return UNSP_DASM_OK;

	case 0xf160: case 0xf360: case 0xf560: case 0xf760: case 0xf960: case 0xfb60: case 0xfd60: case 0xff60:
	case 0xf168: case 0xf368: case 0xf568: case 0xf768: case 0xf968: case 0xfb68: case 0xfd68: case 0xff68:
	case 0xf170: case 0xf370: case 0xf570: case 0xf770: case 0xf970: case 0xfb70: case 0xfd70: case 0xff70:
	case 0xf178: case 0xf378: case 0xf578: case 0xf778: case 0xf978: case 0xfb78: case 0xfd78: case 0xff78:
		util::stream_format(stream, "break");
		return UNSP_DASM_OK;

	case 0xf165: case 0xf365: case 0xf565: case 0xf765: case 0xf965: case 0xfb65: case 0xfd65: case 0xff65:
	case 0xf16d: case 0xf36d: case 0xf56d: case 0xf76d: case 0xf96d: case 0xfb6d: case 0xfd6d: case 0xff6d:
	case 0xf175: case 0xf375: case 0xf575: case 0xf775: case 0xf975: case 0xfb75: case 0xfd75: case 0xff75:
	case 0xf17d: case 0xf37d: case 0xf57d: case 0xf77d: case 0xf97d: case 0xfb7d: case 0xfd7d: case 0xff7d:
		util::stream_format(stream, "nop");
		return UNSP_DASM_OK;

	// these are in code that otherwise seems like ISA1.0 / 1.1 (jak_disf for example) but spec says it's only in ISA 1.2?
	case 0xf148: case 0xf348: case 0xf548: case 0xf748: case 0xf948: case 0xfb48: case 0xfd48: case 0xff48:
		util::stream_format(stream, "irq off");
		return UNSP_DASM_OK;

	case 0xf149: case 0xf349: case 0xf549: case 0xf749: case 0xf949: case 0xfb49: case 0xfd49: case 0xff49:
		util::stream_format(stream, "irq on");
		return UNSP_DASM_OK;

	case 0xf14c: case 0xf34c: case 0xf54c: case 0xf74c: case 0xf94c: case 0xfb4c: case 0xfd4c: case 0xff4c:
		util::stream_format(stream, "fiq off");
		return UNSP_DASM_OK;

	case 0xf14e: case 0xf34e: case 0xf54e: case 0xf74e: case 0xf94e: case 0xfb4e: case 0xfd4e: case 0xff4e:
		util::stream_format(stream, "fiq on");
		return UNSP_DASM_OK;

	default:
		util::stream_format(stream, "<UNDEFINED>");
		return UNSP_DASM_OK;
	}
}

offs_t unsp_disassembler::disassemble_fxxx_110_group(std::ostream& stream, offs_t pc, uint16_t op, uint16_t ximm, const data_buffer &opcodes)
{
	//                         |   | |

	// some sources say this is FFc0, but smartfp clearly uses ff80
	// EXTOP   1 1 1 1   1 1 1 1   1 0 0 0   0 0 0 0    (+16 bit imm)
	if ((op == 0xff80) && m_iso >= 20)
	{
		return disassemble_extended_group(stream, pc, op, ximm, opcodes);
	}

	uint32_t len = 1;
	//                         |   | |
	// signed * signed  (size 16,1,2,3,4,5,6,7)
	// MULS    1 1 1 1*  r r r 1*  1 0*s s   s r r r    (1* = sign bit, 1* = sign bit 0* = upper size bit)

	// MULS ss with upper size bit not set
	print_muls(stream, op);
	return UNSP_DASM_OK;
}

offs_t unsp_disassembler::disassemble_fxxx_111_group(std::ostream& stream, offs_t pc, uint16_t op, uint16_t ximm,const data_buffer &opcodes)
{
	uint32_t len = 1;

	// signed * signed  (size 8,9,10,11,12,13,14,15)
	// MULS    1 1 1 1*  r r r 1*  1 1*s s   s r r r    (1* = sign bit, 1* = sign bit 1* = upper size bit)

	// MULS ss with upper size bit set.
	print_muls(stream, op);
	return UNSP_DASM_OK;
}


offs_t unsp_disassembler::disassemble_fxxx_group(std::ostream &stream, offs_t pc, uint16_t op, uint16_t ximm,const data_buffer &opcodes)
{
	uint32_t len = 1;

	switch ((op & 0x01c0) >> 6)
	{
	case 0x0:
		return disassemble_fxxx_000_group(stream, pc, op, ximm);

	case 0x1:
		return disassemble_fxxx_001_group(stream, pc, op, ximm);

	case 0x2:
		return disassemble_fxxx_010_group(stream, pc, op, ximm);

	case 0x3:
		return disassemble_fxxx_011_group(stream, pc, op, ximm);

	case 0x4:
		return disassemble_fxxx_100_group(stream, pc, op, ximm);

	case 0x5:
		return disassemble_fxxx_101_group(stream, pc, op, ximm);

	case 0x6:
		return disassemble_fxxx_110_group(stream, pc, op, ximm, opcodes);

	case 0x7:
		return disassemble_fxxx_111_group(stream, pc, op, ximm, opcodes);

	}

	return UNSP_DASM_OK;
}
