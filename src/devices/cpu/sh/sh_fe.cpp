// license:BSD-3-Clause
// copyright-holders:David Haywood, R. Belmont
/***************************************************************************

    sh_fe.cpp

    Front end for SuperH recompiler

***************************************************************************/

#include "emu.h"
#include "sh_fe.h"

#include "cpu/drcfe.ipp"


/***************************************************************************
    INSTRUCTION PARSERS
***************************************************************************/

sh_common_execution::frontend::frontend(sh_common_execution *device, uint32_t window_start, uint32_t window_end, uint32_t max_sequence)
	: drc_frontend_base(device->space_config(AS_PROGRAM)->page_shift(), window_start, window_end, max_sequence)
	, m_sh(device)
{
}

sh_common_execution::frontend::~frontend()
{
}

sh_common_execution::opcode_desc const *sh_common_execution::frontend::describe_code(offs_t startpc)
{
	return do_describe_code(
			[this] (opcode_desc &desc, opcode_desc const *prev) { return describe(desc, prev); },
			startpc);
}


inline uint16_t sh_common_execution::frontend::read_word(opcode_desc &desc)
{
	return m_sh->m_pr16(desc.physpc);
}

/*-------------------------------------------------
    describe_instruction - build a description
    of a single instruction
-------------------------------------------------*/

bool sh_common_execution::frontend::describe(opcode_desc &desc, const opcode_desc *prev)
{
	/* fetch the opcode */
	const uint16_t opcode = desc.opptr = read_word(desc);

	/* all instructions are 2 bytes and most are a single cycle */
	desc.length = 2;
	desc.cycles = 1;

	switch (opcode>>12)
	{
	case  0:
		return describe_group_0(desc, prev, opcode);

	case  1:    // MOVLS4
		desc.set_r_used(REG_M);
		desc.set_r_used(REG_N);
		desc.set_writes_memory();
		return true;

	case  2:
		return describe_group_2(desc, prev, opcode);

	case  3:
		return describe_group_3(desc, prev, opcode);

	case  4:
		return describe_group_4(desc, prev, opcode);

	case  5:    // MOVLL4
		desc.set_r_used(REG_M);
		desc.set_r_modified(REG_N);
		desc.set_reads_memory();
		return true;

	case  6:
		return describe_group_6(desc, prev, opcode);

	case  7:    // ADDI
		desc.set_r_used(REG_N);
		desc.set_r_modified(REG_N);
		return true;

	case  8:
		return describe_group_8(desc, prev, opcode);

	case  9:    // MOVWI
		desc.set_r_modified(REG_N);
		desc.set_reads_memory();
		return true;

	case 11:    // BSR
		desc.set_pr_modified();
		[[fallthrough]]; // BSR is BRA with the addition of PR = the return address
	case 10:    // BRA
		{
			int32_t disp = util::sext(opcode, 12);

			desc.set_is_unconditional_branch();
			desc.set_end_sequence();
			desc.targetpc = (desc.pc + 2) + disp * 2 + 2;
			desc.delayslots = 1;
			desc.cycles = 2;
			return true;
		}

	case 12:
		return describe_group_12(desc, prev, opcode);

	case 13:    // MOVLI
		desc.set_r_modified(REG_N);
		desc.set_reads_memory();
		return true;

	case 14:    // MOVI
		desc.set_r_modified(REG_N);
		return true;

	case 15:    // NOP
		return describe_group_15(desc, prev, opcode);
	}

	return false;
}


bool sh_common_execution::frontend::describe_group_2(opcode_desc &desc, const opcode_desc *prev, uint16_t opcode)
{
	switch (opcode & 15)
	{
	case  0: // MOVBS(Rm, Rn);
	case  1: // MOVWS(Rm, Rn);
	case  2: // MOVLS(Rm, Rn);
		desc.set_r_used(REG_M);
		desc.set_r_used(REG_N);
		desc.set_writes_memory();
		return true;

	case  3: // NOP();
		return true;

	case  4: // MOVBM(Rm, Rn);
	case  5: // MOVWM(Rm, Rn);
	case  6: // MOVLM(Rm, Rn);
	case 13: // XTRCT(Rm, Rn);
		desc.set_r_used(REG_M);
		desc.set_r_used(REG_N);
		desc.set_r_modified(REG_N);
		desc.set_writes_memory();
		return true;

	case  7: // DIV0S(Rm, Rn);
	case  8: // TST(Rm, Rn);
	case 12: // CMPSTR(Rm, Rn);
		desc.set_r_used(REG_M);
		desc.set_r_used(REG_N);
		desc.set_sr_modified();
		return true;

	case  9: // AND(Rm, Rn);
	case 10: // XOR(Rm, Rn);
	case 11: // OR(Rm, Rn);
		desc.set_r_used(REG_M);
		desc.set_r_used(REG_N);
		desc.set_r_modified(REG_N);
		return true;

	case 14: // MULU(Rm, Rn);
	case 15: // MULS(Rm, Rn);
		desc.set_r_used(REG_M);
		desc.set_r_used(REG_N);
		desc.set_mac_modified();
		desc.cycles = 2;
		return true;
	}

	return false;
}

