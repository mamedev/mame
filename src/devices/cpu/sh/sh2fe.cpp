// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    sh2fe.c

    Front end for SH-2 recompiler

***************************************************************************/

#include "emu.h"
#include "sh2.h"
#include "sh2comn.h"
#include "cpu/drcfe.h"


/***************************************************************************
    INSTRUCTION PARSERS
***************************************************************************/

sh2_frontend::sh2_frontend(sh_common_execution *device, uint32_t window_start, uint32_t window_end, uint32_t max_sequence)
	: sh_frontend(device, window_start, window_end, max_sequence)
{
}


bool sh2_frontend::describe_group_0(opcode_desc &desc, const opcode_desc *prev, uint16_t opcode)
{
	switch (opcode & 0x3F)
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
		desc.regout[0] |= REGFLAG_R(Rn);
		return true;

	case 0x03: // BSRF(Rn);
		desc.regout[1] |= REGFLAG_PR;

		desc.flags |= OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_END_SEQUENCE;
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
		desc.regin[0] |= REGFLAG_R(Rm) | REGFLAG_R(Rn) | REGFLAG_R(0);
		desc.flags |= OPFLAG_READS_MEMORY;
		return true;

	case 0x07: // MULL(Rm, Rn);
	case 0x17: // MULL(Rm, Rn);
	case 0x27: // MULL(Rm, Rn);
	case 0x37: // MULL(Rm, Rn);
		desc.regin[0] |= REGFLAG_R(Rn) | REGFLAG_R(Rm);
		desc.regout[1] |= REGFLAG_MACL;
		desc.cycles = 2;
		return true;

	case 0x08: // CLRT();
		desc.regout[1] |= REGFLAG_SR;
		return true;

	case 0x0a: // STSMACH(Rn);
		desc.regout[0] |= REGFLAG_R(Rn);
		desc.regout[1] |= REGFLAG_MACH;
		return true;

	case 0x0b: // RTS();
		desc.regin[1] |= REGFLAG_PR;

		desc.flags |= OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_END_SEQUENCE;
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
		desc.regin[0] |= REGFLAG_R(Rm) | REGFLAG_R(0);
		desc.regout[0] |= REGFLAG_R(Rn);
		desc.flags |= OPFLAG_READS_MEMORY;
		return true;

	case 0x0f: // MAC_L(Rm, Rn);
	case 0x1f: // MAC_L(Rm, Rn);
	case 0x2f: // MAC_L(Rm, Rn);
	case 0x3f: // MAC_L(Rm, Rn);
		desc.regin[0] |= REGFLAG_R(Rm) | REGFLAG_R(Rn);
		desc.regout[1] |= REGFLAG_MACL | REGFLAG_MACH;
		desc.cycles = 3;
		return true;

	case 0x12: // STCGBR(Rn);
		desc.regout[0] |= REGFLAG_R(Rn);
		desc.regin[1] |= REGFLAG_GBR;
		return true;

	case 0x18: // SETT();
		desc.regout[1] |= REGFLAG_SR;
		return true;

	case 0x19: // DIV0U();
		desc.regout[1] |= REGFLAG_SR;
		return true;

	case 0x1a: // STSMACL(Rn);
		desc.regin[1] |= REGFLAG_MACL;
		desc.regout[0] |= REGFLAG_R(Rn);
		return true;

	case 0x1b: // SLEEP();
		desc.cycles = 3;
		return true;

	case 0x22: // STCVBR(Rn);
		desc.regin[0] |= REGFLAG_R(Rn);
		desc.regout[1] |= REGFLAG_VBR;
		return true;

	case 0x23: // BRAF(Rn);
		desc.regin[0] |= REGFLAG_R(Rm);
		desc.flags |= OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_END_SEQUENCE;
		desc.targetpc = BRANCH_TARGET_DYNAMIC;
		desc.delayslots = 1;
		desc.cycles = 2;
		return true;

	case 0x28: // CLRMAC();
		desc.regout[1] |= REGFLAG_MACL | REGFLAG_MACH;
		return true;

	case 0x29: // MOVT(Rn);
		desc.regin[1] |= REGFLAG_SR;
		desc.regout[0] |= REGFLAG_R(Rn);
		return true;

	case 0x2a: // STSPR(Rn);
		desc.regin[1] |= REGFLAG_PR;
		desc.regout[0] |= REGFLAG_R(Rn);
		return true;

	case 0x2b: // RTE();
		desc.regin[0] |= REGFLAG_R(15);
		desc.regout[0] |= REGFLAG_R(15);

		desc.flags |= OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_END_SEQUENCE | OPFLAG_CAN_EXPOSE_EXTERNAL_INT;
		desc.targetpc = BRANCH_TARGET_DYNAMIC;
		desc.delayslots = 1;
		desc.cycles = 4;

		return true;
	}

	return false;
}


