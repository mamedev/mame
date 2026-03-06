// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    ppcfe.c

    Front-end for PowerPC recompiler

***************************************************************************/

#include "emu.h"
#include "ppcfe.h"

#include "ppccom.h"

#include "cpu/drcfe.ipp"

#include <iostream>



/*-------------------------------------------------
    log_flags - write a string representing the
    instruction description flags to a stream
-------------------------------------------------*/

void ppc_device::opcode_desc::log_flags(std::ostream &stream) const
{
	// branches
	if (is_unconditional_branch())
		stream << 'U';
	else if (is_conditional_branch())
		stream << 'C';
	else
		stream << '.';

	// intrablock branches
	stream << (intrablock_branch() ? 'i' : '.');

	// branch targets
	stream << (is_branch_target() ? 'B' : '.');

	// delay slots
	stream << (in_delay_slot() ? 'D' : '.');

	// exceptions
	if (will_cause_exception())
		stream << 'E';
	else if (can_cause_exception())
		stream << 'e';
	else
		stream << '.';

	// read/write
	if (reads_memory())
		stream << 'R';
	else if (writes_memory())
		stream << 'W';
	else
		stream << '.';

	// TLB validation
	stream << (validate_tlb() ? 'V' : '.');

	// redispatch
	stream << (redispatch() ? 'R' : '.');
}


/*-------------------------------------------------
    log_register_list - log a list of registers
-------------------------------------------------*/

void ppc_device::opcode_desc::log_registers_used(std::ostream &stream) const
{
	stream << "[use:";
	log_register_list(stream, regin, nullptr);
	stream << ']';
}

void ppc_device::opcode_desc::log_registers_modified(std::ostream &stream) const
{
	stream << "[mod:";
	log_register_list(stream, regout, &regreq);
	stream << ']';
}

void ppc_device::opcode_desc::log_register_list(std::ostream &stream, const regmask &reglist, const regmask *regnostarlist)
{
	static const char *const crtext[4] = { "lt", "gt", "eq", "so" };
	int count = 0;

	for (int regnum = 0; regnum < 32; regnum++)
	{
		if (reglist[REG_BIT_R0 + regnum])
		{
			if (count++)
				stream << ',';
			stream << 'r' << regnum;
			if (regnostarlist && !(*regnostarlist)[REG_BIT_R0 + regnum])
				stream << '*';
		}
	}

	for (int regnum = 0; regnum < 32; regnum++)
	{
		if (reglist[REG_BIT_FR0 + regnum])
		{
			if (count++)
				stream << ',';
			stream << "fr" << regnum;
			if (regnostarlist && !(*regnostarlist)[REG_BIT_FR0 + regnum])
				stream << '*';
		}
	}

	for (int regnum = 0; regnum < 8; regnum++)
	{
		if (!reg_cr(reglist, regnum))
			continue;

		if (count++)
			stream << ',';
		stream << "cr" << regnum;
		if ((reg_cr(reglist, regnum) == 0x0f) && (!regnostarlist || (reg_cr(*regnostarlist, regnum) == 0x0f) || !reg_cr(*regnostarlist, regnum)))
		{
			if (regnostarlist && !reg_cr(*regnostarlist, regnum))
				stream << '*';
		}
		else
		{
			int crcount = 0;
			stream << '[';
			for (int crnum = 0; crnum < 4; crnum++)
			{
				if (reg_cr_bit(reglist, (regnum * 4) + crnum))
				{
					if (crcount++)
						stream << ',';
					stream << crtext[crnum];
					if (regnostarlist && !reg_cr_bit(*regnostarlist, (regnum * 4) + crnum))
						stream << '*';
				}
			}
			stream << ']';
		}
	}

	const auto log_bit =
			[&stream, &count, &reglist, &regnostarlist] (size_t bit, const char *name)
			{
				if (reglist[bit])
				{
					if (count++)
						stream << ',';
					stream << name;
					if (regnostarlist && !(*regnostarlist)[bit])
						stream << '*';
				}
			};

	log_bit(REG_BIT_XER_CA,     "xer_ca");
	log_bit(REG_BIT_XER_OV,     "xer_ov");
	log_bit(REG_BIT_XER_SO,     "xer_so");
	log_bit(REG_BIT_XER_COUNT,  "xer_count");
	log_bit(REG_BIT_CTR,        "ctr");
	log_bit(REG_BIT_LR,         "lr");

	for (int regnum = 0; regnum < 8; regnum++)
	{
		if (reglist[REG_BIT_FPSCR0 + regnum])
		{
			if (count++)
				stream << ',';
			stream << "fpscr" << regnum;
			if (regnostarlist && !(*regnostarlist)[REG_BIT_FPSCR0 + regnum])
				stream << '*';
		}
	}
}



//**************************************************************************
//  PPC FRONTEND
//**************************************************************************

//-------------------------------------------------
//  frontend - constructor
//-------------------------------------------------

ppc_device::frontend::frontend(ppc_device &ppc, uint32_t window_start, uint32_t window_end, uint32_t max_sequence)
	: drc_frontend_base(ppc.space_config(AS_PROGRAM)->page_shift(), window_start, window_end, max_sequence)
	, m_ppc(ppc)
{
}

ppc_device::frontend::~frontend()
{
}

ppc_device::opcode_desc const *ppc_device::frontend::describe_code(offs_t startpc)
{
	return do_describe_code(
			[this] (opcode_desc &desc, opcode_desc const *prev) { return describe(desc, prev); },
			startpc);
}


//-------------------------------------------------
//  describe - build a description of a single
//  instruction
//-------------------------------------------------

