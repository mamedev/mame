// license:BSD-3-Clause
// copyright-holders:Philip Bennett

/******************************************************************************

    Front-end for DSPP recompiler

******************************************************************************/

#include "emu.h"
#include "dsppfe.h"

#include "cpu/drcfe.ipp"


dspp_device::frontend::frontend(dspp_device *dspp, uint32_t window_start, uint32_t window_end, uint32_t max_sequence)
	: drc_frontend_base(dspp->space_config(AS_PROGRAM)->page_shift(), window_start, window_end, max_sequence)
	, m_dspp(dspp)
{

}

dspp_device::frontend::~frontend()
{
}


dspp_device::opcode_desc const *dspp_device::frontend::describe_code(offs_t startpc)
{
	return do_describe_code(
			[this] (opcode_desc &desc, opcode_desc const *prev) { return describe(desc, prev); },
			startpc);
}


bool dspp_device::frontend::describe(opcode_desc &desc, const opcode_desc *prev)
{
	const uint16_t op = desc.opptr[0] = m_dspp->read_op(desc.pc);

	desc.cycles = 1;
	desc.length = 1;

	// Decode and execute
	if (op & 0x8000)
	{
		switch ((op >> 13) & 3)
		{
			case 0:
				describe_special(op, desc);
				break;
			case 1:
			case 2:
				describe_branch(op, desc);
				break;
			case 3:
				describe_complex_branch(op, desc);
				break;
		}
	}
	else
	{
		describe_arithmetic(op, desc);
	}

	return true;
}

void dspp_device::frontend::describe_special(uint16_t op, opcode_desc &desc)
{
	switch ((op >> 10) & 7)
	{
		case 0:
		{
			// Super-special
			switch ((op >> 7) & 7)
			{
				case 1: // BAC
				{
					desc.set_is_unconditional_branch();
					desc.set_end_sequence();
					desc.targetpc = BRANCH_TARGET_DYNAMIC;
					return;
				}
				case 4: // RTS
				{
					desc.set_is_unconditional_branch();
					desc.set_end_sequence();
					desc.targetpc = BRANCH_TARGET_DYNAMIC;
					return;
				}
				case 5: // OP_MASK
				{
					// TODO
					return;
				}

				case 7: // SLEEP
				{
					desc.set_end_sequence();
					desc.set_return_to_start();
					return;
				}

				case 0: // NOP
				case 2: // Unused
				case 3:
				case 6:
					return;
			}

			break;
		}
		case 1: // JUMP
		{
			desc.set_is_unconditional_branch();
			desc.set_end_sequence();
			desc.targetpc = op & 0x3ff;
			return;
		}
		case 2: // JSR
		{
			desc.set_is_unconditional_branch();
			desc.set_end_sequence();
			desc.targetpc = op & 0x3ff;
			return;
		}
		case 3: // BFM
		{
			// TODO: What sort of branch is this?
			return;
		}
		case 4: // MOVEREG
		{
			desc.set_writes_memory();

			// Indirect
			if (op & 0x0010)
				desc.set_reads_memory();

			parse_operands(op, desc, 1);
			return;
		}
		case 5: // RBASE
		{
			return;
		}
		case 6: // MOVED
		{
			desc.set_writes_memory();
			parse_operands(op, desc, 1);
			return;
		}
		case 7: // MOVEI
		{
			desc.set_reads_memory();
			desc.set_writes_memory();
			parse_operands(op, desc, 1);
			return;
		}
	}
}

void dspp_device::frontend::describe_branch(uint16_t op, opcode_desc &desc)
{
	const uint32_t select = (op >> 12) & 1;

	if (select == 0)
	{
		desc.set_cc_n_used();
		desc.set_cc_v_used();
	}
	else
	{
		desc.set_cc_c_used();
		desc.set_cc_z_used();
	}

	// TODO: Can these be unconditional?
	desc.set_is_conditional_branch();
	desc.targetpc = op & 0x3ff;
}