bool sh_common_execution::frontend::describe_group_3(opcode_desc &desc, const opcode_desc *prev, uint16_t opcode)
{
	switch (opcode & 15)
	{
	case  0: // CMPEQ(Rm, Rn);
	case  2: // CMPHS(Rm, Rn);
	case  3: // CMPGE(Rm, Rn);
	case  6: // CMPHI(Rm, Rn);
	case  7: // CMPGT(Rm, Rn);
		desc.set_r_used(REG_M);
		desc.set_r_used(REG_N);
		desc.set_sr_modified();
		return true;

	case  1: // NOP();
	case  9: // NOP();
		return true;

	case  4: // DIV1(Rm, Rn);
		desc.set_r_used(REG_M);
		desc.set_r_used(REG_N);
		desc.set_r_modified(REG_N);
		desc.set_sr_modified();
		return true;

	case  5: // DMULU(Rm, Rn);
	case 13: // DMULS(Rm, Rn);
		desc.set_r_used(REG_M);
		desc.set_r_used(REG_N);
		desc.set_mac_modified();
		desc.cycles = 2;
		return true;

	case  8: // SUB(Rm, Rn);
	case 12: // ADD(Rm, Rn);
		desc.set_r_used(REG_M);
		desc.set_r_used(REG_N);
		desc.set_r_modified(REG_N);
		return true;

	case 10: // SUBC(Rm, Rn);
	case 11: // SUBV(Rm, Rn);
	case 14: // ADDC(Rm, Rn);
	case 15: // ADDV(Rm, Rn);
		desc.set_r_used(REG_M);
		desc.set_r_used(REG_N);
		desc.set_sr_used();
		desc.set_r_modified(REG_N);
		desc.set_sr_modified();
		return true;
	}
	return false;
}


bool sh_common_execution::frontend::describe_group_6(opcode_desc &desc, const opcode_desc *prev, uint16_t opcode)
{
	switch (opcode & 15)
	{
	case  0: // MOVBL(Rm, Rn);
	case  1: // MOVWL(Rm, Rn);
	case  2: // MOVLL(Rm, Rn);
	case  3: // MOV(Rm, Rn);
	case  7: // NOT(Rm, Rn);
	case  9: // SWAPW(Rm, Rn);
	case 11: // NEG(Rm, Rn);
	case 12: // EXTUB(Rm, Rn);
	case 13: // EXTUW(Rm, Rn);
	case 14: // EXTSB(Rm, Rn);
	case 15: // EXTSW(Rm, Rn);
		desc.set_r_used(REG_M);
		desc.set_r_modified(REG_N);
		return true;

	case  4: // MOVBP(Rm, Rn);
	case  5: // MOVWP(Rm, Rn);
	case  6: // MOVLP(Rm, Rn);
		desc.set_r_used(REG_M);
		desc.set_r_used(REG_N);
		desc.set_r_modified(REG_N);
		desc.set_reads_memory();
		return true;

	case  8: // SWAPB(Rm, Rn);
		desc.set_r_used(REG_M);
		desc.set_r_used(REG_N);
		desc.set_r_modified(REG_N);
		return true;

	case 10: // NEGC(Rm, Rn);
		desc.set_r_used(REG_M);
		desc.set_r_modified(REG_N);
		desc.set_sr_modified();
		return true;
	}
	return false;
}

