// license:BSD-3-Clause
// copyright-holders:David Haywood, R. Belmont
/***************************************************************************

    sh_fe.c

    Front end for SH recompiler

***************************************************************************/

#include "emu.h"
#include "sh2.h"
#include "sh2comn.h"
#include "cpu/drcfe.h"


/***************************************************************************
    INSTRUCTION PARSERS
***************************************************************************/

sh_frontend::sh_frontend(sh_common_execution *device, uint32_t window_start, uint32_t window_end, uint32_t max_sequence)
	: drc_frontend(*device, window_start, window_end, max_sequence)
	, m_sh(device)
{
}

/*-------------------------------------------------
    describe_instruction - build a description
    of a single instruction
-------------------------------------------------*/

bool sh_frontend::describe(opcode_desc &desc, const opcode_desc *prev)
{
	uint16_t opcode;

	/* fetch the opcode */
	opcode = desc.opptr.w[0] = m_sh->m_direct->read_word(desc.physpc, SH2_CODE_XOR(0));

	/* all instructions are 2 bytes and most are a single cycle */
	desc.length = 2;
	desc.cycles = 1;

	switch (opcode>>12)
	{
		case  0:
			return describe_group_0(desc, prev, opcode);

		case  1:    // MOVLS4
			desc.regin[0] |= REGFLAG_R(Rn) | REGFLAG_R(Rm);
			desc.flags |= OPFLAG_WRITES_MEMORY;
			return true;

		case  2:
			return describe_group_2(desc, prev, opcode);

		case  3:
			return describe_group_3(desc, prev, opcode);

		case  4:
			return describe_group_4(desc, prev, opcode);

		case  5:    // MOVLL4
			desc.regin[0] |= REGFLAG_R(Rm);
			desc.regout[0] |= REGFLAG_R(Rn);
			desc.flags |= OPFLAG_READS_MEMORY;
			return true;

		case  6:
			return describe_group_6(desc, prev, opcode);

		case  7:    // ADDI
			desc.regin[0] |= REGFLAG_R(Rn);
			desc.regout[0] |= REGFLAG_R(Rn);
			return true;

		case  8:
			return describe_group_8(desc, prev, opcode);

		case  9:    // MOVWI
			desc.regout[0] |= REGFLAG_R(Rn);
			desc.flags |= OPFLAG_READS_MEMORY;
			return true;

		case 11:    // BSR
			desc.regout[1] |= REGFLAG_PR;
			// (intentional fallthrough - BSR is BRA with the addition of PR = the return address)
		case 10:    // BRA
			{
				int32_t disp = ((int32_t)opcode << 20) >> 20;

				desc.flags |= OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_END_SEQUENCE;
				desc.targetpc = (desc.pc + 2) + disp * 2 + 2;
				desc.delayslots = 1;
				desc.cycles = 2;
				return true;
			}

		case 12:
			return describe_group_12(desc, prev, opcode);

		case 13:    // MOVLI
			desc.regout[0] |= REGFLAG_R(Rn);
			desc.flags |= OPFLAG_READS_MEMORY;
			return true;

		case 14:    // MOVI
			desc.regout[0] |= REGFLAG_R(Rn);
			return true;

		case 15:    // NOP
			return describe_group_15(desc, prev, opcode);
	}

	return false;
}


bool sh_frontend::describe_group_2(opcode_desc &desc, const opcode_desc *prev, uint16_t opcode)
{
	switch (opcode & 15)
	{
	case  0: // MOVBS(Rm, Rn);
	case  1: // MOVWS(Rm, Rn);
	case  2: // MOVLS(Rm, Rn);
		desc.regin[0] |= REGFLAG_R(Rm) | REGFLAG_R(Rn);
		desc.flags |= OPFLAG_WRITES_MEMORY;
		return true;

	case  3: // NOP();
		return true;

	case  4: // MOVBM(Rm, Rn);
	case  5: // MOVWM(Rm, Rn);
	case  6: // MOVLM(Rm, Rn);
	case 13: // XTRCT(Rm, Rn);
		desc.regin[0] |= REGFLAG_R(Rm) | REGFLAG_R(Rn);
		desc.regout[0] |= REGFLAG_R(Rn);
		desc.flags |= OPFLAG_WRITES_MEMORY;
		return true;

	case  7: // DIV0S(Rm, Rn);
	case  8: // TST(Rm, Rn);
	case 12: // CMPSTR(Rm, Rn);
		desc.regin[0] |= REGFLAG_R(Rm) | REGFLAG_R(Rn);
		desc.regout[1] |= REGFLAG_SR;
		return true;

	case  9: // AND(Rm, Rn);
	case 10: // XOR(Rm, Rn);
	case 11: // OR(Rm, Rn);
		desc.regin[0] |= REGFLAG_R(Rm) | REGFLAG_R(Rn);
		desc.regout[0] |= REGFLAG_R(Rn);
		return true;

	case 14: // MULU(Rm, Rn);
	case 15: // MULS(Rm, Rn);
		desc.regin[0] |= REGFLAG_R(Rm) | REGFLAG_R(Rn);
		desc.regout[1] |= REGFLAG_MACL | REGFLAG_MACH;
		desc.cycles = 2;
		return true;
	}

	return false;
}

