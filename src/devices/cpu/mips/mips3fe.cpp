// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    mips3fe.c

    Front-end for MIPS3 recompiler

***************************************************************************/

#include "emu.h"
#include "mips3fe.h"
#include "mips3com.h"


//**************************************************************************
//  MIPS3 FRONTEND
//**************************************************************************

//-------------------------------------------------
//  mips3_frontend - constructor
//-------------------------------------------------

mips3_frontend::mips3_frontend(mips3_device *mips3, UINT32 window_start, UINT32 window_end, UINT32 max_sequence)
	: drc_frontend(*mips3, window_start, window_end, max_sequence),
		m_mips3(mips3)
{
}


//-------------------------------------------------
//  describe - build a description of a single
//  instruction
//-------------------------------------------------

bool mips3_frontend::describe(opcode_desc &desc, const opcode_desc *prev)
{
	UINT32 op, opswitch;

	// compute the physical PC
	assert((desc.physpc & 3) == 0);
	if (!m_mips3->memory_translate(AS_PROGRAM, TRANSLATE_FETCH, desc.physpc))
	{
		// uh-oh: a page fault; leave the description empty and just if this is the first instruction, leave it empty and
		// mark as needing to validate; otherwise, just end the sequence here
		desc.flags |= OPFLAG_VALIDATE_TLB | OPFLAG_CAN_CAUSE_EXCEPTION | OPFLAG_COMPILER_PAGE_FAULT | OPFLAG_VIRTUAL_NOOP | OPFLAG_END_SEQUENCE;
		return true;
	}

	// fetch the opcode
	assert((desc.physpc & 3) == 0);
	op = desc.opptr.l[0] = m_mips3->m_direct->read_dword(desc.physpc);

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

		case 0x11:  // COP1
			return describe_cop1(op, desc);

		case 0x12:  // COP2
			return describe_cop2(op, desc);

		case 0x13:  // COP1X - MIPS IV
			if (m_mips3->m_flavor < mips3_device::MIPS3_TYPE_MIPS_IV)
				return false;
			return describe_cop1x(op, desc);

		case 0x1c:  // IDT-specific opcodes: mad/madu/mul on R4640/4650, msub on RC32364
			return describe_idt(op, desc);

		case 0x02:  // J
			desc.flags |= OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_END_SEQUENCE;
			desc.targetpc = (desc.pc & 0xf0000000) | (LIMMVAL << 2);
			desc.delayslots = 1;
			return true;

		case 0x03:  // JAL
			desc.regout[0] |= REGFLAG_R(31);
			desc.flags |= OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_END_SEQUENCE;
			desc.targetpc = (desc.pc & 0xf0000000) | (LIMMVAL << 2);
			desc.delayslots = 1;
			return true;

		case 0x04:  // BEQ
		case 0x05:  // BNE
		case 0x14:  // BEQL
		case 0x15:  // BNEL
			if ((opswitch == 0x04 || opswitch == 0x14) && RSREG == RTREG)
				desc.flags |= OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_END_SEQUENCE;
			else
			{
				desc.regin[0] |= REGFLAG_R(RSREG) | REGFLAG_R(RTREG);
				desc.flags |= OPFLAG_IS_CONDITIONAL_BRANCH;
			}
			desc.targetpc = desc.pc + 4 + SIMMVAL * 4;
			desc.delayslots = 1;
			desc.skipslots = (opswitch & 0x10) ? 1 : 0;
			return true;

		case 0x06:  // BLEZ
		case 0x07:  // BGTZ
		case 0x16:  // BLEZL
		case 0x17:  // BGTZL
			if ((opswitch == 0x06 || opswitch == 0x16) && RSREG == 0)
				desc.flags |= OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_END_SEQUENCE;
			else
			{
				desc.regin[0] |= REGFLAG_R(RSREG);
				desc.flags |= OPFLAG_IS_CONDITIONAL_BRANCH;
			}
			desc.targetpc = desc.pc + 4 + SIMMVAL * 4;
			desc.delayslots = 1;
			desc.skipslots = (opswitch & 0x10) ? 1 : 0;
			return true;

		case 0x08:  // ADDI
		case 0x18:  // DADDI
			desc.regin[0] |= REGFLAG_R(RSREG);
			desc.regout[0] |= REGFLAG_R(RTREG);
			desc.flags |= OPFLAG_CAN_CAUSE_EXCEPTION;
			return true;

		case 0x09:  // ADDIU
		case 0x0a:  // SLTI
		case 0x0b:  // SLTIU
		case 0x0c:  // ANDI
		case 0x0d:  // ORI
		case 0x0e:  // XORI
		case 0x19:  // DADDIU
			desc.regin[0] |= REGFLAG_R(RSREG);
			desc.regout[0] |= REGFLAG_R(RTREG);
			return true;

		case 0x0f:  // LUI
			desc.regout[0] |= REGFLAG_R(RTREG);
			return true;

		case 0x1a:  // LDL
		case 0x1b:  // LDR
		case 0x22:  // LWL
		case 0x26:  // LWR
			desc.regin[0] |= REGFLAG_R(RTREG);
		case 0x20:  // LB
		case 0x21:  // LH
		case 0x23:  // LW
		case 0x24:  // LBU
		case 0x25:  // LHU
		case 0x27:  // LWU
		case 0x30:  // LL
		case 0x34:  // LLD
		case 0x37:  // LD
			desc.regin[0] |= REGFLAG_R(RSREG);
			desc.regout[0] |= REGFLAG_R(RTREG);
			desc.flags |= OPFLAG_READS_MEMORY | OPFLAG_CAN_CAUSE_EXCEPTION;
			return true;

		case 0x28:  // SB
		case 0x29:  // SH
		case 0x2a:  // SWL
		case 0x2b:  // SW
		case 0x2c:  // SDL
		case 0x2d:  // SDR
		case 0x2e:  // SWR
		case 0x3f:  // SD
			desc.regin[0] |= REGFLAG_R(RSREG) | REGFLAG_R(RTREG);
			desc.flags |= OPFLAG_WRITES_MEMORY | OPFLAG_CAN_CAUSE_EXCEPTION;
			return true;

		case 0x38:  // SC
		case 0x3c:  // SCD
			desc.regin[0] |= REGFLAG_R(RSREG) | REGFLAG_R(RTREG);
			desc.regout[0] |= REGFLAG_R(RTREG);
			desc.flags |= OPFLAG_WRITES_MEMORY | OPFLAG_CAN_CAUSE_EXCEPTION;
			return true;

		case 0x31:  // LWC1
		case 0x35:  // LDC1
			desc.regin[0] |= REGFLAG_R(RSREG);
			desc.regout[1] |= REGFLAG_CPR1(RTREG);
			desc.flags |= OPFLAG_READS_MEMORY | OPFLAG_CAN_CAUSE_EXCEPTION;
			return true;

		case 0x39:  // SWC1
		case 0x3d:  // SDC1
			desc.regin[0] |= REGFLAG_R(RSREG);
			desc.regin[1] |= REGFLAG_CPR1(RTREG);
			desc.flags |= OPFLAG_WRITES_MEMORY | OPFLAG_CAN_CAUSE_EXCEPTION;
			return true;

		case 0x32:  // LWC2
		case 0x36:  // LDC2
			desc.regin[0] |= REGFLAG_R(RSREG);
			desc.flags |= OPFLAG_READS_MEMORY | OPFLAG_CAN_CAUSE_EXCEPTION;
			return true;

		case 0x3a:  // SWC2
		case 0x3e:  // SDC2
			desc.regin[0] |= REGFLAG_R(RSREG);
			desc.flags |= OPFLAG_WRITES_MEMORY | OPFLAG_CAN_CAUSE_EXCEPTION;
			return true;

		case 0x33:  // PREF
			if (m_mips3->m_flavor < mips3_device::MIPS3_TYPE_MIPS_IV)
				return false;
		case 0x2f:  // CACHE
			// effective no-op
			return true;
	}

	return false;
}


