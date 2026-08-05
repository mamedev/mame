// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    unspfe.cpp

    Front end for u'nSP recompiler

***************************************************************************/

#include "emu.h"
#include "unspfe.h"

#include "unspdefs.h"

#include "cpu/drcfe.ipp"


/***************************************************************************
    INSTRUCTION PARSERS
***************************************************************************/

unsp_device::frontend::frontend(unsp_device *unsp, uint32_t window_start, uint32_t window_end, uint32_t max_sequence)
	: drc_frontend_base(unsp->space_config(AS_PROGRAM)->page_shift(), window_start, window_end, max_sequence)
	, m_cpu(unsp)
{
}

unsp_device::frontend::~frontend()
{
}

unsp_device::opcode_desc const *unsp_device::frontend::describe_code(offs_t startpc)
{
	return do_describe_code(
			[this] (opcode_desc &desc, opcode_desc const *prev) { return describe(desc, prev); },
			startpc);
}

inline uint16_t unsp_device::frontend::read_op_word(opcode_desc &desc, int offset)
{
	return m_cpu->m_cache.read_word(desc.pc + offset);
}

/*-------------------------------------------------
    describe - build a description of a single
    instruction
-------------------------------------------------*/

bool unsp_device::frontend::describe(opcode_desc &desc, const opcode_desc *prev)
{
	const uint16_t op = desc.opptr[0] = read_op_word(desc, 0);

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
			case 12: // JVC
			case 13: // JVS
				desc.regin.set(unsp_device::REG_SR);
				desc.regout.set(unsp_device::REG_SR);
				desc.regout.set(unsp_device::REG_PC);
				desc.targetpc = BRANCH_TARGET_DYNAMIC;
				desc.set_is_conditional_branch();
				desc.set_end_sequence();
				return true;
			case 14: // JMP
				desc.regout.set(unsp_device::REG_SR);
				desc.regout.set(unsp_device::REG_PC);
				desc.targetpc = desc.pc + 1 + ((op1 == 0) ? opimm : (0 - opimm));
				desc.set_is_unconditional_branch();
				desc.set_end_sequence();
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
		desc.regin.set(opb);
		desc.regout.set(opb);
		desc.set_writes_memory();
		while (r0--)
		{
			desc.regin.set(r1);
			r1--;
		}
		return true;
	}
	else if (lower_op == 0x29) // Pop
	{
		if (op == 0x9a98) // reti
		{
			desc.cycles = 8;
			desc.regin.set(unsp_device::REG_SP);
			desc.regout.set(unsp_device::REG_SP);
			desc.regout.set(unsp_device::REG_SR);
			desc.regout.set(unsp_device::REG_PC);
			desc.targetpc = BRANCH_TARGET_DYNAMIC;
			desc.set_is_unconditional_branch();
			desc.set_end_sequence();
			desc.set_reads_memory();
			return true;
		}
		else
		{
			uint16_t r0 = opn;
			uint16_t r1 = opa;
			desc.cycles = 4 + 2 * r0;
			desc.regin.set(opb);
			desc.regout.set(opb);
			desc.set_reads_memory();

			while (r0--)
			{
				r1++;
				desc.regout.set(r1);
				if (r1 == unsp_device::REG_PC)
				{
					desc.set_is_unconditional_branch();
					desc.set_end_sequence();
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
					desc.regin.set(opa);
					desc.regin.set(opb);
					desc.regout.set(unsp_device::REG_R3);
					desc.regout.set(unsp_device::REG_R4);
					return true;
				}
				return false;

			case 0x01: // Call
				if(!(opa & 1))
				{
					desc.cycles = 9;
					desc.length = 2;
					desc.targetpc = read_op_word(desc, 1) | ((op & 0x3f) << 16);
					desc.set_is_unconditional_branch();
					desc.set_end_sequence();
					desc.set_reads_memory();
					desc.set_writes_memory();
					desc.regin.set(unsp_device::REG_PC);
					desc.regin.set(unsp_device::REG_SP);
					desc.regout.set(unsp_device::REG_SP);
					desc.regout.set(unsp_device::REG_SR);
					desc.regout.set(unsp_device::REG_PC);
					return true;
				}
				return false;

			case 0x02: // Far Jump
				if (opa == 7)
				{
					desc.cycles = 5;
					desc.length = 2;
					desc.targetpc = read_op_word(desc, 1) | ((op & 0x3f) << 16);
					desc.set_is_unconditional_branch();
					desc.set_end_sequence();
					desc.set_reads_memory();
					desc.regin.set(unsp_device::REG_PC);
					desc.regout.set(unsp_device::REG_SR);
					desc.regout.set(unsp_device::REG_PC);
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
					case 4:
					case 5:
					case 8:
					case 9:
					case 12:
					case 14:
					case 37:
						return true;
				}
				return false;

			case 0x06:
			case 0x07: // MULS
				desc.regin.set(opa);
				desc.regin.set(opb);
				desc.regout.set(opa);
				desc.regout.set(opb);
				desc.regout.set(unsp_device::REG_R3);
				desc.regout.set(unsp_device::REG_R4);
				desc.set_reads_memory();
				desc.set_writes_memory();
				return true;

			default:
				return false;
		}
	}

	// At this point, we should be dealing solely with ALU ops.

	desc.regin.set(opa);

	switch (op1)
	{
		case 0x00: // r, [bp+imm6]
			desc.cycles = 6;
			desc.regin.set(unsp_device::REG_BP);
			if (op0 != 0x0d)
				desc.set_reads_memory();
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
						desc.regin.set(unsp_device::REG_SR);
						desc.regin.set(opb);
						if (op0 != 0x0d)
							desc.set_reads_memory();
						break;

					case 1: // r, [<ds:>r--]
					case 2: // r, [<ds:>r++]
					case 3: // r, [<ds:>++r]
						desc.regin.set(unsp_device::REG_SR);
						desc.regin.set(opb);
						desc.regout.set(unsp_device::REG_SR);
						desc.regout.set(opb);
						if (op0 != 0x0d)
							desc.set_reads_memory();
						break;
				}
			}
			else
			{
				switch (lsbits)
				{
					case 0: // r, [r]
						desc.regin.set(opb);
						if (op0 != 0x0d)
							desc.set_reads_memory();
						break;
					case 1: // r, [r--]
					case 2: // r, [r++]
					case 3: // r, [++r]
						desc.regin.set(opb);
						desc.regout.set(opb);
						if (op0 != 0x0d)
							desc.set_reads_memory();
						break;
				}
			}
			break;
		}

		case 0x04: // 16-bit ops
			switch (opn)
			{
				case 0x00: // r
					desc.cycles = (opa == 7) ? 5 : 3;
					desc.regin.set(opb);
					break;

				case 0x01: // imm16
					desc.cycles = (opa == 7) ? 5 : 4;
					desc.length = 2;
					desc.regin.set(opb);
					desc.regin.set(unsp_device::REG_SR);
					desc.regin.set(unsp_device::REG_PC);
					desc.regout.set(unsp_device::REG_SR);
					desc.regout.set(unsp_device::REG_PC);
					desc.set_reads_memory();
					break;

				case 0x02: // [imm16]
				case 0x03: // store [imm16], r
					desc.cycles = (opa == 7) ? 8 : 7;
					desc.length = 2;
					desc.regin.set(opb);
					desc.regin.set(unsp_device::REG_SR);
					desc.regin.set(unsp_device::REG_PC);
					desc.regout.set(unsp_device::REG_SR);
					desc.regout.set(unsp_device::REG_PC);
					desc.set_reads_memory();
					break;

				default: // Shifted ops
					desc.cycles = (opa == 7) ? 5 : 3;
					desc.regin.set(opb);
					break;
			}
			break;

		case 0x05: // More shifted ops
		case 0x06: // Rotated ops
			desc.cycles = (opa == 7) ? 5 : 3;
			desc.regin.set(opb);
			break;

		case 0x07: // Direct 8
			desc.cycles = (opa == 7) ? 6 : 5;
			desc.set_reads_memory();
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
			desc.regout.set(unsp_device::REG_SR);
			break;
		case 0x04: // Compare
		case 0x0c: // Test
			desc.regout.set(unsp_device::REG_SR);
			return true;
		case 0x0d: // Store
			desc.set_writes_memory();
			return true;
	}

	if (op1 == 0x04 && opn == 0x03) // store [imm16], r
	{
		desc.set_writes_memory();
	}
	else
	{
		desc.regout.set(opa);
		if (opa == 7)
		{
			desc.targetpc = BRANCH_TARGET_DYNAMIC;
			desc.set_is_unconditional_branch();
			desc.set_end_sequence();
		}
	}

	return true;
}
