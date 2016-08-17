// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    ppcfe.c

    Front-end for PowerPC recompiler

***************************************************************************/

#include "emu.h"
#include "ppcfe.h"
#include "ppccom.h"


//**************************************************************************
//  MACROS
//**************************************************************************

#define GPR_USED(desc, x)           do { (desc).regin[0] |= REGFLAG_R(x); } while (0)
#define GPR_USED_OR_ZERO(desc, x)   do { (desc).regin[0] |= ((x) == 0 ? 0 : REGFLAG_R(x)); } while (0)
#define GPR_MODIFIED(desc, x)       do { (desc).regout[0] |= REGFLAG_R(x); } while (0)

#define FPR_USED(desc, x)           do { (desc).regin[1] |= REGFLAG_FR(x); } while (0)
#define FPR_MODIFIED(desc, x)       do { (desc).regout[1] |= REGFLAG_FR(x); } while (0)

#define CR_USED(desc, x)            do { (desc).regin[2] |= REGFLAG_CR(x); } while (0)
#define CR_BIT_USED(desc, x)        do { (desc).regin[2] |= REGFLAG_CR_BIT(x); } while (0)
#define CR_MODIFIED(desc, x)        do { (desc).regout[2] |= REGFLAG_CR(x); } while (0)
#define CR_BIT_MODIFIED(desc, x)    do { (desc).regout[2] |= REGFLAG_CR_BIT(x); } while (0)

#define XER_CA_USED(desc)           do { (desc).regin[3] |= REGFLAG_XER_CA; } while (0)
#define XER_OV_USED(desc)           do { (desc).regin[3] |= REGFLAG_XER_OV; } while (0)
#define XER_SO_USED(desc)           do { (desc).regin[3] |= REGFLAG_XER_SO; } while (0)
#define XER_COUNT_USED(desc)        do { (desc).regin[3] |= REGFLAG_XER_COUNT; } while (0)
#define XER_CA_MODIFIED(desc)       do { (desc).regout[3] |= REGFLAG_XER_CA; } while (0)
#define XER_OV_MODIFIED(desc)       do { (desc).regout[3] |= REGFLAG_XER_OV; } while (0)
#define XER_SO_MODIFIED(desc)       do { (desc).regout[3] |= REGFLAG_XER_SO; } while (0)
#define XER_COUNT_MODIFIED(desc)    do { (desc).regout[3] |= REGFLAG_XER_COUNT; } while (0)

#define CTR_USED(desc)              do { (desc).regin[3] |= REGFLAG_CTR; } while (0)
#define CTR_MODIFIED(desc)          do { (desc).regout[3] |= REGFLAG_CTR; } while (0)
#define LR_USED(desc)               do { (desc).regin[3] |= REGFLAG_LR; } while (0)
#define LR_MODIFIED(desc)           do { (desc).regout[3] |= REGFLAG_LR; } while (0)

#define FPSCR_USED(desc, x)         do { (desc).regin[3] |= REGFLAG_FPSCR(x); } while (0)
#define FPSCR_MODIFIED(desc, x)     do { (desc).regout[3] |= REGFLAG_FPSCR(x); } while (0)



//**************************************************************************
//  PPC FRONTEND
//**************************************************************************

//-------------------------------------------------
//  ppc_frontend - constructor
//-------------------------------------------------

ppc_frontend::ppc_frontend(ppc_device *ppc, UINT32 window_start, UINT32 window_end, UINT32 max_sequence)
	: drc_frontend(*ppc, window_start, window_end, max_sequence),
	m_ppc(ppc)
{
}


//-------------------------------------------------
//  describe - build a description of a single
//  instruction
//-------------------------------------------------

