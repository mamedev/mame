// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    arm7fe.hxx

    Front-end for ARM7 DRC

***************************************************************************/

#include "emu.h"
#include "arm7.h"


//**************************************************************************
//  ARM7 FRONTEND
//**************************************************************************

// register flags 0
#define REGFLAG_R(n)                    (((n) == 0) ? 0 : (1 << (n)))

//-------------------------------------------------
//  arm7_frontend - constructor
//-------------------------------------------------

arm7_frontend::arm7_frontend(arm7_cpu_device *arm7, uint32_t window_start, uint32_t window_end, uint32_t max_sequence)
	: drc_frontend(*arm7, window_start, window_end, max_sequence),
		m_arm7(arm7)
{
}


//-------------------------------------------------
//  describe_thumb - build a description of a
//  thumb instruction
//-------------------------------------------------

bool arm7_frontend::describe_thumb(opcode_desc &desc, const opcode_desc *prev)
{
	return false;
}

//-------------------------------------------------
//  describe_ops_* - build a description of
//  an ARM7 instruction
//-------------------------------------------------

bool arm7_frontend::describe_ops_0123(opcode_desc &desc, const opcode_desc *prev, uint32_t op)
{
	/* Branch and Exchange (BX) */
	if ((op & 0x0ffffff0) == 0x012fff10) // BX
	{
	}
	else if ((op & 0x0ff000f0) == 0x01600010) // CLZ - v5
	{
	}
	else if ((op & 0x0ff000f0) == 0x01000050) // QADD - v5
	{
	}
	else if ((op & 0x0ff000f0) == 0x01400050) // QDADD - v5
	{
	}
	else if ((op & 0x0ff000f0) == 0x01200050) // QSUB - v5
	{
	}
	else if ((op & 0x0ff000f0) == 0x01600050) // QDSUB - v5
	{
	}
	else if ((op & 0x0ff00090) == 0x01000080) // SMLAxy - v5
	{
	}
	else if ((op & 0x0ff00090) == 0x01400080) // SMLALxy - v5
	{
	}
	else if ((op & 0x0ff00090) == 0x01600080) // SMULxy - v5
	{
	}
	else if ((op & 0x0ff000b0) == 0x012000a0) // SMULWy - v5
	{
	}
	else if ((op & 0x0ff000b0) == 0x01200080) // SMLAWy - v5
	{
	}
	else if ((op & 0x0e000000) == 0 && (op & 0x80) && (op & 0x10)) // Multiply OR Swap OR Half Word Data Transfer
	{
		if (op & 0x60) // Half Word Data Transfer
		{
			//HandleHalfWordDT(op);
		}
		else if (op & 0x01000000) // Swap
		{
			//HandleSwap(op);
		}
		else // Multiply Or Multiply Long
		{
			if (op & 0x800000) // Multiply long
			{
				if (op & 0x00400000) // Signed multiply
				{
					//HandleSMulLong(op);
				}
				else // Unsigned multiply
				{
					//HandleUMulLong(op);
				}
			}
			else // Multiply
			{
				//HandleMul(op);//
			}
		}
	}
	else if ((op & 0x0c000000) == 0) // Data Processing OR PSR Transfer
	{
		if (((op & 0x00100000) == 0) && ((op & 0x01800000) == 0x01000000)) // PSR Transfer

		{
			//HandlePSRTransfer(insn);
			desc.cycles = 1;
		}
		else // Data processing
		{
			//HandleALU(insn);
		}
	}
	return true;
}