bool sh_common_execution::frontend::describe_group_8(opcode_desc &desc, const opcode_desc *prev, uint16_t opcode)
{
	int32_t disp;

	switch (opcode & (15 << 8))
	{
	case  0 << 8: // MOVBS4(opcode & 0x0f, Rm);
	case  1 << 8: // MOVWS4(opcode & 0x0f, Rm);
		desc.set_r_used(REG_M);
		desc.set_r_used(0);
		desc.set_writes_memory();
		return true;

	case  2 << 8: // NOP();
	case  3 << 8: // NOP();
	case  6 << 8: // NOP();
	case  7 << 8: // NOP();
	case 10 << 8: // NOP();
	case 12 << 8: // NOP();
	case 14 << 8: // NOP();
		return true;

	case  4 << 8: // MOVBL4(Rm, opcode & 0x0f);
	case  5 << 8: // MOVWL4(Rm, opcode & 0x0f);
		desc.set_r_used(REG_M);
		desc.set_r_modified(0);
		desc.set_reads_memory();
		return true;

	case  8 << 8: // CMPIM(opcode & 0xff);
		desc.set_r_used(REG_M);
		desc.set_sr_used();
		desc.set_sr_modified();
		return true;

	case  9 << 8: // BT(opcode & 0xff);
	case 11 << 8: // BF(opcode & 0xff);
		desc.set_is_conditional_branch();
		desc.cycles = 3;
		disp = util::sext(opcode, 8);
		desc.targetpc = (desc.pc + 2) + disp * 2 + 2;
		return true;

	case 13 << 8: // BTS(opcode & 0xff);
	case 15 << 8: // BFS(opcode & 0xff);
		desc.set_is_conditional_branch();
		desc.cycles = 2;
		disp = util::sext(opcode, 8);
		desc.targetpc = (desc.pc + 2) + disp * 2 + 2;
		desc.delayslots = 1;
		return true;
	}

	return false;
}

bool sh_common_execution::frontend::describe_group_12(opcode_desc &desc, const opcode_desc *prev, uint16_t opcode)
{
	switch (opcode & (15<<8))
	{
	case  0 << 8: // MOVBSG(opcode & 0xff);
	case  1 << 8: // MOVWSG(opcode & 0xff);
	case  2 << 8: // MOVLSG(opcode & 0xff);
		desc.set_r_used(0);
		desc.set_writes_memory();
		return true;

	case  3 << 8: // TRAPA(opcode & 0xff);
		desc.set_r_used(15);
		desc.set_vbr_used();
		desc.set_r_modified(15);
		desc.cycles = 8;
		desc.targetpc = BRANCH_TARGET_DYNAMIC;
		desc.set_is_unconditional_branch();
		desc.set_end_sequence();
		desc.set_reads_memory();
		return true;

	case  4 << 8: // MOVBLG(opcode & 0xff);
	case  5 << 8: // MOVWLG(opcode & 0xff);
	case  6 << 8: // MOVLLG(opcode & 0xff);
	case  7 << 8: // MOVA(opcode & 0xff);
		desc.set_r_modified(0);
		desc.set_reads_memory();
		return true;

	case  8 << 8: // TSTI(opcode & 0xff);
		desc.set_r_used(0);
		desc.set_sr_used();
		desc.set_sr_modified();
		return true;

	case  9 << 8: // ANDI(opcode & 0xff);
	case 10 << 8: // XORI(opcode & 0xff);
	case 11 << 8: // ORI(opcode & 0xff);
		desc.set_r_used(0);
		desc.set_r_modified(0);
		return true;

	case 12 << 8: // TSTM(opcode & 0xff);
	case 13 << 8: // ANDM(opcode & 0xff);
	case 14 << 8: // XORM(opcode & 0xff);
	case 15 << 8: // ORM(opcode & 0xff);
		desc.set_r_used(0);
		desc.set_sr_used();
		desc.set_gbr_used();
		desc.set_sr_modified();
		desc.set_reads_memory();
		return true;
	}

	return false;
}



