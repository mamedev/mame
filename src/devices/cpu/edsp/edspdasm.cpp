// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Elan Microelectronics eDSP disassembler

    Instruction encodings for this 16-bit DSP architecture are based on
    guesswork; several instructions remain unidentified. It is also
    possible that different model series add or delete a few instructions.

    Known missing instructions:

        Rd = RAM8
        RAM8 = Rd
        Rd = [R3 - Rt]
        [R3 - Rt] = Rs
        D = Rs * [Rt]
        D = Rs * [Rt++]
        D = Rs * [Rt--]
        D = Rs * P[Rt]
        D = Rs * P[Rt++]
        D = [Rs++] * P[Rt--]
        D = [Rs++] * P[Rt++]
        D = D / Rs
        MAC (all modes)
        MAS (all modes)
        SWAP Rs
        CMP Rs, [Rt++]
        CMP Rs, [Rt--]
        CMP Rs, P[Rt]
        CMP Rs, P[Rt++]
        CMP [Rs++], P[Rt--]
        CMP [Rs++], P[Rt++]
        CALL Rd
        LOOP Rn
        LOOP #imm6
        TRAP #imm6

    System registers are mapped in the I/O space, but the locations differ
    between model series. For example, SPA (stack pointer address) is
    IO[0x11] on the eSL and eSLS series, but IO[0x13] on eMG2000A.

***************************************************************************/

#include "emu.h"
#include "edspdasm.h"

edsp_disassembler::edsp_disassembler()
	: util::disasm_interface()
{
}

u32 edsp_disassembler::opcode_alignment() const
{
	return 1;
}

namespace {

const char *const c_conditions[15] =
{
	"lo/cc",
	"hs/cs",
	"vc", // not used in mylife, unconfirmed
	"vs", // not used in mylife, unconfirmed
	"ne",
	"eq",
	"pl", // not used in mylife, unconfirmed
	"mi", // not used in mylife, unconfirmed
	// next two are possibly reversed
	"tc",
	"ts", // not used in mylife
	"ge",
	"lt",
	"gt",
	"le",
	"ls" // not used in mylife, unconfirmed
};

const char *const c_bit_ops[4] =
{
	"bs",
	"bc",
	"btest",
	"btg" // not used in mylife, unconfirmed
};

const char *const c_mul_modes[4] =
{
	// unknown which of these are SS, SU, US or UU
	"Mode0",
	"Mode1", // not used in mylife
	"Mode2", // not used in mylife
	"Mode3"
};

} // anonymous namespace