bool ppc_frontend::describe(opcode_desc &desc, const opcode_desc *prev)
{
	UINT32 op, opswitch;
	int regnum;

	// compute the physical PC
	if (!m_ppc->memory_translate(AS_PROGRAM, TRANSLATE_FETCH, desc.physpc))
	{
		// uh-oh: a page fault; leave the description empty and just if this is the first instruction, leave it empty and
		// mark as needing to validate; otherwise, just end the sequence here
		desc.flags |= OPFLAG_VALIDATE_TLB | OPFLAG_CAN_CAUSE_EXCEPTION | OPFLAG_COMPILER_PAGE_FAULT | OPFLAG_VIRTUAL_NOOP | OPFLAG_END_SEQUENCE;
		return true;
	}

	// fetch the opcode
	op = desc.opptr.l[0] = m_ppc->m_direct->read_dword(desc.physpc, m_ppc->m_codexor);

	// all instructions are 4 bytes and default to a single cycle each
	desc.length = 4;
	desc.cycles = 1;

	// parse the instruction
	opswitch = op >> 26;
	switch (opswitch)
	{
		case 0x02:  // TDI - 64-bit only
		case 0x1e:  // 0x1e group - 64-bit only
		case 0x3a:  // 0x3a group - 64-bit only
		case 0x3e:  // 0x3e group - 64-bit only
			return false;

		case 0x03:  // TWI
			GPR_USED(desc, G_RA(op));
			desc.flags |= OPFLAG_CAN_CAUSE_EXCEPTION;
			if (is_603_class())
				desc.cycles = 2;    // 603
			return true;

		case 0x07:  // MULLI
			GPR_USED(desc, G_RA(op));
			GPR_MODIFIED(desc, G_RD(op));
			if (is_403_class())
				desc.cycles = 4;    // 4XX
			else if (is_601_class())
				desc.cycles = 5;    // 601
			else if (is_603_class())
				desc.cycles = 2;    // 603: 2-3
			else
				desc.cycles = 2;    // ???
			return true;

		case 0x0e:  // ADDI
		case 0x0f:  // ADDIS
			GPR_USED_OR_ZERO(desc, G_RA(op));
			GPR_MODIFIED(desc, G_RD(op));
			return true;

		case 0x0a:  // CMPLI
		case 0x0b:  // CMPI
			GPR_USED(desc, G_RA(op));
			XER_SO_USED(desc);
			CR_MODIFIED(desc, G_CRFD(op));
			return true;

		case 0x08:  // SUBFIC
		case 0x0c:  // ADDIC
			GPR_USED(desc, G_RA(op));
			GPR_MODIFIED(desc, G_RD(op));
			XER_CA_MODIFIED(desc);
			return true;

		case 0x0d:  // ADDIC.
			GPR_USED(desc, G_RA(op));
			XER_SO_USED(desc);
			GPR_MODIFIED(desc, G_RT(op));
			XER_CA_MODIFIED(desc);
			CR_MODIFIED(desc, 0);
			return true;

		case 0x10:  // BCx
			if (!(G_BO(op) & 0x10))
			{
				CR_BIT_USED(desc, G_BI(op));
				// branch folding
				if (prev == nullptr || prev->regout[2] == 0)
					desc.cycles = 0;
			}
			if (!(G_BO(op) & 0x04))
			{
				CTR_USED(desc);
				CTR_MODIFIED(desc);
			}
			if (op & M_LK)
				LR_MODIFIED(desc);
			if ((G_BO(op) & 0x14) == 0x14)
				desc.flags |= OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_END_SEQUENCE;
			else
				desc.flags |= OPFLAG_IS_CONDITIONAL_BRANCH;
			desc.targetpc = (INT16)(G_BD(op) << 2) + ((op & M_AA) ? 0 : desc.pc);
			if (desc.targetpc == desc.pc && desc.cycles == 0)
				desc.cycles = 1;
			return true;

		case 0x11:  // SC
			if (!(m_ppc->m_cap & (PPCCAP_OEA | PPCCAP_4XX)))
				return false;
			desc.flags |= OPFLAG_WILL_CAUSE_EXCEPTION;
			if (is_601_class())
				desc.cycles = 16;   // 601
			else if (is_603_class())
				desc.cycles = 3;    // 603
			else
				desc.cycles = 3;    // ???
			return true;

		case 0x12:  // Bx
			if (op & M_LK)
				LR_MODIFIED(desc);
			desc.flags |= OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_END_SEQUENCE;
			desc.targetpc = ((INT32)(G_LI(op) << 8) >> 6) + ((op & M_AA) ? 0 : desc.pc);
			// branch folding
			if (desc.targetpc != desc.pc)
				desc.cycles = 0;
			return true;

		case 0x13:  // 0x13 group
			return describe_13(op, desc, prev);

		case 0x14:  // RLWIMIx
			GPR_USED(desc, G_RS(op));
			GPR_USED(desc, G_RA(op));
			GPR_MODIFIED(desc, G_RA(op));
			if (op & M_RC)
			{
				XER_SO_USED(desc);
				CR_MODIFIED(desc, 0);
			}
			return true;

		case 0x15:  // RLWINMx
			GPR_USED(desc, G_RS(op));
			GPR_MODIFIED(desc, G_RA(op));
			if (op & M_RC)
			{
				XER_SO_USED(desc);
				CR_MODIFIED(desc, 0);
			}
			return true;

		case 0x17:  // RLWNMx
			GPR_USED(desc, G_RS(op));
			GPR_USED(desc, G_RB(op));
			GPR_MODIFIED(desc, G_RA(op));
			if (op & M_RC)
			{
				XER_SO_USED(desc);
				CR_MODIFIED(desc, 0);
			}
			return true;

		case 0x18:  // ORI
		case 0x19:  // ORIS
		case 0x1a:  // XORI
		case 0x1b:  // XORIS
			GPR_USED(desc, G_RS(op));
			GPR_MODIFIED(desc, G_RA(op));
			return true;

		case 0x1c:  // ANDI.
		case 0x1d:  // ANDIS.
			GPR_USED(desc, G_RS(op));
			XER_SO_USED(desc);
			GPR_MODIFIED(desc, G_RA(op));
			CR_MODIFIED(desc, 0);
			return true;

		case 0x1f:  // 0x1f group
			return describe_1f(op, desc, prev);

		case 0x20:  // LWZ
		case 0x22:  // LBZ
		case 0x28:  // LHZ
		case 0x2a:  // LHA
			GPR_USED_OR_ZERO(desc, G_RA(op));
			GPR_MODIFIED(desc, G_RD(op));
			desc.flags |= OPFLAG_READS_MEMORY;
			return true;

		case 0x21:  // LWZU
		case 0x23:  // LBZU
		case 0x29:  // LHZU
		case 0x2b:  // LHAU
			if (G_RA(op) == 0 || G_RA(op) == G_RD(op))
				return false;
			GPR_USED(desc, G_RA(op));
			GPR_MODIFIED(desc, G_RD(op));
			GPR_MODIFIED(desc, G_RA(op));
			desc.flags |= OPFLAG_READS_MEMORY;
			return true;

		case 0x24:  // STW
		case 0x26:  // STB
		case 0x2c:  // STH
			GPR_USED_OR_ZERO(desc, G_RA(op));
			GPR_USED(desc, G_RS(op));
			desc.flags |= OPFLAG_WRITES_MEMORY;
			return true;

		case 0x25:  // STWU
		case 0x27:  // STBU
		case 0x2d:  // STHU
			if (G_RA(op) == 0)
				return false;
			GPR_USED(desc, G_RA(op));
			GPR_USED(desc, G_RS(op));
			GPR_MODIFIED(desc, G_RA(op));
			desc.flags |= OPFLAG_WRITES_MEMORY;
			return true;

		case 0x2e:  // LMW
			GPR_USED_OR_ZERO(desc, G_RA(op));
			for (regnum = G_RD(op); regnum < 32; regnum++)
				GPR_MODIFIED(desc, regnum);
			desc.flags |= OPFLAG_READS_MEMORY;
			desc.cycles = 32 - G_RD(op);
			return true;

		case 0x2f:  // STMW
			GPR_USED_OR_ZERO(desc, G_RA(op));
			for (regnum = G_RS(op); regnum < 32; regnum++)
				GPR_USED(desc, regnum);
			desc.flags |= OPFLAG_WRITES_MEMORY;
			desc.cycles = 32 - G_RS(op);
			return true;

		case 0x30:  // LFS
		case 0x32:  // LFD
			if (!(m_ppc->m_cap & PPCCAP_FPU))
				return false;
			GPR_USED_OR_ZERO(desc, G_RA(op));
			FPR_MODIFIED(desc, G_RD(op));
			desc.flags |= OPFLAG_READS_MEMORY;
			return true;

		case 0x31:  // LFSU
		case 0x33:  // LFDU
			if (!(m_ppc->m_cap & PPCCAP_FPU))
				return false;
			if (G_RA(op) == 0)
				return false;
			GPR_USED(desc, G_RA(op));
			GPR_MODIFIED(desc, G_RA(op));
			FPR_MODIFIED(desc, G_RD(op));
			desc.flags |= OPFLAG_READS_MEMORY;
			return true;

		case 0x34:  // STFS
		case 0x36:  // STFD
			if (!(m_ppc->m_cap & PPCCAP_FPU))
				return false;
			GPR_USED_OR_ZERO(desc, G_RA(op));
			FPR_USED(desc, G_RS(op));
			desc.flags |= OPFLAG_WRITES_MEMORY;
			return true;

		case 0x35:  // STFSU
		case 0x37:  // STFDU
			if (!(m_ppc->m_cap & PPCCAP_FPU))
				return false;
			if (G_RA(op) == 0)
				return false;
			GPR_USED(desc, G_RA(op));
			GPR_MODIFIED(desc, G_RA(op));
			FPR_USED(desc, G_RS(op));
			desc.flags |= OPFLAG_WRITES_MEMORY;
			return true;

		case 0x3b:  // 0x3b group
			return describe_3b(op, desc, prev);

		case 0x3f:  // 0x3f group
			return describe_3f(op, desc, prev);
	}

	return false;
}