void dspp_device::frontend::describe_complex_branch(uint16_t op, opcode_desc &desc)
{
	switch ((op >> 10) & 7)
	{
		case 0: // BLT
			desc.set_cc_n_used();
			desc.set_cc_v_used();
			break;
		case 1: // BLE
			desc.set_cc_n_used();
			desc.set_cc_v_used();
			desc.set_cc_z_used();
			break;
		case 2: // BGE
			desc.set_cc_n_used();
			desc.set_cc_v_used();
			break;
		case 3: // BGT
			desc.set_cc_n_used();
			desc.set_cc_v_used();
			desc.set_cc_z_used();
			break;
		case 4: // BHI
		case 5: // BLS
			desc.set_cc_c_used();
			desc.set_cc_z_used();
			break;
		case 6: // BXS
		case 7: // BXC
			desc.set_cc_x_used();
			break;
	}

	desc.set_is_conditional_branch();
	desc.targetpc = op & 0x3ff;
}

void dspp_device::frontend::describe_arithmetic(uint16_t op, opcode_desc &desc)
{
	// Decode the various fields
	uint32_t numops = (op >> 13) & 3;
	const uint32_t muxa = (op >> 10) & 3;
	const uint32_t muxb = (op >> 8) & 3;
	const uint32_t alu_op = (op >> 4) & 0xf;
	const uint32_t barrel_code = op & 0xf;

	// Check for operand overflow
	if (numops == 0 && ((muxa == 1) || (muxa == 2) || (muxb == 1) || (muxb == 2)))
		numops = 4;

	// Implicit barrel shift
	if (barrel_code == 8)
		++numops;

	// Number of cycles
	// Number of bytes
	// Registers read
	// Registers written
	// Does it read memory?
	// Does it write memory?

	parse_operands(op, desc, numops);

	if (muxa > 0 || muxb > 0)
	{
		desc.set_reads_memory();
	}

	switch (alu_op)
	{
		case 0: // _TRA
		{
			break;
		}
		case 1: // _NEG
		{
			break;
		}
		case 2: // _+
		case 4: // _-
		case 6: // _++
		case 7: // _--
		case 8: // _TRL
		case 9: // _NOT
		case 10: // _AND
		case 11: // _NAND
		case 12: // _OR
		case 13: // _NOR
		case 14: // _XOR
		case 15: // _XNOR
		{
			desc.set_cc_c_modified();
			desc.set_cc_v_modified();
			break;
		}
		case 3: // _+C
		case 5: // _-B
		{
			desc.set_cc_c_used();
			desc.set_cc_c_modified();
			break;
		}
	}

	desc.set_cc_n_modified();
	desc.set_cc_z_modified();
	desc.set_cc_x_modified();
}

void dspp_device::frontend::parse_operands(uint16_t op, opcode_desc &desc, uint32_t numops)
{
	uint32_t numregs = 0;
	uint32_t opidx = 0;

	while (opidx < numops)
	{
		uint16_t operand = desc.opptr[opidx + 1] = m_dspp->read_op(desc.pc + opidx + 1);

		desc.length++;
		desc.cycles++;

		if (operand & 0x8000)
		{
			// Immediate value
			if ((operand & 0xc000) == 0xc000)
			{
				opidx++;
			}
			else if((operand & 0xe000) == 0x8000)
			{
				if (operand & 0x0400) // Indirect
					desc.set_reads_memory();

				if (operand & 0x0800 )// Write Back
					desc.set_writes_memory();

				opidx++;
			}
			else if ((op & 0xe000) == 0xa000)
			{
				// 1 or 2 register operand
				numregs = (op & 0x0400) ? 2 : 1;
			}
		}
		else
		{
			numregs = 3;
		}

		if (numregs > 0)
		{
			// Shift successive register operands from a single operand word
			for (uint32_t i = 0; i < numregs; ++i)
			{
				uint32_t shift = ((numregs - i) - 1) * 5;
				uint32_t regdi = (operand >> shift) & 0x1f;

				if (regdi & 0x0010)
				{
					// Indirect
					desc.set_reads_memory();
				}

				if (numregs == 2)
				{
					// Write back
					if ((i == 0) && (operand & 0x1000))
						desc.set_writes_memory();
					else if ((i == 1) && (operand & 0x0800))
						desc.set_writes_memory();
				}
				else if (numregs == 1)
				{
					if (operand & 0x800)
						desc.set_writes_memory();
				}
				opidx++;
			}
			numregs = 0;
		}
	}
}