bool sh_common_execution::frontend::describe_group_0(opcode_desc &desc, const opcode_desc *prev, uint16_t opcode)
{
	switch (opcode & 0x3f)
	{
	case 0x00: // NOP();
	case 0x01: // NOP();
	case 0x09: // NOP();
	case 0x10: // NOP();
	case 0x11: // NOP();
	case 0x13: // NOP();
	case 0x20: // NOP();
	case 0x21: // NOP();
	case 0x30: // NOP();
	case 0x31: // NOP();
	case 0x32: // NOP();
	case 0x33: // NOP();
	case 0x38: // NOP();
	case 0x39: // NOP();
	case 0x3a: // NOP();
	case 0x3b: // NOP();
		return true;

	case 0x02: // STCSR(Rn);
		desc.set_r_modified(REG_N);
		return true;

	case 0x03: // BSRF(Rn);
		desc.set_pr_modified();

		desc.set_is_unconditional_branch();
		desc.set_end_sequence();
		desc.targetpc = BRANCH_TARGET_DYNAMIC;
		desc.delayslots = 1;

		return true;

	case 0x04: // MOVBS0(Rm, Rn);
	case 0x05: // MOVWS0(Rm, Rn);
	case 0x06: // MOVLS0(Rm, Rn);
	case 0x14: // MOVBS0(Rm, Rn);
	case 0x15: // MOVWS0(Rm, Rn);
	case 0x16: // MOVLS0(Rm, Rn);
	case 0x24: // MOVBS0(Rm, Rn);
	case 0x25: // MOVWS0(Rm, Rn);
	case 0x26: // MOVLS0(Rm, Rn);
	case 0x34: // MOVBS0(Rm, Rn);
	case 0x35: // MOVWS0(Rm, Rn);
	case 0x36: // MOVLS0(Rm, Rn);
		desc.set_r_used(REG_M);
		desc.set_r_used(REG_N);
		desc.set_r_used(0);
		desc.set_writes_memory();
		return true;

	case 0x07: // MULL(Rm, Rn);
	case 0x17: // MULL(Rm, Rn);
	case 0x27: // MULL(Rm, Rn);
	case 0x37: // MULL(Rm, Rn);
		desc.set_r_used(REG_M);
		desc.set_r_used(REG_N);
		desc.set_macl_modified();
		desc.cycles = 2;
		return true;

	case 0x08: // CLRT();
		desc.set_sr_modified();
		return true;

	case 0x0a: // STSMACH(Rn);
		desc.set_r_modified(REG_N);
		desc.set_mach_modified();
		return true;

	case 0x0b: // RTS();
		desc.set_pr_used();

		desc.set_is_unconditional_branch();
		desc.set_end_sequence();
		desc.targetpc = BRANCH_TARGET_DYNAMIC;
		desc.delayslots = 1;
		desc.cycles = 2;

		return true;

	case 0x0c: // MOVBL0(Rm, Rn);
	case 0x0d: // MOVWL0(Rm, Rn);
	case 0x0e: // MOVLL0(Rm, Rn);
	case 0x1c: // MOVBL0(Rm, Rn);
	case 0x1d: // MOVWL0(Rm, Rn);
	case 0x1e: // MOVLL0(Rm, Rn);
	case 0x2c: // MOVBL0(Rm, Rn);
	case 0x2d: // MOVWL0(Rm, Rn);
	case 0x2e: // MOVLL0(Rm, Rn);
	case 0x3c: // MOVBL0(Rm, Rn);
	case 0x3d: // MOVWL0(Rm, Rn);
	case 0x3e: // MOVLL0(Rm, Rn);
		desc.set_r_used(REG_M);
		desc.set_r_used(0);
		desc.set_r_modified(REG_N);
		desc.set_reads_memory();
		return true;

	case 0x0f: // MAC_L(Rm, Rn);
	case 0x1f: // MAC_L(Rm, Rn);
	case 0x2f: // MAC_L(Rm, Rn);
	case 0x3f: // MAC_L(Rm, Rn);
		desc.set_r_used(REG_M);
		desc.set_r_used(REG_N);
		desc.set_mac_modified();
		desc.cycles = 3;
		return true;

	case 0x12: // STCGBR(Rn);
		desc.set_r_modified(REG_N);
		desc.set_gbr_used();
		return true;

	case 0x18: // SETT();
		desc.set_sr_modified();
		return true;

	case 0x19: // DIV0U();
		desc.set_sr_modified();
		return true;

	case 0x1a: // STSMACL(Rn);
		desc.set_macl_used();
		desc.set_r_modified(REG_N);
		return true;

	case 0x1b: // SLEEP();
		desc.cycles = 3;
		return true;

	case 0x22: // STCVBR(Rn);
		desc.set_r_used(REG_N);
		desc.set_vbr_modified();
		return true;

	case 0x23: // BRAF(Rn);
		desc.set_r_used(REG_M);
		desc.set_is_unconditional_branch();
		desc.set_end_sequence();
		desc.targetpc = BRANCH_TARGET_DYNAMIC;
		desc.delayslots = 1;
		desc.cycles = 2;
		return true;

	case 0x28: // CLRMAC();
		desc.set_mac_modified();
		return true;

	case 0x29: // MOVT(Rn);
		desc.set_sr_used();
		desc.set_r_modified(REG_N);
		return true;

	case 0x2a: // STSPR(Rn);
		desc.set_pr_used();
		desc.set_r_modified(REG_N);
		return true;

	case 0x2b: // RTE();
		desc.set_r_used(15);
		desc.set_r_modified(15);

		desc.set_is_unconditional_branch();
		desc.set_end_sequence();
		desc.set_can_expose_external_int();
		desc.targetpc = BRANCH_TARGET_DYNAMIC;
		desc.delayslots = 1;
		desc.cycles = 4;

		return true;
	}

	return false;
}