bool sh2_frontend::describe_group_4(opcode_desc &desc, const opcode_desc *prev, uint16_t opcode)
{
	switch (opcode & 0x3F)
	{
	case 0x00: // SHLL(Rn);
	case 0x01: // SHLR(Rn);
	case 0x04: // ROTL(Rn);
	case 0x05: // ROTR(Rn);
		desc.regin[0] |= REGFLAG_R(Rn);
		desc.regout[0] |= REGFLAG_R(Rn);
		desc.regout[1] |= REGFLAG_SR;
		return true;

	case 0x02: // STSMMACH(Rn);
		desc.regin[0] |= REGFLAG_R(Rn);
		desc.regin[1] |= REGFLAG_MACH;
		desc.regout[0] |= REGFLAG_R(Rn);
		desc.flags |= OPFLAG_WRITES_MEMORY;
		return true;

	case 0x03: // STCMSR(Rn);
		desc.regin[0] |= REGFLAG_R(Rn);
		desc.regout[0] |= REGFLAG_R(Rn);
		desc.cycles = 2;
		desc.flags |= OPFLAG_WRITES_MEMORY;
		return true;

	case 0x06: // LDSMMACH(Rn);
		desc.regin[0] |= REGFLAG_R(Rn);
		desc.regout[0] |= REGFLAG_R(Rn);
		desc.regout[1] |= REGFLAG_MACH;
		desc.flags |= OPFLAG_READS_MEMORY;
		return true;

	case 0x07: // LDCMSR(Rn);
		desc.regin[0] |= REGFLAG_R(Rn);
		desc.regout[0] |= REGFLAG_R(Rn);
		desc.regout[1] |= REGFLAG_SR;
		desc.cycles = 3;
		desc.flags |= OPFLAG_READS_MEMORY | OPFLAG_CAN_EXPOSE_EXTERNAL_INT | OPFLAG_END_SEQUENCE;
		return true;

	case 0x08: // SHLL2(Rn);
	case 0x09: // SHLR2(Rn);
	case 0x18: // SHLL8(Rn);
	case 0x19: // SHLR8(Rn);
	case 0x28: // SHLL16(Rn);
	case 0x29: // SHLR16(Rn);
		desc.regin[0] |= REGFLAG_R(Rn);
		desc.regout[0] |= REGFLAG_R(Rn);
		return true;

	case 0x0a: // LDSMACH(Rn);
		desc.regin[0] |= REGFLAG_R(Rn);
		desc.regout[1] |= REGFLAG_MACH;
		return true;

	case 0x0b: // JSR(Rn);
		desc.regin[0] |= REGFLAG_R(Rn);
		desc.regout[1] |= REGFLAG_PR;
		desc.flags |= OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_END_SEQUENCE;
		desc.targetpc = BRANCH_TARGET_DYNAMIC;
		desc.delayslots = 1;
		return true;

	case 0x0e: // LDCSR(Rn);
		desc.regin[0] |= REGFLAG_R(Rn);
		desc.regout[1] |= REGFLAG_SR;
		desc.flags |= OPFLAG_CAN_EXPOSE_EXTERNAL_INT | OPFLAG_END_SEQUENCE;
		return true;

	case 0x0f: // MAC_W(Rm, Rn);
	case 0x1f: // MAC_W(Rm, Rn);
	case 0x2f: // MAC_W(Rm, Rn);
	case 0x3f: // MAC_W(Rm, Rn);
		desc.regin[0] |= REGFLAG_R(Rm) | REGFLAG_R(Rn);
		desc.regin[1] |= REGFLAG_MACL | REGFLAG_MACH;
		desc.regout[0] |= REGFLAG_R(Rm) | REGFLAG_R(Rn);
		desc.regout[1] |= REGFLAG_MACL | REGFLAG_MACH;
		desc.cycles = 3;
		return true;

	case 0x10: // DT(Rn);
		desc.regin[0] |= REGFLAG_R(Rn);
		desc.regin[1] |= REGFLAG_SR;
		desc.regout[0] |= REGFLAG_R(Rn);
		desc.regout[1] |= REGFLAG_SR;
		return true;

	case 0x11: // CMPPZ(Rn);
	case 0x15: // CMPPL(Rn);
		desc.regin[0] |= REGFLAG_R(Rn);
		desc.regin[1] |= REGFLAG_SR;
		desc.regout[1] |= REGFLAG_SR;
		return true;

	case 0x12: // STSMMACL(Rn);
		desc.regin[0] |= REGFLAG_R(Rn);
		desc.regin[1] |= REGFLAG_MACL;
		desc.regout[0] |= REGFLAG_R(Rn);
		desc.flags |= OPFLAG_WRITES_MEMORY;
		return true;

	case 0x13: // STCMGBR(Rn);
		desc.regin[0] |= REGFLAG_R(Rn);
		desc.regin[1] |= REGFLAG_GBR;
		desc.regout[0] |= REGFLAG_R(Rn);
		desc.flags |= OPFLAG_WRITES_MEMORY;
		return true;

	case 0x16: // LDSMMACL(Rn);
		desc.regin[0] |= REGFLAG_R(Rm) | REGFLAG_R(Rn);
		desc.regout[0] |= REGFLAG_R(Rm) | REGFLAG_R(Rn);
		desc.regout[1] |= REGFLAG_MACL;
		desc.flags |= OPFLAG_READS_MEMORY;
		return true;

	case 0x17: // LDCMGBR(Rn);
		desc.regin[0] |= REGFLAG_R(Rn);
		desc.regout[0] |= REGFLAG_R(Rn);
		desc.regout[1] |= REGFLAG_GBR;
		desc.flags |= OPFLAG_READS_MEMORY;
		return true;

	case 0x1a: // LDSMACL(Rn);
		desc.regin[0] |= REGFLAG_R(Rn);
		desc.regout[1] |= REGFLAG_MACL;
		return true;

	case 0x1b: // TAS(Rn);
		desc.regin[0] |= REGFLAG_R(Rn);
		desc.regin[1] |= REGFLAG_SR;
		desc.regout[1] |= REGFLAG_SR;
		desc.cycles = 4;
		desc.flags |= OPFLAG_READS_MEMORY | OPFLAG_WRITES_MEMORY;
		return true;

	case 0x1e: // LDCGBR(Rn);
		desc.regin[0] |= REGFLAG_R(Rn);
		desc.regout[1] |= REGFLAG_GBR;
		return true;

	case 0x20: // SHAL(Rn);
	case 0x21: // SHAR(Rn);
		desc.regin[0] |= REGFLAG_R(Rn);
		desc.regout[0] |= REGFLAG_R(Rn);
		desc.regout[1] |= REGFLAG_SR;
		return true;

	case 0x22: // STSMPR(Rn);
		desc.regin[0] |= REGFLAG_R(Rn);
		desc.regin[1] |= REGFLAG_PR;
		desc.regout[0] |= REGFLAG_R(Rn);
		desc.flags |= OPFLAG_WRITES_MEMORY;
		return true;

	case 0x23: // STCMVBR(Rn);
		desc.regin[0] |= REGFLAG_R(Rn);
		desc.regin[1] |= REGFLAG_VBR;
		desc.regout[0] |= REGFLAG_R(Rn);
		desc.flags |= OPFLAG_WRITES_MEMORY;
		return true;

	case 0x24: // ROTCL(Rn);
	case 0x25: // ROTCR(Rn);
		desc.regin[0] |= REGFLAG_R(Rn);
		desc.regin[1] |= REGFLAG_SR;
		desc.regout[0] |= REGFLAG_R(Rn);
		desc.regout[1] |= REGFLAG_SR;
		return true;

	case 0x26: // LDSMPR(Rn);
		desc.regin[0] |= REGFLAG_R(Rn);
		desc.regout[0] |= REGFLAG_R(Rn);
		desc.regout[1] |= REGFLAG_PR;
		desc.flags |= OPFLAG_READS_MEMORY;
		return true;

	case 0x27: // LDCMVBR(Rn);
		desc.regin[0] |= REGFLAG_R(Rn);
		desc.regout[0] |= REGFLAG_R(Rn);
		desc.regout[1] |= REGFLAG_VBR;
		desc.flags |= OPFLAG_READS_MEMORY;
		return true;

	case 0x2a: // LDSPR(Rn);
		desc.regin[0] |= REGFLAG_R(Rn);
		desc.regout[1] |= REGFLAG_PR;
		return true;

	case 0x2b: // JMP(Rm);
		desc.regin[0] |= REGFLAG_R(Rm);
		desc.flags |= OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_END_SEQUENCE;
		desc.targetpc = BRANCH_TARGET_DYNAMIC;
		desc.delayslots = 1;
		return true;

	case 0x2e: // LDCVBR(Rn);
		desc.regin[0] |= REGFLAG_R(Rn);
		desc.regout[1] |= REGFLAG_VBR;
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

bool sh2_frontend::describe_group_15(opcode_desc &desc, const opcode_desc *prev, uint16_t opcode)
{
	return true;
}