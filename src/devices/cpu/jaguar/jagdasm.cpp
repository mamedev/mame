// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    jagdasm.cpp
    Disassembler for the portable Jaguar DSP emulator.
    Written by Aaron Giles

***************************************************************************/

#include "emu.h"
#include "jagdasm.h"

/***************************************************************************
    STATIC VARIABLES
***************************************************************************/

const u8 jaguar_disassembler::convert_zero[32] =
{ 32,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31 };

const char *const jaguar_disassembler::condition[32] =
{
	"",
	"nz,",
	"z,",
	"???,",
	"nc,",
	"nc nz,",
	"nc z,",
	"???,",
	"c,",
	"c nz,",
	"c z,",
	"???,",
	"???,",
	"???,",
	"???,",
	"???,",

	"???,",
	"???,",
	"???,",
	"???,",
	"nn,",
	"nn nz,",
	"nn z,",
	"???,",
	"n,",
	"n nz,",
	"n z,",
	"???,",
	"???,",
	"???,",
	"???,",
	"never,"
};



/***************************************************************************
    CODE CODE
***************************************************************************/

std::string jaguar_disassembler::signed_16bit(int16_t val)
{
	if (val < 0)
		return util::string_format("-$%x", -val);
	else
		return util::string_format("$%x", val);
}

