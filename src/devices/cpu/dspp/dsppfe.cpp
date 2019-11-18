// license:BSD-3-Clause
// copyright-holders:Philip Bennett

/******************************************************************************

    Front-end for DSPP recompiler

******************************************************************************/

#include "emu.h"
#include "dsppfe.h"


//#define REG_USED(desc,x)            do { (desc).regin[0] |= 1 << (x); } while(0)
//#define REG_MODIFIED(desc,x)        do { (desc).regout[0] |= 1 << (x); } while(0)

#define CC_C_USED(desc)               do { (desc).regin[0] |= 1 << 16; } while(0)
#define CC_C_MODIFIED(desc)           do { (desc).regout[0] |= 1 << 16; } while(0)
#define CC_Z_USED(desc)               do { (desc).regin[0] |= 1 << 16; } while(0)
#define CC_Z_MODIFIED(desc)           do { (desc).regout[0] |= 1 << 16; } while(0)
#define CC_N_USED(desc)               do { (desc).regin[0] |= 1 << 16; } while(0)
#define CC_N_MODIFIED(desc)           do { (desc).regout[0] |= 1 << 16; } while(0)
#define CC_V_USED(desc)               do { (desc).regin[0] |= 1 << 16; } while(0)
#define CC_V_MODIFIED(desc)           do { (desc).regout[0] |= 1 << 16; } while(0)
#define CC_X_USED(desc)               do { (desc).regin[0] |= 1 << 16; } while(0)
#define CC_X_MODIFIED(desc)           do { (desc).regout[0] |= 1 << 16; } while(0)

#define CC_FLAGS_MODIFIED(desc)    do {  } while(0)

dspp_frontend::dspp_frontend(dspp_device *dspp, uint32_t window_start, uint32_t window_end, uint32_t max_sequence)
	: drc_frontend(*dspp, window_start, window_end, max_sequence),
		m_dspp(dspp)
{

}


#if 0
// opcode branch flags
const uint32_t OPFLAG_IS_UNCONDITIONAL_BRANCH = 0x00000001;       // instruction is unconditional branch
const uint32_t OPFLAG_IS_CONDITIONAL_BRANCH   = 0x00000002;       // instruction is conditional branch
const uint32_t OPFLAG_IS_BRANCH               = (OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_IS_CONDITIONAL_BRANCH);
const uint32_t OPFLAG_IS_BRANCH_TARGET        = 0x00000004;       // instruction is the target of a branch
const uint32_t OPFLAG_IN_DELAY_SLOT           = 0x00000008;       // instruction is in the delay slot of a branch
const uint32_t OPFLAG_INTRABLOCK_BRANCH       = 0x00000010;       // instruction branches within the block

// opcode exception flags
const uint32_t OPFLAG_CAN_TRIGGER_SW_INT      = 0x00000020;       // instruction can trigger a software interrupt
const uint32_t OPFLAG_CAN_EXPOSE_EXTERNAL_INT = 0x00000040;       // instruction can expose an external interrupt
const uint32_t OPFLAG_CAN_CAUSE_EXCEPTION     = 0x00000080;       // instruction may generate exception
const uint32_t OPFLAG_WILL_CAUSE_EXCEPTION    = 0x00000100;       // instruction will generate exception
const uint32_t OPFLAG_PRIVILEGED              = 0x00000200;       // instruction is privileged

// opcode virtual->physical translation flags
const uint32_t OPFLAG_VALIDATE_TLB            = 0x00000400;       // instruction must validate TLB before execution
const uint32_t OPFLAG_MODIFIES_TRANSLATION    = 0x00000800;       // instruction modifies the TLB
const uint32_t OPFLAG_COMPILER_PAGE_FAULT     = 0x00001000;       // compiler hit a page fault when parsing
const uint32_t OPFLAG_COMPILER_UNMAPPED       = 0x00002000;       // compiler hit unmapped memory when parsing

// opcode flags
const uint32_t OPFLAG_INVALID_OPCODE          = 0x00004000;       // instruction is invalid
const uint32_t OPFLAG_VIRTUAL_NOOP            = 0x00008000;       // instruction is a virtual no-op

// opcode sequence flow flags
const uint32_t OPFLAG_REDISPATCH              = 0x00010000;       // instruction must redispatch after completion
const uint32_t OPFLAG_RETURN_TO_START         = 0x00020000;       // instruction must jump back to the beginning after completion
const uint32_t OPFLAG_END_SEQUENCE            = 0x00040000;       // this is the last instruction in a sequence
const uint32_t OPFLAG_CAN_CHANGE_MODES        = 0x00080000;       // instruction can change modes

// execution semantics
const uint32_t OPFLAG_READS_MEMORY            = 0x00100000;       // instruction reads memory
const uint32_t OPFLAG_WRITES_MEMORY           = 0x00200000;       // instruction writes memory
#endif