/*-------------------------------------------------
    describe_instruction_13 - build a
    description of a single instruction in the
    0x13 group
-------------------------------------------------*/

bool ppc_frontend::describe_13(UINT32 op, opcode_desc &desc, const opcode_desc *prev)
{
	UINT32 opswitch = (op >> 1) & 0x3ff;

	switch (opswitch)
	{
		case 0x000: // MTCRF
			CR_USED(desc, G_CRFS(op));
			CR_MODIFIED(desc, G_CRFD(op));
			// CR logical folding
			if (prev == nullptr || prev->regout[2] == 0)
				desc.cycles = 0;
			return true;

		case 0x010: // BCLRx
			LR_USED(desc);
			if (!(G_BO(op) & 0x10))
				CR_BIT_USED(desc, G_BI(op));
			if (!(G_BO(op) & 0x04))
			{
				CTR_USED(desc);
				CTR_MODIFIED(desc);
			}
			if (op & M_LK)
				LR_MODIFIED(desc);
			if ((G_BO(op) & 0x14) == 0x14)
				desc.flags |= OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_END_SEQUENCE;
			else
				desc.flags |= OPFLAG_IS_CONDITIONAL_BRANCH;
			desc.targetpc = BRANCH_TARGET_DYNAMIC;
			return true;

		case 0x021: // CRNOR
		case 0x081: // CRANDC
		case 0x0c1: // CRXOR
		case 0x0e1: // CRNAND
		case 0x101: // CRAND
		case 0x121: // CREQV
		case 0x1a1: // CRORC
		case 0x1c1: // CROR
			CR_BIT_USED(desc, G_CRBA(op));
			CR_BIT_USED(desc, G_CRBB(op));
			CR_BIT_MODIFIED(desc, G_CRBD(op));
			// CR logical folding
			if (prev == nullptr || prev->regout[2] == 0)
				desc.cycles = 0;
			return true;

		case 0x032: // RFI
			if (!(m_ppc->m_cap & (PPCCAP_OEA | PPCCAP_4XX)))
				return false;
			desc.flags |= OPFLAG_PRIVILEGED | OPFLAG_CAN_CHANGE_MODES | OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_END_SEQUENCE | OPFLAG_CAN_CAUSE_EXCEPTION;
			desc.targetpc = BRANCH_TARGET_DYNAMIC;
			if (is_601_class())
				desc.cycles = 13;   // 601
			else if (is_603_class())
				desc.cycles = 3;    // 603
			else
				desc.cycles = 3;    // ???
			return true;

		case 0x033: // RFCI
			if (!(m_ppc->m_cap & PPCCAP_4XX))
				return false;
			desc.flags |= OPFLAG_PRIVILEGED | OPFLAG_CAN_CHANGE_MODES | OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_END_SEQUENCE | OPFLAG_CAN_CAUSE_EXCEPTION;
			desc.targetpc = BRANCH_TARGET_DYNAMIC;
			return true;

		case 0x096: // ISYNC
			if (!(m_ppc->m_cap & (PPCCAP_VEA | PPCCAP_4XX)))
				return false;
			if (is_601_class())
				desc.cycles = 6;    // 601
			return true;

		case 0x210: // BCCTRx
			CTR_USED(desc);
			if (!(G_BO(op) & 0x10))
				CR_BIT_USED(desc, G_BI(op));
			if (!(G_BO(op) & 0x04))
				return false;
			if (op & M_LK)
				LR_MODIFIED(desc);
			if ((G_BO(op) & 0x14) == 0x14)
				desc.flags |= OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_END_SEQUENCE;
			else
				desc.flags |= OPFLAG_IS_CONDITIONAL_BRANCH;
			desc.targetpc = BRANCH_TARGET_DYNAMIC;
			return true;
	}

	return false;
}


/*-------------------------------------------------
    describe_instruction_1f - build a
    description of a single instruction in the
    0x1f group
-------------------------------------------------*/