//-------------------------------------------------
//  describe_special - build a description of a
//  single instruction in the 'special' group
//-------------------------------------------------

bool mips3_frontend::describe_special(UINT32 op, opcode_desc &desc)
{
	switch (op & 63)
	{
		case 0x00:  // SLL
		case 0x02:  // SRL
		case 0x03:  // SRA
		case 0x38:  // DSLL
		case 0x3a:  // DSRL
		case 0x3b:  // DSRA
		case 0x3c:  // DSLL32
		case 0x3e:  // DSRL32
		case 0x3f:  // DSRA32
			desc.regin[0] |= REGFLAG_R(RTREG);
			desc.regout[0] |= REGFLAG_R(RDREG);
			return true;

		case 0x0a:  // MOVZ - MIPS IV
		case 0x0b:  // MOVN - MIPS IV
			if (m_mips3->m_flavor < mips3_device::MIPS3_TYPE_MIPS_IV)
				return false;
			desc.regin[0] |= REGFLAG_R(RDREG);
		case 0x04:  // SLLV
		case 0x06:  // SRLV
		case 0x07:  // SRAV
		case 0x14:  // DSLLV
		case 0x16:  // DSRLV
		case 0x17:  // DSRAV
		case 0x21:  // ADDU
		case 0x23:  // SUBU
		case 0x24:  // AND
		case 0x25:  // OR
		case 0x26:  // XOR
		case 0x27:  // NOR
		case 0x2a:  // SLT
		case 0x2b:  // SLTU
		case 0x2d:  // DADDU
		case 0x2f:  // DSUBU
			desc.regin[0] |= REGFLAG_R(RSREG) | REGFLAG_R(RTREG);
			desc.regout[0] |= REGFLAG_R(RDREG);
			return true;

		case 0x20:  // ADD
		case 0x22:  // SUB
		case 0x2c:  // DADD
		case 0x2e:  // DSUB
			desc.regin[0] |= REGFLAG_R(RSREG) | REGFLAG_R(RTREG);
			desc.regout[0] |= REGFLAG_R(RDREG);
			desc.flags |= OPFLAG_CAN_CAUSE_EXCEPTION;
			return true;

		case 0x30:  // TGE
		case 0x31:  // TGEU
		case 0x32:  // TLT
		case 0x33:  // TLTU
		case 0x34:  // TEQ
		case 0x36:  // TNE
			desc.regin[0] |= REGFLAG_R(RSREG) | REGFLAG_R(RTREG);
			desc.flags |= OPFLAG_CAN_CAUSE_EXCEPTION;
			return true;

		case 0x01:  // MOVF - MIPS IV
			if (m_mips3->m_flavor < mips3_device::MIPS3_TYPE_MIPS_IV)
				return false;
			desc.regin[0] |= REGFLAG_R(RSREG);
			desc.regin[2] |= REGFLAG_FCC;
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

		case 0x10:  // MFHI
			desc.regin[0] |= REGFLAG_HI;
			desc.regout[0] |= REGFLAG_R(RDREG);
			return true;

		case 0x11:  // MTHI
			desc.regin[0] |= REGFLAG_R(RSREG);
			desc.regout[0] |= REGFLAG_HI;
			return true;

		case 0x12:  // MFLO
			desc.regin[2] |= REGFLAG_LO;
			desc.regout[0] |= REGFLAG_R(RDREG);
			return true;

		case 0x13:  // MTLO
			desc.regin[0] |= REGFLAG_R(RSREG);
			desc.regout[2] |= REGFLAG_LO;
			return true;

		case 0x18:  // MULT
		case 0x19:  // MULTU
			desc.regin[0] |= REGFLAG_R(RSREG) | REGFLAG_R(RTREG);
			desc.regout[2] |= REGFLAG_LO | REGFLAG_HI;
			desc.cycles = 3;
			return true;

		case 0x1a:  // DIV
		case 0x1b:  // DIVU
			desc.regin[0] |= REGFLAG_R(RSREG) | REGFLAG_R(RTREG);
			desc.regout[2] |= REGFLAG_LO | REGFLAG_HI;
			desc.cycles = 35;
			return true;

		case 0x1c:  // DMULT
		case 0x1d:  // DMULTU
			desc.regin[0] |= REGFLAG_R(RSREG) | REGFLAG_R(RTREG);
			desc.regout[2] |= REGFLAG_LO | REGFLAG_HI;
			desc.cycles = 7;
			return true;

		case 0x1e:  // DDIV
		case 0x1f:  // DDIVU
			desc.regin[0] |= REGFLAG_R(RSREG) | REGFLAG_R(RTREG);
			desc.regout[2] |= REGFLAG_LO | REGFLAG_HI;
			desc.cycles = 67;
			return true;

		case 0x0c:  // SYSCALL
		case 0x0d:  // BREAK
			desc.flags |= OPFLAG_WILL_CAUSE_EXCEPTION | OPFLAG_END_SEQUENCE;
			return true;

		case 0x0f:  // SYNC
			// effective no-op
			return true;
	}

	return false;
}