offs_t edsp_disassembler::disassemble(std::ostream &stream, offs_t pc, const edsp_disassembler::data_buffer &opcodes, const edsp_disassembler::data_buffer &params)
{
	const u16 op = opcodes.r16(pc);
	if (op < 0x1000)
	{
		// ADD/ADC with register direct, indirect or imm6
		switch (BIT(op, 3, 2))
		{
		case 0:
			util::stream_format(stream, "r%d = r%d + r%d", BIT(op, 8, 3), BIT(op, 5, 3), BIT(op, 0, 3));
			break;

		case 1:
			util::stream_format(stream, "[r%d] = r%d + r%d", BIT(op, 8, 3), BIT(op, 5, 3), BIT(op, 0, 3));
			break;

		case 2:
			util::stream_format(stream, "r%d = [r%d] + r%d", BIT(op, 8, 3), BIT(op, 5, 3), BIT(op, 0, 3));
			break;

		case 3:
			util::stream_format(stream, "r%d = r%d + #0x%02X", BIT(op, 8, 3), BIT(op, 8, 3), BIT(op, 5, 3) << 3 | BIT(op, 0, 3));
			break;
		}
		if (BIT(op, 11))
			stream << " + C";
		return 1 | SUPPORTED;
	}
	else if (op < 0x2000)
	{
		// SUB/SUBB with register direct, indirect or imm6
		switch (BIT(op, 3, 2))
		{
		case 0:
			util::stream_format(stream, "r%d = r%d - r%d", BIT(op, 8, 3), BIT(op, 5, 3), BIT(op, 0, 3));
			break;

		case 1:
			util::stream_format(stream, "[r%d] = r%d - r%d", BIT(op, 8, 3), BIT(op, 5, 3), BIT(op, 0, 3));
			break;

		case 2:
			util::stream_format(stream, "r%d = [r%d] - r%d", BIT(op, 8, 3), BIT(op, 5, 3), BIT(op, 0, 3));
			break;

		case 3:
			util::stream_format(stream, "r%d = r%d - #0x%02X", BIT(op, 8, 3), BIT(op, 8, 3), BIT(op, 5, 3) << 3 | BIT(op, 0, 3));
			break;
		}
		if (BIT(op, 11))
			stream << " - B";
		return 1 | SUPPORTED;
	}
	else if (op < 0x2800)
	{
		// AND with register direct, indirect or imm6
		switch (BIT(op, 3, 2))
		{
		case 0:
			util::stream_format(stream, "r%d = r%d AND r%d", BIT(op, 8, 3), BIT(op, 5, 3), BIT(op, 0, 3));
			break;

		case 1:
			util::stream_format(stream, "[r%d] = r%d AND r%d", BIT(op, 8, 3), BIT(op, 5, 3), BIT(op, 0, 3));
			break;

		case 2:
			util::stream_format(stream, "r%d = [r%d] AND r%d", BIT(op, 8, 3), BIT(op, 5, 3), BIT(op, 0, 3));
			break;

		case 3:
			util::stream_format(stream, "r%d = r%d AND #0x%02X", BIT(op, 8, 3), BIT(op, 8, 3), BIT(op, 5, 3) << 3 | BIT(op, 0, 3));
			break;
		}
		return 1 | SUPPORTED;
	}
	else if (op < 0x3000)
	{
		// OR with register direct, indirect or imm6
		if (op == 0x2800 + 0x0121 * BIT(op, 0, 3))
			util::stream_format(stream, "tst r%d", BIT(op, 0, 3));
		else switch (BIT(op, 3, 2))
		{
		case 0:
			util::stream_format(stream, "r%d = r%d OR r%d", BIT(op, 8, 3), BIT(op, 5, 3), BIT(op, 0, 3));
			break;

		case 1:
			util::stream_format(stream, "[r%d] = r%d OR r%d", BIT(op, 8, 3), BIT(op, 5, 3), BIT(op, 0, 3));
			break;

		case 2:
			util::stream_format(stream, "r%d = [r%d] OR r%d", BIT(op, 8, 3), BIT(op, 5, 3), BIT(op, 0, 3));
			break;

		case 3:
			util::stream_format(stream, "r%d = r%d OR #0x%02X", BIT(op, 8, 3), BIT(op, 8, 3), BIT(op, 5, 3) << 3 | BIT(op, 0, 3));
			break;
		}
		return 1 | SUPPORTED;
	}
	else if (op < 0x3800)
	{
		// XOR with register direct, indirect or imm6
		if (op == 0x3000 + 0x0121 * BIT(op, 0, 3))
			util::stream_format(stream, "clr r%d", BIT(op, 0, 3));
		else switch (BIT(op, 3, 2))
		{
		case 0:
			util::stream_format(stream, "r%d = r%d XOR r%d", BIT(op, 8, 3), BIT(op, 5, 3), BIT(op, 0, 3));
			break;

		case 1:
			util::stream_format(stream, "[r%d] = r%d XOR r%d", BIT(op, 8, 3), BIT(op, 5, 3), BIT(op, 0, 3));
			break;

		case 2:
			util::stream_format(stream, "r%d = [r%d] XOR r%d", BIT(op, 8, 3), BIT(op, 5, 3), BIT(op, 0, 3));
			break;

		case 3:
			util::stream_format(stream, "r%d = r%d XOR #0x%02X", BIT(op, 8, 3), BIT(op, 8, 3), BIT(op, 5, 3) << 3 | BIT(op, 0, 3));
			break;
		}
		return 1 | SUPPORTED;
	}
	else if ((op & 0xf816) == 0x3800)
	{
		// ADD/ADC with imm16
		if (BIT(op, 3))
			util::stream_format(stream, "[r%d]", BIT(op, 8, 3));
		else
			util::stream_format(stream, "r%d", BIT(op, 8, 3));
		util::stream_format(stream, " = r%d + #0x%04X", BIT(op, 5, 3), opcodes.r16(pc + 1));
		if (BIT(op, 0))
			stream << " + C";
		return 2 | SUPPORTED;
	}
	else if ((op & 0xf816) == 0x3802)
	{
		// SUB/SUBB with imm16
		if (BIT(op, 3))
			util::stream_format(stream, "[r%d]", BIT(op, 8, 3));
		else
			util::stream_format(stream, "r%d", BIT(op, 8, 3));
		util::stream_format(stream, " = r%d - #0x%04X", BIT(op, 5, 3), opcodes.r16(pc + 1));
		if (BIT(op, 0))
			stream << " - B";
		return 2 | SUPPORTED;
	}
	else if ((op & 0xf817) == 0x3804)
	{
		// AND with imm16
		if (BIT(op, 3))
			util::stream_format(stream, "[r%d]", BIT(op, 8, 3));
		else
			util::stream_format(stream, "r%d", BIT(op, 8, 3));
		util::stream_format(stream, " = r%d AND #0x%04X", BIT(op, 5, 3), opcodes.r16(pc + 1));
		return 2 | SUPPORTED;
	}
	else if ((op & 0xf817) == 0x3805)
	{
		// OR with imm16
		if (BIT(op, 3))
			util::stream_format(stream, "[r%d]", BIT(op, 8, 3));
		else
			util::stream_format(stream, "r%d", BIT(op, 8, 3));
		util::stream_format(stream, " = r%d OR #0x%04X", BIT(op, 5, 3), opcodes.r16(pc + 1));
		return 2 | SUPPORTED;
	}
	else if ((op & 0xf817) == 0x3806)
	{
		// XOR with imm16
		if (BIT(op, 3))
			util::stream_format(stream, "[r%d]", BIT(op, 8, 3));
		else
			util::stream_format(stream, "r%d", BIT(op, 8, 3));
		util::stream_format(stream, " = r%d XOR #0x%04X", BIT(op, 5, 3), opcodes.r16(pc + 1));
		return 2 | SUPPORTED;
	}
	else if ((op & 0xf81e) == 0x3810)
	{
		// ADD/ADC with RAM16
		util::stream_format(stream, "r%d = r%d + 0x%04X", BIT(op, 8, 3), BIT(op, 5, 3), opcodes.r16(pc + 1));
		if (BIT(op, 0))
			stream << " + C";
		return 2 | SUPPORTED;
	}
	else if ((op & 0xf81e) == 0x3812)
	{
		// SUB/SUBB with RAM16
		util::stream_format(stream, "r%d = r%d - 0x%04X", BIT(op, 8, 3), BIT(op, 5, 3), opcodes.r16(pc + 1));
		if (BIT(op, 0))
			stream << " - B";
		return 2 | SUPPORTED;
	}
	else if ((op & 0xf81f) == 0x3814)
	{
		// AND with RAM16 (not used in mylife)
		util::stream_format(stream, "r%d = r%d AND 0x%04X", BIT(op, 8, 3), BIT(op, 5, 3), opcodes.r16(pc + 1));
		return 2 | SUPPORTED;
	}
	else if ((op & 0xf81f) == 0x3815)
	{
		// OR with RAM16
		util::stream_format(stream, "r%d = r%d OR 0x%04X", BIT(op, 8, 3), BIT(op, 5, 3), opcodes.r16(pc + 1));
		return 2 | SUPPORTED;
	}
	else if ((op & 0xf81f) == 0x3816)
	{
		// XOR with RAM16 (not used in mylife)
		util::stream_format(stream, "r%d = r%d XOR 0x%04X", BIT(op, 8, 3), BIT(op, 5, 3), opcodes.r16(pc + 1));
		return 2 | SUPPORTED;
	}
	else if ((op & 0xf8ff) == 0x3817)
	{
		// Repeat next instruction
		util::stream_format(stream, "rpt r%d", BIT(op, 8, 3));
		return 1 | SUPPORTED;
	}
	else if ((op & 0xf87f) == 0x3818)
	{
		// Long branch
		const u8 cond = BIT(op, 7, 4);
		if (cond != 15)
			util::stream_format(stream, "if %s ", c_conditions[cond]);
		util::stream_format(stream, "ljmp 0x%04X", u32(opcodes.r16(pc + 1)));
		return 2 | (cond != 15 ? STEP_COND : 0) | SUPPORTED;
	}
	else if (op == 0x3819)
	{
		// Call absolute address
		util::stream_format(stream, "lcall 0x%04X", u32(opcodes.r16(pc + 1)));
		return 2 | STEP_OVER | SUPPORTED;
	}
	else if (op == 0x381a)
	{
		// No operation (maybe)
		stream << "nop";
		return 1 | SUPPORTED;
	}
	else if (op == 0x383a)
	{
		// Return from subroutine
		stream << "ret";
		return 1 | STEP_OUT | SUPPORTED;
	}
	else if (op == 0x385a)
	{
		// Return from interrupt
		stream << "reti";
		return 1 | STEP_OUT | SUPPORTED;
	}
	else if ((op & 0xf81f) == 0x381b)
	{
		util::stream_format(stream, "sp = sp + #%d", BIT(op, 5, 6));
		return 1 | SUPPORTED;
	}
	else if ((op & 0xf81f) == 0x381c)
	{
		util::stream_format(stream, "sp = sp - #%d", BIT(op, 5, 6));
		return 1 | SUPPORTED;
	}
//  else if (op == 0x381e) - used very rarely in mylife (trap?)
	else if ((op & 0xf81f) == 0x381f)
	{
		// Repeat next instruction (#imm6+1) times
		util::stream_format(stream, "rpt #%d", BIT(op, 5, 6));
		return 1 | SUPPORTED;
	}
	else if ((op & 0xff00) == 0x4000)
	{
		// Multiply
		util::stream_format(stream, "D = r%d * r%d (%s)", BIT(op, 5, 3), BIT(op, 0, 3), c_mul_modes[BIT(op, 3, 2)]);
		return 1 | SUPPORTED;
	}
	else if ((op & 0xff18) == 0x4818)
	{
		// Compare register direct
		util::stream_format(stream, "cmp r%d, r%d", BIT(op, 5, 3), BIT(op, 0, 3));
		return 1 | SUPPORTED;
	}
	else if ((op & 0xff18) == 0x4918)
	{
		// Compare register indirect
		util::stream_format(stream, "cmp r%d, [r%d]", BIT(op, 5, 3), BIT(op, 0, 3));
		return 1 | SUPPORTED;
	}
	else if ((op & 0xf81f) == 0x5800)
	{
		// Logical complement
		util::stream_format(stream, "r%d = COM r%d", BIT(op, 8, 3), BIT(op, 5, 3));
		return 1 | SUPPORTED;
	}
	else if ((op & 0xf81f) == 0x5801)
	{
		util::stream_format(stream, "r%d = r%d", BIT(op, 8, 3), BIT(op, 5, 3));
		return 1 | SUPPORTED;
	}
	else if ((op & 0xf81f) == 0x5802)
	{
		// Two's complement
		util::stream_format(stream, "r%d = NEG r%d", BIT(op, 8, 3), BIT(op, 5, 3));
		return 1 | SUPPORTED;
	}
	else if ((op & 0xf81f) == 0x5803)
	{
		util::stream_format(stream, "r%d = [r%d]", BIT(op, 8, 3), BIT(op, 5, 3));
		return 1 | SUPPORTED;
	}
	else if ((op & 0xf81f) == 0x5805)
	{
		util::stream_format(stream, "r%d = [r%d++]", BIT(op, 8, 3), BIT(op, 5, 3));
		return 1 | SUPPORTED;
	}
	else if ((op & 0xf81f) == 0x5806)
	{
		util::stream_format(stream, "r%d = SHL r%d", BIT(op, 8, 3), BIT(op, 5, 3));
		return 1 | SUPPORTED;
	}
	else if ((op & 0xf81f) == 0x5807)
	{
		util::stream_format(stream, "r%d = [r%d--]", BIT(op, 8, 3), BIT(op, 5, 3));
		return 1 | SUPPORTED;
	}
	else if ((op & 0xf81f) == 0x5808)
	{
		util::stream_format(stream, "r%d = ASR r%d", BIT(op, 8, 3), BIT(op, 5, 3));
		return 1 | SUPPORTED;
	}
	else if ((op & 0xf81f) == 0x5809)
	{
		util::stream_format(stream, "r%d = P[r%d]", BIT(op, 8, 3), BIT(op, 5, 3));
		return 1 | SUPPORTED;
	}
	else if ((op & 0xf81f) == 0x580a)
	{
		util::stream_format(stream, "r%d = SHR r%d", BIT(op, 5, 3), BIT(op, 8, 3));
		return 1 | SUPPORTED;
	}
	else if ((op & 0xf81f) == 0x580b)
	{
		util::stream_format(stream, "r%d = P[r%d++]", BIT(op, 8, 3), BIT(op, 5, 3));
		return 1 | SUPPORTED;
	}
	else if ((op & 0xf81f) == 0x580c)
	{
		// shift direction unclear
		util::stream_format(stream, "r%d = ROR r%d", BIT(op, 8, 3), BIT(op, 5, 3));
		return 1 | SUPPORTED;
	}
	else if ((op & 0xf81f) == 0x580d)
	{
		// not used in mylife
		util::stream_format(stream, "[r%d++] = P[r%d--]", BIT(op, 8, 3), BIT(op, 5, 3));
		return 1 | SUPPORTED;
	}
	else if ((op & 0xf81f) == 0x580e)
	{
		// shift direction unclear
		util::stream_format(stream, "r%d = ROL r%d", BIT(op, 8, 3), BIT(op, 5, 3));
		return 1 | SUPPORTED;
	}
	else if ((op & 0xf81f) == 0x580f)
	{
		util::stream_format(stream, "[r%d++] = P[r%d++]", BIT(op, 8, 3), BIT(op, 5, 3));
		return 1 | SUPPORTED;
	}
	else if ((op & 0xf81f) == 0x5811)
	{
		// not used in mylife
		util::stream_format(stream, "r%d = P[r%d--]", BIT(op, 8, 3), BIT(op, 5, 3));
		return 1 | SUPPORTED;
	}
	else if ((op & 0xf81f) == 0x5813)
	{
		// not used in mylife
		util::stream_format(stream, "[r%d] = P[r%d++]", BIT(op, 8, 3), BIT(op, 5, 3));
		return 1 | SUPPORTED;
	}
	else if ((op & 0xf81f) == 0x5815)
	{
		// not used in mylife
		util::stream_format(stream, "[r%d--] = P[r%d--]", BIT(op, 8, 3), BIT(op, 5, 3));
		return 1 | SUPPORTED;
	}
	else if ((op & 0xf81f) == 0x5817)
	{
		// not used in mylife
		util::stream_format(stream, "[r%d] = P[r%d]", BIT(op, 8, 3), BIT(op, 5, 3));
		return 1 | SUPPORTED;
	}
	else if ((op & 0xf81f) == 0x5819)
	{
		// not used in mylife
		util::stream_format(stream, "[r%d++] = P[r%d]", BIT(op, 8, 3), BIT(op, 5, 3));
		return 1 | SUPPORTED;
	}
	else if ((op & 0xf81f) == 0x581b)
	{
		util::stream_format(stream, "[r%d] = r%d", BIT(op, 8, 3), BIT(op, 5, 3));
		return 1 | SUPPORTED;
	}
	else if ((op & 0xf81f) == 0x581d)
	{
		util::stream_format(stream, "[r%d++] = r%d", BIT(op, 8, 3), BIT(op, 5, 3));
		return 1 | SUPPORTED;
	}
	else if ((op & 0xf8df) == 0x581e)
	{
		// Transfer register to/from stack
		util::stream_format(stream, "%s r%d", BIT(op, 5) ? "pop" : "push", BIT(op, 8, 3));
		return 1 | SUPPORTED;
	}
	else if ((op & 0xf8ff) == 0x585e)
	{
		util::stream_format(stream, "jmp r%d", BIT(op, 8, 3));
		return 1 | SUPPORTED;
	}
	else if ((op & 0xf8ff) == 0x589e)
	{
		// MOV RAM16 to register direct
		util::stream_format(stream, "r%d = 0x%04X", BIT(op, 8, 3), opcodes.r16(pc + 1));
		return 2 | SUPPORTED;
	}
	else if ((op & 0xf8ff) == 0x58be)
	{
		// MOV register direct to RAM16
		util::stream_format(stream, "0x%04X = r%d", opcodes.r16(pc + 1), BIT(op, 8, 3));
		return 2 | SUPPORTED;
	}
	else if ((op & 0xf8ff) == 0x58de)
	{
		// MOV imm16 to register indirect
		util::stream_format(stream, "[r%d] = #0x%04X", BIT(op, 8, 3), opcodes.r16(pc + 1));
		return 2 | SUPPORTED;
	}
	else if ((op & 0xf81f) == 0x581f)
	{
		// not used in mylife
		util::stream_format(stream, "[r%d--] = r%d", BIT(op, 8, 3), BIT(op, 5, 3));
		return 1 | SUPPORTED;
	}
	else if ((op & 0xf800) == 0x6000)
	{
		// MOV imm8 to register (clear high byte)
		util::stream_format(stream, "r%d", BIT(op, 8, 3));
		u16 op2 = opcodes.r16(pc + 1);
		if (((op ^ op2) & 0xff00) == 0x0800)
			util::stream_format(stream, ".l = LO #0x%04X", BIT(op2, 0, 8) << 8 | BIT(op, 0, 8));
		else
			util::stream_format(stream, " = #0x%02X", BIT(op, 0, 8));
		return 1 | SUPPORTED;
	}
	else if ((op & 0xf800) == 0x6800)
	{
		// MOV imm8 to high byte of register
		util::stream_format(stream, "r%d.h = #0x%02X", BIT(op, 8, 3), BIT(op, 0, 8));
		return 1 | SUPPORTED;
	}
//  else if ((op & 0xf000) == 0x7000) - probably MAC and MAS or move to or from RAM8, not used in mylife
	else if ((op & 0xe000) == 0x8000)
	{
		// Short branch
		const u8 cond = BIT(op, 9, 4);
		if (cond != 15)
			util::stream_format(stream, "if %s ", c_conditions[cond]);
		util::stream_format(stream, "jmp 0x%04X", u16(pc + 1 + util::sext(op, 9)));
		return 1 | (cond != 15 ? 0 : STEP_COND) | SUPPORTED;
	}
	else if ((op & 0xff00) == 0xa000)
	{
		// MOV R3 indirect with displacement to register
		util::stream_format(stream, "r%d = [r3 - %d]", BIT(op, 5, 3), BIT(op, 0, 5));
		return 1 | SUPPORTED;
	}
	else if ((op & 0xff00) == 0xa400)
	{
		// MOV register to R3 indirect with displacement
		util::stream_format(stream, "[r3 - %d] = r%d", BIT(op, 0, 5), BIT(op, 5, 3));
		return 1 | SUPPORTED;
	}
	else if ((op & 0xf980) == 0xa800)
	{
		// Bit operations on registers
		util::stream_format(stream, "%s r%d.%d", c_bit_ops[BIT(op, 9, 2)], BIT(op, 0, 3), BIT(op, 3, 4));
		return 1 | SUPPORTED;
	}
	else if ((op & 0xf980) == 0xa880)
	{
		// Bit operations on RAM addresses 0x0000–0x0007 (not used in mylife, unconfirmed)
		util::stream_format(stream, "%s 0x%04X.%d", c_bit_ops[BIT(op, 9, 2)], BIT(op, 0, 3), BIT(op, 3, 4));
		return 1 | SUPPORTED;
	}
	else if ((op & 0xf900) == 0xa900)
	{
		// Bit operations on I/O addresses 0x00–0x0F
		util::stream_format(stream, "%s IO[0x%02X].%d", c_bit_ops[BIT(op, 9, 2)], BIT(op, 7) << 3 | BIT(op, 0, 3), BIT(op, 3, 4));
		return 1 | SUPPORTED;
	}
	else if ((op & 0xf880) == 0xb000)
	{
		// IN I/O port to register
		util::stream_format(stream, "r%d = IO[0x%02X]", BIT(op, 8, 3), BIT(op, 0, 7));
		return 1 | SUPPORTED;
	}
	else if ((op & 0xf880) == 0xb080)
	{
		// OUT register to I/O port
		util::stream_format(stream, "IO[0x%02X] = r%d", BIT(op, 0, 7), BIT(op, 8, 3));
		return 1 | SUPPORTED;
	}
	else if ((op & 0xff00) == 0xb800)
	{
		// Transfer I/O port to/from stack
		util::stream_format(stream, "%s IO[0x%02X]", BIT(op, 7) ? "pop" : "push", BIT(op, 0, 7));
		return 1 | SUPPORTED;
	}
	else if (op >= 0xc000)
	{
		// Short call
		util::stream_format(stream, "call 0x%04X", BIT(op, 0, 14));
		return 1 | STEP_OVER | SUPPORTED;
	}
	else
	{
		util::stream_format(stream, ".dw 0x%04X", op);
		return 1 | SUPPORTED;
	}
}