bool sh_common_execution::frontend::describe_group_4(opcode_desc &desc, const opcode_desc *prev, uint16_t opcode)
{
	switch (opcode & 0x3F)
	{
	case 0x00: // SHLL(Rn);
	case 0x01: // SHLR(Rn);
	case 0x04: // ROTL(Rn);
	case 0x05: // ROTR(Rn);
		desc.set_r_used(REG_N);
		desc.set_r_modified(REG_N);
		desc.set_sr_modified();
		return true;

	case 0x02: // STSMMACH(Rn);
		desc.set_r_used(REG_N);
		desc.set_mach_used();
		desc.set_r_modified(REG_N);
		desc.set_writes_memory();
		return true;

	case 0x03: // STCMSR(Rn);
		desc.set_r_used(REG_N);
		desc.set_r_modified(REG_N);
		desc.cycles = 2;
		desc.set_writes_memory();
		return true;

	case 0x06: // LDSMMACH(Rn);
		desc.set_r_used(REG_N);
		desc.set_r_modified(REG_N);
		desc.set_mach_modified();
		desc.set_reads_memory();
		return true;

	case 0x07: // LDCMSR(Rn);
		desc.set_r_used(REG_N);
		desc.set_r_modified(REG_N);
		desc.set_sr_modified();
		desc.cycles = 3;
		desc.set_end_sequence();
		desc.set_reads_memory();
		desc.set_can_expose_external_int();
		return true;

	case 0x08: // SHLL2(Rn);
	case 0x09: // SHLR2(Rn);
	case 0x18: // SHLL8(Rn);
	case 0x19: // SHLR8(Rn);
	case 0x28: // SHLL16(Rn);
	case 0x29: // SHLR16(Rn);
		desc.set_r_used(REG_N);
		desc.set_r_modified(REG_N);
		return true;

	case 0x0a: // LDSMACH(Rn);
		desc.set_r_used(REG_N);
		desc.set_mach_modified();
		return true;

	case 0x0b: // JSR(Rn);
		desc.set_r_used(REG_N);
		desc.set_pr_modified();
		desc.set_is_unconditional_branch();
		desc.set_end_sequence();
		desc.targetpc = BRANCH_TARGET_DYNAMIC;
		desc.delayslots = 1;
		return true;

	case 0x0e: // LDCSR(Rn);
		desc.set_r_used(REG_N);
		desc.set_sr_modified();
		desc.set_end_sequence();
		desc.set_can_expose_external_int();
		return true;

	case 0x0f: // MAC_W(Rm, Rn);
	case 0x1f: // MAC_W(Rm, Rn);
	case 0x2f: // MAC_W(Rm, Rn);
	case 0x3f: // MAC_W(Rm, Rn);
		desc.set_r_used(REG_M);
		desc.set_r_used(REG_N);
		desc.set_mac_used();
		desc.set_r_modified(REG_M);
		desc.set_r_modified(REG_N);
		desc.set_mac_modified();
		desc.cycles = 3;
		return true;

	case 0x10: // DT(Rn);
		desc.set_r_used(REG_N);
		desc.set_sr_used();
		desc.set_r_modified(REG_N);
		desc.set_sr_modified();
		return true;

	case 0x11: // CMPPZ(Rn);
	case 0x15: // CMPPL(Rn);
		desc.set_r_used(REG_N);
		desc.set_sr_used();
		desc.set_sr_modified();
		return true;

	case 0x12: // STSMMACL(Rn);
		desc.set_r_used(REG_N);
		desc.set_macl_used();
		desc.set_r_modified(REG_N);
		desc.set_writes_memory();
		return true;

	case 0x13: // STCMGBR(Rn);
		desc.set_r_used(REG_N);
		desc.set_gbr_used();
		desc.set_r_modified(REG_N);
		desc.set_writes_memory();
		return true;

	case 0x16: // LDSMMACL(Rn);
		desc.set_r_used(REG_M);
		desc.set_r_used(REG_N);
		desc.set_r_modified(REG_M);
		desc.set_r_modified(REG_N);
		desc.set_macl_modified();
		desc.set_reads_memory();
		return true;

	case 0x17: // LDCMGBR(Rn);
		desc.set_r_used(REG_N);
		desc.set_r_modified(REG_N);
		desc.set_gbr_modified();
		desc.set_reads_memory();
		return true;

	case 0x1a: // LDSMACL(Rn);
		desc.set_r_used(REG_N);
		desc.set_macl_modified();
		return true;

	case 0x1b: // TAS(Rn);
		desc.set_r_used(REG_N);
		desc.set_sr_used();
		desc.set_sr_modified();
		desc.cycles = 4;
		desc.set_reads_memory();
		desc.set_writes_memory();
		return true;

	case 0x1e: // LDCGBR(Rn);
		desc.set_r_used(REG_N);
		desc.set_gbr_modified();
		return true;

	case 0x20: // SHAL(Rn);
	case 0x21: // SHAR(Rn);
		desc.set_r_used(REG_N);
		desc.set_r_modified(REG_N);
		desc.set_sr_modified();
		return true;

	case 0x22: // STSMPR(Rn);
		desc.set_r_used(REG_N);
		desc.set_pr_used();
		desc.set_r_modified(REG_N);
		desc.set_writes_memory();
		return true;

	case 0x23: // STCMVBR(Rn);
		desc.set_r_used(REG_N);
		desc.set_vbr_used();
		desc.set_r_modified(REG_N);
		desc.set_writes_memory();
		return true;

	case 0x24: // ROTCL(Rn);
	case 0x25: // ROTCR(Rn);
		desc.set_r_used(REG_N);
		desc.set_sr_used();
		desc.set_r_modified(REG_N);
		desc.set_sr_modified();
		return true;

	case 0x26: // LDSMPR(Rn);
		desc.set_r_used(REG_N);
		desc.set_r_modified(REG_N);
		desc.set_pr_modified();
		desc.set_reads_memory();
		return true;

	case 0x27: // LDCMVBR(Rn);
		desc.set_r_used(REG_N);
		desc.set_r_modified(REG_N);
		desc.set_vbr_modified();
		desc.set_reads_memory();
		return true;

	case 0x2a: // LDSPR(Rn);
		desc.set_r_used(REG_N);
		desc.set_pr_modified();
		return true;

	case 0x2b: // JMP(Rm);
		desc.set_r_used(REG_M);
		desc.set_is_unconditional_branch();
		desc.set_end_sequence();
		desc.targetpc = BRANCH_TARGET_DYNAMIC;
		desc.delayslots = 1;
		return true;

	case 0x2e: // LDCVBR(Rn);
		desc.set_r_used(REG_N);
		desc.set_vbr_modified();
		return true;

	case 0x0c: // NOP();
	case 0x0d: // NOP();
	case 0x14: // NOP();
	case 0x1c: // NOP();
	case 0x1d: // NOP();
	case 0x2c: // NOP();
	case 0x2d: // NOP();
	case 0x30: // NOP();
	case 0x31: // NOP();
	case 0x32: // NOP();
	case 0x33: // NOP();
	case 0x34: // NOP();
	case 0x35: // NOP();
	case 0x36: // NOP();
	case 0x37: // NOP();
	case 0x38: // NOP();
	case 0x39: // NOP();
	case 0x3a: // NOP();
	case 0x3b: // NOP();
	case 0x3c: // NOP();
	case 0x3d: // NOP();
	case 0x3e: // NOP();
		return true;
	}

	return false;
}
