// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    e132xsfe.cpp

    Front end for Hyperstone recompiler

***************************************************************************/

#include "emu.h"
#include "e132xsfe.h"
#include "32xsdefs.h"

#define FE_FP ((m_cpu->m_core->global_regs[1] & 0xfe000000) >> 25)
#define FE_FL (m_cpu->m_core->fl_lut[((m_cpu->m_core->global_regs[1] >> 21) & 0xf)])
#define FE_DST_CODE ((op & 0xf0) >> 4)
#define FE_SRC_CODE (op & 0x0f)
#define SR_CODE     (1 << 1)

/***************************************************************************
    INSTRUCTION PARSERS
***************************************************************************/

e132xs_frontend::e132xs_frontend(hyperstone_device *e132xs, uint32_t window_start, uint32_t window_end, uint32_t max_sequence)
	: drc_frontend(*e132xs, window_start, window_end, max_sequence)
	, m_cpu(e132xs)
{
}

inline uint16_t e132xs_frontend::read_word(opcode_desc &desc)
{
	return m_cpu->m_pr16(desc.physpc);
}

inline uint16_t e132xs_frontend::read_imm1(opcode_desc &desc)
{
	return m_cpu->m_pr16(desc.physpc + 2);
}

inline uint16_t e132xs_frontend::read_imm2(opcode_desc &desc)
{
	return m_cpu->m_pr16(desc.physpc + 4);
}

inline uint32_t e132xs_frontend::read_ldstxx_imm(opcode_desc &desc)
{
	const uint16_t imm1 = read_imm1(desc);
	uint32_t extra_s;
	if (imm1 & 0x8000)
	{
		extra_s = read_imm2(desc);
		extra_s |= ((imm1 & 0xfff) << 16);

		if (imm1 & 0x4000)
			extra_s |= 0xf0000000;
	}
	else
	{
		extra_s = imm1 & 0xfff;

		if (imm1 & 0x4000)
			extra_s |= 0xfffff000;
	}
	return extra_s;
}

inline uint32_t e132xs_frontend::read_limm(opcode_desc &desc, uint16_t op)
{
	static const int32_t immediate_values[16] =
	{
		16, 0, 0, 0, 32, 64, 128, int32_t(0x80000000),
		-8, -7, -6, -5, -4, -3, -2, -1
	};

	uint8_t nybble = op & 0xf;
	switch (nybble)
	{
		case 0:
			return 16;
		case 1:
			desc.length = 6;
			return (read_imm1(desc) << 16) | read_imm2(desc);
		case 2:
			desc.length = 4;
			return read_imm1(desc);
		case 3:
			desc.length = 4;
			return 0xffff0000 | read_imm1(desc);
		default:
			return immediate_values[nybble];
	}
}

inline int32_t e132xs_frontend::decode_pcrel(opcode_desc &desc, uint16_t op)
{
	if (op & 0x80)
	{
		uint16_t next = read_imm1(desc);

		desc.length = 4;

		int32_t offset = (op & 0x7f) << 16;
		offset |= (next & 0xfffe);

		if (next & 1)
			offset |= 0xff800000;

		return offset;
	}
	else
	{
		int32_t offset = op & 0x7e;
		if (op & 1)
			offset |= 0xffffff80;
		return offset;
	}
}

inline int32_t e132xs_frontend::decode_call(opcode_desc &desc)
{
	const uint16_t imm_1 = read_imm1(desc);
	int32_t extra_s = 0;
	if (imm_1 & 0x8000)
	{
		desc.length = 6;
		extra_s = read_imm2(desc);
		extra_s |= ((imm_1 & 0x3fff) << 16);

		if (imm_1 & 0x4000)
			extra_s |= 0xc0000000;
	}
	else
	{
		desc.length = 4;
		extra_s = imm_1 & 0x3fff;
		if (imm_1 & 0x4000)
			extra_s |= 0xffffc000;
	}
	return extra_s;
}


/*-------------------------------------------------
    describe - build a description of a single
    instruction
-------------------------------------------------*/

