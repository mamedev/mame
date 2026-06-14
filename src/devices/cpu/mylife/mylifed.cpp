// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    This is a disassembler for an unknown 16-bit microcontroller (DSP?)
    architecture found in the "My Life" handheld.

    All instruction mnemonics are guessed, and several are strictly
    placeholders.

***************************************************************************/

#include "emu.h"
#include "mylifed.h"

mylife_disassembler::mylife_disassembler()
	: util::disasm_interface()
{
}

u32 mylife_disassembler::opcode_alignment() const
{
	return 1;
}

namespace {

const char *const c_branch_mnemonics[16] =
{
	"blo",
	"bhs",
	"br.2",
	"br.3",
	"bne",
	"beq",
	"br.6",
	"br.7",
	"br.8",
	"br.9",
	"bge",
	"blt",
	"bgt",
	"ble",
	"br.14",
	"bra"
};

const char *const c_jump_mnemonics[16] =
{
	"jlo",
	"jhs", // not used in mylife
	"jp.2",
	"jp.3",
	"jne",
	"jeq",
	"jp.6",
	"jp.7",
	"jp.8",
	"jp.9",
	"jge",
	"jlt",
	"jgt",
	"jle", // not used in mylife
	"jp.14",
	"jmp"
};

}

offs_t mylife_disassembler::disassemble(std::ostream &stream, offs_t pc, const mylife_disassembler::data_buffer &opcodes, const mylife_disassembler::data_buffer &params)
{
	const u16 op = opcodes.r16(pc);
	if ((op & 0xf818) == 0x0000)
	{
		util::stream_format(stream, "add r%d, r%d, r%d", BIT(op, 8, 3), BIT(op, 5, 3), BIT(op, 0, 3));
		return 1 | SUPPORTED;
	}
	else if ((op & 0xf818) == 0x0008)
	{
		util::stream_format(stream, "op0x r%d, r%d, r%d", BIT(op, 8, 3), BIT(op, 5, 3), BIT(op, 0, 3));
		return 1 | SUPPORTED;
	}
	else if ((op & 0xf818) == 0x0010)
	{
		util::stream_format(stream, "op01 r%d, r%d, r%d", BIT(op, 8, 3), BIT(op, 5, 3), BIT(op, 0, 3));
		return 1 | SUPPORTED;
	}
	else if ((op & 0xf818) == 0x0018)
	{
		util::stream_format(stream, "addi r%d, 0x%02X", BIT(op, 8, 3), BIT(op, 5, 3) << 3 | BIT(op, 0, 3));
		return 1 | SUPPORTED;
	}
	else if ((op & 0xf818) == 0x1000)
	{
		util::stream_format(stream, "op10 r%d, r%d, r%d", BIT(op, 8, 3), BIT(op, 5, 3), BIT(op, 0, 3));
		return 1 | SUPPORTED;
	}
	else if ((op & 0xf818) == 0x1008)
	{
		util::stream_format(stream, "op1x r%d, r%d, r%d", BIT(op, 8, 3), BIT(op, 5, 3), BIT(op, 0, 3));
		return 1 | SUPPORTED;
	}
	else if ((op & 0xf818) == 0x1018)
	{
		util::stream_format(stream, "subi r%d, 0x%02X", BIT(op, 8, 3), BIT(op, 5, 3) << 3 | BIT(op, 0, 3));
		return 1 | SUPPORTED;
	}
	else if ((op & 0xf818) == 0x2000)
	{
		util::stream_format(stream, "and r%d, r%d, r%d", BIT(op, 8, 3), BIT(op, 5, 3), BIT(op, 0, 3));
		return 1 | SUPPORTED;
	}
	else if ((op & 0xf818) == 0x2018)
	{
		util::stream_format(stream, "andi r%d, 0x%02X", BIT(op, 8, 3), BIT(op, 5, 3) << 3 | BIT(op, 0, 3));
		return 1 | SUPPORTED;
	}
	else if ((op & 0xf818) == 0x2800)
	{
		if (op == 0x2800)
			stream << "tst r0";
		else if (op == 0x2921)
			stream << "tst r1";
		else
			util::stream_format(stream, "or r%d, r%d, r%d", BIT(op, 8, 3), BIT(op, 5, 3), BIT(op, 0, 3));
		return 1 | SUPPORTED;
	}
	else if ((op & 0xf818) == 0x2808)
	{
		util::stream_format(stream, "orx r%d, r%d, r%d", BIT(op, 8, 3), BIT(op, 5, 3), BIT(op, 0, 3));
		return 1 | SUPPORTED;
	}
	else if ((op & 0xf818) == 0x2818)
	{
		util::stream_format(stream, "ori r%d, 0x%02X", BIT(op, 8, 3), BIT(op, 5, 3) << 3 | BIT(op, 0, 3));
		return 1 | SUPPORTED;
	}
	else if ((op & 0xf818) == 0x3000)
	{
		if (op == 0x3000)
			stream << "clr r0";
		else if (op == 0x3121)
			stream << "clr r1";
		else
			util::stream_format(stream, "xor r%d, r%d, r%d", BIT(op, 8, 3), BIT(op, 5, 3), BIT(op, 0, 3));
		return 1 | SUPPORTED;
	}
	else if ((op & 0xf81f) == 0x3800)
	{
		util::stream_format(stream, "addi r%d, r%d, 0x%04X", BIT(op, 8, 3), BIT(op, 5, 3), opcodes.r16(pc + 1));
		return 2 | SUPPORTED;
	}
	else if ((op & 0xf81f) == 0x3802)
	{
		util::stream_format(stream, "subi r%d, r%d, 0x%04X", BIT(op, 8, 3), BIT(op, 5, 3), opcodes.r16(pc + 1));
		return 2 | SUPPORTED;
	}
	else if ((op & 0xf81f) == 0x3804)
	{
		util::stream_format(stream, "andi r%d, r%d, 0x%04X", BIT(op, 8, 3), BIT(op, 5, 3), opcodes.r16(pc + 1));
		return 2 | SUPPORTED;
	}
	else if ((op & 0xf81f) == 0x3805)
	{
		util::stream_format(stream, "ori r%d, r%d, 0x%04X", BIT(op, 8, 3), BIT(op, 5, 3), opcodes.r16(pc + 1));
		return 2 | SUPPORTED;
	}
	else if ((op & 0xf81f) == 0x3806)
	{
		util::stream_format(stream, "xori r%d, r%d, 0x%04X", BIT(op, 8, 3), BIT(op, 5, 3), opcodes.r16(pc + 1));
		return 2 | SUPPORTED;
	}
	else if ((op & 0xf81f) == 0x3808)
	{
		util::stream_format(stream, "op38 r%d, r%d, 0x%04X", BIT(op, 8, 3), BIT(op, 5, 3), opcodes.r16(pc + 1));
		return 2 | SUPPORTED;
	}
	else if ((op & 0xf81f) == 0x380a)
	{
		util::stream_format(stream, "op3a r%d, r%d, 0x%04X", BIT(op, 8, 3), BIT(op, 5, 3), opcodes.r16(pc + 1));
		return 2 | SUPPORTED;
	}
	else if ((op & 0xf81f) == 0x380c)
	{
		util::stream_format(stream, "op3c r%d, r%d, 0x%04X", BIT(op, 8, 3), BIT(op, 5, 3), opcodes.r16(pc + 1));
		return 2 | SUPPORTED;
	}
	else if ((op & 0xf81f) == 0x380d)
	{
		util::stream_format(stream, "stx r%d, r%d, 0x%04X", BIT(op, 5, 3), BIT(op, 8, 3), opcodes.r16(pc + 1));
		return 2 | SUPPORTED;
	}
	else if ((op & 0xf81e) == 0x3810)
	{
		util::stream_format(stream, "op%x r%d, r%d, 0x%04X", BIT(op, 0, 5), BIT(op, 8, 3), BIT(op, 5, 3), opcodes.r16(pc + 1));
		return 2 | SUPPORTED;
	}
	else if ((op & 0xf81e) == 0x3812)
	{
		util::stream_format(stream, "op%x r%d, r%d, 0x%04X", BIT(op, 0, 5), BIT(op, 8, 3), BIT(op, 5, 3), opcodes.r16(pc + 1));
		return 2 | SUPPORTED;
	}
	else if ((op & 0xf81f) == 0x3815)
	{
		util::stream_format(stream, "op15 r%d, r%d, 0x%04X", BIT(op, 8, 3), BIT(op, 5, 3), opcodes.r16(pc + 1));
		return 2 | SUPPORTED;
	}
	else if ((op & 0xf87f) == 0x3818)
	{
		const u8 cond = BIT(op, 7, 4);
		util::stream_format(stream, "%s 0x%04X", c_jump_mnemonics[cond], u32(opcodes.r16(pc + 1)));
		return 2 | (cond == 15 ? 0 : STEP_COND) | SUPPORTED;
	}
	else if (op == 0x3819)
	{
		util::stream_format(stream, "jsr 0x%04X", u32(opcodes.r16(pc + 1)));
		return 2 | STEP_OVER | SUPPORTED;
	}
	else if (op == 0x381a)
	{
		stream << "op1a";
		return 1 | SUPPORTED;
	}
	else if (op == 0x383a)
	{
		stream << "rts";
		return 1 | STEP_OUT | SUPPORTED;
	}
	else if (op == 0x385a)
	{
		stream << "rti";
		return 1 | STEP_OUT | SUPPORTED;
	}
	else if ((op & 0xf81f) == 0x381b)
	{
		util::stream_format(stream, "add sp, %d", BIT(op, 5, 6));
		return 1 | SUPPORTED;
	}
	else if ((op & 0xf81f) == 0x381c)
	{
		util::stream_format(stream, "sub sp, %d", BIT(op, 5, 6));
		return 1 | SUPPORTED;
	}
//  else if (op == 0x381e) - used very rarely
	else if ((op & 0xf81f) == 0x381f)
	{
		// possibly rotate instead of logical or arithmetic shift? combined shift with r1?
		util::stream_format(stream, "shr r0, %d", BIT(op, 5, 6) + 1);
		return 1 | SUPPORTED;
	}
	else if ((op & 0xf818) == 0x4000)
	{
		util::stream_format(stream, "op40 r%d, r%d, r%d", BIT(op, 8, 3), BIT(op, 5, 3), BIT(op, 0, 3));
		return 1 | SUPPORTED;
	}
	else if ((op & 0xf818) == 0x4018)
	{
		util::stream_format(stream, "op4x r%d, r%d, r%d", BIT(op, 8, 3), BIT(op, 5, 3), BIT(op, 0, 3));
		return 1 | SUPPORTED;
	}
	else if ((op & 0xff18) == 0x4818)
	{
		util::stream_format(stream, "cmp r%d, r%d", BIT(op, 5, 3), BIT(op, 0, 3));
		return 1 | SUPPORTED;
	}
	else if ((op & 0xff18) == 0x4918)
	{
		util::stream_format(stream, "cmpx r%d, r%d", BIT(op, 5, 3), BIT(op, 0, 3));
		return 1 | SUPPORTED;
	}
//  else if ((op & 0xf81f) == 0x5801) - might have a different format
	else if ((op & 0xf81f) == 0x5802)
	{
		util::stream_format(stream, "op52 r%d, r%d", BIT(op, 8, 3), BIT(op, 5, 3));
		return 1 | SUPPORTED;
	}
	else if ((op & 0xf81f) == 0x5803)
	{
		util::stream_format(stream, "ld r%d, r%d", BIT(op, 8, 3), BIT(op, 5, 3));
		return 1 | SUPPORTED;
	}
	else if ((op & 0xf81f) == 0x5805)
	{
		util::stream_format(stream, "ld r%d, r%d+", BIT(op, 8, 3), BIT(op, 5, 3));
		return 1 | SUPPORTED;
	}
	else if ((op & 0xf81f) == 0x5806)
	{
		util::stream_format(stream, "op56 r%d, r%d", BIT(op, 8, 3), BIT(op, 5, 3));
		return 1 | SUPPORTED;
	}
	else if ((op & 0xf81f) == 0x5807)
	{
		util::stream_format(stream, "ld r%d, r%d-", BIT(op, 8, 3), BIT(op, 5, 3));
		return 1 | SUPPORTED;
	}
	else if ((op & 0xf81f) == 0x5808)
	{
		util::stream_format(stream, "op58 r%d, r%d", BIT(op, 8, 3), BIT(op, 5, 3));
		return 1 | SUPPORTED;
	}
	else if ((op & 0xf81f) == 0x5809)
	{
		util::stream_format(stream, "ldc r%d, r%d", BIT(op, 8, 3), BIT(op, 5, 3));
		return 1 | SUPPORTED;
	}
	else if ((op & 0xf81f) == 0x580a)
	{
		util::stream_format(stream, "op5a r%d, r%d", BIT(op, 8, 3), BIT(op, 5, 3));
		return 1 | SUPPORTED;
	}
	else if ((op & 0xf81f) == 0x580c)
	{
		util::stream_format(stream, "ror r%d, r%d", BIT(op, 8, 3), BIT(op, 5, 3));
		return 1 | SUPPORTED;
	}
	else if ((op & 0xf81f) == 0x580e)
	{
		// is this an arithmetic or logical shift?
		util::stream_format(stream, "lsr r%d, r%d", BIT(op, 8, 3), BIT(op, 5, 3));
		return 1 | SUPPORTED;
	}
//  else if ((op & 0xf81f) == 0x580f) - only used in setup code
	else if ((op & 0xf81f) == 0x581b)
	{
		util::stream_format(stream, "st r%d, r%d", BIT(op, 5, 3), BIT(op, 8, 3));
		return 1 | SUPPORTED;
	}
	else if ((op & 0xf81f) == 0x581d)
	{
		util::stream_format(stream, "st r%d, r%d+", BIT(op, 5, 3), BIT(op, 8, 3));
		return 1 | SUPPORTED;
	}
	else if ((op & 0xf8df) == 0x581e)
	{
		util::stream_format(stream, "%s r%d", BIT(op, 5) ? "pop" : "push", BIT(op, 8, 3));
		return 1 | SUPPORTED;
	}
	else if ((op & 0xf8ff) == 0x585e)
	{
		util::stream_format(stream, "jmp r%d", BIT(op, 8, 3));
		return 1 | SUPPORTED;
	}
	else if ((op & 0xf8df) == 0x589e)
	{
		util::stream_format(stream, "%s r%d, 0x%04X", BIT(op, 5) ? "st" : "ld", BIT(op, 8, 3), opcodes.r16(pc + 1));
		return 2 | SUPPORTED;
	}
	else if ((op & 0xf8ff) == 0x58de)
	{
		util::stream_format(stream, "sti 0x%04X, r%d", opcodes.r16(pc + 1), BIT(op, 8, 3));
		return 2 | SUPPORTED;
	}
	else if ((op & 0xf800) == 0x6000)
	{
		util::stream_format(stream, "li r%d, ", BIT(op, 8, 3));
		u16 op2 = opcodes.r16(pc + 1);
		if (((op ^ op2) & 0xff00) == 0x0800)
			util::stream_format(stream, "lo(0x%04X)", BIT(op2, 0, 8) << 8 | BIT(op, 0, 8));
		else
			util::stream_format(stream, "0x%02X", BIT(op, 0, 8));
		return 1 | SUPPORTED;
	}
	else if ((op & 0xf800) == 0x6800)
	{
		util::stream_format(stream, "lhi r%d, 0x%02X", BIT(op, 8, 3), BIT(op, 0, 8));
		return 1 | SUPPORTED;
	}
	else if (op >= 0x8000 && op < 0xa000)
	{
		const u8 cond = BIT(op, 9, 4);
		util::stream_format(stream, "%s 0x%04X", c_branch_mnemonics[cond], u16(pc + 1 + util::sext(op, 9)));
		return 1 | (cond == 15 ? 0 : STEP_COND) | SUPPORTED;
	}
	else if ((op & 0xfb00) == 0xa000)
	{
		util::stream_format(stream, "%s r%d, 0x%02X", BIT(op, 10) ? "std" : "ldd", BIT(op, 5, 3), BIT(op, 0, 5));
		return 1 | SUPPORTED;
	}
	else if ((op & 0xf800) == 0xb000)
	{
		util::stream_format(stream, "%s r%d, ", BIT(op, 7) ? "sts" : "lds", BIT(op, 8, 3));
		if (BIT(op, 0, 7) == 0x13)
			stream << "sp";
		else
			util::stream_format(stream, "0x%02X", BIT(op, 0, 7));
		return 1 | SUPPORTED;
	}
	else if ((op & 0xff00) == 0xb800)
	{
		util::stream_format(stream, "%s 0x%02X", BIT(op, 7) ? "pops" : "pushs", BIT(op, 0, 7));
		return 1 | SUPPORTED;
	}
	else if (op >= 0xc000)
	{
		util::stream_format(stream, "jsr 0x%04X", BIT(op, 0, 14));
		return 1 | STEP_OVER | SUPPORTED;
	}
	else
	{
		util::stream_format(stream, "? (0x%04X)", op);
		return 1 | SUPPORTED;
	}
}
