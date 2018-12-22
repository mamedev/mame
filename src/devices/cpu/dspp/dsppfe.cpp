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
//#define MULT_FLAGS_MODIFIED(desc)   do { MN_MODIFIED(desc);MV_MODIFIED(desc);MU_MODIFIED(desc);MI_MODIFIED(desc); } while(0)
//#define SHIFT_FLAGS_MODIFIED(desc)  do { SZ_MODIFIED(desc);SV_MODIFIED(desc);SS_MODIFIED(desc); } while(0)

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

	// TODO: NOOOOPE
	desc.cycles = 1; // TODO: Extra cycles for extra operands
	desc.length = 2;

	// Decode and execute
	if (op & 0x8000)
	{
		switch ((op >> 13) & 3)
		{
			case 0:
				return describe_special(op, desc);

			case 1:
			case 2:
				return describe_branch(op, desc);

			case 3:
				return describe_complex_branch(op, desc);
		}

		return false;
	}
	else
	{
		return describe_arithmetic(op, desc);
	}

	return false;
}


bool dspp_frontend::describe_special(uint16_t op, opcode_desc &desc)
{
	switch ((op >> 10) & 7)
	{
		case 0:
		{
			// Super-special
			switch ((op >> 7) & 7)
			{
				case 1:	// BAC - TODO: MERGE?
				{
					//desc.regin[0] = m_acc;
					desc.flags |= OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_END_SEQUENCE;
					desc.targetpc = BRANCH_TARGET_DYNAMIC;
					return true;
				}
				case 4: // RTS
				{
					desc.flags |= OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_END_SEQUENCE;
					desc.targetpc = BRANCH_TARGET_DYNAMIC;
					return true;
				}
				case 5: // OP_MASK
				{
					// TODO
					return true;
				}

				case 7: // SLEEP
				{
					// TODO
					return true;
				}

				case 0: // NOP
				case 2: // Unused
				case 3:
				case 6:
					return true;
			}

			break;
		}
		case 1: // JUMP  - TODO: MERGE?
		{
			desc.flags |= OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_END_SEQUENCE;
			desc.targetpc = op & 0x3ff;
			return true;
		}
		case 2: // JSR
		{
			desc.flags |= OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_END_SEQUENCE;
			desc.targetpc = op & 0x3ff;
			return true;
		}
		case 3: // BFM
		{
			// TODO: What sort of branch is this?
//			desc.flags |= OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_END_SEQUENCE;
//			desc.targetpc = 0; // FIXME
			return false;
		}
		case 4: // MOVEREG
		{
			desc.flags |= OPFLAG_WRITES_MEMORY;

			// Indirect
			if (op & 0x0010)
				desc.flags |= OPFLAG_READS_MEMORY;

			return true;
		}
		case 5: // RBASE
		{
			return true;
		}
		case 6: // MOVED
		{
			desc.flags |= OPFLAG_WRITES_MEMORY;
			return true;
		}
		case 7: // MOVEI
		{
			desc.flags |= OPFLAG_READS_MEMORY | OPFLAG_WRITES_MEMORY;
			return true;
		}
	}

	return false;
}

bool dspp_frontend::describe_branch(uint16_t op, opcode_desc &desc)
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

	return true;
}

bool dspp_frontend::describe_complex_branch(uint16_t op, opcode_desc &desc)
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

	return true;
}

