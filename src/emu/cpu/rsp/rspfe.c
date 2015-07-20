// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    rspfe.c

    Front-end for RSP recompiler

***************************************************************************/

#include "emu.h"
#include "rspfe.h"
#include "rsp.h"

//**************************************************************************
//  RSP FRONTEND
//**************************************************************************

//-------------------------------------------------
//  rsp_frontend - constructor
//-------------------------------------------------

rsp_frontend::rsp_frontend(rsp_device &rsp, UINT32 window_start, UINT32 window_end, UINT32 max_sequence)
	: drc_frontend(rsp, window_start, window_end, max_sequence),
		m_rsp(rsp)
{
}


//-------------------------------------------------
//  describe - build a description of a single
//  instruction
//-------------------------------------------------

bool rsp_frontend::describe(opcode_desc &desc, const opcode_desc *prev)
{
	UINT32 op, opswitch;

	// fetch the opcode
	op = desc.opptr.l[0] = m_rsp.m_direct->read_dword(desc.physpc | 0x1000);

	// all instructions are 4 bytes and default to a single cycle each
	desc.length = 4;
	desc.cycles = 1;

	// parse the instruction
	opswitch = op >> 26;
	switch (opswitch)
	{
		case 0x00:  // SPECIAL
			return describe_special(op, desc);

		case 0x01:  // REGIMM
			return describe_regimm(op, desc);

		case 0x10:  // COP0
			return describe_cop0(op, desc);

		case 0x12:  // COP2
			return describe_cop2(op, desc);

		case 0x02:  // J
			desc.flags |= OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_END_SEQUENCE;
			desc.targetpc = ((LIMMVAL << 2) & 0x00000fff) | 0x1000;
			desc.delayslots = 1;
			return true;

		case 0x03:  // JAL
			desc.regout[0] |= REGFLAG_R(31);
			desc.flags |= OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_END_SEQUENCE;
			desc.targetpc = ((LIMMVAL << 2) & 0x00000fff) | 0x1000;
			desc.delayslots = 1;
			return true;

		case 0x04:  // BEQ
		case 0x05:  // BNE
			if ((opswitch == 0x04 || opswitch == 0x14) && RSREG == RTREG)
				desc.flags |= OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_END_SEQUENCE;
			else
			{
				desc.regin[0] |= REGFLAG_R(RSREG) | REGFLAG_R(RTREG);
				desc.flags |= OPFLAG_IS_CONDITIONAL_BRANCH;
			}
			desc.targetpc = ((desc.pc + 4 + SIMMVAL * 4) & 0x00000fff) | 0x1000;
			desc.delayslots = 1;
			desc.skipslots = (opswitch & 0x10) ? 1 : 0;
			return true;

		case 0x06:  // BLEZ
		case 0x07:  // BGTZ
			if ((opswitch == 0x06 || opswitch == 0x16) && RSREG == 0)
				desc.flags |= OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_END_SEQUENCE;
			else
			{
				desc.regin[0] |= REGFLAG_R(RSREG);
				desc.flags |= OPFLAG_IS_CONDITIONAL_BRANCH;
			}
			desc.targetpc = ((desc.pc + 4 + SIMMVAL * 4) & 0x00000fff) | 0x1000;
			desc.delayslots = 1;
			desc.skipslots = (opswitch & 0x10) ? 1 : 0;
			return true;

		case 0x08:  // ADDI
			desc.regin[0] |= REGFLAG_R(RSREG);
			desc.regout[0] |= REGFLAG_R(RTREG);
			return true;

		case 0x09:  // ADDIU
		case 0x0a:  // SLTI
		case 0x0b:  // SLTIU
		case 0x0c:  // ANDI
		case 0x0d:  // ORI
		case 0x0e:  // XORI
			desc.regin[0] |= REGFLAG_R(RSREG);
			desc.regout[0] |= REGFLAG_R(RTREG);
			return true;

		case 0x0f:  // LUI
			desc.regout[0] |= REGFLAG_R(RTREG);
			return true;

		case 0x20:  // LB
		case 0x21:  // LH
		case 0x23:  // LW
		case 0x24:  // LBU
		case 0x25:  // LHU
		case 0x27:  // LWU
			desc.regin[0] |= REGFLAG_R(RSREG);
			desc.regout[0] |= REGFLAG_R(RTREG);
			desc.flags |= OPFLAG_READS_MEMORY;
			return true;

		case 0x28:  // SB
		case 0x29:  // SH
		case 0x2b:  // SW
			desc.regin[0] |= REGFLAG_R(RSREG) | REGFLAG_R(RTREG);
			desc.flags |= OPFLAG_WRITES_MEMORY;
			return true;

		case 0x32:  // LWC2
			desc.regin[0] |= REGFLAG_R(RSREG);
			desc.flags |= OPFLAG_READS_MEMORY;
			return true;

		case 0x3a:  // SWC2
			desc.regin[0] |= REGFLAG_R(RSREG);
			desc.flags |= OPFLAG_WRITES_MEMORY;
			return true;
	}

	return false;
}