offs_t jaguar_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	u32 flags = 0;
	const u16 op = opcodes.r16(pc);
	const u8 reg1 = (op >> 5) & 31;
	const u8 reg2 = op & 31;
	int size = 2;

	pc += 2;
	switch (op >> 10)
	{
		case 0:     util::stream_format(stream, "add     r%d,r%d", reg1, reg2);                 break;
		case 1:     util::stream_format(stream, "addc    r%d,r%d", reg1, reg2);                 break;
		case 2:     util::stream_format(stream, "addq    $%x,r%d", convert_zero[reg1], reg2);   break;
		case 3:     util::stream_format(stream, "addqt   $%x,r%d", convert_zero[reg1], reg2);   break;
		case 4:     util::stream_format(stream, "sub     r%d,r%d", reg1, reg2);                 break;
		case 5:     util::stream_format(stream, "subc    r%d,r%d", reg1, reg2);                 break;
		case 6:     util::stream_format(stream, "subq    $%x,r%d", convert_zero[reg1], reg2);   break;
		case 7:     util::stream_format(stream, "subqt   $%x,r%d", convert_zero[reg1], reg2);   break;
		case 8:     util::stream_format(stream, "neg     r%d", reg2);                           break;
		case 9:     util::stream_format(stream, "and     r%d,r%d", reg1, reg2);                 break;
		case 10:    util::stream_format(stream, "or      r%d,r%d", reg1, reg2);                 break;
		case 11:    util::stream_format(stream, "xor     r%d,r%d", reg1, reg2);                 break;
		case 12:    util::stream_format(stream, "not     r%d", reg2);                           break;
		case 13:    util::stream_format(stream, "btst    $%x,r%d", reg1, reg2);                 break;
		case 14:    util::stream_format(stream, "bset    $%x,r%d", reg1, reg2);                 break;
		case 15:    util::stream_format(stream, "bclr    $%x,r%d", reg1, reg2);                 break;
		case 16:    util::stream_format(stream, "mult    r%d,r%d", reg1, reg2);                 break;
		case 17:    util::stream_format(stream, "imult   r%d,r%d", reg1, reg2);                 break;
		case 18:    util::stream_format(stream, "imultn  r%d,r%d", reg1, reg2);                 break;
		case 19:    util::stream_format(stream, "resmac  r%d", reg2);                           break;
		case 20:    util::stream_format(stream, "imacn   r%d,r%d", reg1, reg2);                 break;
		case 21:    util::stream_format(stream, "div     r%d,r%d", reg1, reg2);                 break;
		case 22:    util::stream_format(stream, "abs     r%d", reg2);                           break;
		case 23:    util::stream_format(stream, "sh      r%d,r%d", reg1, reg2);                 break;
		case 24:    util::stream_format(stream, "shlq    $%x,r%d", 32 - convert_zero[reg1], reg2);  break;
		case 25:    util::stream_format(stream, "shrq    $%x,r%d", convert_zero[reg1], reg2);   break;
		case 26:    util::stream_format(stream, "sha     r%d,r%d", reg1, reg2);                 break;
		case 27:    util::stream_format(stream, "sharq   $%x,r%d", convert_zero[reg1], reg2);   break;
		case 28:    util::stream_format(stream, "ror     r%d,r%d", reg1, reg2);                 break;
		case 29:    util::stream_format(stream, "rorq    $%x,r%d", convert_zero[reg1], reg2);   break;
		case 30:    util::stream_format(stream, "cmp     r%d,r%d", reg1, reg2);                 break;
		case 31:    util::stream_format(stream, "cmpq    %s,r%d", signed_16bit((int16_t)(reg1 << 11) >> 11), reg2);break;

		case 32:    if (m_variant == variant::GPU)
					util::stream_format(stream, "sat8    r%d", reg2);
					else
					util::stream_format(stream, "subqmod $%x,r%d", convert_zero[reg1], reg2);
					break;
		case 33:    if (m_variant == variant::GPU)
					util::stream_format(stream, "sat16   r%d", reg2);
					else
					util::stream_format(stream, "sat16s  r%d", reg2);
					break;
		case 34:    util::stream_format(stream, "move    r%d,r%d", reg1, reg2);                 break;
		case 35:    util::stream_format(stream, "moveq   %d,r%d", reg1, reg2);                  break;
		case 36:    util::stream_format(stream, "moveta  r%d,r%d", reg1, reg2);                 break;
		case 37:    util::stream_format(stream, "movefa  r%d,r%d", reg1, reg2);                 break;
		case 38:    util::stream_format(stream, "movei   $%x,r%d", opcodes.r32(pc+2), reg2); size = 6; break;
		case 39:    util::stream_format(stream, "loadb   (r%d),r%d", reg1, reg2);                   break;
		case 40:    util::stream_format(stream, "loadw   (r%d),r%d", reg1, reg2);                   break;
		case 41:    util::stream_format(stream, "load    (r%d),r%d", reg1, reg2);                   break;
		case 42:    if (m_variant == variant::GPU)
					util::stream_format(stream, "loadp   (r%d),r%d", reg1, reg2);
					else
					util::stream_format(stream, "sat32s  r%d", reg2);
					break;
		case 43:    util::stream_format(stream, "load    (r14+$%x),r%d", convert_zero[reg1]*4, reg2);break;
		case 44:    util::stream_format(stream, "load    (r15+$%x),r%d", convert_zero[reg1]*4, reg2);break;
		case 45:    util::stream_format(stream, "storeb  r%d,(r%d)", reg2, reg1);               break;
		case 46:    util::stream_format(stream, "storew  r%d,(r%d)", reg2, reg1);               break;
		case 47:    util::stream_format(stream, "store   r%d,(r%d)", reg2, reg1);                   break;
		case 48:    if (m_variant == variant::GPU)
					util::stream_format(stream, "storep  r%d,(r%d)", reg2, reg1);
					else
					util::stream_format(stream, "mirror  r%d", reg2);
					break;
		case 49:    util::stream_format(stream, "store   r%d,(r14+$%x)", reg2, convert_zero[reg1]*4);break;
		case 50:    util::stream_format(stream, "store   r%d,(r15+$%x)", reg2, convert_zero[reg1]*4);break;
		case 51:    util::stream_format(stream, "move    pc,r%d", reg2);                            break;
		case 52:    util::stream_format(stream, "jump    %s(r%d)", condition[reg2], reg1);          break;
		case 53:    util::stream_format(stream, "jr      %s%08X", condition[reg2], pc + ((s8)(reg1 << 3) >> 2)); break;
		case 54:    util::stream_format(stream, "mmult   r%d,r%d", reg1, reg2);                 break;
		case 55:    util::stream_format(stream, "mtoi    r%d,r%d", reg1, reg2);                 break;
		case 56:    util::stream_format(stream, "normi   r%d,r%d", reg1, reg2);                 break;
		case 57:    util::stream_format(stream, "nop");                                         break;
		case 58:    util::stream_format(stream, "load    (r14+r%d),r%d", reg1, reg2);               break;
		case 59:    util::stream_format(stream, "load    (r15+r%d),r%d", reg1, reg2);               break;
		case 60:    util::stream_format(stream, "store   r%d,(r14+r%d)", reg2, reg1);               break;
		case 61:    util::stream_format(stream, "store   r%d,(r15+r%d)", reg2, reg1);               break;
		case 62:    if (m_variant == variant::GPU)
					util::stream_format(stream, "sat24   r%d", reg2);
					else
					util::stream_format(stream, "illegal");
					break;
		case 63:    if (m_variant == variant::GPU)
					util::stream_format(stream, reg1 ?
									"unpack  r%d" :
									"pack    r%d", reg2);
					else
					util::stream_format(stream, "addqmod $%x,r%d", convert_zero[reg1], reg2);
					break;
	}

	return size | flags | SUPPORTED;
}

jaguar_disassembler::jaguar_disassembler(variant var) : m_variant(var)
{
}

u32 jaguar_disassembler::opcode_alignment() const
{
	return 2;
}