bool sh_frontend::describe_group_3(opcode_desc &desc, const opcode_desc *prev, uint16_t opcode)
{
	switch (opcode & 15)
	{
	case  0: // CMPEQ(Rm, Rn);
	case  2: // CMPHS(Rm, Rn);
	case  3: // CMPGE(Rm, Rn);
	case  6: // CMPHI(Rm, Rn);
	case  7: // CMPGT(Rm, Rn);
		desc.regin[0] |= REGFLAG_R(Rm) | REGFLAG_R(Rn);
		desc.regout[1] |= REGFLAG_SR;
		return true;

	case  1: // NOP();
	case  9: // NOP();
		return true;

	case  4: // DIV1(Rm, Rn);
		desc.regin[0] |= REGFLAG_R(Rm) | REGFLAG_R(Rn);
		desc.regout[0] |= REGFLAG_R(Rn);
		desc.regout[1] |= REGFLAG_SR;
		return true;

	case  5: // DMULU(Rm, Rn);
	case 13: // DMULS(Rm, Rn);
		desc.regin[0] |= REGFLAG_R(Rm) | REGFLAG_R(Rn);
		desc.regout[1] |= REGFLAG_MACL | REGFLAG_MACH;
		desc.cycles = 2;
		return true;

	case  8: // SUB(Rm, Rn);
	case 12: // ADD(Rm, Rn);
		desc.regin[0] |= REGFLAG_R(Rm) | REGFLAG_R(Rn);
		desc.regout[0] |= REGFLAG_R(Rn);
		return true;

	case 10: // SUBC(Rm, Rn);
	case 11: // SUBV(Rm, Rn);
	case 14: // ADDC(Rm, Rn);
	case 15: // ADDV(Rm, Rn);
		desc.regin[0] |= REGFLAG_R(Rm) | REGFLAG_R(Rn);
		desc.regin[1] |= REGFLAG_SR;
		desc.regout[0] |= REGFLAG_R(Rn);
		desc.regout[1] |= REGFLAG_SR;
		return true;
	}
	return false;
}


bool sh_frontend::describe_group_6(opcode_desc &desc, const opcode_desc *prev, uint16_t opcode)
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
		desc.regin[0] |= REGFLAG_R(Rm);
		desc.regout[0] |= REGFLAG_R(Rn);
		return true;

	case  4: // MOVBP(Rm, Rn);
	case  5: // MOVWP(Rm, Rn);
	case  6: // MOVLP(Rm, Rn);
		desc.regin[0] |= REGFLAG_R(Rm) | REGFLAG_R(Rn);
		desc.regout[0] |= REGFLAG_R(Rn);
		desc.flags |= OPFLAG_READS_MEMORY;
		return true;

	case  8: // SWAPB(Rm, Rn);
		desc.regin[0] |= REGFLAG_R(Rm) | REGFLAG_R(Rn);
		desc.regout[0] |= REGFLAG_R(Rn);
		return true;

	case 10: // NEGC(Rm, Rn);
		desc.regin[0] |= REGFLAG_R(Rm);
		desc.regout[0] |= REGFLAG_R(Rn);
		desc.regout[1] |= REGFLAG_SR;
		return true;
	}
	return false;
}