bool dspp_frontend::describe(opcode_desc &desc, const opcode_desc *prev)
{
	uint16_t op = desc.opptr.w[0] = m_dspp->read_op(desc.physpc);

	desc.cycles = 1;
	desc.length = 1;

	// Decode and execute
	if (op & 0x8000)
	{
		switch ((op >> 13) & 3)
		{
			case 0:
				describe_special(op, desc);

			case 1:
			case 2:
				describe_branch(op, desc);

			case 3:
				describe_complex_branch(op, desc);
		}
	}
	else
	{
		describe_arithmetic(op, desc);
	}

	return true;
}

void dspp_frontend::describe_special(uint16_t op, opcode_desc &desc)
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
					desc.flags |= OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_END_SEQUENCE;
					desc.targetpc = BRANCH_TARGET_DYNAMIC;
					return;
				}
				case 4: // RTS
				{
					desc.flags |= OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_END_SEQUENCE;
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
					desc.flags |= OPFLAG_END_SEQUENCE | OPFLAG_RETURN_TO_START;
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
			desc.flags |= OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_END_SEQUENCE;
			desc.targetpc = op & 0x3ff;
			return;
		}
		case 2: // JSR
		{
			desc.flags |= OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_END_SEQUENCE;
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
			desc.flags |= OPFLAG_WRITES_MEMORY;

			// Indirect
			if (op & 0x0010)
				desc.flags |= OPFLAG_READS_MEMORY;

			parse_operands(op, desc, 1);
			return;
		}
		case 5: // RBASE
		{
			return;
		}
		case 6: // MOVED
		{
			desc.flags |= OPFLAG_WRITES_MEMORY;
			parse_operands(op, desc, 1);
			return;
		}
		case 7: // MOVEI
		{
			desc.flags |= OPFLAG_READS_MEMORY | OPFLAG_WRITES_MEMORY;
			parse_operands(op, desc, 1);
			return;
		}
	}
}

void dspp_frontend::describe_branch(uint16_t op, opcode_desc &desc)
{
	const uint32_t select = (op >> 12) & 1;

	if (select == 0)
	{
		CC_N_USED(desc);
		CC_V_USED(desc);
	}
	else
	{
		CC_C_USED(desc);
		CC_Z_USED(desc);
	}

	// TODO: Can these be unconditional?
	desc.flags |= OPFLAG_IS_CONDITIONAL_BRANCH;
	desc.targetpc = op & 0x3ff;
}

void dspp_frontend::describe_complex_branch(uint16_t op, opcode_desc &desc)
{
	switch ((op >> 10) & 7)
	{
		case 0: // BLT
			CC_N_USED(desc);
			CC_V_USED(desc);
			break;
		case 1: // BLE
			CC_N_USED(desc);
			CC_V_USED(desc);
			CC_Z_USED(desc);
			break;
		case 2: // BGE
			CC_N_USED(desc);
			CC_V_USED(desc);
			break;
		case 3: // BGT
			CC_N_USED(desc);
			CC_V_USED(desc);
			CC_Z_USED(desc);
			break;
		case 4: // BHI
		case 5: // BLS
			CC_C_USED(desc);
			CC_Z_USED(desc);
			break;
		case 6: // BXS
		case 7: // BXC
			CC_X_USED(desc);
			break;
	}

	desc.flags |= OPFLAG_IS_CONDITIONAL_BRANCH;
	desc.targetpc = op & 0x3ff;
}

void dspp_frontend::describe_arithmetic(uint16_t op, opcode_desc &desc)
{
	// Decode the various fields
	uint32_t numops = (op >> 13) & 3;
	uint32_t muxa = (op >> 10) & 3;
	uint32_t muxb = (op >> 8) & 3;
	uint32_t alu_op = (op >> 4) & 0xf;
	uint32_t barrel_code = op & 0xf;

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
		desc.flags |= OPFLAG_READS_MEMORY;
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
			CC_C_MODIFIED(desc);
			CC_V_MODIFIED(desc);
			break;
		}
		case 3: // _+C
		case 5: // _-B
		{
			CC_C_USED(desc);
			CC_C_MODIFIED(desc);
			break;
		}
	}

	CC_N_MODIFIED(desc);
	CC_Z_MODIFIED(desc);
	CC_X_MODIFIED(desc);
}

void dspp_frontend::parse_operands(uint16_t op, opcode_desc &desc, uint32_t numops)
{
	uint32_t numregs = 0;
	uint32_t opidx = 0;

	while (opidx < numops)
	{
		uint16_t operand = desc.opptr.w[opidx + 1] = m_dspp->read_op(desc.physpc + opidx + 1);

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
					desc.flags |= OPFLAG_READS_MEMORY;

				if (operand & 0x0800 )// Write Back
					desc.flags |= OPFLAG_WRITES_MEMORY;

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
					desc.flags |= OPFLAG_READS_MEMORY;
				}

				if (numregs == 2)
				{
					// Write back
					if ((i == 0) && (operand & 0x1000))
						desc.flags |= OPFLAG_WRITES_MEMORY;
					else if ((i == 1) && (operand & 0x0800))
						desc.flags |= OPFLAG_WRITES_MEMORY;
				}
				else if (numregs == 1)
				{
					if (operand & 0x800)
						desc.flags |= OPFLAG_WRITES_MEMORY;
				}
				opidx++;
			}
			numregs = 0;
		}
	}
}