bool ppc_device::frontend::describe(opcode_desc &desc, const opcode_desc *prev)
{
	int regnum;

	// compute the physical PC
	if (m_ppc.ppccom_translate_address_internal(TR_FETCH, false, desc.physpc) > 1)
	{
		// uh-oh: a page fault; leave the description empty and just if this is the first instruction, leave it empty and
		// mark as needing to validate; otherwise, just end the sequence here
		desc.set_validate_tlb();
		desc.set_can_cause_exception();
		desc.set_compiler_page_fault();
		desc.set_virtual_noop();
		desc.set_end_sequence();
		return true;
	}

	// fetch the opcode
	const uint32_t op = desc.opptr = m_ppc.m_pr32(desc.physpc);

	// all instructions are 4 bytes and default to a single cycle each
	desc.length = 4;
	desc.cycles = 1;

	// parse the instruction
	const uint32_t opswitch = op >> 26;
	switch (opswitch)
	{
		case 0x02:  // TDI - 64-bit only
		case 0x1e:  // 0x1e group - 64-bit only
		case 0x3a:  // 0x3a group - 64-bit only
		case 0x3e:  // 0x3e group - 64-bit only
			return false;

		case 0x03:  // TWI
			desc.set_gpr_used(G_RA(op));
			desc.set_can_cause_exception();
			if (is_603_class())
				desc.cycles = 2;    // 603
			return true;

		case 0x07:  // MULLI
			desc.set_gpr_used(G_RA(op));
			desc.set_gpr_modified(G_RD(op));
			if (is_403_class())
				desc.cycles = 4;    // 4XX
			else if (is_601_class())
				desc.cycles = 5;    // 601
			else if (is_603_class())
				desc.cycles = 2;    // 603: 2-3
			else
				desc.cycles = 2;    // ???
			return true;

		case 0x09:  // DOZI (POWER)
			if (!(m_ppc.m_cap & PPCCAP_LEGACY_POWER))
			{
				return false;
			}

			desc.set_gpr_used_or_zero(G_RA(op));
			desc.set_gpr_modified(G_RD(op));
			return true;

		case 0x0e:  // ADDI
		case 0x0f:  // ADDIS
			desc.set_gpr_used_or_zero(G_RA(op));
			desc.set_gpr_modified(G_RD(op));
			return true;

		case 0x0a:  // CMPLI
		case 0x0b:  // CMPI
			desc.set_gpr_used(G_RA(op));
			desc.set_xer_so_used();
			desc.set_cr_modified(G_CRFD(op));
			return true;

		case 0x08:  // SUBFIC
		case 0x0c:  // ADDIC
			desc.set_gpr_used(G_RA(op));
			desc.set_gpr_modified(G_RD(op));
			desc.set_xer_ca_modified();
			return true;

		case 0x0d:  // ADDIC.
			desc.set_gpr_used(G_RA(op));
			desc.set_xer_so_used();
			desc.set_gpr_modified(G_RT(op));
			desc.set_xer_ca_modified();
			desc.set_cr_modified(0);
			return true;

		case 0x10:  // BCx
			if (!(G_BO(op) & 0x10))
			{
				desc.set_cr_bit_used(G_BI(op));
				// branch folding
				if (!prev || (prev->cr_modified() == 0))
					desc.cycles = 0;
			}
			if (!(G_BO(op) & 0x04))
			{
				desc.set_ctr_used();
				desc.set_ctr_modified();
			}
			if (op & M_LK)
				desc.set_lr_modified();
			if ((G_BO(op) & 0x14) == 0x14)
			{
				desc.set_is_unconditional_branch();
				desc.set_end_sequence();
			}
			else
			{
				desc.set_is_conditional_branch();
			}
			desc.targetpc = (int16_t)(G_BD(op) << 2) + ((op & M_AA) ? 0 : desc.pc);
			if (desc.targetpc == desc.pc && desc.cycles == 0)
				desc.cycles = 1;
			return true;

		case 0x11:  // SC
			if (!(m_ppc.m_cap & (PPCCAP_OEA | PPCCAP_4XX)))
				return false;
			desc.set_will_cause_exception();
			desc.set_end_sequence();
			if (is_601_class())
				desc.cycles = 16;   // 601
			else if (is_603_class())
				desc.cycles = 3;    // 603
			else
				desc.cycles = 3;    // ???
			return true;

		case 0x12:  // Bx
			if (op & M_LK)
				desc.set_lr_modified();
			desc.set_is_unconditional_branch();
			desc.set_end_sequence();
			desc.targetpc = ((int32_t)(G_LI(op) << 8) >> 6) + ((op & M_AA) ? 0 : desc.pc);
			// branch folding
			if (desc.targetpc != desc.pc)
				desc.cycles = 0;
			return true;

		case 0x13:  // 0x13 group
			return describe_13(op, desc, prev);

		case 0x14:  // RLWIMIx
			desc.set_gpr_used(G_RS(op));
			desc.set_gpr_used(G_RA(op));
			desc.set_gpr_modified(G_RA(op));
			if (op & M_RC)
			{
				desc.set_xer_so_used();
				desc.set_cr_modified(0);
			}
			return true;

		case 0x15:  // RLWINMx
			desc.set_gpr_used(G_RS(op));
			desc.set_gpr_modified(G_RA(op));
			if (op & M_RC)
			{
				desc.set_xer_so_used();
				desc.set_cr_modified(0);
			}
			return true;

		case 0x17:  // RLWNMx
			desc.set_gpr_used(G_RS(op));
			desc.set_gpr_used(G_RB(op));
			desc.set_gpr_modified(G_RA(op));
			if (op & M_RC)
			{
				desc.set_xer_so_used();
				desc.set_cr_modified(0);
			}
			return true;

		case 0x18:  // ORI
		case 0x19:  // ORIS
		case 0x1a:  // XORI
		case 0x1b:  // XORIS
			desc.set_gpr_used(G_RS(op));
			desc.set_gpr_modified(G_RA(op));
			return true;

		case 0x1c:  // ANDI.
		case 0x1d:  // ANDIS.
			desc.set_gpr_used(G_RS(op));
			desc.set_xer_so_used();
			desc.set_gpr_modified(G_RA(op));
			desc.set_cr_modified(0);
			return true;

		case 0x1f:  // 0x1f group
			return describe_1f(op, desc, prev);

		case 0x20:  // LWZ
		case 0x22:  // LBZ
		case 0x28:  // LHZ
		case 0x2a:  // LHA
			desc.set_gpr_used_or_zero(G_RA(op));
			desc.set_gpr_modified(G_RD(op));
			desc.set_reads_memory();
			return true;

		case 0x21:  // LWZU
		case 0x23:  // LBZU
		case 0x29:  // LHZU
		case 0x2b:  // LHAU
			if (G_RA(op) == 0 || G_RA(op) == G_RD(op))
				return false;
			desc.set_gpr_used(G_RA(op));
			desc.set_gpr_modified(G_RD(op));
			desc.set_gpr_modified(G_RA(op));
			desc.set_reads_memory();
			return true;

		case 0x24:  // STW
		case 0x26:  // STB
		case 0x2c:  // STH
			desc.set_gpr_used_or_zero(G_RA(op));
			desc.set_gpr_used(G_RS(op));
			desc.set_writes_memory();
			return true;

		case 0x25:  // STWU
		case 0x27:  // STBU
		case 0x2d:  // STHU
			if (G_RA(op) == 0)
				return false;
			desc.set_gpr_used(G_RA(op));
			desc.set_gpr_used(G_RS(op));
			desc.set_gpr_modified(G_RA(op));
			desc.set_writes_memory();
			return true;

		case 0x2e:  // LMW
			desc.set_gpr_used_or_zero(G_RA(op));

			for (regnum = G_RD(op); regnum < 32; regnum++)
			{
				if (regnum != G_RA(op) || ((m_ppc.m_cap & PPCCAP_4XX) && regnum == 31))
					desc.set_gpr_modified(regnum);
			}
			desc.set_reads_memory();
			desc.cycles = 32 - G_RD(op);
			return true;

		case 0x2f:  // STMW
			desc.set_gpr_used_or_zero(G_RA(op));
			for (regnum = G_RS(op); regnum < 32; regnum++)
				desc.set_gpr_used(regnum);
			desc.set_writes_memory();
			desc.cycles = 32 - G_RS(op);
			return true;

		case 0x30:  // LFS
		case 0x32:  // LFD
			if (!(m_ppc.m_cap & PPCCAP_FPU))
				return false;
			desc.set_gpr_used_or_zero(G_RA(op));
			desc.set_fpr_modified(G_RD(op));
			desc.set_reads_memory();
			return true;

		case 0x31:  // LFSU
		case 0x33:  // LFDU
			if (!(m_ppc.m_cap & PPCCAP_FPU))
				return false;
			if (G_RA(op) == 0)
				return false;
			desc.set_gpr_used(G_RA(op));
			desc.set_gpr_modified(G_RA(op));
			desc.set_fpr_modified(G_RD(op));
			desc.set_reads_memory();
			return true;

		case 0x34:  // STFS
		case 0x36:  // STFD
			if (!(m_ppc.m_cap & PPCCAP_FPU))
				return false;
			desc.set_gpr_used_or_zero(G_RA(op));
			desc.set_fpr_used(G_RS(op));
			desc.set_writes_memory();
			return true;

		case 0x35:  // STFSU
		case 0x37:  // STFDU
			if (!(m_ppc.m_cap & PPCCAP_FPU))
				return false;
			if (G_RA(op) == 0)
				return false;
			desc.set_gpr_used(G_RA(op));
			desc.set_gpr_modified(G_RA(op));
			desc.set_fpr_used(G_RS(op));
			desc.set_writes_memory();
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

bool ppc_device::frontend::describe_13(uint32_t op, opcode_desc &desc, const opcode_desc *prev)
{
	uint32_t opswitch = (op >> 1) & 0x3ff;

	switch (opswitch)
	{
		case 0x000: // MTCRF
			desc.set_cr_used(G_CRFS(op));
			desc.set_cr_modified(G_CRFD(op));
			// CR logical folding
			if (!prev || (prev->cr_modified() == 0))
				desc.cycles = 0;
			return true;

		case 0x010: // BCLRx
			desc.set_lr_used();
			if (!(G_BO(op) & 0x10))
				desc.set_cr_bit_used(G_BI(op));
			if (!(G_BO(op) & 0x04))
			{
				desc.set_ctr_used();
				desc.set_ctr_modified();
			}
			if (op & M_LK)
				desc.set_lr_modified();
			if ((G_BO(op) & 0x14) == 0x14)
			{
				desc.set_is_unconditional_branch();
				desc.set_end_sequence();
			}
			else
			{
				desc.set_is_conditional_branch();
			}
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
			desc.set_cr_bit_used(G_CRBA(op));
			desc.set_cr_bit_used(G_CRBB(op));
			desc.set_cr_bit_modified(G_CRBD(op));
			// CR logical folding
			if (!prev || (prev->cr_modified() == 0))
				desc.cycles = 0;
			return true;

		case 0x032: // RFI
			if (!(m_ppc.m_cap & (PPCCAP_OEA | PPCCAP_4XX)))
				return false;
			desc.set_is_unconditional_branch();
			desc.set_can_cause_exception();
			desc.set_end_sequence();
			desc.set_privileged();
			desc.set_can_change_modes();
			desc.targetpc = BRANCH_TARGET_DYNAMIC;
			if (is_601_class())
				desc.cycles = 13;   // 601
			else if (is_603_class())
				desc.cycles = 3;    // 603
			else
				desc.cycles = 3;    // ???
			return true;

		case 0x033: // RFCI
			if (!(m_ppc.m_cap & PPCCAP_4XX))
				return false;
			desc.set_is_unconditional_branch();
			desc.set_can_cause_exception();
			desc.set_end_sequence();
			desc.set_privileged();
			desc.set_can_change_modes();
			desc.targetpc = BRANCH_TARGET_DYNAMIC;
			return true;

		case 0x096: // ISYNC
			if (!(m_ppc.m_cap & (PPCCAP_VEA | PPCCAP_4XX)))
				return false;
			if (is_601_class())
				desc.cycles = 6;    // 601
			return true;

		case 0x210: // BCCTRx
			desc.set_ctr_used();
			if (!(G_BO(op) & 0x10))
				desc.set_cr_bit_used(G_BI(op));
			if (!(G_BO(op) & 0x04))
				return false;
			if (op & M_LK)
				desc.set_lr_modified();
			if ((G_BO(op) & 0x14) == 0x14)
			{
				desc.set_is_unconditional_branch();
				desc.set_end_sequence();
			}
			else
			{
				desc.set_is_conditional_branch();
			}
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

bool ppc_device::frontend::describe_1f(uint32_t op, opcode_desc &desc, const opcode_desc *prev)
{
	uint32_t opswitch = (op >> 1) & 0x3ff;
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
			desc.set_gpr_used(G_RA(op));
			desc.set_gpr_used(G_RB(op));
			desc.set_xer_so_used();
			desc.set_cr_modified(G_CRFD(op));
			return true;

		case 0x004: // TW
			desc.set_gpr_used(G_RA(op));
			desc.set_gpr_used(G_RB(op));
			desc.set_can_cause_exception();
			if (is_603_class())
				desc.cycles = 2;    // 603
			return true;

		case 0x008: // SUBFCx
		case 0x00a: // ADDCx
			desc.set_gpr_used(G_RA(op));
			desc.set_gpr_used(G_RB(op));
			desc.set_gpr_modified(G_RD(op));
			desc.set_xer_ca_modified();
			if (op & M_RC)
			{
				desc.set_xer_so_used();
				desc.set_cr_modified(0);
			}
			return true;

		case 0x088: // SUBFEx
		case 0x08a: // ADDEx
			desc.set_gpr_used(G_RA(op));
			desc.set_gpr_used(G_RB(op));
			desc.set_xer_ca_used();
			desc.set_gpr_modified(G_RD(op));
			desc.set_xer_ca_modified();
			if (op & M_RC)
			{
				desc.set_xer_so_used();
				desc.set_cr_modified(0);
			}
			return true;

		case 0x0c8: // SUBFZEx
		case 0x0ca: // ADDZEx
		case 0x0e8: // SUBFMEx
		case 0x0ea: // ADDMEx
			desc.set_gpr_used(G_RA(op));
			desc.set_xer_ca_used();
			desc.set_gpr_modified(G_RD(op));
			desc.set_xer_ca_modified();
			if (op & M_RC)
			{
				desc.set_xer_so_used();
				desc.set_cr_modified(0);
			}
			return true;

		case 0x00b: // MULHWUx
		case 0x04b: // MULHWx
		case 0x0eb: // MULLWx
			desc.set_gpr_used(G_RA(op));
			desc.set_gpr_used(G_RB(op));
			desc.set_gpr_modified(G_RD(op));
			if (op & M_RC)
			{
				desc.set_xer_so_used();
				desc.set_cr_modified(0);
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
			desc.set_gpr_used(G_RA(op));
			desc.set_gpr_used(G_RB(op));
			desc.set_gpr_modified(G_RD(op));
			if (op & M_RC)
			{
				desc.set_xer_so_used();
				desc.set_cr_modified(0);
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
			desc.set_gpr_used(G_RA(op));
			desc.set_gpr_used(G_RB(op));
			desc.set_gpr_modified(G_RD(op));
			if (op & M_RC)
			{
				desc.set_xer_so_used();
				desc.set_cr_modified(0);
			}
			return true;

		case 0x208: // SUBFCOx
		case 0x20a: // ADDCOx
			desc.set_gpr_used(G_RA(op));
			desc.set_gpr_used(G_RB(op));
			desc.set_gpr_modified(G_RD(op));
			desc.set_xer_ov_modified();
			desc.set_xer_so_modified();
			desc.set_xer_ca_modified();
			if (op & M_RC)
				desc.set_cr_modified(0);
			return true;

		case 0x288: // SUBFEOx
		case 0x28a: // ADDEOx
			desc.set_gpr_used(G_RA(op));
			desc.set_gpr_used(G_RB(op));
			desc.set_xer_ca_used();
			desc.set_gpr_modified(G_RD(op));
			desc.set_xer_ov_modified();
			desc.set_xer_so_modified();
			desc.set_xer_ca_modified();
			if (op & M_RC)
				desc.set_cr_modified(0);
			return true;

		case 0x2c8: // SUBFZEOx
		case 0x2ca: // ADDZEOx
		case 0x2e8: // SUBFMEOx
		case 0x2ea: // ADDMEOx
			desc.set_gpr_used(G_RA(op));
			desc.set_xer_ca_used();
			desc.set_gpr_modified(G_RD(op));
			desc.set_xer_ov_modified();
			desc.set_xer_so_modified();
			desc.set_xer_ca_modified();
			if (op & M_RC)
				desc.set_cr_modified(0);
			return true;

		case 0x2eb: // MULLWOx
			desc.set_gpr_used(G_RA(op));
			desc.set_gpr_used(G_RB(op));
			desc.set_gpr_modified(G_RD(op));
			desc.set_xer_ov_modified();
			desc.set_xer_so_modified();
			if (op & M_RC)
				desc.set_cr_modified(0);
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
			desc.set_gpr_used(G_RA(op));
			desc.set_gpr_used(G_RB(op));
			desc.set_gpr_modified(G_RD(op));
			desc.set_xer_ov_modified();
			desc.set_xer_so_modified();
			if (op & M_RC)
				desc.set_cr_modified(0);
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
			desc.set_gpr_used(G_RA(op));
			desc.set_gpr_used(G_RB(op));
			desc.set_gpr_modified(G_RD(op));
			desc.set_xer_ov_modified();
			desc.set_xer_so_modified();
			if (op & M_RC)
				desc.set_cr_modified(0);
			return true;

		case 0x013: // MFCR
			desc.set_cr_used(0);
			desc.set_cr_used(1);
			desc.set_cr_used(2);
			desc.set_cr_used(3);
			desc.set_cr_used(4);
			desc.set_cr_used(5);
			desc.set_cr_used(6);
			desc.set_cr_used(7);
			desc.set_gpr_modified(G_RD(op));
			return true;

		case 0x136: // ECIWX
			if (!(m_ppc.m_cap & PPCCAP_VEA))
				return false;
			[[fallthrough]];
		case 0x014: // LWARX
		case 0x017: // LWZX
		case 0x057: // LBZX
		case 0x117: // LHZX
		case 0x157: // LHAX
		case 0x216: // LWBRX
		case 0x316: // LHBRX
			desc.set_gpr_used_or_zero(G_RA(op));
			desc.set_gpr_used(G_RB(op));
			desc.set_gpr_modified(G_RD(op));
			desc.set_reads_memory();
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
			desc.set_gpr_used(G_RS(op));
			desc.set_gpr_used(G_RB(op));
			desc.set_gpr_modified(G_RA(op));
			if (op & M_RC)
			{
				desc.set_xer_so_used();
				desc.set_cr_modified(0);
			}
			return true;

		case 0x218: // SRWx
		case 0x318: // SRAWx
			desc.set_gpr_used(G_RS(op));
			desc.set_gpr_used(G_RB(op));
			desc.set_gpr_modified(G_RA(op));
			desc.set_xer_ca_modified();
			if (op & M_RC)
			{
				desc.set_xer_so_used();
				desc.set_cr_modified(0);
			}
			return true;

		case 0x01a: // CNTLZWx
		case 0x39a: // EXTSHx
		case 0x3ba: // EXTSBx
			desc.set_gpr_used(G_RS(op));
			desc.set_gpr_modified(G_RA(op));
			if (op & M_RC)
			{
				desc.set_xer_so_used();
				desc.set_cr_modified(0);
			}
			return true;

		case 0x036: // DCBST
		case 0x056: // DCBF
		case 0x0f6: // DCBTST
		case 0x116: // DCBT
		case 0x2f6: // DCBA
		case 0x3d6: // ICBI
			if (!(m_ppc.m_cap & (PPCCAP_VEA | PPCCAP_4XX)))
				return false;
			desc.set_gpr_used_or_zero(G_RA(op));
			desc.set_gpr_used(G_RB(op));
			return true;

		case 0x1d6: // DCBI
			if (!(m_ppc.m_cap & (PPCCAP_OEA | PPCCAP_4XX)))
				return false;
			desc.set_gpr_used_or_zero(G_RA(op));
			desc.set_gpr_used(G_RB(op));
			desc.set_can_cause_exception();
			desc.set_privileged();
			return true;

		case 0x037: // LWZUX
		case 0x077: // LBZUX
		case 0x137: // LHZUX
		case 0x177: // LHAUX
			if (G_RA(op) == 0 || G_RA(op) == G_RD(op))
				return false;
			desc.set_gpr_used(G_RA(op));
			desc.set_gpr_used(G_RB(op));
			desc.set_gpr_modified(G_RD(op));
			desc.set_gpr_modified(G_RA(op));
			desc.set_reads_memory();
			return true;

		case 0x153: // MFSPR
			desc.set_gpr_modified(G_RD(op));
			spr = compute_spr(G_SPR(op));
			if (spr == SPR_LR)
				desc.set_lr_used();
			if (spr == SPR_CTR)
				desc.set_ctr_used();
			if (spr == SPR_XER)
			{
				desc.set_xer_count_used();
				desc.set_xer_ca_used();
				desc.set_xer_ov_used();
				desc.set_xer_so_used();
			}
			if (spr & 0x010)
			{
				desc.set_can_cause_exception();
				desc.set_privileged();
			}
			if ((m_ppc.m_cap & PPCCAP_4XX) && spr == SPR4XX_TBLU)
				desc.cycles = POWERPC_COUNT_READ_TBL;
			else if ((m_ppc.m_cap & PPCCAP_VEA) && spr == SPRVEA_TBL_R)
				desc.cycles = POWERPC_COUNT_READ_TBL;
			else if ((m_ppc.m_cap & PPCCAP_OEA) && spr == SPROEA_DEC)
				desc.cycles = POWERPC_COUNT_READ_DEC;
			return true;

		case 0x053: // MFMSR
			desc.set_gpr_modified(G_RD(op));
			desc.set_can_cause_exception();
			desc.set_can_expose_external_int();
			desc.set_privileged();
			if (is_601_class())
				desc.cycles = 2;    // 601
			return true;

		case 0x253: // MFSR
			desc.set_gpr_modified(G_RD(op));
			desc.set_can_cause_exception();
			desc.set_privileged();
			return true;

		case 0x293: // MFSRIN
			desc.set_gpr_used(G_RB(op));
			desc.set_gpr_modified(G_RD(op));
			desc.set_can_cause_exception();
			desc.set_privileged();
			return true;

		case 0x173: // MFTB
			if (!(m_ppc.m_cap & PPCCAP_VEA))
				return false;
			desc.set_gpr_modified(G_RD(op));
			spr = compute_spr(G_SPR(op));
			if (spr == SPRVEA_TBL_R)
				desc.cycles = POWERPC_COUNT_READ_TBL;
			return true;

		case 0x068: // NEGx
			desc.set_gpr_used(G_RA(op));
			desc.set_gpr_modified(G_RD(op));
			if (op & M_RC)
			{
				desc.set_xer_so_used();
				desc.set_cr_modified(0);
			}
			return true;

		case 0x268: // NEGOx
			desc.set_gpr_used(G_RA(op));
			desc.set_gpr_modified(G_RD(op));
			desc.set_xer_ov_modified();
			desc.set_xer_so_modified();
			if (op & M_RC)
				desc.set_cr_modified(0);
			return true;

		case 0x090: // MTCRF
			desc.set_gpr_used(G_RS(op));
			if (G_CRM(op) & 0x80) desc.set_cr_modified(0);
			if (G_CRM(op) & 0x40) desc.set_cr_modified(1);
			if (G_CRM(op) & 0x20) desc.set_cr_modified(2);
			if (G_CRM(op) & 0x10) desc.set_cr_modified(3);
			if (G_CRM(op) & 0x08) desc.set_cr_modified(4);
			if (G_CRM(op) & 0x04) desc.set_cr_modified(5);
			if (G_CRM(op) & 0x02) desc.set_cr_modified(6);
			if (G_CRM(op) & 0x01) desc.set_cr_modified(7);
			return true;

		case 0x092: // MTMSR
			desc.set_gpr_used(G_RS(op));
			desc.set_can_cause_exception();
			desc.set_end_sequence();
			desc.set_privileged();
			desc.set_can_change_modes();
			if (is_601_class())
				desc.cycles = 17;   // 601
			else if (is_603_class())
				desc.cycles = 2;    // 603
			return true;

		case 0x0d2: // MTSR
			if (!(m_ppc.m_cap & PPCCAP_OEA))
				return false;
			desc.set_gpr_used(G_RS(op));
			desc.set_can_cause_exception();
			desc.set_privileged();
			return true;

		case 0x1d3: // MTSPR
			desc.set_gpr_used(G_RS(op));
			spr = compute_spr(G_SPR(op));
			if (spr == SPR_LR)
				desc.set_lr_modified();
			if (spr == SPR_CTR)
				desc.set_ctr_modified();
			if (spr == SPR_XER)
			{
				desc.set_xer_count_modified();
				desc.set_xer_ca_modified();
				desc.set_xer_ov_modified();
				desc.set_xer_so_modified();
			}
			if (spr & 0x010)
			{
				desc.set_can_cause_exception();
				desc.set_privileged();
			}
			return true;

		case 0x1b6: // ECOWX
			if (!(m_ppc.m_cap & PPCCAP_VEA))
				return false;
			[[fallthrough]];
		case 0x096: // STWCX.
		case 0x097: // STWX
		case 0x0d7: // STBX
		case 0x197: // STHX
		case 0x296: // STWBRX
		case 0x396: // STHBRX
			desc.set_gpr_used_or_zero(G_RA(op));
			desc.set_gpr_used(G_RB(op));
			desc.set_gpr_used(G_RS(op));
			desc.set_writes_memory();
			return true;

		case 0x0b7: // STWUX
		case 0x0f7: // STBUX
		case 0x1b7: // STHUX
			if (G_RA(op) == 0)
				return false;
			desc.set_gpr_used(G_RA(op));
			desc.set_gpr_used(G_RB(op));
			desc.set_gpr_used(G_RS(op));
			desc.set_gpr_modified(G_RA(op));
			desc.set_writes_memory();
			return true;

		case 0x0f2: // MTSRIN
			if (!(m_ppc.m_cap & PPCCAP_OEA))
				return false;
			desc.set_gpr_used(G_RS(op));
			desc.set_gpr_used(G_RB(op));
			desc.set_can_cause_exception();
			desc.set_privileged();
			return true;

		case 0x132: // TLBIE
			if (!(m_ppc.m_cap & PPCCAP_OEA))
				return false;
			desc.set_gpr_used(G_RB(op));
			desc.set_can_cause_exception();
			desc.set_privileged();
			return true;

		case 0x172: // TLBIA
			if (!(m_ppc.m_cap & PPCCAP_OEA) || (m_ppc.m_cap & PPCCAP_603_MMU))
				return false;
			desc.set_can_cause_exception();
			desc.set_privileged();
			return true;

		case 0x3d2: // TLBLD
		case 0x3f2: // TLBLI
			if (!(m_ppc.m_cap & PPCCAP_603_MMU) && !is_602_class())
				return false;
			desc.set_can_cause_exception();
			desc.set_privileged();
			return true;

		case 0x200: // MCRXR
			desc.set_xer_ca_used();
			desc.set_xer_ov_used();
			desc.set_xer_so_used();
			desc.set_cr_modified(G_CRFD(op));
			desc.set_xer_ca_modified();
			desc.set_xer_ov_modified();
			desc.set_xer_so_modified();
			return true;

		case 0x215: // LSWX
			desc.set_gpr_used_or_zero(G_RA(op));
			desc.set_gpr_used(G_RB(op));
			desc.set_xer_count_used();
			for (regnum = 0; regnum < 32; regnum++)
				desc.set_gpr_modified(regnum);
			desc.set_reads_memory();
			return true;

		case 0x217: // LFSX
		case 0x257: // LFDX
			if (!(m_ppc.m_cap & PPCCAP_FPU))
				return false;
			desc.set_gpr_used_or_zero(G_RA(op));
			desc.set_gpr_used(G_RB(op));
			desc.set_fpr_modified(G_RD(op));
			desc.set_reads_memory();
			return true;

		case 0x236: // TLBSYNC
			if (!(m_ppc.m_cap & PPCCAP_OEA))
				return false;
			desc.set_can_cause_exception();
			desc.set_privileged();
			return true;

		case 0x256: // SYNC
			return true;

		case 0x356: // EIEIO
			if (!(m_ppc.m_cap & (PPCCAP_VEA | PPCCAP_4XX)))
				return false;
			return true;

		case 0x237: // LFSUX
		case 0x277: // LFDUX
			if (!(m_ppc.m_cap & PPCCAP_FPU))
				return false;
			if (G_RA(op) == 0)
				return false;
			desc.set_gpr_used(G_RA(op));
			desc.set_gpr_used(G_RB(op));
			desc.set_gpr_modified(G_RA(op));
			desc.set_fpr_modified(G_RD(op));
			desc.set_reads_memory();
			return true;

		case 0x255: // LSWI
			desc.set_gpr_used_or_zero(G_RA(op));
			for (regnum = 0; regnum < ((G_NB(op) - 1) & 0x1f) + 1; regnum += 4)
				desc.set_gpr_modified((G_RD(op) + regnum / 4) % 32);
			desc.set_reads_memory();
			desc.cycles = (((G_NB(op) - 1) & 0x1f) + 1 + 3) / 4;
			return true;

		case 0x295: // STSWX
			desc.set_gpr_used_or_zero(G_RA(op));
			desc.set_gpr_used(G_RB(op));
			desc.set_xer_count_used();
			for (regnum = 0; regnum < 32; regnum++)
				desc.set_gpr_used(regnum);
			desc.set_writes_memory();
			return true;

		case 0x2d5: // STSWI
			desc.set_gpr_used_or_zero(G_RA(op));
			for (regnum = 0; regnum < ((G_NB(op) - 1) & 0x1f) + 1; regnum += 4)
				desc.set_gpr_used((G_RD(op) + regnum / 4) % 32);
			desc.set_writes_memory();
			desc.cycles = (((G_NB(op) - 1) & 0x1f) + 1 + 3) / 4;
			return true;

		case 0x297: // STFSX
		case 0x2d7: // STFDX
		case 0x3d7: // STFIWX
			if (!(m_ppc.m_cap & PPCCAP_FPU))
				return false;
			desc.set_gpr_used_or_zero(G_RA(op));
			desc.set_gpr_used(G_RB(op));
			desc.set_fpr_used(G_RS(op));
			desc.set_writes_memory();
			return true;

		case 0x2b7: // STFSUX
		case 0x2f7: // STFDUX
			if (!(m_ppc.m_cap & PPCCAP_FPU))
				return false;
			if (G_RA(op) == 0)
				return false;
			desc.set_gpr_used_or_zero(G_RA(op));
			desc.set_gpr_used(G_RB(op));
			desc.set_gpr_modified(G_RA(op));
			desc.set_fpr_used(G_RS(op));
			desc.set_writes_memory();
			return true;

		case 0x338: // SRAWIx
			desc.set_gpr_used(G_RS(op));
			desc.set_gpr_modified(G_RA(op));
			desc.set_xer_ca_modified();
			if (op & M_RC)
			{
				desc.set_xer_so_used();
				desc.set_cr_modified(0);
			}
			return true;

		case 0x3f6: // DCBZ
			if (!(m_ppc.m_cap & (PPCCAP_VEA | PPCCAP_4XX)))
				return false;
			desc.set_gpr_used_or_zero(G_RA(op));
			desc.set_gpr_used(G_RB(op));
			desc.set_writes_memory();
			return true;

		case 0x106: // ICBT
		case 0x1c6: // DCCCI
		case 0x3c6: // ICCCI
			if (!(m_ppc.m_cap & PPCCAP_4XX))
				return false;
			desc.set_gpr_used_or_zero(G_RA(op));
			desc.set_gpr_used(G_RB(op));
			desc.set_can_cause_exception();
			desc.set_privileged();
			return true;

		case 0x1e6: // DCREAD
		case 0x3e6: // ICREAD
			if (!(m_ppc.m_cap & PPCCAP_4XX))
				return false;
			desc.set_gpr_used_or_zero(G_RA(op));
			desc.set_gpr_used(G_RB(op));
			desc.set_gpr_modified(G_RT(op));
			desc.set_can_cause_exception();
			desc.set_privileged();
			return true;

		case 0x143: // MFDCR
			if (!(m_ppc.m_cap & PPCCAP_4XX))
				return false;
			desc.set_gpr_modified(G_RD(op));
			desc.set_can_cause_exception();
			desc.set_privileged();
			return true;

		case 0x1c3: // MTDCR
			if (!(m_ppc.m_cap & PPCCAP_4XX))
				return false;
			desc.set_gpr_used(G_RS(op));
			desc.set_can_cause_exception();
			desc.set_can_expose_external_int();
			desc.set_privileged();
			return true;

		case 0x083: // WRTEE
			if (!(m_ppc.m_cap & PPCCAP_4XX))
				return false;
			desc.set_gpr_used(G_RS(op));
			desc.set_can_expose_external_int();
			return true;

		case 0x0a3: // WRTEEI
			if (!(m_ppc.m_cap & PPCCAP_4XX))
				return false;
			if (op & MSR_EE)
				desc.set_can_expose_external_int();
			return true;

		case 0x254: // ESA
		case 0x274: // DSA
			if (!is_602_class())
				return false;
			desc.set_can_cause_exception();
			desc.set_end_sequence();
			desc.set_can_change_modes();
			return true;

		case 0x14b: // DIV (POWER)
		case 0x16b: // DIVS (POWER)
			if (!(m_ppc.m_cap & PPCCAP_LEGACY_POWER))
			{
				return false;
			}

			desc.set_gpr_used(G_RA(op));
			desc.set_gpr_used(G_RB(op));
			desc.set_gpr_modified(G_RD(op));
			desc.set_xer_ov_modified();
			desc.set_xer_so_modified();
			if (op & M_RC)
				desc.set_cr_modified(0);
			return true;

		case 0x6b: // MUL (POWER)
			if (!(m_ppc.m_cap & PPCCAP_LEGACY_POWER))
			{
				return false;
			}

			desc.set_gpr_used(G_RA(op));
			desc.set_gpr_used(G_RB(op));
			desc.set_gpr_modified(G_RD(op));
			desc.set_xer_ov_modified();
			desc.set_xer_so_modified();
			if (op & M_RC)
				desc.set_cr_modified(0);
			return true;

		case 0x108: // DOZ (POWER)
			if (!(m_ppc.m_cap & PPCCAP_LEGACY_POWER))
			{
				return false;
			}

			desc.set_gpr_used(G_RA(op));
			desc.set_gpr_used(G_RB(op));
			desc.set_gpr_modified(G_RD(op));
			desc.set_xer_ov_modified();
			desc.set_xer_so_modified();
			if (op & M_RC)
				desc.set_cr_modified(0);
			return true;

		case 0x168: // ABS (POWER)
		case 0x1e8: // NABS (POWER)
			if (!(m_ppc.m_cap & PPCCAP_LEGACY_POWER))
			{
				return false;
			}

			desc.set_gpr_used(G_RA(op));
			desc.set_gpr_used(G_RB(op));
			desc.set_gpr_modified(G_RD(op));
			desc.set_xer_ov_modified();
			desc.set_xer_so_modified();
			if (op & M_RC)
				desc.set_cr_modified(0);
			return true;

		case 0x21d: // MASKIR (POWER)
			if (!(m_ppc.m_cap & PPCCAP_LEGACY_POWER))
			{
				return false;
			}

			desc.set_gpr_used(G_RS(op));
			desc.set_gpr_used(G_RB(op));
			desc.set_gpr_modified(G_RA(op));
			if (op & M_RC)
				desc.set_cr_modified(0);
			return true;
	}

	return false;
}


/*-------------------------------------------------
    describe_instruction_3b - build a
    description of a single instruction in the
    0x3b group
-------------------------------------------------*/

bool ppc_device::frontend::describe_3b(uint32_t op, opcode_desc &desc, const opcode_desc *prev)
{
	uint32_t opswitch = (op >> 1) & 0x1f;

	if (!(m_ppc.m_cap & PPCCAP_FPU))
		return false;

	switch (opswitch)
	{
		case 0x12:  // FDIVSx
			desc.set_fpr_used(G_RA(op));
			desc.set_fpr_used(G_RB(op));
			desc.set_fpr_modified(G_RD(op));
			if (op & M_RC)
				desc.set_cr_modified(1);
			if (is_601_class())
				desc.cycles = 17;   // 601
			else if (is_603_class())
				desc.cycles = 18;   // 603
			else
				desc.cycles = 17;   // ???
			desc.set_fpscr_modified(4);
			return true;

		case 0x14:  // FSUBSx
		case 0x15:  // FADDSx
			desc.set_fpr_used(G_RA(op));
			desc.set_fpr_used(G_RB(op));
			desc.set_fpr_modified(G_RD(op));
			if (op & M_RC)
				desc.set_cr_modified(1);
			desc.set_fpscr_modified(4);
			return true;

		case 0x19:  // FMULSx - not the same form as FSUB/FADD!
			desc.set_fpr_used(G_RA(op));
			desc.set_fpr_used(G_REGC(op));
			desc.set_fpr_modified(G_RD(op));
			if (op & M_RC)
				desc.set_cr_modified(1);
			desc.set_fpscr_modified(4);
			return true;

		case 0x16:  // FSQRTSx
		case 0x18:  // FRESx
			desc.set_fpr_used(G_RB(op));
			desc.set_fpr_modified(G_RD(op));
			if (op & M_RC)
				desc.set_cr_modified(1);
			desc.set_fpscr_modified(4);
			return true;

		case 0x1c:  // FMSUBSx
		case 0x1d:  // FMADDSx
		case 0x1e:  // FNMSUBSx
		case 0x1f:  // FNMADDSx
			desc.set_fpr_used(G_RA(op));
			desc.set_fpr_used(G_RB(op));
			desc.set_fpr_used(G_REGC(op));
			desc.set_fpr_modified(G_RD(op));
			if (op & M_RC)
				desc.set_cr_modified(1);
			desc.set_fpscr_modified(4);
			return true;
	}

	return false;
}


/*-------------------------------------------------
    describe_instruction_3f - build a
    description of a single instruction in the
    0x3f group
-------------------------------------------------*/

bool ppc_device::frontend::describe_3f(uint32_t op, opcode_desc &desc, const opcode_desc *prev)
{
	uint32_t opswitch = (op >> 1) & 0x3ff;

	if (!(m_ppc.m_cap & PPCCAP_FPU))
		return false;

	if (opswitch & 0x10)
	{
		opswitch &= 0x1f;
		switch (opswitch)
		{
			case 0x12:  // FDIVx
				desc.set_fpr_used(G_RA(op));
				desc.set_fpr_used(G_RB(op));
				desc.set_fpr_modified(G_RD(op));
				if (op & M_RC)
					desc.set_cr_modified(1);
				if (is_601_class())
					desc.cycles = 31;   // 601
				else if (is_603_class())
					desc.cycles = 33;   // 603
				else
					desc.cycles = 31;   // ???
				desc.set_fpscr_modified(4);
				return true;

			case 0x19:  // FMULx
				desc.set_fpr_used(G_RA(op));
				desc.set_fpr_used(G_REGC(op));
				desc.set_fpr_modified(G_RD(op));
				if (op & M_RC)
					desc.set_cr_modified(1);
				desc.cycles = 2;    // 601/603
				desc.set_fpscr_modified(4);
				return true;

			case 0x14:  // FSUBx
			case 0x15:  // FADDx
				desc.set_fpr_used(G_RA(op));
				desc.set_fpr_used(G_RB(op));
				desc.set_fpr_modified(G_RD(op));
				if (op & M_RC)
					desc.set_cr_modified(1);
				desc.set_fpscr_modified(4);
				return true;

			case 0x16:  // FSQRTx
			case 0x1a:  // FSQRTEx
				desc.set_fpr_used(G_RB(op));
				desc.set_fpr_modified(G_RD(op));
				if (op & M_RC)
					desc.set_cr_modified(1);
				desc.set_fpscr_modified(4);
				return true;

			case 0x17:  // FSELx
				desc.set_fpr_used(G_RA(op));
				desc.set_fpr_used(G_RB(op));
				desc.set_fpr_used(G_REGC(op));
				desc.set_fpr_modified(G_RD(op));
				if (op & M_RC)
					desc.set_cr_modified(1);
				desc.cycles = 2;    // 601/603
				return true;

			case 0x1c:  // FMSUBx
			case 0x1d:  // FMADDx
			case 0x1e:  // FNMSUBx
			case 0x1f:  // FNMADDx
				desc.set_fpr_used(G_RA(op));
				desc.set_fpr_used(G_RB(op));
				desc.set_fpr_used(G_REGC(op));
				desc.set_fpr_modified(G_RD(op));
				if (op & M_RC)
					desc.set_cr_modified(1);
				desc.cycles = 2;    // 601/603
				desc.set_fpscr_modified(4);
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
				desc.set_fpr_used(G_RA(op));
				desc.set_fpr_used(G_RB(op));
				desc.set_cr_modified(G_CRFD(op));
				return true;

			case 0x00c: // FRSPx
			case 0x00e: // FCTIWx
			case 0x00f: // FCTIWZx
				desc.set_fpscr_modified(4);
				[[fallthrough]];
			case 0x028: // FNEGx
			case 0x048: // FMRx
			case 0x088: // FNABSx
			case 0x108: // FABSx
				desc.set_fpr_used(G_RB(op));
				desc.set_fpr_modified(G_RD(op));
				if (op & M_RC)
					desc.set_cr_modified(1);
				return true;

			case 0x026: // MTFSB1x
			case 0x046: // MTFSB0x
				desc.set_fpscr_modified(G_CRBD(op) / 4);
				return true;

			case 0x040: // MCRFS
				desc.set_fpscr_used(G_CRFS(op));
				desc.set_cr_modified(G_CRFD(op));
				return true;

			case 0x086: // MTFSFIx
				desc.set_fpscr_modified(G_CRFD(op));
				return true;

			case 0x247: // MFFSx
				desc.set_fpscr_used(0);
				desc.set_fpscr_used(1);
				desc.set_fpscr_used(2);
				desc.set_fpscr_used(3);
				desc.set_fpscr_used(4);
				desc.set_fpscr_used(5);
				desc.set_fpscr_used(6);
				desc.set_fpscr_used(7);
				desc.set_fpr_modified(G_RD(op));
				return true;

			case 0x2c7: // MTFSFx
				desc.set_fpr_used(G_RB(op));
				if (G_CRM(op) & 0x80) desc.set_fpscr_modified(0);
				if (G_CRM(op) & 0x40) desc.set_fpscr_modified(1);
				if (G_CRM(op) & 0x20) desc.set_fpscr_modified(2);
				if (G_CRM(op) & 0x10) desc.set_fpscr_modified(3);
				if (G_CRM(op) & 0x08) desc.set_fpscr_modified(4);
				if (G_CRM(op) & 0x04) desc.set_fpscr_modified(5);
				if (G_CRM(op) & 0x02) desc.set_fpscr_modified(6);
				if (G_CRM(op) & 0x01) desc.set_fpscr_modified(7);
				return true;
		}
	}

	return false;
}