//-------------------------------------------------
//  describe_regimm - build a description of a
//  single instruction in the 'regimm' group
//-------------------------------------------------

bool mips3_frontend::describe_regimm(UINT32 op, opcode_desc &desc)
{
	switch (RTREG)
	{
		case 0x00:  // BLTZ
		case 0x01:  // BGEZ
		case 0x02:  // BLTZL
		case 0x03:  // BGEZL
			if (RTREG == 0x01 && RSREG == 0)
				desc.flags |= OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_END_SEQUENCE;
			else
			{
				desc.regin[0] |= REGFLAG_R(RSREG);
				desc.flags |= OPFLAG_IS_CONDITIONAL_BRANCH;
			}
			desc.targetpc = desc.pc + 4 + SIMMVAL * 4;
			desc.delayslots = 1;
			desc.skipslots = (RTREG & 0x02) ? 1 : 0;
			return true;

		case 0x08:  // TGEI
		case 0x09:  // TGEIU
		case 0x0a:  // TLTI
		case 0x0b:  // TLTIU
		case 0x0c:  // TEQI
		case 0x0e:  // TNEI
			desc.regin[0] |= REGFLAG_R(RSREG);
			desc.flags |= OPFLAG_CAN_CAUSE_EXCEPTION;
			return true;

		case 0x10:  // BLTZAL
		case 0x11:  // BGEZAL
		case 0x12:  // BLTZALL
		case 0x13:  // BGEZALL
			if (RTREG == 0x11 && RSREG == 0)
				desc.flags |= OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_END_SEQUENCE;
			else
			{
				desc.regin[0] |= REGFLAG_R(RSREG);
				desc.flags |= OPFLAG_IS_CONDITIONAL_BRANCH;
			}
			desc.regout[0] |= REGFLAG_R(31);
			desc.targetpc = desc.pc + 4 + SIMMVAL * 4;
			desc.delayslots = 1;
			desc.skipslots = (RTREG & 0x02) ? 1 : 0;
			return true;
	}

	return false;
}