bool sh_frontend::describe_group_8(opcode_desc &desc, const opcode_desc *prev, uint16_t opcode)
{
	int32_t disp;

	switch ( opcode  & (15<<8) )
	{
	case  0 << 8: // MOVBS4(opcode & 0x0f, Rm);
	case  1 << 8: // MOVWS4(opcode & 0x0f, Rm);
		desc.regin[0] |= REGFLAG_R(Rm) | REGFLAG_R(0);
		desc.flags |= OPFLAG_WRITES_MEMORY;
		return true;

	case  2<< 8: // NOP();
	case  3<< 8: // NOP();
	case  6<< 8: // NOP();
	case  7<< 8: // NOP();
	case 10<< 8: // NOP();
	case 12<< 8: // NOP();
	case 14<< 8: // NOP();
		return true;

	case  4<< 8: // MOVBL4(Rm, opcode & 0x0f);
	case  5<< 8: // MOVWL4(Rm, opcode & 0x0f);
		desc.regin[0] |= REGFLAG_R(Rm);
		desc.regout[0] |= REGFLAG_R(0);
		desc.flags |= OPFLAG_READS_MEMORY;
		return true;

	case  8<< 8: // CMPIM(opcode & 0xff);
		desc.regin[0] |= REGFLAG_R(Rm);
		desc.regin[1] |= REGFLAG_SR;
		desc.regout[1] |= REGFLAG_SR;
		return true;

	case  9<< 8: // BT(opcode & 0xff);
	case 11<< 8: // BF(opcode & 0xff);
		desc.flags |= OPFLAG_IS_CONDITIONAL_BRANCH;
		desc.cycles = 3;
		disp = ((int32_t)opcode << 24) >> 24;
		desc.targetpc = (desc.pc + 2) + disp * 2 + 2;
		return true;

	case 13<< 8: // BTS(opcode & 0xff);
	case 15<< 8: // BFS(opcode & 0xff);
		desc.flags |= OPFLAG_IS_CONDITIONAL_BRANCH;
		desc.cycles = 2;
		disp = ((int32_t)opcode << 24) >> 24;
		desc.targetpc = (desc.pc + 2) + disp * 2 + 2;
		desc.delayslots = 1;
		return true;
	}

	return false;
}

bool sh_frontend::describe_group_12(opcode_desc &desc, const opcode_desc *prev, uint16_t opcode)
{
	switch (opcode & (15<<8))
	{
	case  0<<8: // MOVBSG(opcode & 0xff);
	case  1<<8: // MOVWSG(opcode & 0xff);
	case  2<<8: // MOVLSG(opcode & 0xff);
		desc.regin[0] |= REGFLAG_R(0);
		desc.flags |= OPFLAG_WRITES_MEMORY;
		return true;

	case  3<<8: // TRAPA(opcode & 0xff);
		desc.regin[0] |= REGFLAG_R(15);
		desc.regin[1] |= REGFLAG_VBR;
		desc.regout[0] |= REGFLAG_R(15);
		desc.cycles = 8;
		desc.targetpc = BRANCH_TARGET_DYNAMIC;
		desc.flags |= OPFLAG_READS_MEMORY | OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_END_SEQUENCE;
		return true;

	case  4<<8: // MOVBLG(opcode & 0xff);
	case  5<<8: // MOVWLG(opcode & 0xff);
	case  6<<8: // MOVLLG(opcode & 0xff);
	case  7<<8: // MOVA(opcode & 0xff);
		desc.regout[0] |= REGFLAG_R(0);
		desc.flags |= OPFLAG_READS_MEMORY;
		return true;

	case  8<<8: // TSTI(opcode & 0xff);
		desc.regin[0] |= REGFLAG_R(0);
		desc.regin[1] |= REGFLAG_SR;
		desc.regout[1] |= REGFLAG_SR;
		return true;

	case  9<<8: // ANDI(opcode & 0xff);
	case 10<<8: // XORI(opcode & 0xff);
	case 11<<8: // ORI(opcode & 0xff);
		desc.regin[0] |= REGFLAG_R(0);
		desc.regout[0] |= REGFLAG_R(0);
		return true;

	case 12<<8: // TSTM(opcode & 0xff);
	case 13<<8: // ANDM(opcode & 0xff);
	case 14<<8: // XORM(opcode & 0xff);
	case 15<<8: // ORM(opcode & 0xff);
		desc.regin[0] |= REGFLAG_R(0);
		desc.regin[1] |= REGFLAG_SR | REGFLAG_GBR;
		desc.regout[1] |= REGFLAG_SR;
		desc.flags |= OPFLAG_READS_MEMORY;
		return true;
	}

	return false;
}