bool e132xs_frontend::describe(opcode_desc &desc, const opcode_desc *prev)
{
	uint16_t op = desc.opptr.w[0] = read_word(desc);

	/* most instructions are 2 bytes and a single cycle */
	desc.length = 2;
	desc.delayslots = 0;

	const uint32_t fp = FE_FP;
	const uint32_t gdst_code = FE_DST_CODE;
	const uint32_t gdstf_code = gdst_code + 1;
	const uint32_t gsrc_code = FE_SRC_CODE;
	const uint32_t gsrcf_code = gsrc_code + 1;
	const uint32_t ldst_code = (gdst_code + fp) & 0x1f;
	const uint32_t ldstf_code = (gdstf_code + fp) & 0x1f;
	const uint32_t lsrc_code = (gsrc_code + fp) & 0x1f;
	const uint32_t lsrcf_code = (gsrcf_code + fp) & 0x1f;
	const uint32_t ldst_group = 1 + (((FE_DST_CODE + fp) & 0x20) >> 5);
	const uint32_t ldstf_group = 1 + (((FE_DST_CODE + fp + 1) & 0x20) >> 5);
	const uint32_t lsrc_group = 1 + (((FE_SRC_CODE + fp) & 0x20) >> 5);
	const uint32_t lsrcf_group = 1 + ((FE_SRC_CODE + fp + 1) >> 5);

	switch (op >> 8)
	{
		case 0x00: // chk global,global
			desc.regin[0] |= 1 << gdst_code;
			desc.regin[0] |= 1 << gsrc_code;
			desc.flags |= OPFLAG_CAN_CAUSE_EXCEPTION;
			break;
		case 0x01: // chk global,local
			desc.regin[0] |= SR_CODE;
			desc.regin[0] |= 1 << gdst_code;
			desc.regin[lsrc_group] |= 1 << lsrc_code;
			desc.flags |= OPFLAG_CAN_CAUSE_EXCEPTION;
			break;
		case 0x02: // chk local,global
			desc.regin[0] |= SR_CODE;
			desc.regin[ldst_group] |= 1 << ldst_code;
			desc.regin[0] |= 1 << gsrc_code;
			desc.flags |= OPFLAG_CAN_CAUSE_EXCEPTION;
			break;
		case 0x03: // chk local,local
			desc.regin[0] |= SR_CODE;
			desc.regin[ldst_group] |= 1 << ldst_code;
			desc.regin[lsrc_group] |= 1 << lsrc_code;
			desc.flags |= OPFLAG_CAN_CAUSE_EXCEPTION;
			break;
		case 0x04: // movd global,global
			desc.regin[0] |= 1 << gsrc_code;
			desc.regin[0] |= 1 << gsrcf_code;
			desc.regout[0] |= 1 << gdst_code;
			desc.regout[0] |= 1 << gdstf_code;
			if (gdst_code == 0)
			{
				desc.regout[1] = 0xffffffff;
				desc.regout[2] = 0xffffffff;
				desc.targetpc = BRANCH_TARGET_DYNAMIC;
				desc.flags |= OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_END_SEQUENCE | OPFLAG_CAN_CAUSE_EXCEPTION;
			}
			desc.regout[0] |= SR_CODE;
			break;
		case 0x05: // movd global,local
			desc.regin[0] |= SR_CODE;
			desc.regin[lsrc_group] |= 1 << lsrc_code;
			desc.regin[lsrcf_group] |= 1 << lsrcf_code;
			desc.regout[0] |= 1 << gdst_code;
			desc.regout[0] |= 1 << gdstf_code;
			if (gdst_code == 0)
			{
				desc.regout[1] = 0xffffffff;
				desc.regout[2] = 0xffffffff;
				desc.targetpc = BRANCH_TARGET_DYNAMIC;
				desc.flags |= OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_END_SEQUENCE | OPFLAG_CAN_CAUSE_EXCEPTION;
			}
			desc.regout[0] |= SR_CODE;
			break;
		case 0x06: // movd local,global
			desc.regin[0] |= SR_CODE;
			desc.regin[0] |= 1 << gsrc_code;
			desc.regin[0] |= 1 << gsrcf_code;
			desc.regout[ldst_group] |= 1 << ldst_code;
			desc.regout[ldstf_group] |= 1 << ldstf_code;
			desc.regout[0] |= SR_CODE;
			break;
		case 0x07: // movd local,local
			desc.regin[0] |= SR_CODE;
			desc.regin[lsrc_group] |= 1 << lsrc_code;
			desc.regin[lsrcf_group] |= 1 << lsrcf_code;
			desc.regout[ldst_group] |= 1 << ldst_code;
			desc.regout[ldstf_group] |= 1 << ldstf_code;
			desc.regout[0] |= SR_CODE;
			break;
		case 0x08: // divu global,global
		case 0x0c: // divs global,global
			desc.flags |= OPFLAG_CAN_CAUSE_EXCEPTION;
			desc.regin[0] |= 1 << gsrc_code;
			desc.regin[0] |= 1 << gdst_code;
			desc.regin[0] |= 1 << gdstf_code;
			desc.regout[0] |= 1 << gdst_code;
			desc.regout[0] |= 1 << gdstf_code;
			desc.regout[0] |= SR_CODE;
			break;
		case 0x09: // divu global,local
		case 0x0d: // divs global,local
			desc.flags |= OPFLAG_CAN_CAUSE_EXCEPTION;
			desc.regin[0] |= SR_CODE;
			desc.regin[lsrc_group] |= 1 << lsrc_code;
			desc.regin[0] |= 1 << gdst_code;
			desc.regin[0] |= 1 << gdstf_code;
			desc.regout[0] |= 1 << gdst_code;
			desc.regout[0] |= 1 << gdstf_code;
			desc.regout[0] |= SR_CODE;
			break;
		case 0x0a: // divu local,global
		case 0x0e: // divs local,global
			desc.flags |= OPFLAG_CAN_CAUSE_EXCEPTION;
			desc.regin[0] |= SR_CODE;
			desc.regin[0] |= 1 << gsrc_code;
			desc.regin[ldst_group] |= 1 << ldst_code;
			desc.regin[ldstf_group] |= 1 << ldstf_code;
			desc.regout[ldst_group] |= 1 << ldst_code;
			desc.regout[ldstf_group] |= 1 << ldstf_code;
			desc.regout[0] |= SR_CODE;
			break;
		case 0x0b: // divu local,local
		case 0x0f: // divs local,local
			desc.flags |= OPFLAG_CAN_CAUSE_EXCEPTION;
			desc.regin[0] |= SR_CODE;
			desc.regin[lsrc_group] |= 1 << lsrc_code;
			desc.regin[ldst_group] |= 1 << ldst_code;
			desc.regin[ldstf_group] |= 1 << ldstf_code;
			desc.regout[ldst_group] |= 1 << ldst_code;
			desc.regout[ldstf_group] |= 1 << ldstf_code;
			desc.regout[0] |= SR_CODE;
			break;
		case 0x10: // xm global,global
			desc.flags |= OPFLAG_CAN_CAUSE_EXCEPTION;
			desc.regin[0] |= 1 << gsrc_code;
			desc.regout[0] |= 1 << gdst_code;
			desc.length = (read_imm1(desc) & 0x8000) ? 6 : 4;
			break;
		case 0x11: // xm global,local
			desc.flags |= OPFLAG_CAN_CAUSE_EXCEPTION;
			desc.regin[0] |= SR_CODE;
			desc.regin[lsrc_group] |= 1 << lsrc_code;
			desc.regout[0] |= 1 << gdst_code;
			desc.length = (read_imm1(desc) & 0x8000) ? 6 : 4;
			break;
		case 0x12: // xm local,global
			desc.flags |= OPFLAG_CAN_CAUSE_EXCEPTION;
			desc.regin[0] |= SR_CODE;
			desc.regin[0] |= 1 << gsrc_code;
			desc.regout[ldst_group] |= 1 << ldst_code;
			desc.length = (read_imm1(desc) & 0x8000) ? 6 : 4;
			break;
		case 0x13: // xm local,local
			desc.flags |= OPFLAG_CAN_CAUSE_EXCEPTION;
			desc.regin[0] |= SR_CODE;
			desc.regin[lsrc_group] |= 1 << lsrc_code;
			desc.regout[ldst_group] |= 1 << ldst_code;
			desc.length = (read_imm1(desc) & 0x8000) ? 6 : 4;
			break;
		case 0x14: // mask global,global
			desc.regin[0] |= 1 << gsrc_code;
			desc.regout[0] |= 1 << gdst_code;
			desc.regout[0] |= SR_CODE;
			desc.length = (read_imm1(desc) & 0x8000) ? 6 : 4;
			break;
		case 0x15: // mask global,local
			desc.regin[0] |= SR_CODE;
			desc.regin[lsrc_group] |= 1 << lsrc_code;
			desc.regout[0] |= 1 << gdst_code;
			desc.regout[0] |= SR_CODE;
			desc.length = (read_imm1(desc) & 0x8000) ? 6 : 4;
			break;
		case 0x16: // mask local,global
			desc.regin[0] |= SR_CODE;
			desc.regin[0] |= 1 << gsrc_code;
			desc.regout[ldst_group] |= 1 << ldst_code;
			desc.regout[0] |= SR_CODE;
			desc.length = (read_imm1(desc) & 0x8000) ? 6 : 4;
			break;
		case 0x17: // mask local,local
			desc.regin[0] |= SR_CODE;
			desc.regin[lsrc_group] |= 1 << lsrc_code;
			desc.regout[ldst_group] |= 1 << ldst_code;
			desc.regout[0] |= SR_CODE;
			desc.length = (read_imm1(desc) & 0x8000) ? 6 : 4;
			break;
		case 0x18: // sum global,global
		case 0x1c: // sums global,global
			desc.regin[0] |= 1 << gsrc_code;
			desc.regout[0] |= 1 << gdst_code;
			desc.regout[0] |= SR_CODE;
			desc.length = (read_imm1(desc) & 0x8000) ? 6 : 4;
			if (op & 0x4 && gsrc_code != SR_REGISTER) desc.flags |= OPFLAG_CAN_CAUSE_EXCEPTION;
			break;
		case 0x19: // sum global,local
		case 0x1d: // sums global,local
			desc.regin[0] |= SR_CODE;
			desc.regin[lsrc_group] |= 1 << lsrc_code;
			desc.regout[0] |= 1 << gdst_code;
			desc.regout[0] |= SR_CODE;
			desc.length = (read_imm1(desc) & 0x8000) ? 6 : 4;
			if (op & 0x4) desc.flags |= OPFLAG_CAN_CAUSE_EXCEPTION;
			break;
		case 0x1a: // sum local,global
		case 0x1e: // sums local,global
			desc.regin[0] |= SR_CODE;
			desc.regin[0] |= 1 << gsrc_code;
			desc.regout[ldst_group] |= 1 << ldst_code;
			desc.regout[0] |= SR_CODE;
			desc.length = (read_imm1(desc) & 0x8000) ? 6 : 4;
			if (op & 0x4 && gsrc_code != SR_REGISTER) desc.flags |= OPFLAG_CAN_CAUSE_EXCEPTION;
			break;
		case 0x1b: // sum local,local
		case 0x1f: // sums local,local
			desc.regin[0] |= SR_CODE;
			desc.regin[lsrc_group] |= 1 << lsrc_code;
			desc.regout[ldst_group] |= 1 << ldst_code;
			desc.regout[0] |= SR_CODE;
			desc.length = (read_imm1(desc) & 0x8000) ? 6 : 4;
			if (op & 0x4) desc.flags |= OPFLAG_CAN_CAUSE_EXCEPTION;
			break;
		case 0x20: // cmp global,global
		case 0x30: // cmpb global,global
			desc.regin[0] |= 1 << gsrc_code;
			desc.regin[0] |= 1 << gdst_code;
			desc.regout[0] |= SR_CODE;
			break;
		case 0x21: // cmp global,local
		case 0x31: // cmpb global,local
			desc.regin[0] |= SR_CODE;
			desc.regin[lsrc_group] |= 1 << lsrc_code;
			desc.regin[0] |= 1 << gdst_code;
			desc.regout[0] |= SR_CODE;
			break;
		case 0x22: // cmp local,global
		case 0x32: // cmpb local,global
			desc.regin[0] |= SR_CODE;
			desc.regin[0] |= 1 << gsrc_code;
			desc.regin[ldst_group] |= 1 << ldst_code;
			desc.regout[0] |= SR_CODE;
			break;
		case 0x23: // cmp local,local
		case 0x33: // cmpb local,local
			desc.regin[0] |= SR_CODE;
			desc.regin[lsrc_group] |= 1 << lsrc_code;
			desc.regin[ldst_group] |= 1 << ldst_code;
			desc.regout[0] |= SR_CODE;
			break;
		case 0x24: // mov global,global
			desc.regin[0] |= SR_CODE;
			desc.regin[0] |= 1 << gsrc_code;
			desc.regin[0] |= 1 << (gsrc_code + 16);
			desc.regout[0] |= 1 << gdst_code;
			desc.regout[0] |= SR_CODE;
			desc.flags |= OPFLAG_CAN_CAUSE_EXCEPTION;
			if (gdst_code == PC_REGISTER)
			{
				desc.targetpc = BRANCH_TARGET_DYNAMIC;
				desc.flags |= OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_END_SEQUENCE;
			}
			else if (gdst_code == 5) // TPR_REGISTER & 0xf
			{
				desc.flags |= OPFLAG_END_SEQUENCE;
			}
			break;
		case 0x25: // mov global,local
			desc.regin[0] |= SR_CODE;
			desc.regin[lsrc_group] |= 1 << lsrc_code;
			desc.regout[0] |= 1 << gdst_code;
			desc.regout[0] |= SR_CODE;
			desc.flags |= OPFLAG_CAN_CAUSE_EXCEPTION;
			if (gdst_code == PC_REGISTER)
			{
				desc.targetpc = BRANCH_TARGET_DYNAMIC;
				desc.flags |= OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_END_SEQUENCE;
			}
			else if (gdst_code == 5) // TPR_REGISTER & 0xf
			{
				desc.flags |= OPFLAG_END_SEQUENCE;
			}
			break;
		case 0x26: // mov local,global
			desc.regin[0] |= SR_CODE;
			desc.regin[0] |= 1 << gsrc_code;
			desc.regin[0] |= 1 << (gsrc_code + 16);
			desc.regout[ldst_group] |= 1 << ldst_code;
			desc.regout[0] |= SR_CODE;
			break;
		case 0x27: // mov local,local
			desc.regin[0] |= SR_CODE;
			desc.regin[lsrc_group] |= 1 << lsrc_code;
			desc.regout[ldst_group] |= 1 << ldst_code;
			desc.regout[0] |= SR_CODE;
			break;
		case 0x28: // add global,global
		case 0x2c: // adds global,global
		case 0x48: // sub global,global
		case 0x4c: // subs global,global
			desc.regin[0] |= 1 << gsrc_code;
			desc.regin[0] |= 1 << gdst_code;
			desc.regout[0] |= 1 << gdst_code;
			desc.regout[0] |= SR_CODE;
			if (op & 0x04) desc.flags |= OPFLAG_CAN_CAUSE_EXCEPTION; // adds, subs
			if (gdst_code == PC_REGISTER)
			{
				desc.targetpc = BRANCH_TARGET_DYNAMIC;
				desc.flags |= OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_END_SEQUENCE;
			}
			break;
		case 0x29: // add global,local
		case 0x2d: // adds global,local
		case 0x49: // sub global,local
		case 0x4d: // subs global,local
			desc.regin[0] |= SR_CODE;
			desc.regin[lsrc_group] |= 1 << lsrc_code;
			desc.regin[0] |= 1 << gdst_code;
			desc.regout[0] |= 1 << gdst_code;
			desc.regout[0] |= SR_CODE;
			if (op & 0x04) desc.flags |= OPFLAG_CAN_CAUSE_EXCEPTION; // adds, subs
			if (gdst_code == PC_REGISTER)
			{
				desc.targetpc = BRANCH_TARGET_DYNAMIC;
				desc.flags |= OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_END_SEQUENCE;
			}
			break;
		case 0x2a: // add local,global
		case 0x2e: // adds local,global
		case 0x4a: // sub local,global
		case 0x4e: // subs local,global
			desc.regin[0] |= SR_CODE;
			desc.regin[0] |= 1 << gsrc_code;
			desc.regin[ldst_group] |= 1 << ldst_code;
			desc.regout[ldst_group] |= 1 << ldst_code;
			desc.regout[0] |= SR_CODE;
			if (op & 0x04) desc.flags |= OPFLAG_CAN_CAUSE_EXCEPTION; // adds, subs
			break;
		case 0x2b: // add local,local
		case 0x2f: // adds local,local
		case 0x4b: // sub local,local
		case 0x4f: // subs local,local
			desc.regin[0] |= SR_CODE;
			desc.regin[lsrc_group] |= 1 << lsrc_code;
			desc.regin[ldst_group] |= 1 << ldst_code;
			desc.regout[ldst_group] |= 1 << ldst_code;
			desc.regout[0] |= SR_CODE;
			if (op & 0x04) desc.flags |= OPFLAG_CAN_CAUSE_EXCEPTION; // adds, subs
			break;
		case 0x34: // andn global,global
		case 0x38: // or global,global
		case 0x3c: // xor global,global
		case 0x54: // and global,global
			desc.regin[0] |= 1 << gsrc_code;
			desc.regin[0] |= 1 << gdst_code;
			desc.regout[0] |= 1 << gdst_code;
			desc.regout[0] |= SR_CODE;
			if (gdst_code == PC_REGISTER)
			{
				desc.targetpc = BRANCH_TARGET_DYNAMIC;
				desc.flags |= OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_END_SEQUENCE;
			}
			break;
		case 0x35: // andn global,local
		case 0x39: // or global,local
		case 0x3d: // xor global,local
		case 0x55: // and global,local
			desc.regin[0] |= SR_CODE;
			desc.regin[lsrc_group] |= 1 << lsrc_code;
			desc.regin[0] |= 1 << gdst_code;
			desc.regout[0] |= 1 << gdst_code;
			desc.regout[0] |= SR_CODE;
			if (gdst_code == PC_REGISTER)
			{
				desc.targetpc = BRANCH_TARGET_DYNAMIC;
				desc.flags |= OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_END_SEQUENCE;
			}
			break;
		case 0x36: // andn local,global
		case 0x3a: // or local,global
		case 0x3e: // xor local,global
		case 0x56: // and local,global
			desc.regin[0] |= SR_CODE;
			desc.regin[0] |= 1 << gsrc_code;
			desc.regin[ldst_group] |= 1 << ldst_code;
			desc.regout[ldst_group] |= 1 << ldst_code;
			desc.regout[0] |= SR_CODE;
			break;
		case 0x37: // andn local,local
		case 0x3b: // or local,local
		case 0x3f: // xor local,local
		case 0x57: // and local,local
			desc.regin[0] |= SR_CODE;
			desc.regin[lsrc_group] |= 1 << lsrc_code;
			desc.regin[ldst_group] |= 1 << ldst_code;
			desc.regout[ldst_group] |= 1 << ldst_code;
			desc.regout[0] |= SR_CODE;
			break;
		case 0x40: // subc global,global
		case 0x50: // addc global,global
			desc.regin[0] |= SR_CODE;
			desc.regin[0] |= 1 << gsrc_code;
			desc.regin[0] |= 1 << gdst_code;
			desc.regout[0] |= 1 << gdst_code;
			desc.regout[0] |= SR_CODE;
			if (gdst_code == PC_REGISTER)
			{
				desc.targetpc = BRANCH_TARGET_DYNAMIC;
				desc.flags |= OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_END_SEQUENCE;
			}
			break;
		case 0x41: // subc global,local
		case 0x51: // addc global,local
			desc.regin[0] |= SR_CODE;
			desc.regin[lsrc_group] |= 1 << lsrc_code;
			desc.regin[0] |= 1 << gdst_code;
			desc.regout[0] |= 1 << gdst_code;
			desc.regout[0] |= SR_CODE;
			if (gdst_code == PC_REGISTER)
			{
				desc.targetpc = BRANCH_TARGET_DYNAMIC;
				desc.flags |= OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_END_SEQUENCE;
			}
			break;
		case 0x42: // subc local,global
		case 0x52: // addc local,global
			desc.regin[0] |= SR_CODE;
			desc.regin[0] |= 1 << gsrc_code;
			desc.regin[ldst_group] |= 1 << ldst_code;
			desc.regout[ldst_group] |= 1 << ldst_code;
			desc.regout[0] |= SR_CODE;
			break;
		case 0x43: // subc local,local
		case 0x53: // addc local,local
			desc.regin[0] |= SR_CODE;
			desc.regin[lsrc_group] |= 1 << lsrc_code;
			desc.regin[ldst_group] |= 1 << ldst_code;
			desc.regout[ldst_group] |= 1 << ldst_code;
			desc.regout[0] |= SR_CODE;
			break;
		case 0x44: // not global,global
		case 0x58: // neg global,global
			desc.regin[0] |= 1 << gsrc_code;
			desc.regout[0] |= 1 << gdst_code;
			desc.regout[0] |= SR_CODE;
			if (gdst_code == PC_REGISTER)
			{
				desc.targetpc = BRANCH_TARGET_DYNAMIC;
				desc.flags |= OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_END_SEQUENCE;
			}
			break;
		case 0x45: // not global,local
		case 0x59: // neg global,local
			desc.regin[0] |= SR_CODE;
			desc.regin[lsrc_group] |= 1 << lsrc_code;
			desc.regout[0] |= 1 << gdst_code;
			desc.regout[0] |= SR_CODE;
			if (gdst_code == PC_REGISTER)
			{
				desc.targetpc = BRANCH_TARGET_DYNAMIC;
				desc.flags |= OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_END_SEQUENCE;
			}
			break;
		case 0x46: // not local,global
		case 0x5a: // neg local,global
			desc.regin[0] |= SR_CODE;
			desc.regin[0] |= 1 << gsrc_code;
			desc.regout[ldst_group] |= 1 << ldst_code;
			desc.regout[0] |= SR_CODE;
			break;
		case 0x47: // not local,local
		case 0x5b: // neg local,local
			desc.regin[0] |= SR_CODE;
			desc.regin[lsrc_group] |= 1 << lsrc_code;
			desc.regout[ldst_group] |= 1 << ldst_code;
			desc.regout[0] |= SR_CODE;
			break;
		case 0x5c: // negs global,global
			desc.regin[0] |= 1 << gsrc_code;
			desc.regout[0] |= 1 << gdst_code;
			desc.regout[0] |= SR_CODE;
			desc.flags |= OPFLAG_CAN_CAUSE_EXCEPTION;
			if (gdst_code == PC_REGISTER)
			{
				desc.targetpc = BRANCH_TARGET_DYNAMIC;
				desc.flags |= OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_END_SEQUENCE;
			}
			break;
		case 0x5d: // negs global,local
			desc.regin[0] |= SR_CODE;
			desc.regin[lsrc_group] |= 1 << lsrc_code;
			desc.regout[0] |= 1 << gdst_code;
			desc.regout[0] |= SR_CODE;
			desc.flags |= OPFLAG_CAN_CAUSE_EXCEPTION;
			if (gdst_code == PC_REGISTER)
			{
				desc.targetpc = BRANCH_TARGET_DYNAMIC;
				desc.flags |= OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_END_SEQUENCE;
			}
			break;
		case 0x5e: // negs local,global
			desc.regin[0] |= SR_CODE;
			desc.regin[0] |= 1 << gsrc_code;
			desc.regout[ldst_group] |= 1 << ldst_code;
			desc.regout[0] |= SR_CODE;
			desc.flags |= OPFLAG_CAN_CAUSE_EXCEPTION;
			break;
		case 0x5f: // negs local,local
			desc.regin[0] |= SR_CODE;
			desc.regin[lsrc_group] |= 1 << lsrc_code;
			desc.regout[ldst_group] |= 1 << ldst_code;
			desc.regout[0] |= SR_CODE;
			desc.flags |= OPFLAG_CAN_CAUSE_EXCEPTION;
			break;
		case 0x60: // cmpi global,simm
		case 0x70: // cmpbi global,simm
			desc.regout[0] |= 1 << gdst_code;
			desc.regout[0] |= SR_CODE;
			break;
		case 0x61: // cmpi global,limm
		case 0x71: // cmpbi global,limm
			desc.regout[0] |= 1 << gdst_code;
			desc.regout[0] |= SR_CODE;
			desc.length = hyperstone_device::imm_length(op) << 1;
			break;
		case 0x62: // cmpi local,simm
		case 0x72: // cmpbi local,simm
			desc.regin[0] |= SR_CODE;
			desc.regout[ldst_group] |= 1 << ldst_code;
			desc.regout[0] |= SR_CODE;
			break;
		case 0x63: // cmpi local,limm
		case 0x73: // cmpbi local,limm
			desc.regin[0] |= SR_CODE;
			desc.regout[ldst_group] |= 1 << ldst_code;
			desc.regout[0] |= SR_CODE;
			desc.length = hyperstone_device::imm_length(op) << 1;
			break;
		case 0x64: // movi global,simm
			desc.regin[0] |= SR_CODE;
			desc.regout[0] |= 1 << gdst_code;
			desc.regout[0] |= 1 << (gdst_code + 16);
			desc.regout[0] |= SR_CODE;
			desc.flags |= OPFLAG_CAN_CAUSE_EXCEPTION;
			if (gdst_code == PC_REGISTER)
			{
				desc.targetpc = op & 0xf;
				desc.flags |= OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_END_SEQUENCE;
			}
			else if (gdst_code == 5) // TPR_REGISTER & 0xf
			{
				desc.flags |= OPFLAG_END_SEQUENCE;
			}
			break;
		case 0x65: // movi global,limm
			desc.regin[0] |= SR_CODE;
			desc.regout[0] |= 1 << gdst_code;
			desc.regout[0] |= 1 << (gdst_code + 16);
			desc.regout[0] |= SR_CODE;
			desc.length = hyperstone_device::imm_length(op) << 1;
			desc.flags |= OPFLAG_CAN_CAUSE_EXCEPTION;
			if (gdst_code == PC_REGISTER)
			{
				desc.targetpc = read_limm(desc, op);
				desc.flags |= OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_END_SEQUENCE;
			}
			else if (gdst_code == 5) // TPR_REGISTER & 0xf
			{
				desc.flags |= OPFLAG_END_SEQUENCE;
			}
			break;
		case 0x66: // movi local,simm
			desc.regin[0] |= SR_CODE;
			desc.regout[ldst_group] |= 1 << ldst_code;
			desc.regout[0] |= SR_CODE;
			break;
		case 0x67: // movi local,limm
			desc.regin[0] |= SR_CODE;
			desc.regout[ldst_group] |= 1 << ldst_code;
			desc.regout[0] |= SR_CODE;
			desc.length = hyperstone_device::imm_length(op) << 1;
			break;
		case 0x68: // addi global,simm
		case 0x6c: // addsi global,simm
			desc.regin[0] |= SR_CODE;
			desc.regin[0] |= 1 << gdst_code;
			desc.regout[0] |= 1 << gdst_code;
			desc.regout[0] |= SR_CODE;
			if (op & 0x04) desc.flags |= OPFLAG_CAN_CAUSE_EXCEPTION; // addsi
			if (gdst_code == PC_REGISTER)
			{
				desc.targetpc = BRANCH_TARGET_DYNAMIC;
				desc.flags |= OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_END_SEQUENCE;
			}
			break;
		case 0x69: // addi global,limm
		case 0x6d: // addsi global,limm
			desc.regin[0] |= SR_CODE;
			desc.regin[0] |= 1 << gdst_code;
			desc.regout[0] |= 1 << gdst_code;
			desc.regout[0] |= SR_CODE;
			desc.length = hyperstone_device::imm_length(op) << 1;
			if (op & 0x04) desc.flags |= OPFLAG_CAN_CAUSE_EXCEPTION; // addsi
			if (gdst_code == PC_REGISTER)
			{
				desc.targetpc = desc.pc + desc.length + read_limm(desc, op);
				desc.flags |= OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_END_SEQUENCE;
			}
			break;
		case 0x6a: // addi local,simm
		case 0x6e: // addsi local,simm
			desc.regin[0] |= SR_CODE;
			desc.regin[ldst_group] |= 1 << ldst_code;
			desc.regout[ldst_group] |= 1 << ldst_code;
			desc.regout[0] |= SR_CODE;
			if (op & 0x04) desc.flags |= OPFLAG_CAN_CAUSE_EXCEPTION; // addsi
			break;
		case 0x6b: // addi local,limm
		case 0x6f: // addsi local,limm
			desc.regin[0] |= SR_CODE;
			desc.regin[ldst_group] |= 1 << ldst_code;
			desc.regout[ldst_group] |= 1 << ldst_code;
			desc.regout[0] |= SR_CODE;
			desc.length = hyperstone_device::imm_length(op) << 1;
			if (op & 0x04) desc.flags |= OPFLAG_CAN_CAUSE_EXCEPTION; // addsi
			break;
		case 0x74: // andni global,simm
		case 0x78: // ori global,simm
		case 0x7c: // xori global,simm
			desc.regin[0] |= SR_CODE;
			desc.regin[0] |= 1 << gdst_code;
			desc.regout[0] |= 1 << gdst_code;
			desc.regout[0] |= SR_CODE;
			if (gdst_code == PC_REGISTER)
			{
				desc.targetpc = BRANCH_TARGET_DYNAMIC;
				desc.flags |= OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_END_SEQUENCE;
			}
			break;
		case 0x75: // andni global,limm
		case 0x79: // ori global,limm
		case 0x7d: // xori global,limm
			desc.regin[0] |= SR_CODE;
			desc.regin[0] |= 1 << gdst_code;
			desc.regout[0] |= 1 << gdst_code;
			desc.regout[0] |= SR_CODE;
			desc.length = hyperstone_device::imm_length(op) << 1;
			if (gdst_code == PC_REGISTER)
			{
				desc.targetpc = BRANCH_TARGET_DYNAMIC;
				desc.flags |= OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_END_SEQUENCE;
			}
			break;
		case 0x76: // andni local,simm
		case 0x7a: // ori local,simm
		case 0x7e: // xori local,simm
			desc.regin[0] |= SR_CODE;
			desc.regin[ldst_group] |= 1 << ldst_code;
			desc.regout[ldst_group] |= 1 << ldst_code;
			desc.regout[0] |= SR_CODE;
			break;
		case 0x77: // andni local,limm
		case 0x7b: // ori local,limm
		case 0x7f: // xori local,limm
			desc.regin[0] |= SR_CODE;
			desc.regin[ldst_group] |= 1 << ldst_code;
			desc.regout[ldst_group] |= 1 << ldst_code;
			desc.regout[0] |= SR_CODE;
			desc.length = hyperstone_device::imm_length(op) << 1;
			break;
		case 0x80: case 0x81: // shrdi
		case 0x84: case 0x85: // sardi
		case 0x88: case 0x89: // shldi
			desc.regin[0] |= SR_CODE;
			desc.regout[ldst_group] |= 1 << ldst_code;
			desc.regout[ldstf_group] |= 1 << ldstf_code;
			desc.regout[0] |= SR_CODE;
			break;
		case 0x82: // shrd
		case 0x86: // sard
		case 0x8a: // shld
			desc.regin[0] |= SR_CODE;
			desc.regin[lsrc_group] |= 1 << lsrc_code;
			desc.regout[ldst_group] |= 1 << ldst_code;
			desc.regout[ldstf_group] |= 1 << ldstf_code;
			desc.regout[0] |= SR_CODE;
			break;
		case 0x83: // shr
		case 0x87: // sar
		case 0x8b: // shl
		case 0x8f: // rol
			desc.regin[0] |= SR_CODE;
			desc.regin[lsrc_group] |= 1 << lsrc_code;
			desc.regout[ldst_group] |= 1 << ldst_code;
			desc.regout[0] |= SR_CODE;
			break;
		case 0x8c: case 0x8d: // reserved
			return false;
		case 0x8e: // testlz
			desc.regin[0] |= SR_CODE;
			desc.regin[lsrc_group] |= 1 << lsrc_code;
			desc.regout[ldst_group] |= 1 << ldst_code;
			break;
		case 0x90: // ldxx1 global,global
		{
			const uint16_t imm1 = read_imm1(desc);
			const uint32_t extra_s = read_ldstxx_imm(desc);

			desc.regin[0] |= 1 << gdst_code;
			desc.regout[0] |= 1 << gsrc_code;
			if ((imm1 & 0x3000) == 0x3000 && (extra_s & 2)) desc.regout[0] |= 1 << gsrcf_code;

			desc.length = (imm1 & 0x8000) ? 6 : 4;
			desc.flags |= OPFLAG_READS_MEMORY;
			if (gdst_code == PC_REGISTER)
			{
				desc.targetpc = BRANCH_TARGET_DYNAMIC;
				desc.flags |= OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_END_SEQUENCE;
			}
			break;
		}
		case 0x91: // ldxx1 global,local
		{
			const uint16_t imm1 = read_imm1(desc);
			const uint32_t extra_s = read_ldstxx_imm(desc);

			desc.regin[0] |= SR_CODE;
			desc.regin[0] |= 1 << gdst_code;
			desc.regout[lsrc_group] |= 1 << lsrc_code;
			if ((imm1 & 0x3000) == 0x3000 && (extra_s & 2)) desc.regout[lsrcf_group] |= 1 << lsrcf_code;

			desc.length = (imm1 & 0x8000) ? 6 : 4;
			desc.flags |= OPFLAG_READS_MEMORY;
			if (gdst_code == PC_REGISTER)
			{
				desc.targetpc = BRANCH_TARGET_DYNAMIC;
				desc.flags |= OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_END_SEQUENCE;
			}
			break;
		}
		case 0x92: // ldxx1 local,global
		{
			const uint16_t imm1 = read_imm1(desc);
			const uint32_t extra_s = read_ldstxx_imm(desc);

			desc.regin[0] |= SR_CODE;
			desc.regin[ldst_group] |= 1 << ldst_code;
			desc.regout[0] |= 1 << gsrc_code;
			if ((imm1 & 0x3000) == 0x3000 && (extra_s & 2)) desc.regout[0] |= 1 << gsrcf_code;

			desc.length = (imm1 & 0x8000) ? 6 : 4;
			desc.flags |= OPFLAG_READS_MEMORY;
			break;
		}
		case 0x93: // ldxx1 local,local
		{
			const uint16_t imm1 = read_imm1(desc);
			const uint32_t extra_s = read_ldstxx_imm(desc);

			desc.regin[0] |= SR_CODE;
			desc.regin[ldst_group] |= 1 << ldst_code;
			desc.regout[lsrc_group] |= 1 << lsrc_code;
			if ((imm1 & 0x3000) == 0x3000 && (extra_s & 2)) desc.regout[lsrcf_group] |= 1 << lsrcf_code;

			desc.length = (imm1 & 0x8000) ? 6 : 4;
			desc.flags |= OPFLAG_READS_MEMORY;
			break;
		}
		case 0x94: // ldxx2 global,global
		{
			const uint16_t imm1 = read_imm1(desc);
			const uint32_t extra_s = read_ldstxx_imm(desc);

			desc.regin[0] |= 1 << gdst_code;
			desc.regout[0] |= 1 << gdst_code;
			desc.regout[0] |= 1 << gsrc_code;
			if ((imm1 & 0x3000) == 0x3000 && (extra_s & 2)) desc.regout[0] |= 1 << gsrcf_code;

			desc.length = (imm1 & 0x8000) ? 6 : 4;
			desc.flags |= OPFLAG_READS_MEMORY;
			if (gdst_code == PC_REGISTER)
			{
				desc.targetpc = BRANCH_TARGET_DYNAMIC;
				desc.flags |= OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_END_SEQUENCE;
			}
			break;
		}
		case 0x95: // ldxx2 global,local
		{
			const uint16_t imm1 = read_imm1(desc);
			const uint32_t extra_s = read_ldstxx_imm(desc);

			desc.regin[0] |= SR_CODE;
			desc.regin[0] |= 1 << gdst_code;
			desc.regout[0] |= 1 << gdst_code;
			desc.regout[lsrc_group] |= 1 << lsrc_code;
			if ((imm1 & 0x3000) == 0x3000 && (extra_s & 2)) desc.regout[lsrcf_group] |= 1 << lsrcf_code;

			desc.length = (imm1 & 0x8000) ? 6 : 4;
			desc.flags |= OPFLAG_READS_MEMORY;
			if (gdst_code == PC_REGISTER)
			{
				desc.targetpc = BRANCH_TARGET_DYNAMIC;
				desc.flags |= OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_END_SEQUENCE;
			}
			break;
		}
		case 0x96: // ldxx2 local,global
		{
			const uint16_t imm1 = read_imm1(desc);
			const uint32_t extra_s = read_ldstxx_imm(desc);

			desc.regin[0] |= SR_CODE;
			desc.regin[ldst_group] |= 1 << ldst_code;
			desc.regout[ldst_group] |= 1 << ldst_code;
			desc.regout[0] |= 1 << gsrc_code;
			if ((imm1 & 0x3000) == 0x3000 && (extra_s & 2)) desc.regout[0] |= 1 << gsrcf_code;

			desc.length = (imm1 & 0x8000) ? 6 : 4;
			desc.flags |= OPFLAG_READS_MEMORY;
			break;
		}
		case 0x97: // ldxx2 local,local
		{
			const uint16_t imm1 = read_imm1(desc);
			const uint32_t extra_s = read_ldstxx_imm(desc);

			desc.regin[0] |= SR_CODE;
			desc.regin[ldst_group] |= 1 << ldst_code;
			desc.regout[ldst_group] |= 1 << ldst_code;
			desc.regout[lsrc_group] |= 1 << lsrc_code;
			if ((imm1 & 0x3000) == 0x3000 && (extra_s & 2)) desc.regout[lsrcf_group] |= 1 << lsrcf_code;

			desc.length = (imm1 & 0x8000) ? 6 : 4;
			desc.flags |= OPFLAG_READS_MEMORY;
			break;
		}
		case 0x98: // stxx1 global,global
		{
			const uint16_t imm1 = read_imm1(desc);
			const uint32_t extra_s = read_ldstxx_imm(desc);

			desc.regin[0] |= 1 << gdst_code;
			desc.regin[0] |= 1 << gsrc_code;
			if ((imm1 & 0x3000) == 0x3000 && (extra_s & 2)) desc.regin[0] |= 1 << gsrcf_code;

			desc.length = (imm1 & 0x8000) ? 6 : 4;
			desc.flags |= OPFLAG_WRITES_MEMORY;
			break;
		}
		case 0x99: // stxx1 global,local
		{
			const uint16_t imm1 = read_imm1(desc);
			const uint32_t extra_s = read_ldstxx_imm(desc);

			desc.regin[0] |= SR_CODE;
			desc.regin[0] |= 1 << gdst_code;
			desc.regin[lsrc_group] |= 1 << lsrc_code;
			if ((imm1 & 0x3000) == 0x3000 && (extra_s & 2)) desc.regin[lsrcf_group] |= 1 << lsrcf_code;

			desc.length = (imm1 & 0x8000) ? 6 : 4;
			desc.flags |= OPFLAG_WRITES_MEMORY;
			break;
		}
		case 0x9a: // stxx1 local,global
		{
			const uint16_t imm1 = read_imm1(desc);
			const uint32_t extra_s = read_ldstxx_imm(desc);

			desc.regin[0] |= SR_CODE;
			desc.regin[ldst_group] |= 1 << ldst_code;
			desc.regin[0] |= 1 << gsrc_code;
			if ((imm1 & 0x3000) == 0x3000 && (extra_s & 2)) desc.regin[0] |= 1 << gsrcf_code;

			desc.length = (imm1 & 0x8000) ? 6 : 4;
			desc.flags |= OPFLAG_WRITES_MEMORY;
			break;
		}
		case 0x9b: // stxx1 local,local
		{
			const uint16_t imm1 = read_imm1(desc);
			const uint32_t extra_s = read_ldstxx_imm(desc);

			desc.regin[0] |= SR_CODE;
			desc.regin[ldst_group] |= 1 << ldst_code;
			desc.regin[lsrc_group] |= 1 << lsrc_code;
			if ((imm1 & 0x3000) == 0x3000 && (extra_s & 2)) desc.regin[lsrcf_group] |= 1 << lsrcf_code;

			desc.length = (imm1 & 0x8000) ? 6 : 4;
			desc.flags |= OPFLAG_WRITES_MEMORY;
			break;
		}
		case 0x9c: // stxx2 global,global
		{
			const uint16_t imm1 = read_imm1(desc);
			const uint32_t extra_s = read_ldstxx_imm(desc);

			desc.regin[0] |= 1 << gdst_code;
			desc.regin[0] |= 1 << gsrc_code;
			if ((imm1 & 0x3000) == 0x3000 && (extra_s & 2)) desc.regin[0] |= 1 << gsrcf_code;
			desc.regout[0] |= 1 << gdst_code;

			desc.length = (imm1 & 0x8000) ? 6 : 4;
			desc.flags |= OPFLAG_WRITES_MEMORY;
			break;
		}
		case 0x9d: // stxx2 global,local
		{
			const uint16_t imm1 = read_imm1(desc);
			const uint32_t extra_s = read_ldstxx_imm(desc);

			desc.regin[0] |= SR_CODE;
			desc.regin[0] |= 1 << gdst_code;
			desc.regin[lsrc_group] |= 1 << lsrc_code;
			if ((imm1 & 0x3000) == 0x3000 && (extra_s & 2)) desc.regin[lsrcf_group] |= 1 << lsrcf_code;
			desc.regout[0] |= 1 << gdst_code;

			desc.length = (imm1 & 0x8000) ? 6 : 4;
			desc.flags |= OPFLAG_WRITES_MEMORY;
			break;
		}
		case 0x9e: // stxx2 local,global
		{
			const uint16_t imm1 = read_imm1(desc);
			const uint32_t extra_s = read_ldstxx_imm(desc);

			desc.regin[0] |= SR_CODE;
			desc.regin[ldst_group] |= 1 << ldst_code;
			desc.regin[0] |= 1 << gsrc_code;
			if ((imm1 & 0x3000) == 0x3000 && (extra_s & 2)) desc.regin[0] |= 1 << gsrcf_code;
			desc.regout[ldst_group] |= 1 << ldst_code;

			desc.length = (imm1 & 0x8000) ? 6 : 4;
			desc.flags |= OPFLAG_WRITES_MEMORY;
			break;
		}
		case 0x9f: // stxx2 local,local
		{
			const uint16_t imm1 = read_imm1(desc);
			const uint32_t extra_s = read_ldstxx_imm(desc);

			desc.regin[0] |= SR_CODE;
			desc.regin[ldst_group] |= 1 << ldst_code;
			desc.regin[lsrc_group] |= 1 << lsrc_code;
			if ((imm1 & 0x3000) == 0x3000 && (extra_s & 2)) desc.regin[lsrcf_group] |= 1 << lsrcf_code;
			desc.regout[ldst_group] |= 1 << ldst_code;

			desc.length = (imm1 & 0x8000) ? 6 : 4;
			desc.flags |= OPFLAG_WRITES_MEMORY;
			break;
		}
		case 0xa0: // shri global (lo n)
		case 0xa1: // shri global (hi n)
		case 0xa4: // sari global (lo n)
		case 0xa5: // sari global (hi n)
		case 0xa8: // shli global (lo n)
		case 0xa9: // shli global (hi n)
			desc.regin[0] |= 1 << gdst_code;
			desc.regout[0] |= 1 << gdst_code;
			desc.regout[0] |= SR_CODE;
			break;
		case 0xa2: // shri local (lo n)
		case 0xa3: // shri local (hi n)
		case 0xa6: // sari local (lo n)
		case 0xa7: // sari local (hi n)
		case 0xaa: // shli local (lo n)
		case 0xab: // shli local (hi n)
			desc.regin[0] |= SR_CODE;
			desc.regin[ldst_group] |= 1 << ldst_code;
			desc.regout[ldst_group] |= 1 << ldst_code;
			desc.regout[0] |= SR_CODE;
			break;
		case 0xac: case 0xad: case 0xae: case 0xaf: // reserved
			return false;
		case 0xb0: // mulu global,global
		case 0xb4: // muls global,global
			desc.regin[0] |= 1 << gsrc_code;
			desc.regin[0] |= 1 << gdst_code;
			desc.regout[0] |= 1 << gdst_code;
			desc.regout[0] |= 1 << gdstf_code;
			desc.regout[0] |= SR_CODE;
			break;
		case 0xb1: // mulu global,local
		case 0xb5: // muls global,local
			desc.regin[lsrc_group] |= 1 << lsrc_code;
			desc.regin[0] |= 1 << gdst_code;
			desc.regout[0] |= 1 << gdst_code;
			desc.regout[0] |= 1 << gdstf_code;
			desc.regout[0] |= SR_CODE;
			break;
		case 0xb2: // mulu local,global
		case 0xb6: // muls local,global
			desc.regin[0] |= 1 << gsrc_code;
			desc.regin[ldst_group] |= 1 << ldst_code;
			desc.regout[ldst_group] |= 1 << ldst_code;
			desc.regout[ldstf_group] |= 1 << ldstf_code;
			desc.regout[0] |= SR_CODE;
			break;
		case 0xb3: // mulu local,local
		case 0xb7: // muls local,local
			desc.regin[lsrc_group] |= 1 << lsrc_code;
			desc.regin[ldst_group] |= 1 << ldst_code;
			desc.regout[ldst_group] |= 1 << ldst_code;
			desc.regout[ldstf_group] |= 1 << ldstf_code;
			desc.regout[0] |= SR_CODE;
			break;
		case 0xb8: // set global (lo n)
		case 0xb9: // set global (hi n)
			desc.regin[0] |= SR_CODE;
			desc.regout[0] |= 1 << gdst_code;
			break;
		case 0xba: // set local (lo n)
		case 0xbb: // set local (hi n)
			desc.regout[ldst_group] |= 1 << ldst_code;
			break;
		case 0xbc: // mul global,global
			desc.regin[0] |= 1 << gsrc_code;
			desc.regin[0] |= 1 << gdst_code;
			desc.regout[0] |= 1 << gdst_code;
			desc.regout[0] |= SR_CODE;
			break;
		case 0xbd: // muls global,local
			desc.regin[lsrc_group] |= 1 << lsrc_code;
			desc.regin[0] |= 1 << gdst_code;
			desc.regout[0] |= 1 << gdst_code;
			desc.regout[0] |= SR_CODE;
			break;
		case 0xbe: // muls local,global
			desc.regin[0] |= 1 << gsrc_code;
			desc.regin[ldst_group] |= 1 << ldst_code;
			desc.regout[ldst_group] |= 1 << ldst_code;
			desc.regout[0] |= SR_CODE;
			break;
		case 0xbf: // mulu local,local
			desc.regin[lsrc_group] |= 1 << lsrc_code;
			desc.regin[ldst_group] |= 1 << ldst_code;
			desc.regout[ldst_group] |= 1 << ldst_code;
			desc.regout[0] |= SR_CODE;
			break;
		case 0xc0: case 0xc1: case 0xc2: case 0xc3: // software
		case 0xc4: case 0xc5: case 0xc6: case 0xc7: // software
		case 0xc8: case 0xc9: case 0xca: case 0xcb: // software
		case 0xcc: case 0xcd: // software
		{
			desc.regin[0] |= SR_CODE;
			desc.regin[lsrc_group] |= 1 << lsrc_code;
			desc.regin[lsrcf_group] |= 1 << lsrcf_code;

			const uint32_t reg = FE_FP + FE_FL;
			desc.regout[1 + (((reg + 0) & 0x20) >> 5)] |= 1 << ((reg + 0) & 0x1f);
			desc.regout[1 + (((reg + 1) & 0x20) >> 5)] |= 1 << ((reg + 1) & 0x1f);
			desc.regout[1 + (((reg + 2) & 0x20) >> 5)] |= 1 << ((reg + 2) & 0x1f);
			desc.regout[1 + (((reg + 3) & 0x20) >> 5)] |= 1 << ((reg + 3) & 0x1f);
			desc.regout[1 + (((reg + 4) & 0x20) >> 5)] |= 1 << ((reg + 4) & 0x1f);

			desc.regout[0] |= SR_CODE;

			desc.targetpc = BRANCH_TARGET_DYNAMIC;
			desc.flags |= OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_END_SEQUENCE;
			break;
		}
		case 0xce: // extend - 4 bytes
			desc.regin[0] |= SR_CODE;
			desc.regin[0] |= (3 << 14); // global regs 14, 15
			desc.regin[lsrc_group] |= 1 << lsrc_code;
			desc.regin[ldst_group] |= 1 << ldst_code;
			desc.regout[0] |= (3 << 14); // global regs 14, 15
			desc.length = 4;
			break;
		case 0xcf: // do
			return false;
		case 0xd0: // ldwr global
		case 0xd4: // ldwp global
			desc.regin[0] |= SR_CODE;
			desc.regin[ldst_group] |= 1 << ldst_code;
			desc.regout[0] |= 1 << gsrc_code;
			if (op & 0x04) desc.regout[ldst_group] |= 1 << ldst_code;
			desc.flags |= OPFLAG_READS_MEMORY;
			break;
		case 0xd1: // ldwr local
		case 0xd5: // ldwp local
			desc.regin[0] |= SR_CODE;
			desc.regin[ldst_group] |= 1 << ldst_code;
			desc.regout[lsrc_group] |= 1 << lsrc_code;
			if (op & 0x04) desc.regout[ldst_group] |= 1 << ldst_code;
			desc.flags |= OPFLAG_READS_MEMORY;
			break;
		case 0xd2: // lddr global
		case 0xd6: // lddp global
			desc.regin[0] |= SR_CODE;
			desc.regin[ldst_group] |= 1 << ldst_code;
			desc.regout[0] |= 1 << gsrc_code;
			desc.regout[0] |= 1 << gsrcf_code;
			if (op & 0x04) desc.regout[ldst_group] |= 1 << ldst_code;
			desc.flags |= OPFLAG_READS_MEMORY;
			break;
		case 0xd3: // lddr local
		case 0xd7: // lddp local
			desc.regin[0] |= SR_CODE;
			desc.regin[ldst_group] |= 1 << ldst_code;
			desc.regout[lsrc_group] |= 1 << lsrc_code;
			desc.regout[lsrcf_group] |= 1 << lsrcf_code;
			if (op & 0x04) desc.regout[ldst_group] |= 1 << ldst_code;
			desc.flags |= OPFLAG_READS_MEMORY;
			break;
		case 0xd8: // stwr global
		case 0xdc: // stwp global
			desc.regin[0] |= SR_CODE;
			desc.regin[ldst_group] |= 1 << ldst_code;
			desc.regin[0] |= 1 << gsrc_code;
			if (op & 0x04) desc.regout[ldst_group] |= 1 << ldst_code;
			desc.flags |= OPFLAG_WRITES_MEMORY;
			break;
		case 0xd9: // stwr local
		case 0xdd: // stwp local
			desc.regin[0] |= SR_CODE;
			desc.regin[ldst_group] |= 1 << ldst_code;
			desc.regin[lsrc_group] |= 1 << lsrc_code;
			if (op & 0x04) desc.regout[ldst_group] |= 1 << ldst_code;
			desc.flags |= OPFLAG_WRITES_MEMORY;
			break;
		case 0xda: // stdr global
		case 0xde: // stdp global
			desc.regin[0] |= SR_CODE;
			desc.regin[ldst_group] |= 1 << ldst_code;
			desc.regin[0] |= 1 << gsrc_code;
			desc.regin[0] |= 1 << gsrcf_code;
			if (op & 0x04) desc.regout[ldst_group] |= 1 << ldst_code;
			desc.flags |= OPFLAG_WRITES_MEMORY;
			break;
		case 0xdb: // stdr local
		case 0xdf: // stdp local
			desc.regin[0] |= SR_CODE;
			desc.regin[ldst_group] |= 1 << ldst_code;
			desc.regin[lsrc_group] |= 1 << lsrc_code;
			desc.regin[lsrcf_group] |= 1 << lsrcf_code;
			if (op & 0x04) desc.regout[ldst_group] |= 1 << ldst_code;
			desc.flags |= OPFLAG_WRITES_MEMORY;
			break;
		case 0xe0: case 0xe1: case 0xe2: case 0xe3: // dbv, dbnv, dbe, dbne - could be 4 bytes (pcrel)
		case 0xe4: case 0xe5: case 0xe6: case 0xe7: // dbc, dbnc, dbse, dbht - could be 4 bytes (pcrel)
		case 0xe8: case 0xe9: case 0xea: case 0xeb: // dbn, dbnn, dblt, dbgt - could be 4 bytes (pcrel)
			decode_pcrel(desc, op);
			desc.regin[0] |= SR_CODE;
			desc.targetpc = BRANCH_TARGET_DYNAMIC;
			desc.flags |= OPFLAG_IS_CONDITIONAL_BRANCH | OPFLAG_END_SEQUENCE;
			desc.delayslots = 1;
			desc.length = (op & 0x80) ? 4 : 2;
			break;
		case 0xec: // dbr
			decode_pcrel(desc, op);
			desc.targetpc = BRANCH_TARGET_DYNAMIC;
			desc.flags |= OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_END_SEQUENCE;
			desc.delayslots = 1;
			desc.length = (op & 0x80) ? 4 : 2;
			break;
		case 0xed: // frame
			desc.regin[0] |= SR_CODE;
			desc.regin[1] = 0xffffffff;
			desc.regin[2] = 0xffffffff;
			desc.regout[0] |= SR_CODE;
			desc.flags |= OPFLAG_CAN_CAUSE_EXCEPTION | OPFLAG_END_SEQUENCE;
			break;
		case 0xee: // call global
			desc.regin[0] |= SR_CODE;
			desc.regin[0] |= 1 << gsrc_code;
			desc.regout[ldst_group] |= 1 << ldst_code;
			desc.regout[ldstf_group] |= 1 << ldstf_code;
			desc.targetpc = BRANCH_TARGET_DYNAMIC;
			desc.flags |= OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_END_SEQUENCE;
			desc.length = (read_imm1(desc) & 0x8000) ? 6 : 4;
			break;
		case 0xef: // call local
			desc.regin[0] |= SR_CODE;
			desc.regin[lsrc_group] |= 1 << lsrc_code;
			desc.regout[ldst_group] |= 1 << ldst_code;
			desc.regout[ldstf_group] |= 1 << ldstf_code;
			desc.targetpc = BRANCH_TARGET_DYNAMIC;
			desc.flags |= OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_END_SEQUENCE;
			desc.length = (read_imm1(desc) & 0x8000) ? 6 : 4;
			break;
		case 0xf0: case 0xf1: case 0xf2: case 0xf3: // bv, bnv, be, bne
		case 0xf4: case 0xf5: case 0xf6: case 0xf7: // bc, bnc, bse, bht
		case 0xf8: case 0xf9: case 0xfa: case 0xfb: // bn, bnn, blt, bgt
			decode_pcrel(desc, op);
			desc.regin[0] |= SR_CODE;
			desc.targetpc = BRANCH_TARGET_DYNAMIC;
			desc.flags |= OPFLAG_IS_CONDITIONAL_BRANCH;
			desc.length = (op & 0x80) ? 4 : 2;
			break;
		case 0xfc: // br
		{
			int32_t offset = decode_pcrel(desc, op);
			desc.targetpc = (desc.pc + desc.length) + offset;
			desc.flags |= OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_END_SEQUENCE;
			desc.length = (op & 0x80) ? 4 : 2;
			break;
		}
		case 0xfd: case 0xfe: case 0xff: // trap
			desc.regin[0] |= SR_CODE;
			desc.targetpc = BRANCH_TARGET_DYNAMIC;
			desc.flags |= OPFLAG_IS_CONDITIONAL_BRANCH | OPFLAG_END_SEQUENCE | OPFLAG_CAN_CAUSE_EXCEPTION;
			break;
	}
	return true;
}