bool ppc_frontend::describe_1f(UINT32 op, opcode_desc &desc, const opcode_desc *prev)
{
	UINT32 opswitch = (op >> 1) & 0x3ff;
	int spr, regnum;

	switch (opswitch)
	{
		case 0x009: // MULHDUx - 64-bit only
		case 0x015: // LDX - 64-bit only
		case 0x01b: // SLDx - 64-bit only
		case 0x035: // LDUX - 64-bit only
		case 0x03a: // CNTLZDx - 64-bit only
		case 0x044: // TD - 64-bit only
		case 0x049: // MULHDx - 64-bit only
		case 0x054: // LDARX - 64-bit only
		case 0x095: // STDX - 64-bit only
		case 0x0b5: // STDUX - 64-bit only
		case 0x0d6: // STDCX. - 64-bit only
		case 0x0e9: // MULLD - 64-bit only
		case 0x2e9: // MULLDO - 64-bit only
		case 0x155: // LWAX - 64-bit only
		case 0x175: // LWAUX - 64-bit only
		case 0x33a: // SRADIx - 64-bit only
		case 0x33b: // SRADIx - 64-bit only
		case 0x1b2: // SLBIE - 64-bit only
		case 0x1c9: // DIVDUx - 64-bit only
		case 0x3c9: // DIVDUOx - 64-bit only
		case 0x1e9: // DIVDx - 64-bit only
		case 0x3e9: // DIVDOx - 64-bit only
		case 0x1f2: // SLBIA - 64-bit only
		case 0x21b: // SRDx - 64-bit only
		case 0x31a: // SRADx - 64-bit only
		case 0x3da: // EXTSW - 64-bit only
			return false;

		case 0x000: // CMP
		case 0x020: // CMPL
			GPR_USED(desc, G_RA(op));
			GPR_USED(desc, G_RB(op));
			XER_SO_USED(desc);
			CR_MODIFIED(desc, G_CRFD(op));
			return true;

		case 0x004: // TW
			GPR_USED(desc, G_RA(op));
			GPR_USED(desc, G_RB(op));
			desc.flags |= OPFLAG_CAN_CAUSE_EXCEPTION;
			if (is_603_class())
				desc.cycles = 2;    // 603
			return true;

		case 0x008: // SUBFCx
		case 0x00a: // ADDCx
			GPR_USED(desc, G_RA(op));
			GPR_USED(desc, G_RB(op));
			GPR_MODIFIED(desc, G_RD(op));
			XER_CA_MODIFIED(desc);
			if (op & M_RC)
			{
				XER_SO_USED(desc);
				CR_MODIFIED(desc, 0);
			}
			return true;

		case 0x088: // SUBFEx
		case 0x08a: // ADDEx
			GPR_USED(desc, G_RA(op));
			GPR_USED(desc, G_RB(op));
			XER_CA_USED(desc);
			GPR_MODIFIED(desc, G_RD(op));
			XER_CA_MODIFIED(desc);
			if (op & M_RC)
			{
				XER_SO_USED(desc);
				CR_MODIFIED(desc, 0);
			}
			return true;

		case 0x0c8: // SUBFZEx
		case 0x0ca: // ADDZEx
		case 0x0e8: // SUBFMEx
		case 0x0ea: // ADDMEx
			GPR_USED(desc, G_RA(op));
			XER_CA_USED(desc);
			GPR_MODIFIED(desc, G_RD(op));
			XER_CA_MODIFIED(desc);
			if (op & M_RC)
			{
				XER_SO_USED(desc);
				CR_MODIFIED(desc, 0);
			}
			return true;

		case 0x00b: // MULHWUx
		case 0x04b: // MULHWx
		case 0x0eb: // MULLWx
			GPR_USED(desc, G_RA(op));
			GPR_USED(desc, G_RB(op));
			GPR_MODIFIED(desc, G_RD(op));
			if (op & M_RC)
			{
				XER_SO_USED(desc);
				CR_MODIFIED(desc, 0);
			}
			if (is_403_class())
				desc.cycles = 4;    // 4XX
			else if (is_601_class())
				desc.cycles = 5;    // 601: 5/9/10
			else if (is_603_class())
				desc.cycles = 2;    // 603: 2,3,4,5,6
			else
				desc.cycles = 2;    // ???
			return true;

		case 0x1cb: // DIVWUx
		case 0x1eb: // DIVWx
			GPR_USED(desc, G_RA(op));
			GPR_USED(desc, G_RB(op));
			GPR_MODIFIED(desc, G_RD(op));
			if (op & M_RC)
			{
				XER_SO_USED(desc);
				CR_MODIFIED(desc, 0);
			}
			if (is_403_class())
				desc.cycles = 33;   // 4XX
			else if (is_601_class())
				desc.cycles = 36;   // 601
			else if (is_603_class())
				desc.cycles = 37;   // 603
			else
				desc.cycles = 33;   // ???
			return true;

		case 0x028: // SUBFx
		case 0x10a: // ADDx
			GPR_USED(desc, G_RA(op));
			GPR_USED(desc, G_RB(op));
			GPR_MODIFIED(desc, G_RD(op));
			if (op & M_RC)
			{
				XER_SO_USED(desc);
				CR_MODIFIED(desc, 0);
			}
			return true;

		case 0x208: // SUBFCOx
		case 0x20a: // ADDCOx
			GPR_USED(desc, G_RA(op));
			GPR_USED(desc, G_RB(op));
			GPR_MODIFIED(desc, G_RD(op));
			XER_OV_MODIFIED(desc);
			XER_SO_MODIFIED(desc);
			XER_CA_MODIFIED(desc);
			if (op & M_RC)
				CR_MODIFIED(desc, 0);
			return true;

		case 0x288: // SUBFEOx
		case 0x28a: // ADDEOx
			GPR_USED(desc, G_RA(op));
			GPR_USED(desc, G_RB(op));
			XER_CA_USED(desc);
			GPR_MODIFIED(desc, G_RD(op));
			XER_OV_MODIFIED(desc);
			XER_SO_MODIFIED(desc);
			XER_CA_MODIFIED(desc);
			if (op & M_RC)
				CR_MODIFIED(desc, 0);
			return true;

		case 0x2c8: // SUBFZEOx
		case 0x2ca: // ADDZEOx
		case 0x2e8: // SUBFMEOx
		case 0x2ea: // ADDMEOx
			GPR_USED(desc, G_RA(op));
			XER_CA_USED(desc);
			GPR_MODIFIED(desc, G_RD(op));
			XER_OV_MODIFIED(desc);
			XER_SO_MODIFIED(desc);
			XER_CA_MODIFIED(desc);
			if (op & M_RC)
				CR_MODIFIED(desc, 0);
			return true;

		case 0x2eb: // MULLWOx
			GPR_USED(desc, G_RA(op));
			GPR_USED(desc, G_RB(op));
			GPR_MODIFIED(desc, G_RD(op));
			XER_OV_MODIFIED(desc);
			XER_SO_MODIFIED(desc);
			if (op & M_RC)
				CR_MODIFIED(desc, 0);
			if (is_403_class())
				desc.cycles = 4;    // 4XX
			else if (is_601_class())
				desc.cycles = 5;    // 601: 5/9/10
			else if (is_603_class())
				desc.cycles = 2;    // 603: 2,3,4,5,6
			else
				desc.cycles = 2;    // ???
			return true;

		case 0x3cb: // DIVWUOx
		case 0x3eb: // DIVWOx
			GPR_USED(desc, G_RA(op));
			GPR_USED(desc, G_RB(op));
			GPR_MODIFIED(desc, G_RD(op));
			XER_OV_MODIFIED(desc);
			XER_SO_MODIFIED(desc);
			if (op & M_RC)
				CR_MODIFIED(desc, 0);
			if (is_403_class())
				desc.cycles = 33;   // 4XX
			else if (is_601_class())
				desc.cycles = 36;   // 601
			else if (is_603_class())
				desc.cycles = 37;   // 603
			else
				desc.cycles = 33;   // ???
			return true;

		case 0x228: // SUBFOx
		case 0x30a: // ADDOx
			GPR_USED(desc, G_RA(op));
			GPR_USED(desc, G_RB(op));
			GPR_MODIFIED(desc, G_RD(op));
			XER_OV_MODIFIED(desc);
			XER_SO_MODIFIED(desc);
			if (op & M_RC)
				CR_MODIFIED(desc, 0);
			return true;

		case 0x013: // MFCR
			CR_USED(desc, 0);
			CR_USED(desc, 1);
			CR_USED(desc, 2);
			CR_USED(desc, 3);
			CR_USED(desc, 4);
			CR_USED(desc, 5);
			CR_USED(desc, 6);
			CR_USED(desc, 7);
			GPR_MODIFIED(desc, G_RD(op));
			return true;

		case 0x136: // ECIWX
			if (!(m_ppc->m_cap & PPCCAP_VEA))
				return false;
		case 0x014: // LWARX
		case 0x017: // LWZX
		case 0x057: // LBZX
		case 0x117: // LHZX
		case 0x157: // LHAX
		case 0x216: // LWBRX
		case 0x316: // LHBRX
			GPR_USED_OR_ZERO(desc, G_RA(op));
			GPR_USED(desc, G_RB(op));
			GPR_MODIFIED(desc, G_RD(op));
			desc.flags |= OPFLAG_READS_MEMORY;
			return true;

		case 0x018: // SLWx
		case 0x01c: // ANDx
		case 0x03c: // ANDCx
		case 0x07c: // NORx
		case 0x11c: // EQVx
		case 0x13c: // XORx
		case 0x19c: // ORCx
		case 0x1bc: // ORx
		case 0x1dc: // NANDx
			GPR_USED(desc, G_RS(op));
			GPR_USED(desc, G_RB(op));
			GPR_MODIFIED(desc, G_RA(op));
			if (op & M_RC)
			{
				XER_SO_USED(desc);
				CR_MODIFIED(desc, 0);
			}
			return true;

		case 0x218: // SRWx
		case 0x318: // SRAWx
			GPR_USED(desc, G_RS(op));
			GPR_USED(desc, G_RB(op));
			GPR_MODIFIED(desc, G_RA(op));
			XER_CA_MODIFIED(desc);
			if (op & M_RC)
			{
				XER_SO_USED(desc);
				CR_MODIFIED(desc, 0);
			}
			return true;

		case 0x01a: // CNTLZWx
		case 0x39a: // EXTSHx
		case 0x3ba: // EXTSBx
			GPR_USED(desc, G_RS(op));
			GPR_MODIFIED(desc, G_RA(op));
			if (op & M_RC)
			{
				XER_SO_USED(desc);
				CR_MODIFIED(desc, 0);
			}
			return true;

		case 0x036: // DCBST
		case 0x056: // DCBF
		case 0x0f6: // DCBTST
		case 0x116: // DCBT
		case 0x2f6: // DCBA
		case 0x3d6: // ICBI
			if (!(m_ppc->m_cap & (PPCCAP_VEA | PPCCAP_4XX)))
				return false;
			GPR_USED_OR_ZERO(desc, G_RA(op));
			GPR_USED(desc, G_RB(op));
			return true;

		case 0x1d6: // DCBI
			if (!(m_ppc->m_cap & (PPCCAP_OEA | PPCCAP_4XX)))
				return false;
			GPR_USED_OR_ZERO(desc, G_RA(op));
			GPR_USED(desc, G_RB(op));
			desc.flags |= OPFLAG_PRIVILEGED | OPFLAG_CAN_CAUSE_EXCEPTION;
			return true;

		case 0x037: // LWZUX
		case 0x077: // LBZUX
		case 0x137: // LHZUX
		case 0x177: // LHAUX
			if (G_RA(op) == 0 || G_RA(op) == G_RD(op))
				return false;
			GPR_USED(desc, G_RA(op));
			GPR_USED(desc, G_RB(op));
			GPR_MODIFIED(desc, G_RD(op));
			GPR_MODIFIED(desc, G_RA(op));
			desc.flags |= OPFLAG_READS_MEMORY;
			return true;

		case 0x153: // MFSPR
			GPR_MODIFIED(desc, G_RD(op));
			spr = compute_spr(G_SPR(op));
			if (spr == SPR_LR)
				LR_USED(desc);
			if (spr == SPR_CTR)
				CTR_USED(desc);
			if (spr == SPR_XER)
			{
				XER_COUNT_USED(desc);
				XER_CA_USED(desc);
				XER_OV_USED(desc);
				XER_SO_USED(desc);
			}
			if (spr & 0x010)
				desc.flags |= OPFLAG_PRIVILEGED | OPFLAG_CAN_CAUSE_EXCEPTION;
			if ((m_ppc->m_cap & PPCCAP_4XX) && spr == SPR4XX_TBLU)
				desc.cycles = POWERPC_COUNT_READ_TBL;
			else if ((m_ppc->m_cap & PPCCAP_VEA) && spr == SPRVEA_TBL_R)
				desc.cycles = POWERPC_COUNT_READ_TBL;
			else if ((m_ppc->m_cap & PPCCAP_OEA) && spr == SPROEA_DEC)
				desc.cycles = POWERPC_COUNT_READ_DEC;
			return true;

		case 0x053: // MFMSR
			GPR_MODIFIED(desc, G_RD(op));
			desc.flags |= OPFLAG_PRIVILEGED | OPFLAG_CAN_CAUSE_EXCEPTION | OPFLAG_CAN_EXPOSE_EXTERNAL_INT;
			if (is_601_class())
				desc.cycles = 2;    // 601
			return true;

		case 0x253: // MFSR
			GPR_MODIFIED(desc, G_RD(op));
			desc.flags |= OPFLAG_PRIVILEGED | OPFLAG_CAN_CAUSE_EXCEPTION;
			return true;

		case 0x293: // MFSRIN
			GPR_USED(desc, G_RB(op));
			GPR_MODIFIED(desc, G_RD(op));
			desc.flags |= OPFLAG_PRIVILEGED | OPFLAG_CAN_CAUSE_EXCEPTION;
			return true;

		case 0x173: // MFTB
			if (!(m_ppc->m_cap & PPCCAP_VEA))
				return false;
			GPR_MODIFIED(desc, G_RD(op));
			spr = compute_spr(G_SPR(op));
			if (spr == SPRVEA_TBL_R)
				desc.cycles = POWERPC_COUNT_READ_TBL;
			return true;

		case 0x068: // NEGx
			GPR_USED(desc, G_RA(op));
			GPR_MODIFIED(desc, G_RD(op));
			if (op & M_RC)
			{
				XER_SO_USED(desc);
				CR_MODIFIED(desc, 0);
			}
			return true;

		case 0x268: // NEGOx
			GPR_USED(desc, G_RA(op));
			GPR_MODIFIED(desc, G_RD(op));
			XER_OV_MODIFIED(desc);
			XER_SO_MODIFIED(desc);
			if (op & M_RC)
				CR_MODIFIED(desc, 0);
			return true;

		case 0x090: // MTCRF
			GPR_USED(desc, G_RS(op));
			if (G_CRM(op) & 0x80) CR_MODIFIED(desc, 0);
			if (G_CRM(op) & 0x40) CR_MODIFIED(desc, 1);
			if (G_CRM(op) & 0x20) CR_MODIFIED(desc, 2);
			if (G_CRM(op) & 0x10) CR_MODIFIED(desc, 3);
			if (G_CRM(op) & 0x08) CR_MODIFIED(desc, 4);
			if (G_CRM(op) & 0x04) CR_MODIFIED(desc, 5);
			if (G_CRM(op) & 0x02) CR_MODIFIED(desc, 6);
			if (G_CRM(op) & 0x01) CR_MODIFIED(desc, 7);
			return true;

		case 0x092: // MTMSR
			GPR_USED(desc, G_RS(op));
			desc.flags |= OPFLAG_PRIVILEGED | OPFLAG_CAN_CAUSE_EXCEPTION | OPFLAG_CAN_CHANGE_MODES | OPFLAG_END_SEQUENCE;
			if (is_601_class())
				desc.cycles = 17;   // 601
			else if (is_603_class())
				desc.cycles = 2;    // 603
			return true;

		case 0x0d2: // MTSR
			if (!(m_ppc->m_cap & PPCCAP_OEA))
				return false;
			GPR_USED(desc, G_RS(op));
			desc.flags |= OPFLAG_PRIVILEGED | OPFLAG_CAN_CAUSE_EXCEPTION;
			return true;

		case 0x1d3: // MTSPR
			GPR_USED(desc, G_RS(op));
			spr = compute_spr(G_SPR(op));
			if (spr == SPR_LR)
				LR_MODIFIED(desc);
			if (spr == SPR_CTR)
				CTR_MODIFIED(desc);
			if (spr == SPR_XER)
			{
				XER_COUNT_MODIFIED(desc);
				XER_CA_MODIFIED(desc);
				XER_OV_MODIFIED(desc);
				XER_SO_MODIFIED(desc);
			}
			if (spr & 0x010)
				desc.flags |= OPFLAG_PRIVILEGED | OPFLAG_CAN_CAUSE_EXCEPTION;
			return true;

		case 0x1b6: // ECOWX
			if (!(m_ppc->m_cap & PPCCAP_VEA))
				return false;
		case 0x096: // STWCX.
		case 0x097: // STWX
		case 0x0d7: // STBX
		case 0x197: // STHX
		case 0x296: // STWBRX
		case 0x396: // STHBRX
			GPR_USED_OR_ZERO(desc, G_RA(op));
			GPR_USED(desc, G_RB(op));
			GPR_USED(desc, G_RS(op));
			desc.flags |= OPFLAG_WRITES_MEMORY;
			return true;

		case 0x0b7: // STWUX
		case 0x0f7: // STBUX
		case 0x1b7: // STHUX
			if (G_RA(op) == 0)
				return false;
			GPR_USED(desc, G_RA(op));
			GPR_USED(desc, G_RB(op));
			GPR_USED(desc, G_RS(op));
			GPR_MODIFIED(desc, G_RA(op));
			desc.flags |= OPFLAG_WRITES_MEMORY;
			return true;

		case 0x0f2: // MTSRIN
			if (!(m_ppc->m_cap & PPCCAP_OEA))
				return false;
			GPR_USED(desc, G_RS(op));
			GPR_USED(desc, G_RB(op));
			desc.flags |= OPFLAG_PRIVILEGED | OPFLAG_CAN_CAUSE_EXCEPTION;
			return true;

		case 0x132: // TLBIE
			if (!(m_ppc->m_cap & PPCCAP_OEA))
				return false;
			GPR_USED(desc, G_RB(op));
			desc.flags |= OPFLAG_PRIVILEGED | OPFLAG_CAN_CAUSE_EXCEPTION;
			return true;

		case 0x172: // TLBIA
			if (!(m_ppc->m_cap & PPCCAP_OEA) || (m_ppc->m_cap & PPCCAP_603_MMU))
				return false;
			desc.flags |= OPFLAG_PRIVILEGED | OPFLAG_CAN_CAUSE_EXCEPTION;
			return true;

		case 0x3d2: // TLBLD
		case 0x3f2: // TLBLI
			if (!(m_ppc->m_cap & PPCCAP_603_MMU) && !is_602_class())
				return false;
			desc.flags |= OPFLAG_PRIVILEGED | OPFLAG_CAN_CAUSE_EXCEPTION;
			return true;

		case 0x200: // MCRXR
			XER_CA_USED(desc);
			XER_OV_USED(desc);
			XER_SO_USED(desc);
			CR_MODIFIED(desc, G_CRFD(op));
			XER_CA_MODIFIED(desc);
			XER_OV_MODIFIED(desc);
			XER_SO_MODIFIED(desc);
			return true;

		case 0x215: // LSWX
			GPR_USED_OR_ZERO(desc, G_RA(op));
			GPR_USED(desc, G_RB(op));
			XER_COUNT_USED(desc);
			for (regnum = 0; regnum < 32; regnum++)
				GPR_MODIFIED(desc, regnum);
			desc.flags |= OPFLAG_READS_MEMORY;
			return true;

		case 0x217: // LFSX
		case 0x257: // LFDX
			if (!(m_ppc->m_cap & PPCCAP_FPU))
				return false;
			GPR_USED_OR_ZERO(desc, G_RA(op));
			GPR_USED(desc, G_RB(op));
			FPR_MODIFIED(desc, G_RD(op));
			desc.flags |= OPFLAG_READS_MEMORY;
			return true;

		case 0x236: // TLBSYNC
			if (!(m_ppc->m_cap & PPCCAP_OEA))
				return false;
			desc.flags |= OPFLAG_PRIVILEGED | OPFLAG_CAN_CAUSE_EXCEPTION;
			return true;

		case 0x256: // SYNC
			return true;

		case 0x356: // EIEIO
			if (!(m_ppc->m_cap & (PPCCAP_VEA | PPCCAP_4XX)))
				return false;
			return true;

		case 0x237: // LFSUX
		case 0x277: // LFDUX
			if (!(m_ppc->m_cap & PPCCAP_FPU))
				return false;
			if (G_RA(op) == 0)
				return false;
			GPR_USED(desc, G_RA(op));
			GPR_USED(desc, G_RB(op));
			GPR_MODIFIED(desc, G_RA(op));
			FPR_MODIFIED(desc, G_RD(op));
			desc.flags |= OPFLAG_READS_MEMORY;
			return true;

		case 0x255: // LSWI
			GPR_USED_OR_ZERO(desc, G_RA(op));
			for (regnum = 0; regnum < ((G_NB(op) - 1) & 0x1f) + 1; regnum += 4)
				GPR_MODIFIED(desc, (G_RD(op) + regnum / 4) % 32);
			desc.flags |= OPFLAG_READS_MEMORY;
			desc.cycles = (((G_NB(op) - 1) & 0x1f) + 1 + 3) / 4;
			return true;

		case 0x295: // STSWX
			GPR_USED_OR_ZERO(desc, G_RA(op));
			GPR_USED(desc, G_RB(op));
			XER_COUNT_USED(desc);
			for (regnum = 0; regnum < 32; regnum++)
				GPR_USED(desc, regnum);
			desc.flags |= OPFLAG_WRITES_MEMORY;
			return true;

		case 0x2d5: // STSWI
			GPR_USED_OR_ZERO(desc, G_RA(op));
			for (regnum = 0; regnum < ((G_NB(op) - 1) & 0x1f) + 1; regnum += 4)
				GPR_USED(desc, (G_RD(op) + regnum / 4) % 32);
			desc.flags |= OPFLAG_WRITES_MEMORY;
			desc.cycles = (((G_NB(op) - 1) & 0x1f) + 1 + 3) / 4;
			return true;

		case 0x297: // STFSX
		case 0x2d7: // STFDX
		case 0x3d7: // STFIWX
			if (!(m_ppc->m_cap & PPCCAP_FPU))
				return false;
			GPR_USED_OR_ZERO(desc, G_RA(op));
			GPR_USED(desc, G_RB(op));
			FPR_USED(desc, G_RS(op));
			desc.flags |= OPFLAG_WRITES_MEMORY;
			return true;

		case 0x2b7: // STFSUX
		case 0x2f7: // STFDUX
			if (!(m_ppc->m_cap & PPCCAP_FPU))
				return false;
			if (G_RA(op) == 0)
				return false;
			GPR_USED_OR_ZERO(desc, G_RA(op));
			GPR_USED(desc, G_RB(op));
			GPR_MODIFIED(desc, G_RA(op));
			FPR_USED(desc, G_RS(op));
			desc.flags |= OPFLAG_WRITES_MEMORY;
			return true;

		case 0x338: // SRAWIx
			GPR_USED(desc, G_RS(op));
			GPR_MODIFIED(desc, G_RA(op));
			XER_CA_MODIFIED(desc);
			if (op & M_RC)
			{
				XER_SO_USED(desc);
				CR_MODIFIED(desc, 0);
			}
			return true;

		case 0x3f6: // DCBZ
			if (!(m_ppc->m_cap & (PPCCAP_VEA | PPCCAP_4XX)))
				return false;
			GPR_USED_OR_ZERO(desc, G_RA(op));
			GPR_USED(desc, G_RB(op));
			desc.flags |= OPFLAG_WRITES_MEMORY;
			return true;

		case 0x106: // ICBT
		case 0x1c6: // DCCCI
		case 0x3c6: // ICCCI
			if (!(m_ppc->m_cap & PPCCAP_4XX))
				return false;
			GPR_USED_OR_ZERO(desc, G_RA(op));
			GPR_USED(desc, G_RB(op));
			desc.flags |= OPFLAG_PRIVILEGED | OPFLAG_CAN_CAUSE_EXCEPTION;
			return true;

		case 0x1e6: // DCREAD
		case 0x3e6: // ICREAD
			if (!(m_ppc->m_cap & PPCCAP_4XX))
				return false;
			GPR_USED_OR_ZERO(desc, G_RA(op));
			GPR_USED(desc, G_RB(op));
			GPR_MODIFIED(desc, G_RT(op));
			desc.flags |= OPFLAG_PRIVILEGED | OPFLAG_CAN_CAUSE_EXCEPTION;
			return true;

		case 0x143: // MFDCR
			if (!(m_ppc->m_cap & PPCCAP_4XX))
				return false;
			GPR_MODIFIED(desc, G_RD(op));
			desc.flags |= OPFLAG_PRIVILEGED | OPFLAG_CAN_CAUSE_EXCEPTION;
			return true;

		case 0x1c3: // MTDCR
			if (!(m_ppc->m_cap & PPCCAP_4XX))
				return false;
			GPR_USED(desc, G_RS(op));
			desc.flags |= OPFLAG_PRIVILEGED | OPFLAG_CAN_CAUSE_EXCEPTION | OPFLAG_CAN_EXPOSE_EXTERNAL_INT;
			return true;

		case 0x083: // WRTEE
			if (!(m_ppc->m_cap & PPCCAP_4XX))
				return false;
			GPR_USED(desc, G_RS(op));
			desc.flags |= OPFLAG_CAN_EXPOSE_EXTERNAL_INT;
			return true;

		case 0x0a3: // WRTEEI
			if (!(m_ppc->m_cap & PPCCAP_4XX))
				return false;
			if (op & MSR_EE)
				desc.flags |= OPFLAG_CAN_EXPOSE_EXTERNAL_INT;
			return true;

		case 0x254: // ESA
		case 0x274: // DSA
			if (!is_602_class())
				return false;
			desc.flags |= OPFLAG_PRIVILEGED | OPFLAG_CAN_CAUSE_EXCEPTION;
			return true;
	}

	return false;
}