bool arm7_frontend::describe_ops_4567(opcode_desc &desc, const opcode_desc *prev, uint32_t op)
{
	if (op & INSN_I)
	{
		/* Register Shift */
		//off = decodeShift(op, nullptr);
	}
	else
	{
		/* Immediate Value */
		//off = op & INSN_SDT_IMM;
	}

	uint32_t rn = (op & INSN_RN) >> INSN_RN_SHIFT;

	if (op & INSN_SDT_P)
	{
		/* Pre-indexed addressing */
		if ((get_mode32()) || (rn != eR15))
		{
			//rnv = (GetRegister(rn) + off);
		}
		else
		{
			//rnv = (GET_PC + off);
		}

		if (op & INSN_SDT_W)
		{
			//rnv_old = GetRegister(rn);
			//SetRegister(rn, rnv);
		}
		else if (rn == eR15)
		{
			//rnv = rnv + 8;
		}
	}
	else
	{
		/* Post-indexed addressing */
		if (rn == eR15)
		{
			if (get_mode32())
			{
				//rnv = R15 + 8;
			}
			else
			{
				//rnv = GET_PC + 8;
			}
		}
		else
		{
			//rnv = GetRegister(rn);
		}
	}

	/* Do the transfer */
	uint32_t rd = (op & INSN_RD) >> INSN_RD_SHIFT;
	if (op & INSN_SDT_L)
	{
		/* Load */
		if (op & INSN_SDT_B)
		{
			//uint32_t data = READ8(rnv);
			//SetRegister(rd, data);
		}
		else
		{
			//uint32_t data = READ32(rnv);
			if (rd == eR15)
			{
				//R15 = data - 4;
				// LDR, PC takes 2S + 2N + 1I (5 total cycles)
				desc.cycles = 5;
			}
			else
			{
				//SetRegister(rd, data);
			}
		}
	}
	else
	{
		/* Store */
		if (op & INSN_SDT_B)
		{
			//WRITE8(rnv, (uint8_t) GetRegister(rd) & 0xffu);
		}
		else
		{
			//WRITE32(rnv, rd == eR15 ? R15 + 8 + 4 : GetRegister(rd)); // manual says STR rd = PC, +12
		}
		// Store takes only 2 N Cycles, so add + 1
		desc.cycles = 2;
	}

	/* Do post-indexing writeback */
	if (!(op & INSN_SDT_P)/* && (insn & INSN_SDT_W)*/)
	{
		if (rd == rn) {
			//SetRegister(rn, GetRegister(rd));
		}
		else {
			//SetRegister(rn, (rnv +/- off));
		}
	}
	return true;
}

bool arm7_frontend::describe_ops_89(opcode_desc &desc, const opcode_desc *prev, uint32_t op)
{
	if ((op & 0x005f0f00) == 0x004d0500)
	{
		// unsupported (armv6 onwards only)
		//arm9ops_undef(insn);
		//R15 += 4;
	}
	else if ((op & 0x00500f00) == 0x00100a00) /* Return From Exception (RFE) */
	{
		// unsupported (armv6 onwards only)
		//arm9ops_undef(insn);
		//R15 += 4;
	}
	else
	{
		//arm9ops_undef(insn);
		//R15 += 4;
	}
	return false;
}

bool arm7_frontend::describe_ops_ab(opcode_desc &desc, const opcode_desc *prev, uint32_t op)
{
	// BLX
	//HandleBranch(insn, true);
	//set_cpsr(GET_CPSR|T_MASK);
	return true;
}

bool arm7_frontend::describe_ops_cd(opcode_desc &desc, const opcode_desc *prev, uint32_t op)
{
	/* Additional coprocessor double register transfer */
	if ((op & 0x00e00000) == 0x00400000)
	{
		// unsupported
		//arm9ops_undef(insn);
		//R15 += 4;
	}
	else
	{
		//arm9ops_undef(insn);
		//R15 += 4;
	}
	return false;
}

bool arm7_frontend::describe_ops_e(opcode_desc &desc, const opcode_desc *prev, uint32_t op)
{
	/* Additional coprocessor register transfer */
	// unsupported
	//arm9ops_undef(insn);
	//R15 += 4;
	return false;
}

bool arm7_frontend::describe_ops_f(opcode_desc &desc, const opcode_desc *prev, uint32_t op)
{
	return false;
}

//-------------------------------------------------
//  describe_arm9_ops_* - build a description of
//  an ARM9 instruction
//-------------------------------------------------

bool describe_arm9_ops_1(opcode_desc &desc, const opcode_desc *prev, uint32_t op)
{
	/* Change processor state (CPS) */
	if ((op & 0x00f10020) == 0x00000000)
	{
		// unsupported (armv6 onwards only)
		//arm9ops_undef(insn);
		//R15 += 4;
	}
	else if ((op & 0x00ff00f0) == 0x00010000) /* set endianness (SETEND) */
	{
		// unsupported (armv6 onwards only)
		//arm9ops_undef(insn);
		//R15 += 4;
	}
	else
	{
		//arm9ops_undef(insn);
		//R15 += 4;
	}
	return false;
}