//-------------------------------------------------
//  describe_special - build a description of a
//  single instruction in the 'special' group
//-------------------------------------------------

bool rsp_frontend::describe_special(UINT32 op, opcode_desc &desc)
{
	switch (op & 63)
	{
		case 0x00:  // SLL
		case 0x02:  // SRL
		case 0x03:  // SRA
			desc.regin[0] |= REGFLAG_R(RTREG);
			desc.regout[0] |= REGFLAG_R(RDREG);
			return true;

		case 0x04:  // SLLV
		case 0x06:  // SRLV
		case 0x07:  // SRAV
		case 0x21:  // ADDU
		case 0x23:  // SUBU
		case 0x24:  // AND
		case 0x25:  // OR
		case 0x26:  // XOR
		case 0x27:  // NOR
		case 0x2a:  // SLT
		case 0x2b:  // SLTU
			desc.regin[0] |= REGFLAG_R(RSREG) | REGFLAG_R(RTREG);
			desc.regout[0] |= REGFLAG_R(RDREG);
			return true;

		case 0x20:  // ADD
		case 0x22:  // SUB
			desc.regin[0] |= REGFLAG_R(RSREG) | REGFLAG_R(RTREG);
			desc.regout[0] |= REGFLAG_R(RDREG);
			return true;

		case 0x08:  // JR
			desc.regin[0] |= REGFLAG_R(RSREG);
			desc.flags |= OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_END_SEQUENCE;
			desc.targetpc = BRANCH_TARGET_DYNAMIC;
			desc.delayslots = 1;
			return true;

		case 0x09:  // JALR
			desc.regin[0] |= REGFLAG_R(RSREG);
			desc.regout[0] |= REGFLAG_R(RDREG);
			desc.flags |= OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_END_SEQUENCE;
			desc.targetpc = BRANCH_TARGET_DYNAMIC;
			desc.delayslots = 1;
			return true;

		case 0x0d:  // BREAK
			desc.flags |= OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_END_SEQUENCE;
			desc.targetpc = (op >> 5) & 0x000fffff;
			return true;
	}

	return false;
}


//-------------------------------------------------
//  describe_regimm - build a description of a
//  single instruction in the 'regimm' group
//-------------------------------------------------

bool rsp_frontend::describe_regimm(UINT32 op, opcode_desc &desc)
{
	switch (RTREG)
	{
		case 0x00:  // BLTZ
		case 0x01:  // BGEZ
			if (RTREG == 0x01 && RSREG == 0)
				desc.flags |= OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_END_SEQUENCE;
			else
			{
				desc.regin[0] |= REGFLAG_R(RSREG);
				desc.flags |= OPFLAG_IS_CONDITIONAL_BRANCH;
			}
			desc.targetpc = ((desc.pc + 4 + SIMMVAL * 4) & 0x00000fff) | 0x1000;
			desc.delayslots = 1;
			desc.skipslots = (RTREG & 0x02) ? 1 : 0;
			return true;

		case 0x10:  // BLTZAL
		case 0x11:  // BGEZAL
			if (RTREG == 0x11 && RSREG == 0)
				desc.flags |= OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_END_SEQUENCE;
			else
			{
				desc.regin[0] |= REGFLAG_R(RSREG);
				desc.flags |= OPFLAG_IS_CONDITIONAL_BRANCH;
			}
			desc.regout[0] |= REGFLAG_R(31);
			desc.targetpc = ((desc.pc + 4 + SIMMVAL * 4) & 0x00000fff) | 0x1000;
			desc.delayslots = 1;
			desc.skipslots = (RTREG & 0x02) ? 1 : 0;
			return true;
	}

	return false;
}


//-------------------------------------------------
//  describe_cop0 - build a description of a
//  single instruction in the COP0 group
//-------------------------------------------------

bool rsp_frontend::describe_cop0(UINT32 op, opcode_desc &desc)
{
	switch (RSREG)
	{
		case 0x00:  // MFCz
			desc.regout[0] |= REGFLAG_R(RTREG);
			return true;

		case 0x04:  // MTCz
			desc.regin[0] |= REGFLAG_R(RTREG);
			if(RDREG == 2) // SP_RD_LEN, initiating DMA
			{
				desc.flags |= OPFLAG_END_SEQUENCE;
			}
			return true;
	}

	return false;
}

//-------------------------------------------------
//  describe_cop2 - build a description of a
//  single instruction in the COP2 group
//-------------------------------------------------

bool rsp_frontend::describe_cop2(UINT32 op, opcode_desc &desc)
{
	switch (RSREG)
	{
		case 0x00:  // MFCz
		case 0x02:  // CFCz
			desc.regout[0] |= REGFLAG_R(RTREG);
			return true;

		case 0x04:  // MTCz
		case 0x06:  // CTCz
			desc.regin[0] |= REGFLAG_R(RTREG);
			return true;
	}

	return false;
}