/*-------------------------------------------------
    describe_instruction_3b - build a
    description of a single instruction in the
    0x3b group
-------------------------------------------------*/

bool ppc_frontend::describe_3b(UINT32 op, opcode_desc &desc, const opcode_desc *prev)
{
	UINT32 opswitch = (op >> 1) & 0x1f;

	if (!(m_ppc->m_cap & PPCCAP_FPU))
		return false;

	switch (opswitch)
	{
		case 0x12:  // FDIVSx
			FPR_USED(desc, G_RA(op));
			FPR_USED(desc, G_RB(op));
			FPR_MODIFIED(desc, G_RD(op));
			if (op & M_RC)
				CR_MODIFIED(desc, 1);
			if (is_601_class())
				desc.cycles = 17;   // 601
			else if (is_603_class())
				desc.cycles = 18;   // 603
			else
				desc.cycles = 17;   // ???
			FPSCR_MODIFIED(desc, 4);
			return true;

		case 0x14:  // FSUBSx
		case 0x15:  // FADDSx
			FPR_USED(desc, G_RA(op));
			FPR_USED(desc, G_RB(op));
			FPR_MODIFIED(desc, G_RD(op));
			if (op & M_RC)
				CR_MODIFIED(desc, 1);
			FPSCR_MODIFIED(desc, 4);
			return true;

		case 0x19:  // FMULSx - not the same form as FSUB/FADD!
			FPR_USED(desc, G_RA(op));
			FPR_USED(desc, G_REGC(op));
			FPR_MODIFIED(desc, G_RD(op));
			if (op & M_RC)
				CR_MODIFIED(desc, 1);
			FPSCR_MODIFIED(desc, 4);
			return true;

		case 0x16:  // FSQRTSx
		case 0x18:  // FRESx
			FPR_USED(desc, G_RB(op));
			FPR_MODIFIED(desc, G_RD(op));
			if (op & M_RC)
				CR_MODIFIED(desc, 1);
			FPSCR_MODIFIED(desc, 4);
			return true;

		case 0x1c:  // FMSUBSx
		case 0x1d:  // FMADDSx
		case 0x1e:  // FNMSUBSx
		case 0x1f:  // FNMADDSx
			FPR_USED(desc, G_RA(op));
			FPR_USED(desc, G_RB(op));
			FPR_USED(desc, G_REGC(op));
			FPR_MODIFIED(desc, G_RD(op));
			if (op & M_RC)
				CR_MODIFIED(desc, 1);
			FPSCR_MODIFIED(desc, 4);
			return true;
	}

	return false;
}