//-------------------------------------------------
//  describe_idt - build a description of a single
//  instruction in the IDT-specific group
//-------------------------------------------------

bool mips3_frontend::describe_idt(UINT32 op, opcode_desc &desc)
{
	// only on the R4650
	if (m_mips3->m_flavor != mips3_device::MIPS3_TYPE_R4650)
		return false;

	switch (op & 0x1f)
	{
		case 0: // MAD
		case 1: // MADU
			desc.regin[0] |= REGFLAG_R(RSREG) | REGFLAG_R(RTREG);
			desc.regin[2] |= REGFLAG_LO | REGFLAG_HI;
			desc.regout[2] |= REGFLAG_LO | REGFLAG_HI;
			return true;

		case 2: // MUL
			desc.regin[0] |= REGFLAG_R(RSREG) | REGFLAG_R(RTREG);
			desc.regout[0] |= REGFLAG_R(RDREG);
			desc.cycles = 3;
			return true;
	}

	return false;
}


//-------------------------------------------------
//  describe_cop0 - build a description of a
//  single instruction in the COP0 group
//-------------------------------------------------

bool mips3_frontend::describe_cop0(UINT32 op, opcode_desc &desc)
{
	// any COP0 instruction can potentially cause an exception
	desc.flags |= OPFLAG_CAN_CAUSE_EXCEPTION;

	switch (RSREG)
	{
		case 0x00:  // MFCz
		case 0x01:  // DMFCz
			if (RDREG == COP0_Count)
				desc.cycles += MIPS3_COUNT_READ_CYCLES;
			if (RDREG == COP0_Cause)
				desc.cycles += MIPS3_CAUSE_READ_CYCLES;
			desc.regout[0] |= REGFLAG_R(RTREG);
			return true;

		case 0x02:  // CFCz
			desc.regout[0] |= REGFLAG_R(RTREG);
			return true;

		case 0x04:  // MTCz
		case 0x05:  // DMTCz
		case 0x06:  // CTCz
			desc.regin[0] |= REGFLAG_R(RTREG);
			if (RSREG == 0x04 || RSREG == 0x05)
			{
				if (RDREG == COP0_Cause)
					desc.flags |= OPFLAG_CAN_TRIGGER_SW_INT;
				if (RDREG == COP0_Status)
					desc.flags |= OPFLAG_CAN_EXPOSE_EXTERNAL_INT | OPFLAG_CAN_CHANGE_MODES | OPFLAG_END_SEQUENCE;
			}
			return true;

		case 0x08:  // BC
			switch (RTREG)
			{
				case 0x00:  // BCzF
				case 0x01:  // BCzT
					desc.flags |= OPFLAG_IS_CONDITIONAL_BRANCH;
					desc.targetpc = desc.pc + 4 + SIMMVAL * 4;
					desc.delayslots = 1;
					return true;
			}
			return false;

		case 0x10:  case 0x11:  case 0x12:  case 0x13:  case 0x14:  case 0x15:  case 0x16:  case 0x17:
		case 0x18:  case 0x19:  case 0x1a:  case 0x1b:  case 0x1c:  case 0x1d:  case 0x1e:  case 0x1f:  // COP
			switch (op & 0x01ffffff)
			{
				case 0x01:  // TLBR
				case 0x08:  // TLBP
				case 0x20:  // WAIT
					return true;

				case 0x02:  // TLBWI
				case 0x06:  // TLBWR
					desc.flags |= OPFLAG_MODIFIES_TRANSLATION;
					return true;

				case 0x18:  // ERET
					desc.flags |= OPFLAG_CAN_CHANGE_MODES | OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_END_SEQUENCE;
					return true;
			}
			return false;
	}

	return false;
}


