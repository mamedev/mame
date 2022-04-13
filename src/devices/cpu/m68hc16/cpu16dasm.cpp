// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Motorola CPU16 (M68HC16 family) disassembler

    The instruction set was presented as an upgrade of M68HC11. Its
    encoding is quite different, however. One particular change is that
    the 8-bit direct mode no longer exists. The use of PC+6 as the base
    for relative jump destinations is an artifact of pipelining.

***************************************************************************/

#include "emu.h"
#include "cpu16dasm.h"

cpu16_disassembler::cpu16_disassembler()
	: util::disasm_interface()
{
}

u32 cpu16_disassembler::opcode_alignment() const
{
	return 2;
}

const cpu16_disassembler::opcode_info cpu16_disassembler::s_opinfo[4][256] =
{
	{
		// 0X
		{ "com", cpu16_disassembler::mode::IND, 0 },
		{ "dec", cpu16_disassembler::mode::IND, 0 },
		{ "neg", cpu16_disassembler::mode::IND, 0 },
		{ "inc", cpu16_disassembler::mode::IND, 0 },
		{ "asl", cpu16_disassembler::mode::IND, 0 },
		{ "clr", cpu16_disassembler::mode::IND, 0 },
		{ "tst", cpu16_disassembler::mode::IND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "bclr", cpu16_disassembler::mode::BIT, 0 },
		{ "bset", cpu16_disassembler::mode::BIT, 0 },
		{ "brclr", cpu16_disassembler::mode::BIT, STEP_COND },
		{ "brset", cpu16_disassembler::mode::BIT, STEP_COND },
		{ "rol", cpu16_disassembler::mode::IND, 0 },
		{ "asr", cpu16_disassembler::mode::IND, 0 },
		{ "ror", cpu16_disassembler::mode::IND, 0 },
		{ "lsr", cpu16_disassembler::mode::IND, 0 },

		// 1X
		{ "com", cpu16_disassembler::mode::IND, 0 },
		{ "dec", cpu16_disassembler::mode::IND, 0 },
		{ "neg", cpu16_disassembler::mode::IND, 0 },
		{ "inc", cpu16_disassembler::mode::IND, 0 },
		{ "asl", cpu16_disassembler::mode::IND, 0 },
		{ "clr", cpu16_disassembler::mode::IND, 0 },
		{ "tst", cpu16_disassembler::mode::IND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 }, // actually prebyte for page 1
		{ "bclr", cpu16_disassembler::mode::BIT, 0 },
		{ "bset", cpu16_disassembler::mode::BIT, 0 },
		{ "brclr", cpu16_disassembler::mode::BIT, STEP_COND },
		{ "brset", cpu16_disassembler::mode::BIT, STEP_COND },
		{ "rol", cpu16_disassembler::mode::IND, 0 },
		{ "asr", cpu16_disassembler::mode::IND, 0 },
		{ "ror", cpu16_disassembler::mode::IND, 0 },
		{ "lsr", cpu16_disassembler::mode::IND, 0 },

		// 2X
		{ "com", cpu16_disassembler::mode::IND, 0 },
		{ "dec", cpu16_disassembler::mode::IND, 0 },
		{ "neg", cpu16_disassembler::mode::IND, 0 },
		{ "inc", cpu16_disassembler::mode::IND, 0 },
		{ "asl", cpu16_disassembler::mode::IND, 0 },
		{ "clr", cpu16_disassembler::mode::IND, 0 },
		{ "tst", cpu16_disassembler::mode::IND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 }, // actually prebyte for page 2
		{ "bclr", cpu16_disassembler::mode::BIT, 0 },
		{ "bset", cpu16_disassembler::mode::BIT, 0 },
		{ "brclr", cpu16_disassembler::mode::BIT, STEP_COND },
		{ "brset", cpu16_disassembler::mode::BIT, STEP_COND },
		{ "rol", cpu16_disassembler::mode::IND, 0 },
		{ "asr", cpu16_disassembler::mode::IND, 0 },
		{ "ror", cpu16_disassembler::mode::IND, 0 },
		{ "lsr", cpu16_disassembler::mode::IND, 0 },

		// 3X
		{ "movb", cpu16_disassembler::mode::IXP, 0 },
		{ "movw", cpu16_disassembler::mode::IXP, 0 },
		{ "movb", cpu16_disassembler::mode::IXP, 0 },
		{ "movw", cpu16_disassembler::mode::IXP, 0 },
		{ "pshm", cpu16_disassembler::mode::REGM, 0 },
		{ "pulm", cpu16_disassembler::mode::REGM, 0 },
		{ "bsr", cpu16_disassembler::mode::REL, STEP_OVER },
		{ "", cpu16_disassembler::mode::UND, 0 }, // actually prebyte for page 3
		{ "bclr", cpu16_disassembler::mode::BIT, 0 },
		{ "bset", cpu16_disassembler::mode::BIT, 0 },
		{ "brclr", cpu16_disassembler::mode::BIT, STEP_COND },
		{ "brset", cpu16_disassembler::mode::BIT, STEP_COND },
		{ "aix", cpu16_disassembler::mode::IMMS, 0 },
		{ "aiy", cpu16_disassembler::mode::IMMS, 0 },
		{ "aiz", cpu16_disassembler::mode::IMMS, 0 },
		{ "ais", cpu16_disassembler::mode::IMMS, 0 },

		// 4X
		{ "suba", cpu16_disassembler::mode::IND, 0 },
		{ "adda", cpu16_disassembler::mode::IND, 0 },
		{ "sbca", cpu16_disassembler::mode::IND, 0 },
		{ "adca", cpu16_disassembler::mode::IND, 0 },
		{ "eora", cpu16_disassembler::mode::IND, 0 },
		{ "ldaa", cpu16_disassembler::mode::IND, 0 },
		{ "anda", cpu16_disassembler::mode::IND, 0 },
		{ "oraa", cpu16_disassembler::mode::IND, 0 },
		{ "cmpa", cpu16_disassembler::mode::IND, 0 },
		{ "bita", cpu16_disassembler::mode::IND, 0 },
		{ "staa", cpu16_disassembler::mode::IND, 0 },
		{ "jmp", cpu16_disassembler::mode::IND20, 0 },
		{ "cpx", cpu16_disassembler::mode::IND, 0 },
		{ "cpy", cpu16_disassembler::mode::IND, 0 },
		{ "cpz", cpu16_disassembler::mode::IND, 0 },
		{ "cps", cpu16_disassembler::mode::IND, 0 },

		// 5X
		{ "suba", cpu16_disassembler::mode::IND, 0 },
		{ "adda", cpu16_disassembler::mode::IND, 0 },
		{ "sbca", cpu16_disassembler::mode::IND, 0 },
		{ "adca", cpu16_disassembler::mode::IND, 0 },
		{ "eora", cpu16_disassembler::mode::IND, 0 },
		{ "ldaa", cpu16_disassembler::mode::IND, 0 },
		{ "anda", cpu16_disassembler::mode::IND, 0 },
		{ "oraa", cpu16_disassembler::mode::IND, 0 },
		{ "cmpa", cpu16_disassembler::mode::IND, 0 },
		{ "bita", cpu16_disassembler::mode::IND, 0 },
		{ "staa", cpu16_disassembler::mode::IND, 0 },
		{ "jmp", cpu16_disassembler::mode::IND20, 0 },
		{ "cpx", cpu16_disassembler::mode::IND, 0 },
		{ "cpy", cpu16_disassembler::mode::IND, 0 },
		{ "cpz", cpu16_disassembler::mode::IND, 0 },
		{ "cps", cpu16_disassembler::mode::IND, 0 },

		// 6X
		{ "suba", cpu16_disassembler::mode::IND, 0 },
		{ "adda", cpu16_disassembler::mode::IND, 0 },
		{ "sbca", cpu16_disassembler::mode::IND, 0 },
		{ "adca", cpu16_disassembler::mode::IND, 0 },
		{ "eora", cpu16_disassembler::mode::IND, 0 },
		{ "ldaa", cpu16_disassembler::mode::IND, 0 },
		{ "anda", cpu16_disassembler::mode::IND, 0 },
		{ "oraa", cpu16_disassembler::mode::IND, 0 },
		{ "cmpa", cpu16_disassembler::mode::IND, 0 },
		{ "bita", cpu16_disassembler::mode::IND, 0 },
		{ "staa", cpu16_disassembler::mode::IND, 0 },
		{ "jmp", cpu16_disassembler::mode::IND20, 0 },
		{ "cpx", cpu16_disassembler::mode::IND, 0 },
		{ "cpy", cpu16_disassembler::mode::IND, 0 },
		{ "cpz", cpu16_disassembler::mode::IND, 0 },
		{ "cps", cpu16_disassembler::mode::IND, 0 },

		// 7X
		{ "suba", cpu16_disassembler::mode::IMM, 0 },
		{ "adda", cpu16_disassembler::mode::IMM, 0 },
		{ "sbca", cpu16_disassembler::mode::IMM, 0 },
		{ "adca", cpu16_disassembler::mode::IMM, 0 },
		{ "eora", cpu16_disassembler::mode::IMM, 0 },
		{ "ldaa", cpu16_disassembler::mode::IMM, 0 },
		{ "anda", cpu16_disassembler::mode::IMM, 0 },
		{ "oraa", cpu16_disassembler::mode::IMM, 0 },
		{ "cmpa", cpu16_disassembler::mode::IMM, 0 },
		{ "bita", cpu16_disassembler::mode::IMM, 0 },
		{ "jmp", cpu16_disassembler::mode::EXT20, 0 },
		{ "mac", cpu16_disassembler::mode::XYO, 0 },
		{ "adde", cpu16_disassembler::mode::IMMS, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },

		// 8X
		{ "subd", cpu16_disassembler::mode::IND, 0 },
		{ "addd", cpu16_disassembler::mode::IND, 0 },
		{ "sbcd", cpu16_disassembler::mode::IND, 0 },
		{ "adcd", cpu16_disassembler::mode::IND, 0 },
		{ "eord", cpu16_disassembler::mode::IND, 0 },
		{ "ldd", cpu16_disassembler::mode::IND, 0 },
		{ "andd", cpu16_disassembler::mode::IND, 0 },
		{ "ord", cpu16_disassembler::mode::IND, 0 },
		{ "cpd", cpu16_disassembler::mode::IND, 0 },
		{ "jsr", cpu16_disassembler::mode::IND20, STEP_OVER },
		{ "std", cpu16_disassembler::mode::IND, 0 },
		{ "brset", cpu16_disassembler::mode::BIT, STEP_COND },
		{ "stx", cpu16_disassembler::mode::IND, 0 },
		{ "sty", cpu16_disassembler::mode::IND, 0 },
		{ "stz", cpu16_disassembler::mode::IND, 0 },
		{ "sts", cpu16_disassembler::mode::IND, 0 },

		// 9X
		{ "subd", cpu16_disassembler::mode::IND, 0 },
		{ "addd", cpu16_disassembler::mode::IND, 0 },
		{ "sbcd", cpu16_disassembler::mode::IND, 0 },
		{ "adcd", cpu16_disassembler::mode::IND, 0 },
		{ "eord", cpu16_disassembler::mode::IND, 0 },
		{ "ldd", cpu16_disassembler::mode::IND, 0 },
		{ "andd", cpu16_disassembler::mode::IND, 0 },
		{ "ord", cpu16_disassembler::mode::IND, 0 },
		{ "cpd", cpu16_disassembler::mode::IND, 0 },
		{ "jsr", cpu16_disassembler::mode::IND20, STEP_OVER },
		{ "std", cpu16_disassembler::mode::IND, 0 },
		{ "brset", cpu16_disassembler::mode::BIT, STEP_COND },
		{ "stx", cpu16_disassembler::mode::IND, 0 },
		{ "sty", cpu16_disassembler::mode::IND, 0 },
		{ "stz", cpu16_disassembler::mode::IND, 0 },
		{ "sts", cpu16_disassembler::mode::IND, 0 },

		// AX
		{ "subd", cpu16_disassembler::mode::IND, 0 },
		{ "addd", cpu16_disassembler::mode::IND, 0 },
		{ "sbcd", cpu16_disassembler::mode::IND, 0 },
		{ "adcd", cpu16_disassembler::mode::IND, 0 },
		{ "eord", cpu16_disassembler::mode::IND, 0 },
		{ "ldd", cpu16_disassembler::mode::IND, 0 },
		{ "andd", cpu16_disassembler::mode::IND, 0 },
		{ "ord", cpu16_disassembler::mode::IND, 0 },
		{ "cpd", cpu16_disassembler::mode::IND, 0 },
		{ "jsr", cpu16_disassembler::mode::IND20, STEP_OVER },
		{ "std", cpu16_disassembler::mode::IND, 0 },
		{ "brset", cpu16_disassembler::mode::BIT, STEP_COND },
		{ "stx", cpu16_disassembler::mode::IND, 0 },
		{ "sty", cpu16_disassembler::mode::IND, 0 },
		{ "stz", cpu16_disassembler::mode::IND, 0 },
		{ "sts", cpu16_disassembler::mode::IND, 0 },

		// BX
		{ "bra", cpu16_disassembler::mode::REL, 0 },
		{ "brn", cpu16_disassembler::mode::REL, 0 },
		{ "bhi", cpu16_disassembler::mode::REL, STEP_COND },
		{ "bls", cpu16_disassembler::mode::REL, STEP_COND },
		{ "bcc", cpu16_disassembler::mode::REL, STEP_COND },
		{ "bcs", cpu16_disassembler::mode::REL, STEP_COND },
		{ "bne", cpu16_disassembler::mode::REL, STEP_COND },
		{ "beq", cpu16_disassembler::mode::REL, STEP_COND },
		{ "bvc", cpu16_disassembler::mode::REL, STEP_COND },
		{ "bvs", cpu16_disassembler::mode::REL, STEP_COND },
		{ "bpl", cpu16_disassembler::mode::REL, STEP_COND },
		{ "bmi", cpu16_disassembler::mode::REL, STEP_COND },
		{ "bge", cpu16_disassembler::mode::REL, STEP_COND },
		{ "blt", cpu16_disassembler::mode::REL, STEP_COND },
		{ "bgt", cpu16_disassembler::mode::REL, STEP_COND },
		{ "ble", cpu16_disassembler::mode::REL, STEP_COND },

		// CX
		{ "subb", cpu16_disassembler::mode::IND, 0 },
		{ "addb", cpu16_disassembler::mode::IND, 0 },
		{ "sbcb", cpu16_disassembler::mode::IND, 0 },
		{ "adcb", cpu16_disassembler::mode::IND, 0 },
		{ "eorb", cpu16_disassembler::mode::IND, 0 },
		{ "ldab", cpu16_disassembler::mode::IND, 0 },
		{ "andb", cpu16_disassembler::mode::IND, 0 },
		{ "orab", cpu16_disassembler::mode::IND, 0 },
		{ "cmpb", cpu16_disassembler::mode::IND, 0 },
		{ "bitb", cpu16_disassembler::mode::IND, 0 },
		{ "stab", cpu16_disassembler::mode::IND, 0 },
		{ "brclr", cpu16_disassembler::mode::BIT, STEP_COND },
		{ "ldx", cpu16_disassembler::mode::IND, 0 },
		{ "ldy", cpu16_disassembler::mode::IND, 0 },
		{ "ldz", cpu16_disassembler::mode::IND, 0 },
		{ "lds", cpu16_disassembler::mode::IND, 0 },

		// DX
		{ "subb", cpu16_disassembler::mode::IND, 0 },
		{ "addb", cpu16_disassembler::mode::IND, 0 },
		{ "sbcb", cpu16_disassembler::mode::IND, 0 },
		{ "adcb", cpu16_disassembler::mode::IND, 0 },
		{ "eorb", cpu16_disassembler::mode::IND, 0 },
		{ "ldab", cpu16_disassembler::mode::IND, 0 },
		{ "andb", cpu16_disassembler::mode::IND, 0 },
		{ "orab", cpu16_disassembler::mode::IND, 0 },
		{ "cmpb", cpu16_disassembler::mode::IND, 0 },
		{ "bitb", cpu16_disassembler::mode::IND, 0 },
		{ "stab", cpu16_disassembler::mode::IND, 0 },
		{ "brclr", cpu16_disassembler::mode::BIT, STEP_COND },
		{ "ldx", cpu16_disassembler::mode::IND, 0 },
		{ "ldy", cpu16_disassembler::mode::IND, 0 },
		{ "ldz", cpu16_disassembler::mode::IND, 0 },
		{ "lds", cpu16_disassembler::mode::IND, 0 },

		// EX
		{ "subb", cpu16_disassembler::mode::IND, 0 },
		{ "addb", cpu16_disassembler::mode::IND, 0 },
		{ "sbcb", cpu16_disassembler::mode::IND, 0 },
		{ "adcb", cpu16_disassembler::mode::IND, 0 },
		{ "eorb", cpu16_disassembler::mode::IND, 0 },
		{ "ldab", cpu16_disassembler::mode::IND, 0 },
		{ "andb", cpu16_disassembler::mode::IND, 0 },
		{ "orab", cpu16_disassembler::mode::IND, 0 },
		{ "cmpb", cpu16_disassembler::mode::IND, 0 },
		{ "bitb", cpu16_disassembler::mode::IND, 0 },
		{ "stab", cpu16_disassembler::mode::IND, 0 },
		{ "brclr", cpu16_disassembler::mode::BIT, STEP_COND },
		{ "ldx", cpu16_disassembler::mode::IND, 0 },
		{ "ldy", cpu16_disassembler::mode::IND, 0 },
		{ "ldz", cpu16_disassembler::mode::IND, 0 },
		{ "lds", cpu16_disassembler::mode::IND, 0 },

		// FX
		{ "subb", cpu16_disassembler::mode::IMM, 0 },
		{ "addb", cpu16_disassembler::mode::IMM, 0 },
		{ "sbcb", cpu16_disassembler::mode::IMM, 0 },
		{ "adcb", cpu16_disassembler::mode::IMM, 0 },
		{ "eorb", cpu16_disassembler::mode::IMM, 0 },
		{ "ldab", cpu16_disassembler::mode::IMM, 0 },
		{ "andb", cpu16_disassembler::mode::IMM, 0 },
		{ "orab", cpu16_disassembler::mode::IMM, 0 },
		{ "cmpb", cpu16_disassembler::mode::IMM, 0 },
		{ "bitb", cpu16_disassembler::mode::IMM, 0 },
		{ "jsr", cpu16_disassembler::mode::EXT20, STEP_OVER },
		{ "rmac", cpu16_disassembler::mode::XYO, 0 },
		{ "addd", cpu16_disassembler::mode::IMMS, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 }
	},
	{
		// 170X
		{ "com", cpu16_disassembler::mode::IND, 0 },
		{ "dec", cpu16_disassembler::mode::IND, 0 },
		{ "neg", cpu16_disassembler::mode::IND, 0 },
		{ "inc", cpu16_disassembler::mode::IND, 0 },
		{ "asl", cpu16_disassembler::mode::IND, 0 },
		{ "clr", cpu16_disassembler::mode::IND, 0 },
		{ "tst", cpu16_disassembler::mode::IND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "bclr", cpu16_disassembler::mode::BIT, 0 },
		{ "bset", cpu16_disassembler::mode::BIT, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "rol", cpu16_disassembler::mode::IND, 0 },
		{ "asr", cpu16_disassembler::mode::IND, 0 },
		{ "ror", cpu16_disassembler::mode::IND, 0 },
		{ "lsr", cpu16_disassembler::mode::IND, 0 },

		// 171X
		{ "com", cpu16_disassembler::mode::IND, 0 },
		{ "dec", cpu16_disassembler::mode::IND, 0 },
		{ "neg", cpu16_disassembler::mode::IND, 0 },
		{ "inc", cpu16_disassembler::mode::IND, 0 },
		{ "asl", cpu16_disassembler::mode::IND, 0 },
		{ "clr", cpu16_disassembler::mode::IND, 0 },
		{ "tst", cpu16_disassembler::mode::IND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "bclr", cpu16_disassembler::mode::BIT, 0 },
		{ "bset", cpu16_disassembler::mode::BIT, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "rol", cpu16_disassembler::mode::IND, 0 },
		{ "asr", cpu16_disassembler::mode::IND, 0 },
		{ "ror", cpu16_disassembler::mode::IND, 0 },
		{ "lsr", cpu16_disassembler::mode::IND, 0 },

		// 172X
		{ "com", cpu16_disassembler::mode::IND, 0 },
		{ "dec", cpu16_disassembler::mode::IND, 0 },
		{ "neg", cpu16_disassembler::mode::IND, 0 },
		{ "inc", cpu16_disassembler::mode::IND, 0 },
		{ "asl", cpu16_disassembler::mode::IND, 0 },
		{ "clr", cpu16_disassembler::mode::IND, 0 },
		{ "tst", cpu16_disassembler::mode::IND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "bclr", cpu16_disassembler::mode::IND, 0 },
		{ "bset", cpu16_disassembler::mode::IND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "rol", cpu16_disassembler::mode::IND, 0 },
		{ "asr", cpu16_disassembler::mode::IND, 0 },
		{ "ror", cpu16_disassembler::mode::IND, 0 },
		{ "lsr", cpu16_disassembler::mode::IND, 0 },

		// 173X
		{ "com", cpu16_disassembler::mode::EXT, 0 },
		{ "dec", cpu16_disassembler::mode::EXT, 0 },
		{ "neg", cpu16_disassembler::mode::EXT, 0 },
		{ "inc", cpu16_disassembler::mode::EXT, 0 },
		{ "asl", cpu16_disassembler::mode::EXT, 0 },
		{ "clr", cpu16_disassembler::mode::EXT, 0 },
		{ "tst", cpu16_disassembler::mode::EXT, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "rol", cpu16_disassembler::mode::EXT, 0 },
		{ "asr", cpu16_disassembler::mode::EXT, 0 },
		{ "ror", cpu16_disassembler::mode::EXT, 0 },
		{ "lsr", cpu16_disassembler::mode::EXT, 0 },

		// 174X
		{ "suba", cpu16_disassembler::mode::IND, 0 },
		{ "adda", cpu16_disassembler::mode::IND, 0 },
		{ "sbca", cpu16_disassembler::mode::IND, 0 },
		{ "adca", cpu16_disassembler::mode::IND, 0 },
		{ "eora", cpu16_disassembler::mode::IND, 0 },
		{ "ldaa", cpu16_disassembler::mode::IND, 0 },
		{ "anda", cpu16_disassembler::mode::IND, 0 },
		{ "ora", cpu16_disassembler::mode::IND, 0 },
		{ "cmpa", cpu16_disassembler::mode::IND, 0 },
		{ "bita", cpu16_disassembler::mode::IND, 0 },
		{ "staa", cpu16_disassembler::mode::IND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "cpx", cpu16_disassembler::mode::IND, 0 },
		{ "cpy", cpu16_disassembler::mode::IND, 0 },
		{ "cpz", cpu16_disassembler::mode::IND, 0 },
		{ "cps", cpu16_disassembler::mode::IND, 0 },

		// 175X
		{ "suba", cpu16_disassembler::mode::IND, 0 },
		{ "adda", cpu16_disassembler::mode::IND, 0 },
		{ "sbca", cpu16_disassembler::mode::IND, 0 },
		{ "adca", cpu16_disassembler::mode::IND, 0 },
		{ "eora", cpu16_disassembler::mode::IND, 0 },
		{ "ldaa", cpu16_disassembler::mode::IND, 0 },
		{ "anda", cpu16_disassembler::mode::IND, 0 },
		{ "ora", cpu16_disassembler::mode::IND, 0 },
		{ "cmpa", cpu16_disassembler::mode::IND, 0 },
		{ "bita", cpu16_disassembler::mode::IND, 0 },
		{ "staa", cpu16_disassembler::mode::IND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "cpx", cpu16_disassembler::mode::IND, 0 },
		{ "cpy", cpu16_disassembler::mode::IND, 0 },
		{ "cpz", cpu16_disassembler::mode::IND, 0 },
		{ "cps", cpu16_disassembler::mode::IND, 0 },

		// 176X
		{ "suba", cpu16_disassembler::mode::IND, 0 },
		{ "adda", cpu16_disassembler::mode::IND, 0 },
		{ "sbca", cpu16_disassembler::mode::IND, 0 },
		{ "adca", cpu16_disassembler::mode::IND, 0 },
		{ "eora", cpu16_disassembler::mode::IND, 0 },
		{ "ldaa", cpu16_disassembler::mode::IND, 0 },
		{ "anda", cpu16_disassembler::mode::IND, 0 },
		{ "ora", cpu16_disassembler::mode::IND, 0 },
		{ "cmpa", cpu16_disassembler::mode::IND, 0 },
		{ "bita", cpu16_disassembler::mode::IND, 0 },
		{ "staa", cpu16_disassembler::mode::IND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "cpx", cpu16_disassembler::mode::IND, 0 },
		{ "cpy", cpu16_disassembler::mode::IND, 0 },
		{ "cpz", cpu16_disassembler::mode::IND, 0 },
		{ "cps", cpu16_disassembler::mode::IND, 0 },

		// 177X
		{ "suba", cpu16_disassembler::mode::EXT, 0 },
		{ "adda", cpu16_disassembler::mode::EXT, 0 },
		{ "sbca", cpu16_disassembler::mode::EXT, 0 },
		{ "adca", cpu16_disassembler::mode::EXT, 0 },
		{ "eora", cpu16_disassembler::mode::EXT, 0 },
		{ "ldaa", cpu16_disassembler::mode::EXT, 0 },
		{ "anda", cpu16_disassembler::mode::EXT, 0 },
		{ "ora", cpu16_disassembler::mode::EXT, 0 },
		{ "cmpa", cpu16_disassembler::mode::EXT, 0 },
		{ "bita", cpu16_disassembler::mode::EXT, 0 },
		{ "staa", cpu16_disassembler::mode::EXT, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "cpx", cpu16_disassembler::mode::EXT, 0 },
		{ "cpy", cpu16_disassembler::mode::EXT, 0 },
		{ "cpz", cpu16_disassembler::mode::EXT, 0 },
		{ "cps", cpu16_disassembler::mode::EXT, 0 },

		// 178X
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "stx", cpu16_disassembler::mode::IND, 0 },
		{ "sty", cpu16_disassembler::mode::IND, 0 },
		{ "stz", cpu16_disassembler::mode::IND, 0 },
		{ "sts", cpu16_disassembler::mode::IND, 0 },

		// 179X
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "stx", cpu16_disassembler::mode::IND, 0 },
		{ "sty", cpu16_disassembler::mode::IND, 0 },
		{ "stz", cpu16_disassembler::mode::IND, 0 },
		{ "sts", cpu16_disassembler::mode::IND, 0 },

		// 17AX
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "stx", cpu16_disassembler::mode::IND, 0 },
		{ "sty", cpu16_disassembler::mode::IND, 0 },
		{ "stz", cpu16_disassembler::mode::IND, 0 },
		{ "sts", cpu16_disassembler::mode::IND, 0 },

		// 17BX
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "stx", cpu16_disassembler::mode::EXT, 0 },
		{ "sty", cpu16_disassembler::mode::EXT, 0 },
		{ "stz", cpu16_disassembler::mode::EXT, 0 },
		{ "sts", cpu16_disassembler::mode::EXT, 0 },

		// 17CX
		{ "subb", cpu16_disassembler::mode::IND, 0 },
		{ "addb", cpu16_disassembler::mode::IND, 0 },
		{ "sbcb", cpu16_disassembler::mode::IND, 0 },
		{ "adcb", cpu16_disassembler::mode::IND, 0 },
		{ "eorb", cpu16_disassembler::mode::IND, 0 },
		{ "ldab", cpu16_disassembler::mode::IND, 0 },
		{ "andb", cpu16_disassembler::mode::IND, 0 },
		{ "orab", cpu16_disassembler::mode::IND, 0 },
		{ "cmpb", cpu16_disassembler::mode::IND, 0 },
		{ "bitb", cpu16_disassembler::mode::IND, 0 },
		{ "stab", cpu16_disassembler::mode::IND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "ldx", cpu16_disassembler::mode::IND, 0 },
		{ "ldy", cpu16_disassembler::mode::IND, 0 },
		{ "ldz", cpu16_disassembler::mode::IND, 0 },
		{ "lds", cpu16_disassembler::mode::IND, 0 },

		// 17DX
		{ "subb", cpu16_disassembler::mode::IND, 0 },
		{ "addb", cpu16_disassembler::mode::IND, 0 },
		{ "sbcb", cpu16_disassembler::mode::IND, 0 },
		{ "adcb", cpu16_disassembler::mode::IND, 0 },
		{ "eorb", cpu16_disassembler::mode::IND, 0 },
		{ "ldab", cpu16_disassembler::mode::IND, 0 },
		{ "andb", cpu16_disassembler::mode::IND, 0 },
		{ "orab", cpu16_disassembler::mode::IND, 0 },
		{ "cmpb", cpu16_disassembler::mode::IND, 0 },
		{ "bitb", cpu16_disassembler::mode::IND, 0 },
		{ "stab", cpu16_disassembler::mode::IND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "ldx", cpu16_disassembler::mode::IND, 0 },
		{ "ldy", cpu16_disassembler::mode::IND, 0 },
		{ "ldz", cpu16_disassembler::mode::IND, 0 },
		{ "lds", cpu16_disassembler::mode::IND, 0 },

		// 17EX
		{ "subb", cpu16_disassembler::mode::IND, 0 },
		{ "addb", cpu16_disassembler::mode::IND, 0 },
		{ "sbcb", cpu16_disassembler::mode::IND, 0 },
		{ "adcb", cpu16_disassembler::mode::IND, 0 },
		{ "eorb", cpu16_disassembler::mode::IND, 0 },
		{ "ldab", cpu16_disassembler::mode::IND, 0 },
		{ "andb", cpu16_disassembler::mode::IND, 0 },
		{ "orab", cpu16_disassembler::mode::IND, 0 },
		{ "cmpb", cpu16_disassembler::mode::IND, 0 },
		{ "bitb", cpu16_disassembler::mode::IND, 0 },
		{ "stab", cpu16_disassembler::mode::IND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "ldx", cpu16_disassembler::mode::IND, 0 },
		{ "ldy", cpu16_disassembler::mode::IND, 0 },
		{ "ldz", cpu16_disassembler::mode::IND, 0 },
		{ "lds", cpu16_disassembler::mode::IND, 0 },

		// 17FX
		{ "subb", cpu16_disassembler::mode::EXT, 0 },
		{ "addb", cpu16_disassembler::mode::EXT, 0 },
		{ "sbcb", cpu16_disassembler::mode::EXT, 0 },
		{ "adcb", cpu16_disassembler::mode::EXT, 0 },
		{ "eorb", cpu16_disassembler::mode::EXT, 0 },
		{ "ldab", cpu16_disassembler::mode::EXT, 0 },
		{ "andb", cpu16_disassembler::mode::EXT, 0 },
		{ "orab", cpu16_disassembler::mode::EXT, 0 },
		{ "cmpb", cpu16_disassembler::mode::EXT, 0 },
		{ "bitb", cpu16_disassembler::mode::EXT, 0 },
		{ "stab", cpu16_disassembler::mode::EXT, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "ldx", cpu16_disassembler::mode::EXT, 0 },
		{ "ldy", cpu16_disassembler::mode::EXT, 0 },
		{ "ldz", cpu16_disassembler::mode::EXT, 0 },
		{ "lds", cpu16_disassembler::mode::EXT, 0 }
	},
	{
		// 270X
		{ "comw", cpu16_disassembler::mode::IND, 0 },
		{ "decw", cpu16_disassembler::mode::IND, 0 },
		{ "negw", cpu16_disassembler::mode::IND, 0 },
		{ "incw", cpu16_disassembler::mode::IND, 0 },
		{ "aslw", cpu16_disassembler::mode::IND, 0 },
		{ "clrw", cpu16_disassembler::mode::IND, 0 },
		{ "tstw", cpu16_disassembler::mode::IND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "bclrw", cpu16_disassembler::mode::BIT16, 0 },
		{ "bsetw", cpu16_disassembler::mode::BIT16, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "rolw", cpu16_disassembler::mode::IND, 0 },
		{ "asrw", cpu16_disassembler::mode::IND, 0 },
		{ "rorw", cpu16_disassembler::mode::IND, 0 },
		{ "lsrw", cpu16_disassembler::mode::IND, 0 },

		// 271X
		{ "comw", cpu16_disassembler::mode::IND, 0 },
		{ "decw", cpu16_disassembler::mode::IND, 0 },
		{ "negw", cpu16_disassembler::mode::IND, 0 },
		{ "incw", cpu16_disassembler::mode::IND, 0 },
		{ "aslw", cpu16_disassembler::mode::IND, 0 },
		{ "clrw", cpu16_disassembler::mode::IND, 0 },
		{ "tstw", cpu16_disassembler::mode::IND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "bclrw", cpu16_disassembler::mode::BIT16, 0 },
		{ "bsetw", cpu16_disassembler::mode::BIT16, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "rolw", cpu16_disassembler::mode::IND, 0 },
		{ "asrw", cpu16_disassembler::mode::IND, 0 },
		{ "rorw", cpu16_disassembler::mode::IND, 0 },
		{ "lsrw", cpu16_disassembler::mode::IND, 0 },

		// 272X
		{ "comw", cpu16_disassembler::mode::IND, 0 },
		{ "decw", cpu16_disassembler::mode::IND, 0 },
		{ "negw", cpu16_disassembler::mode::IND, 0 },
		{ "incw", cpu16_disassembler::mode::IND, 0 },
		{ "aslw", cpu16_disassembler::mode::IND, 0 },
		{ "clrw", cpu16_disassembler::mode::IND, 0 },
		{ "tstw", cpu16_disassembler::mode::IND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "bclrw", cpu16_disassembler::mode::BIT16, 0 },
		{ "bsetw", cpu16_disassembler::mode::BIT16, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "rolw", cpu16_disassembler::mode::IND, 0 },
		{ "asrw", cpu16_disassembler::mode::IND, 0 },
		{ "rorw", cpu16_disassembler::mode::IND, 0 },
		{ "lsrw", cpu16_disassembler::mode::IND, 0 },

		// 273X
		{ "comw", cpu16_disassembler::mode::EXT, 0 },
		{ "decw", cpu16_disassembler::mode::EXT, 0 },
		{ "negw", cpu16_disassembler::mode::EXT, 0 },
		{ "incw", cpu16_disassembler::mode::EXT, 0 },
		{ "aslw", cpu16_disassembler::mode::EXT, 0 },
		{ "clrw", cpu16_disassembler::mode::EXT, 0 },
		{ "tstw", cpu16_disassembler::mode::EXT, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "bclrw", cpu16_disassembler::mode::BIT16, 0 },
		{ "bsetw", cpu16_disassembler::mode::BIT16, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "rolw", cpu16_disassembler::mode::EXT, 0 },
		{ "asrw", cpu16_disassembler::mode::EXT, 0 },
		{ "rorw", cpu16_disassembler::mode::EXT, 0 },
		{ "lsrw", cpu16_disassembler::mode::EXT, 0 },

		// 274X
		{ "suba", cpu16_disassembler::mode::E, 0 },
		{ "adda", cpu16_disassembler::mode::E, 0 },
		{ "sbca", cpu16_disassembler::mode::E, 0 },
		{ "adca", cpu16_disassembler::mode::E, 0 },
		{ "eora", cpu16_disassembler::mode::E, 0 },
		{ "ldaa", cpu16_disassembler::mode::E, 0 },
		{ "anda", cpu16_disassembler::mode::E, 0 },
		{ "oraa", cpu16_disassembler::mode::E, 0 },
		{ "cmpa", cpu16_disassembler::mode::E, 0 },
		{ "bita", cpu16_disassembler::mode::E, 0 },
		{ "staa", cpu16_disassembler::mode::E, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "nop", cpu16_disassembler::mode::INH, 0 },
		{ "tyx", cpu16_disassembler::mode::INH, 0 },
		{ "tzx", cpu16_disassembler::mode::INH, 0 },
		{ "tsx", cpu16_disassembler::mode::INH, 0 },

		// 275X
		{ "suba", cpu16_disassembler::mode::E, 0 },
		{ "adda", cpu16_disassembler::mode::E, 0 },
		{ "sbca", cpu16_disassembler::mode::E, 0 },
		{ "adca", cpu16_disassembler::mode::E, 0 },
		{ "eora", cpu16_disassembler::mode::E, 0 },
		{ "ldaa", cpu16_disassembler::mode::E, 0 },
		{ "anda", cpu16_disassembler::mode::E, 0 },
		{ "oraa", cpu16_disassembler::mode::E, 0 },
		{ "cmpa", cpu16_disassembler::mode::E, 0 },
		{ "bita", cpu16_disassembler::mode::E, 0 },
		{ "staa", cpu16_disassembler::mode::E, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "txy", cpu16_disassembler::mode::INH, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "tzy", cpu16_disassembler::mode::INH, 0 },
		{ "tsy", cpu16_disassembler::mode::INH, 0 },

		// 275X
		{ "suba", cpu16_disassembler::mode::E, 0 },
		{ "adda", cpu16_disassembler::mode::E, 0 },
		{ "sbca", cpu16_disassembler::mode::E, 0 },
		{ "adca", cpu16_disassembler::mode::E, 0 },
		{ "eora", cpu16_disassembler::mode::E, 0 },
		{ "ldaa", cpu16_disassembler::mode::E, 0 },
		{ "anda", cpu16_disassembler::mode::E, 0 },
		{ "oraa", cpu16_disassembler::mode::E, 0 },
		{ "cmpa", cpu16_disassembler::mode::E, 0 },
		{ "bita", cpu16_disassembler::mode::E, 0 },
		{ "staa", cpu16_disassembler::mode::E, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "txz", cpu16_disassembler::mode::INH, 0 },
		{ "tyz", cpu16_disassembler::mode::INH, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "tsz", cpu16_disassembler::mode::INH, 0 },

		// 277X
		{ "come", cpu16_disassembler::mode::INH, 0 },
		{ "lded", cpu16_disassembler::mode::EXT, 0 },
		{ "nege", cpu16_disassembler::mode::INH, 0 },
		{ "sted", cpu16_disassembler::mode::EXT, 0 },
		{ "asle", cpu16_disassembler::mode::INH, 0 },
		{ "clre", cpu16_disassembler::mode::INH, 0 },
		{ "tste", cpu16_disassembler::mode::INH, 0 },
		{ "rti", cpu16_disassembler::mode::INH, STEP_OUT },
		{ "ade", cpu16_disassembler::mode::INH, 0 },
		{ "sde", cpu16_disassembler::mode::INH, 0 },
		{ "xgde", cpu16_disassembler::mode::INH, 0 },
		{ "tde", cpu16_disassembler::mode::INH, 0 },
		{ "role", cpu16_disassembler::mode::INH, 0 },
		{ "asre", cpu16_disassembler::mode::INH, 0 },
		{ "rore", cpu16_disassembler::mode::INH, 0 },
		{ "lsre", cpu16_disassembler::mode::INH, 0 },

		// 278X
		{ "subd", cpu16_disassembler::mode::E, 0 },
		{ "addd", cpu16_disassembler::mode::E, 0 },
		{ "sbcd", cpu16_disassembler::mode::E, 0 },
		{ "adcd", cpu16_disassembler::mode::E, 0 },
		{ "eord", cpu16_disassembler::mode::E, 0 },
		{ "ldd", cpu16_disassembler::mode::E, 0 },
		{ "andd", cpu16_disassembler::mode::E, 0 },
		{ "ord", cpu16_disassembler::mode::E, 0 },
		{ "cpd", cpu16_disassembler::mode::E, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "std", cpu16_disassembler::mode::E, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },

		// 279X
		{ "subd", cpu16_disassembler::mode::E, 0 },
		{ "addd", cpu16_disassembler::mode::E, 0 },
		{ "sbcd", cpu16_disassembler::mode::E, 0 },
		{ "adcd", cpu16_disassembler::mode::E, 0 },
		{ "eord", cpu16_disassembler::mode::E, 0 },
		{ "ldd", cpu16_disassembler::mode::E, 0 },
		{ "andd", cpu16_disassembler::mode::E, 0 },
		{ "ord", cpu16_disassembler::mode::E, 0 },
		{ "cpd", cpu16_disassembler::mode::E, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "std", cpu16_disassembler::mode::E, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },

		// 27AX
		{ "subd", cpu16_disassembler::mode::E, 0 },
		{ "addd", cpu16_disassembler::mode::E, 0 },
		{ "sbcd", cpu16_disassembler::mode::E, 0 },
		{ "adcd", cpu16_disassembler::mode::E, 0 },
		{ "eord", cpu16_disassembler::mode::E, 0 },
		{ "ldd", cpu16_disassembler::mode::E, 0 },
		{ "andd", cpu16_disassembler::mode::E, 0 },
		{ "ord", cpu16_disassembler::mode::E, 0 },
		{ "cpd", cpu16_disassembler::mode::E, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "std", cpu16_disassembler::mode::E, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },

		// 27BX
		{ "ldhi", cpu16_disassembler::mode::EXT, 0 },
		{ "tedm", cpu16_disassembler::mode::INH, 0 },
		{ "tem", cpu16_disassembler::mode::INH, 0 },
		{ "tmexd", cpu16_disassembler::mode::INH, 0 },
		{ "tmer", cpu16_disassembler::mode::INH, 0 },
		{ "tmet", cpu16_disassembler::mode::INH, 0 },
		{ "aslm", cpu16_disassembler::mode::INH, 0 },
		{ "clrm", cpu16_disassembler::mode::INH, 0 },
		{ "pshmac", cpu16_disassembler::mode::INH, 0 },
		{ "pulmac", cpu16_disassembler::mode::INH, 0 },
		{ "asrm", cpu16_disassembler::mode::INH, 0 },
		{ "tekb", cpu16_disassembler::mode::INH, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },

		// 27CX
		{ "subb", cpu16_disassembler::mode::E, 0 },
		{ "addb", cpu16_disassembler::mode::E, 0 },
		{ "sbcb", cpu16_disassembler::mode::E, 0 },
		{ "adcb", cpu16_disassembler::mode::E, 0 },
		{ "eorb", cpu16_disassembler::mode::E, 0 },
		{ "ldab", cpu16_disassembler::mode::E, 0 },
		{ "andb", cpu16_disassembler::mode::E, 0 },
		{ "orab", cpu16_disassembler::mode::E, 0 },
		{ "cmpb", cpu16_disassembler::mode::E, 0 },
		{ "bitb", cpu16_disassembler::mode::E, 0 },
		{ "stab", cpu16_disassembler::mode::E, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },

		// 27DX
		{ "subb", cpu16_disassembler::mode::E, 0 },
		{ "addb", cpu16_disassembler::mode::E, 0 },
		{ "sbcb", cpu16_disassembler::mode::E, 0 },
		{ "adcb", cpu16_disassembler::mode::E, 0 },
		{ "eorb", cpu16_disassembler::mode::E, 0 },
		{ "ldab", cpu16_disassembler::mode::E, 0 },
		{ "andb", cpu16_disassembler::mode::E, 0 },
		{ "orab", cpu16_disassembler::mode::E, 0 },
		{ "cmpb", cpu16_disassembler::mode::E, 0 },
		{ "bitb", cpu16_disassembler::mode::E, 0 },
		{ "stab", cpu16_disassembler::mode::E, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },

		// 27EX
		{ "subb", cpu16_disassembler::mode::E, 0 },
		{ "addb", cpu16_disassembler::mode::E, 0 },
		{ "sbcb", cpu16_disassembler::mode::E, 0 },
		{ "adcb", cpu16_disassembler::mode::E, 0 },
		{ "eorb", cpu16_disassembler::mode::E, 0 },
		{ "ldab", cpu16_disassembler::mode::E, 0 },
		{ "andb", cpu16_disassembler::mode::E, 0 },
		{ "orab", cpu16_disassembler::mode::E, 0 },
		{ "cmpb", cpu16_disassembler::mode::E, 0 },
		{ "bitb", cpu16_disassembler::mode::E, 0 },
		{ "stab", cpu16_disassembler::mode::E, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },

		// 27FX
		{ "comd", cpu16_disassembler::mode::INH, 0 },
		{ "lpstop", cpu16_disassembler::mode::INH, 0 },
		{ "negd", cpu16_disassembler::mode::INH, 0 },
		{ "wai", cpu16_disassembler::mode::INH, 0 },
		{ "asld", cpu16_disassembler::mode::INH, 0 },
		{ "clrd", cpu16_disassembler::mode::INH, 0 },
		{ "tstd", cpu16_disassembler::mode::INH, 0 },
		{ "rts", cpu16_disassembler::mode::INH, STEP_OUT },
		{ "sxt", cpu16_disassembler::mode::INH, 0 },
		{ "lbsr", cpu16_disassembler::mode::REL, STEP_OVER },
		{ "tbek", cpu16_disassembler::mode::INH, 0 },
		{ "ted", cpu16_disassembler::mode::INH, 0 },
		{ "rold", cpu16_disassembler::mode::INH, 0 },
		{ "asrd", cpu16_disassembler::mode::INH, 0 },
		{ "rord", cpu16_disassembler::mode::INH, 0 },
		{ "lsrd", cpu16_disassembler::mode::INH, 0 }
	},
	{
		// 370X
		{ "coma", cpu16_disassembler::mode::INH, 0 },
		{ "deca", cpu16_disassembler::mode::INH, 0 },
		{ "nega", cpu16_disassembler::mode::INH, 0 },
		{ "inca", cpu16_disassembler::mode::INH, 0 },
		{ "asla", cpu16_disassembler::mode::INH, 0 },
		{ "clra", cpu16_disassembler::mode::INH, 0 },
		{ "tsta", cpu16_disassembler::mode::INH, 0 },
		{ "tba", cpu16_disassembler::mode::INH, 0 },
		{ "psha", cpu16_disassembler::mode::INH, 0 },
		{ "pula", cpu16_disassembler::mode::INH, 0 },
		{ "sba", cpu16_disassembler::mode::INH, 0 },
		{ "aba", cpu16_disassembler::mode::INH, 0 },
		{ "rola", cpu16_disassembler::mode::INH, 0 },
		{ "asra", cpu16_disassembler::mode::INH, 0 },
		{ "rora", cpu16_disassembler::mode::INH, 0 },
		{ "lsra", cpu16_disassembler::mode::INH, 0 },

		// 371X
		{ "comb", cpu16_disassembler::mode::INH, 0 },
		{ "decb", cpu16_disassembler::mode::INH, 0 },
		{ "negb", cpu16_disassembler::mode::INH, 0 },
		{ "incb", cpu16_disassembler::mode::INH, 0 },
		{ "aslb", cpu16_disassembler::mode::INH, 0 },
		{ "clrb", cpu16_disassembler::mode::INH, 0 },
		{ "tstb", cpu16_disassembler::mode::INH, 0 },
		{ "tab", cpu16_disassembler::mode::INH, 0 },
		{ "pshb", cpu16_disassembler::mode::INH, 0 },
		{ "pulb", cpu16_disassembler::mode::INH, 0 },
		{ "xgab", cpu16_disassembler::mode::INH, 0 },
		{ "cba", cpu16_disassembler::mode::INH, 0 },
		{ "rolb", cpu16_disassembler::mode::INH, 0 },
		{ "asrb", cpu16_disassembler::mode::INH, 0 },
		{ "rorb", cpu16_disassembler::mode::INH, 0 },
		{ "lsrb", cpu16_disassembler::mode::INH, 0 },

		// 372X
		{ "swi", cpu16_disassembler::mode::INH, STEP_OVER },
		{ "daa", cpu16_disassembler::mode::INH, 0 },
		{ "ace", cpu16_disassembler::mode::INH, 0 },
		{ "aced", cpu16_disassembler::mode::INH, 0 },
		{ "mul", cpu16_disassembler::mode::INH, 0 },
		{ "emul", cpu16_disassembler::mode::INH, 0 },
		{ "emuls", cpu16_disassembler::mode::INH, 0 },
		{ "fmuls", cpu16_disassembler::mode::INH, 0 },
		{ "ediv", cpu16_disassembler::mode::INH, 0 },
		{ "edivs", cpu16_disassembler::mode::INH, 0 },
		{ "idiv", cpu16_disassembler::mode::INH, 0 },
		{ "fdiv", cpu16_disassembler::mode::INH, 0 },
		{ "tpd", cpu16_disassembler::mode::INH, 0 },
		{ "tdp", cpu16_disassembler::mode::INH, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "tdmsk", cpu16_disassembler::mode::INH, 0 },

		// 373X
		{ "sube", cpu16_disassembler::mode::IMM, 0 },
		{ "adde", cpu16_disassembler::mode::IMM, 0 },
		{ "sbcd", cpu16_disassembler::mode::IMM, 0 },
		{ "adce", cpu16_disassembler::mode::IMM, 0 },
		{ "eore", cpu16_disassembler::mode::IMM, 0 },
		{ "lde", cpu16_disassembler::mode::IMM, 0 },
		{ "ande", cpu16_disassembler::mode::IMM, 0 },
		{ "ore", cpu16_disassembler::mode::IMM, 0 },
		{ "cpe", cpu16_disassembler::mode::IMM, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "andp", cpu16_disassembler::mode::IMM, 0 },
		{ "orp", cpu16_disassembler::mode::IMM, 0 },
		{ "aix", cpu16_disassembler::mode::IMMS, 0 },
		{ "aiy", cpu16_disassembler::mode::IMMS, 0 },
		{ "aiz", cpu16_disassembler::mode::IMMS, 0 },
		{ "ais", cpu16_disassembler::mode::IMMS, 0 },

		// 374X
		{ "sube", cpu16_disassembler::mode::IND, 0 },
		{ "adde", cpu16_disassembler::mode::IND, 0 },
		{ "sbce", cpu16_disassembler::mode::IND, 0 },
		{ "adce", cpu16_disassembler::mode::IND, 0 },
		{ "eore", cpu16_disassembler::mode::IND, 0 },
		{ "lde", cpu16_disassembler::mode::IND, 0 },
		{ "ande", cpu16_disassembler::mode::IND, 0 },
		{ "ore", cpu16_disassembler::mode::IND, 0 },
		{ "cpe", cpu16_disassembler::mode::IND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "ste", cpu16_disassembler::mode::IND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "xgex", cpu16_disassembler::mode::INH, 0 },
		{ "aex", cpu16_disassembler::mode::INH, 0 },
		{ "txs", cpu16_disassembler::mode::INH, 0 },
		{ "abx", cpu16_disassembler::mode::INH, 0 },

		// 375X
		{ "sube", cpu16_disassembler::mode::IND, 0 },
		{ "adde", cpu16_disassembler::mode::IND, 0 },
		{ "sbce", cpu16_disassembler::mode::IND, 0 },
		{ "adce", cpu16_disassembler::mode::IND, 0 },
		{ "eore", cpu16_disassembler::mode::IND, 0 },
		{ "lde", cpu16_disassembler::mode::IND, 0 },
		{ "ande", cpu16_disassembler::mode::IND, 0 },
		{ "ore", cpu16_disassembler::mode::IND, 0 },
		{ "cpe", cpu16_disassembler::mode::IND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "ste", cpu16_disassembler::mode::IND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "xgey", cpu16_disassembler::mode::INH, 0 },
		{ "aey", cpu16_disassembler::mode::INH, 0 },
		{ "tys", cpu16_disassembler::mode::INH, 0 },
		{ "aby", cpu16_disassembler::mode::INH, 0 },

		// 376X
		{ "sube", cpu16_disassembler::mode::IND, 0 },
		{ "adde", cpu16_disassembler::mode::IND, 0 },
		{ "sbce", cpu16_disassembler::mode::IND, 0 },
		{ "adce", cpu16_disassembler::mode::IND, 0 },
		{ "eore", cpu16_disassembler::mode::IND, 0 },
		{ "lde", cpu16_disassembler::mode::IND, 0 },
		{ "ande", cpu16_disassembler::mode::IND, 0 },
		{ "ore", cpu16_disassembler::mode::IND, 0 },
		{ "cpe", cpu16_disassembler::mode::IND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "ste", cpu16_disassembler::mode::IND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "xgez", cpu16_disassembler::mode::INH, 0 },
		{ "aez", cpu16_disassembler::mode::INH, 0 },
		{ "tzs", cpu16_disassembler::mode::INH, 0 },
		{ "abz", cpu16_disassembler::mode::INH, 0 },

		// 377X
		{ "sube", cpu16_disassembler::mode::EXT, 0 },
		{ "adde", cpu16_disassembler::mode::EXT, 0 },
		{ "sbce", cpu16_disassembler::mode::EXT, 0 },
		{ "adce", cpu16_disassembler::mode::EXT, 0 },
		{ "eore", cpu16_disassembler::mode::EXT, 0 },
		{ "lde", cpu16_disassembler::mode::EXT, 0 },
		{ "ande", cpu16_disassembler::mode::EXT, 0 },
		{ "ore", cpu16_disassembler::mode::EXT, 0 },
		{ "cpe", cpu16_disassembler::mode::EXT, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "ste", cpu16_disassembler::mode::EXT, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "cpx", cpu16_disassembler::mode::IMM, 0 },
		{ "cpy", cpu16_disassembler::mode::IMM, 0 },
		{ "cpz", cpu16_disassembler::mode::IMM, 0 },
		{ "cps", cpu16_disassembler::mode::IMM, 0 },

		// 378X
		{ "lbra", cpu16_disassembler::mode::REL, 0 },
		{ "lbrn", cpu16_disassembler::mode::REL, 0 },
		{ "lbhi", cpu16_disassembler::mode::REL, 0 },
		{ "lbls", cpu16_disassembler::mode::REL, 0 },
		{ "lbcc", cpu16_disassembler::mode::REL, 0 },
		{ "lbcs", cpu16_disassembler::mode::REL, 0 },
		{ "lbne", cpu16_disassembler::mode::REL, 0 },
		{ "lbeq", cpu16_disassembler::mode::REL, 0 },
		{ "lbvc", cpu16_disassembler::mode::REL, 0 },
		{ "lbvs", cpu16_disassembler::mode::REL, 0 },
		{ "lbpl", cpu16_disassembler::mode::REL, 0 },
		{ "lbmi", cpu16_disassembler::mode::REL, 0 },
		{ "lbge", cpu16_disassembler::mode::REL, 0 },
		{ "lblt", cpu16_disassembler::mode::REL, 0 },
		{ "lbgt", cpu16_disassembler::mode::REL, 0 },
		{ "lble", cpu16_disassembler::mode::REL, 0 },

		// 379X
		{ "lbmv", cpu16_disassembler::mode::REL, 0 },
		{ "lbev", cpu16_disassembler::mode::REL, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "tbxk", cpu16_disassembler::mode::INH, 0 },
		{ "tbyk", cpu16_disassembler::mode::INH, 0 },
		{ "tbzk", cpu16_disassembler::mode::INH, 0 },
		{ "tbsk", cpu16_disassembler::mode::INH, 0 },

		// 37AX
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "bgnd", cpu16_disassembler::mode::INH, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "txkb", cpu16_disassembler::mode::INH, 0 },
		{ "tykb", cpu16_disassembler::mode::INH, 0 },
		{ "tzkb", cpu16_disassembler::mode::INH, 0 },
		{ "tskb", cpu16_disassembler::mode::INH, 0 },

		// 37BX
		{ "subd", cpu16_disassembler::mode::IMM, 0 },
		{ "addd", cpu16_disassembler::mode::IMM, 0 },
		{ "sbcd", cpu16_disassembler::mode::IMM, 0 },
		{ "adcd", cpu16_disassembler::mode::IMM, 0 },
		{ "eord", cpu16_disassembler::mode::IMM, 0 },
		{ "ldd", cpu16_disassembler::mode::IMM, 0 },
		{ "andd", cpu16_disassembler::mode::IMM, 0 },
		{ "ord", cpu16_disassembler::mode::IMM, 0 },
		{ "cpd", cpu16_disassembler::mode::IMM, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "ldx", cpu16_disassembler::mode::IMM, 0 },
		{ "ldy", cpu16_disassembler::mode::IMM, 0 },
		{ "ldz", cpu16_disassembler::mode::IMM, 0 },
		{ "lds", cpu16_disassembler::mode::IMM, 0 },

		// 37CX
		{ "subd", cpu16_disassembler::mode::IND, 0 },
		{ "addd", cpu16_disassembler::mode::IND, 0 },
		{ "sbcd", cpu16_disassembler::mode::IND, 0 },
		{ "adcd", cpu16_disassembler::mode::IND, 0 },
		{ "eord", cpu16_disassembler::mode::IND, 0 },
		{ "ldd", cpu16_disassembler::mode::IND, 0 },
		{ "andd", cpu16_disassembler::mode::IND, 0 },
		{ "ord", cpu16_disassembler::mode::IND, 0 },
		{ "cpd", cpu16_disassembler::mode::IND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "std", cpu16_disassembler::mode::IND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "xgdx", cpu16_disassembler::mode::INH, 0 },
		{ "adx", cpu16_disassembler::mode::INH, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },

		// 37DX
		{ "subd", cpu16_disassembler::mode::IND, 0 },
		{ "addd", cpu16_disassembler::mode::IND, 0 },
		{ "sbcd", cpu16_disassembler::mode::IND, 0 },
		{ "adcd", cpu16_disassembler::mode::IND, 0 },
		{ "eord", cpu16_disassembler::mode::IND, 0 },
		{ "ldd", cpu16_disassembler::mode::IND, 0 },
		{ "andd", cpu16_disassembler::mode::IND, 0 },
		{ "ord", cpu16_disassembler::mode::IND, 0 },
		{ "cpd", cpu16_disassembler::mode::IND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "std", cpu16_disassembler::mode::IND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "xgdy", cpu16_disassembler::mode::INH, 0 },
		{ "ady", cpu16_disassembler::mode::INH, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },

		// 37EX
		{ "subd", cpu16_disassembler::mode::IND, 0 },
		{ "addd", cpu16_disassembler::mode::IND, 0 },
		{ "sbcd", cpu16_disassembler::mode::IND, 0 },
		{ "adcd", cpu16_disassembler::mode::IND, 0 },
		{ "eord", cpu16_disassembler::mode::IND, 0 },
		{ "ldd", cpu16_disassembler::mode::IND, 0 },
		{ "andd", cpu16_disassembler::mode::IND, 0 },
		{ "ord", cpu16_disassembler::mode::IND, 0 },
		{ "cpd", cpu16_disassembler::mode::IND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "std", cpu16_disassembler::mode::IND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "xgdz", cpu16_disassembler::mode::INH, 0 },
		{ "adz", cpu16_disassembler::mode::INH, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },

		// 37FX
		{ "subd", cpu16_disassembler::mode::EXT, 0 },
		{ "addd", cpu16_disassembler::mode::EXT, 0 },
		{ "sbcd", cpu16_disassembler::mode::EXT, 0 },
		{ "adcd", cpu16_disassembler::mode::EXT, 0 },
		{ "eord", cpu16_disassembler::mode::EXT, 0 },
		{ "ldd", cpu16_disassembler::mode::EXT, 0 },
		{ "andd", cpu16_disassembler::mode::EXT, 0 },
		{ "ord", cpu16_disassembler::mode::EXT, 0 },
		{ "cpd", cpu16_disassembler::mode::EXT, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "std", cpu16_disassembler::mode::EXT, 0 },
		{ "", cpu16_disassembler::mode::UND, 0 },
		{ "tpa", cpu16_disassembler::mode::INH, 0 },
		{ "tap", cpu16_disassembler::mode::INH, 0 },
		{ "movb", cpu16_disassembler::mode::EXT, 0 },
		{ "movw", cpu16_disassembler::mode::EXT, 0 }
	}
};