/*-------------------------------------------------
    describe_instruction_3f - build a
    description of a single instruction in the
    0x3f group
-------------------------------------------------*/

bool ppc_frontend::describe_3f(UINT32 op, opcode_desc &desc, const opcode_desc *prev)
{
	UINT32 opswitch = (op >> 1) & 0x3ff;

	if (!(m_ppc->m_cap & PPCCAP_FPU))
		return false;

	if (opswitch & 0x10)
	{
		opswitch &= 0x1f;
		switch (opswitch)
		{
			case 0x12:  // FDIVx
				FPR_USED(desc, G_RA(op));
				FPR_USED(desc, G_RB(op));
				FPR_MODIFIED(desc, G_RD(op));
				if (op & M_RC)
					CR_MODIFIED(desc, 1);
				if (is_601_class())
					desc.cycles = 31;   // 601
				else if (is_603_class())
					desc.cycles = 33;   // 603
				else
					desc.cycles = 31;   // ???
				FPSCR_MODIFIED(desc, 4);
				return true;

			case 0x19:  // FMULx
				FPR_USED(desc, G_RA(op));
				FPR_USED(desc, G_REGC(op));
				FPR_MODIFIED(desc, G_RD(op));
				if (op & M_RC)
					CR_MODIFIED(desc, 1);
				desc.cycles = 2;    // 601/603
				FPSCR_MODIFIED(desc, 4);
				return true;

			case 0x14:  // FSUBx
			case 0x15:  // FADDx
				FPR_USED(desc, G_RA(op));
				FPR_USED(desc, G_RB(op));
				FPR_MODIFIED(desc, G_RD(op));
				if (op & M_RC)
					CR_MODIFIED(desc, 1);
				FPSCR_MODIFIED(desc, 4);
				return true;

			case 0x16:  // FSQRTx
			case 0x1a:  // FSQRTEx
				FPR_USED(desc, G_RB(op));
				FPR_MODIFIED(desc, G_RD(op));
				if (op & M_RC)
					CR_MODIFIED(desc, 1);
				FPSCR_MODIFIED(desc, 4);
				return true;

			case 0x17:  // FSELx
				FPR_USED(desc, G_RA(op));
				FPR_USED(desc, G_RB(op));
				FPR_USED(desc, G_REGC(op));
				FPR_MODIFIED(desc, G_RD(op));
				if (op & M_RC)
					CR_MODIFIED(desc, 1);
				desc.cycles = 2;    // 601/603
				return true;

			case 0x1c:  // FMSUBx
			case 0x1d:  // FMADDx
			case 0x1e:  // FNMSUBx
			case 0x1f:  // FNMADDx
				FPR_USED(desc, G_RA(op));
				FPR_USED(desc, G_RB(op));
				FPR_USED(desc, G_REGC(op));
				FPR_MODIFIED(desc, G_RD(op));
				if (op & M_RC)
					CR_MODIFIED(desc, 1);
				desc.cycles = 2;    // 601/603
				FPSCR_MODIFIED(desc, 4);
				return true;
		}
	}
	else
	{
		switch (opswitch)
		{
			case 0x32e: // FCTIDx - 64-bit only
			case 0x32f: // FCTIDZx - 64-bit only
			case 0x34e: // FCFIDx - 64-bit only
				return false;

			case 0x000: // FCMPU
			case 0x020: // FCMPO
				FPR_USED(desc, G_RA(op));
				FPR_USED(desc, G_RB(op));
				CR_MODIFIED(desc, G_CRFD(op));
				return true;

			case 0x00c: // FRSPx
			case 0x00e: // FCTIWx
			case 0x00f: // FCTIWZx
				FPSCR_MODIFIED(desc, 4);
			case 0x028: // FNEGx
			case 0x048: // FMRx
			case 0x088: // FNABSx
			case 0x108: // FABSx
				FPR_USED(desc, G_RB(op));
				FPR_MODIFIED(desc, G_RD(op));
				if (op & M_RC)
					CR_MODIFIED(desc, 1);
				return true;

			case 0x026: // MTFSB1x
			case 0x046: // MTFSB0x
				FPSCR_MODIFIED(desc, G_CRBD(op) / 4);
				return true;

			case 0x040: // MCRFS
				FPSCR_USED(desc, G_CRFS(op));
				CR_MODIFIED(desc, G_CRFD(op));
				return true;

			case 0x086: // MTFSFIx
				FPSCR_MODIFIED(desc, G_CRFD(op));
				return true;

			case 0x247: // MFFSx
				FPSCR_USED(desc, 0);
				FPSCR_USED(desc, 1);
				FPSCR_USED(desc, 2);
				FPSCR_USED(desc, 3);
				FPSCR_USED(desc, 4);
				FPSCR_USED(desc, 5);
				FPSCR_USED(desc, 6);
				FPSCR_USED(desc, 7);
				FPR_MODIFIED(desc, G_RD(op));
				return true;

			case 0x2c7: // MTFSFx
				FPR_USED(desc, G_RB(op));
				if (G_CRM(op) & 0x80) FPSCR_MODIFIED(desc, 0);
				if (G_CRM(op) & 0x40) FPSCR_MODIFIED(desc, 1);
				if (G_CRM(op) & 0x20) FPSCR_MODIFIED(desc, 2);
				if (G_CRM(op) & 0x10) FPSCR_MODIFIED(desc, 3);
				if (G_CRM(op) & 0x08) FPSCR_MODIFIED(desc, 4);
				if (G_CRM(op) & 0x04) FPSCR_MODIFIED(desc, 5);
				if (G_CRM(op) & 0x02) FPSCR_MODIFIED(desc, 6);
				if (G_CRM(op) & 0x01) FPSCR_MODIFIED(desc, 7);
				return true;
		}
	}

	return false;
}