bool describe_arm9_ops_57(opcode_desc &desc, const opcode_desc *prev, uint32_t op)
{
	/* Cache Preload (PLD) */
	if ((op & 0x0070f000) == 0x0050f000)
	{
		// unsupported (armv6 onwards only)
		//arm9ops_undef(insn);
		//R15 += 4;
	}
	else
	{
		//arm9ops_undef(insn);
		//R15 += 4;
	}
	return false;
}

bool describe_arm9_ops_89(opcode_desc &desc, const opcode_desc *prev, uint32_t op)
{
	return false;
}

bool describe_arm9_ops_ab(opcode_desc &desc, const opcode_desc *prev, uint32_t op)
{
	return false;
}

bool describe_arm9_ops_c(opcode_desc &desc, const opcode_desc *prev, uint32_t op)
{
	return false;
}

bool describe_arm9_ops_e(opcode_desc &desc, const opcode_desc *prev, uint32_t op)
{
	return false;
}


uint32_t arm7_frontend::get_cpsr()
{
	return m_arm7->m_r[eCPSR];
}

bool arm7_frontend::get_mode32()
{
	return m_arm7->m_r[eCPSR] & SR_MODE32;
}

//-------------------------------------------------
//  describe - build a description of a single
//  instruction
//-------------------------------------------------

bool arm7_frontend::describe(opcode_desc &desc, const opcode_desc *prev)
{
	// compute the physical PC
	const uint32_t cpsr = get_cpsr();
	assert((desc.physpc & (T_IS_SET(cpsr) ? 1 : 3)) == 0);
	if (!m_arm7->arm7_tlb_translate(desc.physpc, ARM7_TLB_ABORT_P | ARM7_TLB_READ))
	{
		// uh-oh: a page fault; leave the description empty and just if this is the first instruction, leave it empty and
		// mark as needing to validate; otherwise, just end the sequence here
		desc.flags |= OPFLAG_VALIDATE_TLB | OPFLAG_CAN_CAUSE_EXCEPTION | OPFLAG_COMPILER_PAGE_FAULT | OPFLAG_VIRTUAL_NOOP | OPFLAG_END_SEQUENCE;
		return true;
	}

	if (T_IS_SET(cpsr))
	{
		return describe_thumb(desc, prev);
	}

	// fetch the opcode
	uint32_t op = desc.opptr.l[0] = m_arm7->m_direct->read_dword(desc.physpc);

	// all non-THUMB instructions are 4 bytes and default to 3 cycles each
	desc.length = 4;
	desc.cycles = 3;

	// parse the instruction

	int op_offset = 0;
	if ((op >> INSN_COND_SHIFT) == COND_NV && m_arm7->m_archRev >= 5)
	{
		op_offset = 0x10;
	}

	switch (((op & 0xF000000) >> 24) + op_offset)
	{
		case 0x0: case 0x1: case 0x2: case 0x3:
			return describe_ops_0123(desc, prev, op);

		case 0x4: case 0x5: case 0x6: case 0x7:
			return describe_ops_4567(desc, prev, op);

		case 0x8: case 0x9:
			return describe_ops_89(desc, prev, op);

		case 0xa: case 0xb:
			return describe_ops_ab(desc, prev, op);

		case 0xc: case 0xd:
			return describe_ops_cd(desc, prev, op);

		case 0xe:
			return describe_ops_e(desc, prev, op);

		case 0xf:
			return describe_ops_f(desc, prev, op);

		case 0x11: // ARM9
			return describe_arm9_ops_1(desc, prev, op);

		case 0x15: case 0x17: // ARM9
			return describe_arm9_ops_57(desc, prev, op);

		case 0x18: case 0x19: // ARM9
			return describe_arm9_ops_89(desc, prev, op);

		case 0x1a: case 0x1b: // ARM9
			return describe_arm9_ops_ab(desc, prev, op);

		case 0x1c: // ARM9
			return describe_arm9_ops_c(desc, prev, op);

		case 0x1e: // ARM9
			return describe_arm9_ops_e(desc, prev, op);

		default:
			return false;
	}
}