//-------------------------------------------------
//  describe_cop1 - build a description of a
//  single instruction in the COP1 group
//-------------------------------------------------

bool mips3_frontend::describe_cop1(UINT32 op, opcode_desc &desc)
{
	// any COP1 instruction can potentially cause an exception
//  desc.flags |= OPFLAG_CAN_CAUSE_EXCEPTION;

	switch (RSREG)
	{
		case 0x00:  // MFCz
		case 0x01:  // DMFCz
			desc.regin[1] |= REGFLAG_CPR1(RDREG);
			desc.regout[0] |= REGFLAG_R(RTREG);
			return true;

		case 0x02:  // CFCz
			desc.regout[0] |= REGFLAG_R(RTREG);
			return true;

		case 0x04:  // MTCz
		case 0x05:  // DMTCz
			desc.regin[0] |= REGFLAG_R(RTREG);
			desc.regout[1] |= REGFLAG_CPR1(RDREG);
			return true;

		case 0x06:  // CTCz
			desc.regin[0] |= REGFLAG_R(RTREG);
			return true;

		case 0x08:  // BC
			switch (RTREG & 3)
			{
				case 0x00:  // BCzF
				case 0x01:  // BCzT
				case 0x02:  // BCzFL
				case 0x03:  // BCzTL
					desc.regin[2] |= REGFLAG_FCC;
					desc.flags |= OPFLAG_IS_CONDITIONAL_BRANCH;
					desc.targetpc = desc.pc + 4 + SIMMVAL * 4;
					desc.delayslots = 1;
					desc.skipslots = (RTREG & 0x02) ? 1 : 0;
					return true;
			}
			return false;

		case 0x10:  case 0x11:  case 0x12:  case 0x13:  case 0x14:  case 0x15:  case 0x16:  case 0x17:
		case 0x18:  case 0x19:  case 0x1a:  case 0x1b:  case 0x1c:  case 0x1d:  case 0x1e:  case 0x1f:  // COP
			switch (op & 0x3f)
			{
				case 0x12:  // MOVZ - MIPS IV
				case 0x13:  // MOVN - MIPS IV
					if (m_mips3->m_flavor < mips3_device::MIPS3_TYPE_MIPS_IV)
						return false;
				case 0x00:  // ADD
				case 0x01:  // SUB
				case 0x02:  // MUL
				case 0x03:  // DIV
					desc.regin[1] |= REGFLAG_CPR1(FSREG) | REGFLAG_CPR1(FTREG);
					desc.regout[1] |= REGFLAG_CPR1(FDREG);
					return true;

				case 0x15:  // RECIP - MIPS IV
				case 0x16:  // RSQRT - MIPS IV
					if (m_mips3->m_flavor < mips3_device::MIPS3_TYPE_MIPS_IV)
						return false;
				case 0x04:  // SQRT
				case 0x05:  // ABS
				case 0x06:  // MOV
				case 0x07:  // NEG
				case 0x08:  // ROUND.L
				case 0x09:  // TRUNC.L
				case 0x0a:  // CEIL.L
				case 0x0b:  // FLOOR.L
				case 0x0c:  // ROUND.W
				case 0x0d:  // TRUNC.W
				case 0x0e:  // CEIL.W
				case 0x0f:  // FLOOR.W
				case 0x20:  // CVT.S
				case 0x21:  // CVT.D
				case 0x24:  // CVT.W
				case 0x25:  // CVT.L
					desc.regin[1] |= REGFLAG_CPR1(FSREG);
					desc.regout[1] |= REGFLAG_CPR1(FDREG);
					return true;

				case 0x11:  // MOVT/F - MIPS IV
					if (m_mips3->m_flavor < mips3_device::MIPS3_TYPE_MIPS_IV)
						return false;
					desc.regin[1] |= REGFLAG_CPR1(FSREG);
					desc.regin[2] |= REGFLAG_FCC;
					desc.regout[1] |= REGFLAG_CPR1(FDREG);
					return true;

				case 0x30:  case 0x38:  // C.F
				case 0x31:  case 0x39:  // C.UN
					desc.regout[2] |= REGFLAG_FCC;
					return true;

				case 0x32:  case 0x3a:  // C.EQ
				case 0x33:  case 0x3b:  // C.UEQ
				case 0x34:  case 0x3c:  // C.OLT
				case 0x35:  case 0x3d:  // C.ULT
				case 0x36:  case 0x3e:  // C.OLE
				case 0x37:  case 0x3f:  // C.ULE
					desc.regin[1] |= REGFLAG_CPR1(FSREG) | REGFLAG_CPR1(FTREG);
					desc.regout[2] |= REGFLAG_FCC;
					return true;
			}
			return false;
	}

	return false;
}