const std::string_view cpu16_disassembler::s_regset[7] =
{
	"d", "e", "x", "y", "z", "k", "cc"
};

void cpu16_disassembler::format_signed(std::ostream &stream, u16 value)
{
	if (s16(value) < 0)
	{
		stream << '-';
		value = -value;
	}
	if (value > 9)
		stream << '$';
	util::stream_format(stream, "%X", value);
}

void cpu16_disassembler::format_index8(std::ostream &stream, u8 offset, char reg)
{
	// 8-bit offsets are unsigned
	if (offset > 9)
		stream << '$';
	util::stream_format(stream, "%X, %c", offset, reg);
}

void cpu16_disassembler::format_index16(std::ostream &stream, u16 offset, char reg)
{
	// 16-bit offsets are signed
	format_signed(stream, offset);
	util::stream_format(stream, ", %c", reg);
}

offs_t cpu16_disassembler::disassemble(std::ostream &stream, offs_t pc, const cpu16_disassembler::data_buffer &opcodes, const cpu16_disassembler::data_buffer &params)
{
	const u16 opcode = opcodes.r16(pc);
	const u8 page = (opcode & 0xcf00) == 0x0700 ? BIT(opcode, 12, 2) : 0;
	const opcode_info &info = s_opinfo[page][page == 0 ? opcode >> 8 : opcode & 0x00ff];

	switch (info.m_mode)
	{
	default:
	case mode::UND:
		assert(info.m_name.empty());
		util::stream_format(stream, "%-6s$%04X", "fdb", opcode);
		return 2 | SUPPORTED;

	case mode::INH:
		stream << info.m_name;
		return 2 | SUPPORTED | info.m_flags;

	case mode::IMM:
		util::stream_format(stream, "%-6s", info.m_name);
		if (page == 0)
		{
			util::stream_format(stream, "#$%02X", opcode & 0x00ff);
			return 2 | SUPPORTED | info.m_flags;
		}
		else
		{
			util::stream_format(stream, "#$%04X", opcodes.r16(pc + 2));
			return 4 | SUPPORTED | info.m_flags;
		}

	case mode::IMMS: // Sign-extended immediate
		util::stream_format(stream, "%-6s#", info.m_name);
		if (page == 0)
		{
			format_signed(stream, s16(s8(opcode & 0x00ff)));
			return 2 | SUPPORTED | info.m_flags;
		}
		else
		{
			format_signed(stream, opcodes.r16(pc + 2));
			return 4 | SUPPORTED | info.m_flags;
		}

	case mode::REGM:
	{
		assert(page == 0);
		util::stream_format(stream, "%-6s", info.m_name);
		bool any = false;
		for (int i = 0; i < 7; i++)
		{
			// PSHM and POPM scan registers in the opposite order
			if (BIT(opcode, 8) ? BIT(opcode, 6 - i) : BIT(opcode, i))
			{
				if (any)
					stream << ", ";
				else
					any = true;
				stream << s_regset[i];
			}
		}
		if (BIT(opcode, 7))
		{
			if (any)
				stream << ", ";
			stream << "(reserved)";
		}
		else if (!any)
			stream << '0';
		return 2 | SUPPORTED | info.m_flags;
	}

	case mode::XYO:
		assert(page == 0);
		util::stream_format(stream, "%-6s", info.m_name);
		if (BIT(opcode, 7))
			util::stream_format(stream, "-%d, ", 8 - BIT(opcode, 4, 3));
		else
			util::stream_format(stream, "%d, ", BIT(opcode, 4, 3));
		if (BIT(opcode, 3))
			util::stream_format(stream, "-%d", 8 - BIT(opcode, 0, 3));
		else
			util::stream_format(stream, "%d", BIT(opcode, 0, 3));
		return 2 | SUPPORTED | info.m_flags;

	case mode::IND:
		util::stream_format(stream, "%-6s", info.m_name);
		if (page == 0)
		{
			format_index8(stream, opcode & 0x00ff, 'x' + BIT(opcode, 12, 2));
			return 2 | SUPPORTED | info.m_flags;
		}
		else
		{
			format_index16(stream, opcodes.r16(pc + 2), 'x' + BIT(opcode, 4, 2));
			return 4 | SUPPORTED | info.m_flags;
		}

	case mode::IND20:
		assert(page == 0);
		util::stream_format(stream, "%-6s$%05X, %c", info.m_name, u32(opcode & 0x000f) << 16 | opcodes.r16(pc + 2), 'x' + BIT(opcode, 12, 2));
		return 4 | SUPPORTED | info.m_flags;

	case mode::EXT:
		assert(page != 0);
		util::stream_format(stream, "%-6s$%04X", info.m_name, opcodes.r16(pc + 2));
		if ((opcode & 0xfffe) == 0x37fe)
		{
			util::stream_format(stream, ", $%04X", opcodes.r16(pc + 4));
			return 6 | SUPPORTED | info.m_flags;
		}
		else
			return 4 | SUPPORTED | info.m_flags;

	case mode::EXT20:
		assert(page == 0);
		util::stream_format(stream, "%-6s$%05X", info.m_name, u32(opcode & 0x000f) << 16 | opcodes.r16(pc + 2));
		return 4 | SUPPORTED | info.m_flags;

	case mode::E:
		assert(page != 0);
		util::stream_format(stream, "%-6se, %c", info.m_name, 'x' + BIT(opcode, 4, 2));
		return 2 | SUPPORTED | info.m_flags;

	case mode::REL:
		util::stream_format(stream, "%-6s", info.m_name);
		if (page == 0)
		{
			util::stream_format(stream, "$%05X", (pc + 6 + s8(opcode & 0x00ff)) & 0xfffff);
			return 2 | SUPPORTED | info.m_flags;
		}
		else
		{
			util::stream_format(stream, "$%05X", (pc + 6 + s16(opcodes.r16(pc + 2))) & 0xfffff);
			return 4 | SUPPORTED | info.m_flags;
		}

	case mode::BIT:
		util::stream_format(stream, "%-6s", info.m_name);
		if (page != 0)
		{
			const u16 operand = opcodes.r16(pc + 2);
			format_index8(stream, operand & 0x00ff, 'x' + BIT(opcode, 4, 2));
			util::stream_format(stream, ", #$%02X", operand >> 8);
			return 4 | SUPPORTED | info.m_flags;
		}
		else if (opcode >= 0x8000)
		{
			const u16 operand = opcodes.r16(pc + 2);
			format_index8(stream, operand >> 8, 'x' + BIT(opcode, 12, 2));
			util::stream_format(stream, ", #$%02X, $%04X", opcode & 0x00ff, (pc + 6 + s8(operand & 0x00ff)) & 0xfffff);
			return 4 | SUPPORTED | info.m_flags;
		}
		else
		{
			if (BIT(opcode, 12, 2) != 3)
				format_index16(stream, opcodes.r16(pc + 2), 'x' + BIT(opcode, 12, 2));
			else
				util::stream_format(stream, "$%04X", opcodes.r16(pc + 2));
			util::stream_format(stream, ", #$%02X", opcode & 0x00ff);
			if (BIT(opcode, 9))
			{
				util::stream_format(stream, ", $%04X", (pc + 6 + s16(opcodes.r16(pc + 4))) & 0xfffff);
				return 6 | SUPPORTED | info.m_flags;
			}
			else
				return 4 | SUPPORTED | info.m_flags;
		}

	case mode::BIT16:
		assert(page != 0);
		util::stream_format(stream, "%-6s", info.m_name);
		if (BIT(opcode, 4, 2) != 3)
			format_index16(stream, opcodes.r16(pc + 2), 'x' + BIT(opcode, 4, 2));
		else
			util::stream_format(stream, "$%04X", opcodes.r16(pc + 2));
		util::stream_format(stream, ", #$%04X", opcodes.r16(pc + 4));
		return 6 | SUPPORTED | info.m_flags;

	case mode::IXP:
		assert(page == 0);
		util::stream_format(stream, "%-6s", info.m_name);
		if (BIT(opcode, 9))
			util::stream_format(stream, "$%04X, ", opcodes.r16(pc + 2));
		format_index8(stream,  opcode & 0x00ff, 'x');
		if (!BIT(opcode, 9))
			util::stream_format(stream, ", $%04X", opcodes.r16(pc + 2));
		return 4 | SUPPORTED | info.m_flags;
	}
}
