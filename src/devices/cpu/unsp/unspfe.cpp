// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    unspfe.cpp

    Front end for u'nSP recompiler

***************************************************************************/

#include "emu.h"
#include "unspfe.h"
#include "unspdefs.h"

/***************************************************************************
    INSTRUCTION PARSERS
***************************************************************************/

unsp_frontend::unsp_frontend(unsp_device *unsp, uint32_t window_start, uint32_t window_end, uint32_t max_sequence)
	: drc_frontend(*unsp, window_start, window_end, max_sequence)
	, m_cpu(unsp)
{
}

inline uint16_t unsp_frontend::read_op_word(opcode_desc &desc, int offset)
{
	return m_cpu->m_pr16(desc.physpc + offset);
}

/*-------------------------------------------------
    describe - build a description of a single
    instruction
-------------------------------------------------*/

bool unsp_frontend::describe(opcode_desc &desc, const opcode_desc *prev)
{
	uint16_t op = desc.opptr.w[0] = read_op_word(desc, 0);

	/* most instructions are 1 word */
	desc.length = 1;
	desc.delayslots = 0;

	const uint16_t op0 = (op >> 12) & 15;
	const uint16_t opa = (op >> 9) & 7;
	const uint16_t op1 = (op >> 6) & 7;
	const uint16_t opn = (op >> 3) & 7;
	const uint16_t opb = op & 7;

	const uint8_t lower_op = (op1 << 4) | op0;

	if(op0 < 0xf && opa == 0x7 && op1 < 2)
	{
		desc.cycles = 2; // cycles for a taken branch will be added at execute time

		const uint32_t opimm = op & 0x3f;
		switch(op0)
		{
			case 0: // JB
			case 1: // JAE
			case 2: // JGE
			case 3: // JL
			case 4: // JNE
			case 5: // JE
			case 6: // JPL
			case 7: // JMI
			case 8: // JBE
			case 9: // JA
			case 10: // JLE
			case 11: // JG
				desc.regin[0] |= 1 << unsp_device::REG_SR;
				desc.regout[0] |= 1 << unsp_device::REG_SR;
				desc.regout[0] |= 1 << unsp_device::REG_PC;
				desc.targetpc = BRANCH_TARGET_DYNAMIC;
				desc.flags |= OPFLAG_IS_CONDITIONAL_BRANCH | OPFLAG_END_SEQUENCE;
				return true;
			case 14: // JMP
				desc.regout[0] |= 1 << unsp_device::REG_SR;
				desc.regout[0] |= 1 << unsp_device::REG_PC;
				desc.targetpc = desc.pc + 1 + ((op1 == 0) ? opimm : (0 - opimm));
				desc.flags |= OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_END_SEQUENCE;
				return true;
			default:
				return false;
		}
	}
	else if (lower_op == 0x2d) // Push
	{
		uint16_t r0 = opn;
		uint16_t r1 = opa;
		desc.cycles = 4 + 2 * r0;
		desc.regin[0] |= 1 << opb;
		desc.regout[0] |= 1 << opb;
		desc.flags |= OPFLAG_WRITES_MEMORY;
		while (r0--)
		{
			desc.regin[0] |= 1 << r1;
			r1--;
		}
		return true;
	}
	else if (lower_op == 0x29) // Pop
	{
		if (op == 0x9a98) // reti
		{
			desc.cycles = 8;
			desc.regin[0] |= 1 << unsp_device::REG_SP;
			desc.regout[0] |= 1 << unsp_device::REG_SP;
			desc.regout[0] |= 1 << unsp_device::REG_SR;
			desc.regout[0] |= 1 << unsp_device::REG_PC;
			desc.targetpc = BRANCH_TARGET_DYNAMIC;
			desc.flags |= OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_END_SEQUENCE | OPFLAG_READS_MEMORY;
			return true;
		}
		else
		{
			uint16_t r0 = opn;
			uint16_t r1 = opa;
			desc.cycles = 4 + 2 * r0;
			desc.regin[0] |= 1 << opb;
			desc.regout[0] |= 1 << opb;
			desc.flags |= OPFLAG_READS_MEMORY;

			while (r0--)
			{
				r1++;
				desc.regout[0] |= 1 << r1;
				if (r1 == unsp_device::REG_PC) {
					desc.flags |= OPFLAG_END_SEQUENCE | OPFLAG_IS_UNCONDITIONAL_BRANCH;
				}
			}
			return true;
		}
	}
	else if (op0 == 0xf)
	{
		switch (op1)
		{
			case 0x00: // Multiply, Unsigned * Signed
			case 0x04: // Multiply, Signed * Signed
				if(opn == 1 && opa != 7)
				{
					desc.cycles = 12;
					desc.regin[0] |= 1 << opa;
					desc.regin[0] |= 1 << opb;
					desc.regout[0] |= 1 << unsp_device::REG_R3;
					desc.regout[0] |= 1 << unsp_device::REG_R4;
					return true;
				}
				return false;

			case 0x01: // Call
				if(!(opa & 1))
				{
					desc.cycles = 9;
					desc.length = 2;
					desc.targetpc = read_op_word(desc, 1) | ((op & 0x3f) << 16);
					desc.flags = OPFLAG_READS_MEMORY | OPFLAG_WRITES_MEMORY | OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_END_SEQUENCE;
					desc.regin[0] |= 1 << unsp_device::REG_PC;
					desc.regin[0] |= 1 << unsp_device::REG_SP;
					desc.regout[0] |= 1 << unsp_device::REG_SP;
					desc.regout[0] |= 1 << unsp_device::REG_SR;
					desc.regout[0] |= 1 << unsp_device::REG_PC;
					return true;
				}
				return false;

			case 0x02: // Far Jump
				if (opa == 7)
				{
					desc.cycles = 5;
					desc.length = 2;
					desc.targetpc = read_op_word(desc, 1) | ((op & 0x3f) << 16);
					desc.flags = OPFLAG_READS_MEMORY | OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_END_SEQUENCE;
					desc.regin[0] |= 1 << unsp_device::REG_PC;
					desc.regout[0] |= 1 << unsp_device::REG_SR;
					desc.regout[0] |= 1 << unsp_device::REG_PC;
					return true;
				}
				return false;

			case 0x05: // Interrupt flags
				desc.cycles = 2;
				switch(op & 0x3f)
				{
					case 0:
					case 1:
					case 2:
					case 3:
					case 8:
					case 9:
					case 12:
					case 14:
					case 37:
						return true;
				}
				return false;

			default:
				return false;
		}
	}

	// At this point, we should be dealing solely with ALU ops.

	desc.regin[0] |= 1 << opa;

	switch (op1)
	{
		case 0x00: // r, [bp+imm6]
			desc.cycles = 6;
			desc.regin[0] |= 1 << unsp_device::REG_BP;
			if (op0 != 0x0d)
				desc.flags |= OPFLAG_READS_MEMORY;
			break;

		case 0x01: // r, imm6
			desc.cycles = 2;
			break;

		case 0x03: // Indirect
		{
			desc.cycles = (opa == 7 ? 7 : 6);
			const uint8_t lsbits = opn & 3;
			if (opn & 4)
			{
				switch (lsbits)
				{
					case 0: // r, [r]
						desc.regin[0] |= 1 << unsp_device::REG_SR;
						desc.regin[0] |= 1 << opb;
						if (op0 != 0x0d)
							desc.flags |= OPFLAG_READS_MEMORY;
						break;

					case 1: // r, [<ds:>r--]
					case 2: // r, [<ds:>r++]
					case 3: // r, [<ds:>++r]
						desc.regin[0] |= 1 << unsp_device::REG_SR;
						desc.regin[0] |= 1 << opb;
						desc.regout[0] |= 1 << unsp_device::REG_SR;
						desc.regout[0] |= 1 << opb;
						if (op0 != 0x0d)
							desc.flags |= OPFLAG_READS_MEMORY;
						break;
				}
			}
			else
			{
				switch (lsbits)
				{
					case 0: // r, [r]
						desc.regin[0] |= 1 << opb;
						if (op0 != 0x0d)
							desc.flags |= OPFLAG_READS_MEMORY;
						break;
					case 1: // r, [r--]
					case 2: // r, [r++]
					case 3: // r, [++r]
						desc.regin[0] |= 1 << opb;
						desc.regout[0] |= 1 << opb;
						if (op0 != 0x0d)
							desc.flags |= OPFLAG_READS_MEMORY;
						break;
				}
			}
			break;
		}

		case 0x04: // 16-bit ops
			switch (opn)
			{
				case 0x00: // r
					desc.cycles = (opa == 7 ? 5 : 3);
					desc.regin[0] |= 1 << opb;
					break;

				case 0x01: // imm16
					desc.cycles = (opa == 7 ? 5 : 4);
					desc.length = 2;
					desc.regin[0] |= 1 << opb;
					desc.regin[0] |= 1 << unsp_device::REG_SR;
					desc.regin[0] |= 1 << unsp_device::REG_PC;
					desc.regout[0] |= 1 << unsp_device::REG_SR;
					desc.regout[0] |= 1 << unsp_device::REG_PC;
					desc.flags |= OPFLAG_READS_MEMORY;
					break;

				case 0x02: // [imm16]
				case 0x03: // store [imm16], r
					desc.cycles = (opa == 7 ? 8 : 7);
					desc.length = 2;
					desc.regin[0] |= 1 << opb;
					desc.regin[0] |= 1 << unsp_device::REG_SR;
					desc.regin[0] |= 1 << unsp_device::REG_PC;
					desc.regout[0] |= 1 << unsp_device::REG_SR;
					desc.regout[0] |= 1 << unsp_device::REG_PC;
					desc.flags |= OPFLAG_READS_MEMORY;
					break;

				default: // Shifted ops
					desc.cycles = (opa == 7 ? 5 : 3);
					desc.regin[0] |= 1 << opb;
					break;
			}
			break;

		case 0x05: // More shifted ops
		case 0x06: // Rotated ops
			desc.cycles = (opa == 7 ? 5 : 3);
			desc.regin[0] |= 1 << opb;
			break;

		case 0x07: // Direct 8
			desc.cycles = (opa == 7 ? 6 : 5);
			desc.flags |= OPFLAG_READS_MEMORY;
			break;

		default:
			// Can't happen
			break;
	}

	switch (op0)
	{
		case 0x00: // Add
		case 0x01: // Add w/ carry
		case 0x02: // Subtract
		case 0x03: // Subtract w/ carry
		case 0x06: // Negate
		case 0x08: // XOR
		case 0x09: // Load
		case 0x0a: // OR
		case 0x0b: // AND
			desc.regout[0] |= 1 << unsp_device::REG_SR;
			break;
		case 0x04: // Compare
		case 0x0c: // Test
			desc.regout[0] |= 1 << unsp_device::REG_SR;
			return true;
		case 0x0d: // Store
			desc.flags |= OPFLAG_WRITES_MEMORY;
			return true;
	}

	if (op1 == 0x04 && opn == 0x03) // store [imm16], r
	{
		desc.flags |= OPFLAG_WRITES_MEMORY;
	}
	else
	{
		desc.regout[0] |= 1 << opa;
		if (opa == 7)
		{
			desc.targetpc = BRANCH_TARGET_DYNAMIC;
			desc.flags |= OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_END_SEQUENCE;
		}
	}

	return true;
}