//-------------------------------------------------
//  describe_cop1x - build a description of a
//  single instruction in the COP1X group
//-------------------------------------------------

bool mips3_frontend::describe_cop1x(UINT32 op, opcode_desc &desc)
{
	// any COP1 instruction can potentially cause an exception
//  desc.flags |= OPFLAG_CAN_CAUSE_EXCEPTION;

	switch (op & 0x3f)
	{
		case 0x00:  // LWXC1
		case 0x01:  // LDXC1
			desc.regin[0] |= REGFLAG_R(RSREG) | REGFLAG_R(RTREG);
			desc.regout[1] |= REGFLAG_CPR1(FDREG);
			desc.flags |= OPFLAG_READS_MEMORY;
			return true;

		case 0x08:  // SWXC1
		case 0x09:  // SDXC1
			desc.regin[0] |= REGFLAG_R(RSREG) | REGFLAG_R(RTREG);
			desc.regin[1] |= REGFLAG_CPR1(FDREG);
			desc.flags |= OPFLAG_WRITES_MEMORY;
			return true;

		case 0x0f:  // PREFX
			// effective no-op
			return true;

		case 0x20:  case 0x21:  // MADD
		case 0x28:  case 0x29:  // MSUB
		case 0x30:  case 0x31:  // NMADD
		case 0x38:  case 0x39:  // NMSUB
			desc.regin[1] |= REGFLAG_CPR1(FSREG) | REGFLAG_CPR1(FTREG) | REGFLAG_CPR1(FRREG);
			desc.regout[1] |= REGFLAG_CPR1(FDREG);
			return true;
	}

	return false;
}


//-------------------------------------------------
//  describe_cop2 - build a description of a
//  single instruction in the COP2 group
//-------------------------------------------------

bool mips3_frontend::describe_cop2(UINT32 op, opcode_desc &desc)
{
	// any COP2 instruction can potentially cause an exception
	desc.flags |= OPFLAG_CAN_CAUSE_EXCEPTION;

	switch (RSREG)
	{
		case 0x00:  // MFCz
		case 0x01:  // DMFCz
		case 0x02:  // CFCz
			desc.regout[0] |= REGFLAG_R(RTREG);
			return true;

		case 0x04:  // MTCz
		case 0x05:  // DMTCz
		case 0x06:  // CTCz
			desc.regin[0] |= REGFLAG_R(RTREG);
			return true;

		case 0x08:  // BC
			switch (RTREG)
			{
				case 0x00:  // BCzF
				case 0x01:  // BCzT
					desc.flags |= OPFLAG_IS_CONDITIONAL_BRANCH;
					desc.targetpc = desc.pc + 4 + SIMMVAL * 4;
					desc.delayslots = 1;
					return true;
			}
			return false;
	}

	return false;
}