bool dspp_frontend::describe_arithmetic(uint16_t op, opcode_desc &desc)
{
	#if 0
	// Decode the various fields
	uint32_t numops = (op >> 13) & 3;
	uint32_t muxa = (op >> 10) & 3;
	uint32_t muxb = (op >> 8) & 3;
	uint32_t alu_op = (op >> 4) & 0xf;
	uint32_t barrel_code = op & 0xf;

	int32_t mul_res = 0;
	uint32_t alu_res = 0;

	// Check for operand overflow
	if (numops == 0 && ((muxa == 1) || (muxa == 2) || (muxb == 1) || (muxb == 2)))
		numops = 4;

	// Implicit barrel shift
	if (barrel_code == 8)
		++numops;

	// TODO: We need to know:
	// Number of cycles
	// Number of bytes
	// Registers read
	// Registers written
	// Does it read memory?
	// Does it write memory?

//	parse_operands(numops);

	if (muxa == 3 || muxb == 3)
	{
		uint32_t mul_sel = (op >> 12) & 1;

		int32_t op1 = sign_extend16(read_next_operand());
		int32_t op2 = sign_extend16(mul_sel ? read_next_operand() : m_core->m_acc >> 4);

		mul_res = (op1 * op2) >> 11;
	}

	int32_t alu_a, alu_b;

	switch (muxa)
	{
		case 0:
		{
			ACCUMULATOR_USED
			alu_a = m_core->m_acc;
			break;
		}
		case 1: case 2:
		{
			alu_a = read_next_operand() << 4;
			break;
		}
		case 3:
		{
			alu_a = mul_res;
			break;
		}
	}

	switch (muxb)
	{
		case 0:
		{
			ACCUMULATOR_USED
			alu_b = m_core->m_acc;
			break;
		}
		case 1: case 2:
		{
			alu_b = read_next_operand() << 4;
			break;
		}
		case 3:
		{
			alu_b = mul_res;
			break;
		}
	}

	// All flags ar emodifed
	CC_FLAGS_MODIFIED(desc);

	switch (alu_op)
	{
		case 0:	// _TRA
		{
//			alu_res = alu_a;
			break;
		}
		case 1:	// _NEG
		{
//			alu_res = -alu_b;
			break;
		}
		case 2:	// _+
		{
//			alu_res = alu_a + alu_b;

//			if ((alu_a & 0x80000) == (alu_b & 0x80000) &&
//				(alu_a & 0x80000) != (alu_res & 0x80000))
//				m_core->m_flags |= DSPI_FLAG_CC_OVER;

//			if (alu_res & 0x00100000)
//				m_core->m_flags |= DSPI_FLAG_CC_CARRY;

//			CC_V_MODIFIED(desc);
//			CC_C_MODIFIED(desc);
			break;
		}
		case 3:	// _+C
		{
//			alu_res = alu_a + (m_core->m_flags & DSPI_FLAG_CC_CARRY) ? (1 << 4) : 0;

//			if (alu_res & 0x00100000)
//				m_core->m_flags |= DSPI_FLAG_CC_CARRY;

			CC_C_USED(desc);
			CC_C_MODIFIED(desc);
			break;
		}
		case 4:	// _-
		{
//			alu_res = alu_a - alu_b;

//			if ((alu_a & 0x80000) == (~alu_b & 0x80000) &&
//				(alu_a & 0x80000) != (alu_res & 0x80000))
//				m_core->m_flags |= DSPI_FLAG_CC_OVER;

//			if (alu_res & 0x00100000)
//				m_core->m_flags |= DSPI_FLAG_CC_CARRY;

			CC_C_MODIFIED(desc);
			CC_V_MODIFIED(desc);
			break;
		}
		case 5:	// _-B
		{
//			alu_res = alu_a - (m_core->m_flags & DSPI_FLAG_CC_CARRY) ? (1 << 4) : 0;

//			if (alu_res & 0x00100000)
//				m_core->m_flags |= DSPI_FLAG_CC_CARRY;

			CC_C_USED(desc);
			CC_C_MODIFIED(desc);
			break;
		}
		case 6:	// _++
		{
//			alu_res = alu_a + 1;

//			if (!(alu_a & 0x80000) && (alu_res & 0x80000))
//				m_core->m_flags |= DSPI_FLAG_CC_OVER;

			CC_V_MODIFIED(desc);
			break;
		}
		case 7:	// _--
		{
//			alu_res = alu_a - 1;

//			if ((alu_a & 0x80000) && !(alu_res & 0x80000))
//				m_core->m_flags |= DSPI_FLAG_CC_OVER;

			CC_V_MODIFIED(desc);
			break;
		}
		case 8:	// _TRL
		{
			//alu_res = alu_a;
			break;
		}
		case 9:	// _NOT
		{
			//alu_res = ~alu_a;
			break;
		}
		case 10: // _AND
		{
			//alu_res = alu_a & alu_b;
			break;
		}
		case 11: // _NAND
		{
			//alu_res = ~(alu_a & alu_b);
			break;
		}
		case 12: // _OR
		{
			//alu_res = alu_a | alu_b;
			break;
		}
		case 13: // _NOR
		{
			//alu_res = ~(alu_a | alu_b);
			break;
		}
		case 14: // _XOR
		{
			//alu_res = alu_a ^ alu_b;
			break;
		}
		case 15: // _XNOR
		{
			//alu_res = ~(alu_a ^ alu_b);
			break;
		}
	}


	if (alu_res & 0x00080000)
		m_core->m_flags |= DSPI_FLAG_CC_NEG;

	if ((alu_res & 0x000ffff0) == 0)
		m_core->m_flags |= DSPI_FLAG_CC_ZERO;

	if ((alu_res & 0x0000000f) == 0)
		m_core->m_flags |= DSPI_FLAG_CC_EXACT;

	CC_N_MODIFIED(desc);
	CC_Z_MODIFIED(desc);
	CC_X_MODIFIED(desc);

	ACCUMULATOR_MODIFIED_ALWAYS;

	// Barrel shift
	static const int32_t shifts[8] = { 0, 1, 2, 3, 4, 5, 8, 16 };

	if (barrel_code == 8)
		barrel_code = read_next_operand();

	if (barrel_code & 8)
	{
		// Right shift
		uint32_t shift = shifts[(~barrel_code + 1) & 7];

		if (alu_op < 8)
		{
			// Arithmetic
			m_core->m_acc = sign_extend20(alu_res) >> shift;
		}
		else
		{
			// Logical
			m_core->m_acc = (alu_res & 0xfffff) >> shift;
		}
	}
	else
	{
		// Left shift
		uint32_t shift = shifts[barrel_code];

		if (shift == 16)
		{
			// Clip and saturate
			if (m_core->m_flags & DSPI_FLAG_CC_OVER)
				m_core->m_acc = (m_core->m_flags & DSPI_FLAG_CC_NEG) ? 0x7ffff : 0xfff80000;
			else
				m_core->m_acc = sign_extend20(alu_res);
		}
		else
		{
			m_core->m_acc = sign_extend20(alu_res) << shift;
		}
	}

	if (m_core->m_writeback >= 0)
	{
		write_data(m_core->m_writeback, m_core->m_acc >> 4);
		m_core->m_writeback = -1;
	}
	else if (opidx < numops)
	{
		write_next_operand(m_core->m_acc >> 4);
	}
#endif
		return true;
}


#if 0
void dspp_device::parse_operands(uint32_t numops)
{
	uint32_t addr, val = 0xBAD;
	uint32_t opidx = 0;
	uint32_t operand = 0;
	uint32_t numregs = 0;

	uint32_t opidx = 0;

	while (opidx < numops)
	{
		uint16_t op = desc.opptr.w[opidx + 1] = m_dspp->read_op(desc.physpc + opidx * 2);

		desc.length += 2;
		++desc.cycles;

		if (op & 0x8000)
		{
			// Immediate value
			if ((op & 0xc000) == 0xc000)
			{
				// Nothing?-
			}
			else if((op & 0xe000) == 0x8000)
			{
				if (op & 0x0400) // Indirect
					desc.flags |= OPFLAG_READS_MEMORY;

				if (op & 0x0800 )// Write Back
					desc.flags |= OPFLAG_WRITES_MEMORY;

				++opidx;
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
				// OP USES REGBASE?

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
			}
			numregs = 0;
		}
	}
}
#endif
