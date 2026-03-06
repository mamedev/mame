// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    e1fe.cpp

    Hyperstone E1 instruction decoder

***************************************************************************/

#include "emu.h"
#include "e1fe.h"

#include "e1defs.h"

#include "cpu/drcfe.ipp"



void hyperstone_device::opcode_desc::log_flags(std::ostream &stream) const
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

	// check H flag
	stream << (check_h() ? 'H' : '.');

	// modes
	stream << (can_change_modes() ? 'M' : '.');

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

void hyperstone_device::opcode_desc::log_registers_used(std::ostream &stream) const
{
	stream << "[use:";
	log_register_list(stream, regin, nullptr);
	stream << ']';
}

void hyperstone_device::opcode_desc::log_registers_modified(std::ostream &stream) const
{
	stream << "[mod:";
	log_register_list(stream, regout, &regreq);
	stream << ']';
}

void hyperstone_device::opcode_desc::log_register_list(std::ostream &stream, const regmask &reglist, const regmask *regnostarlist)
{
	int count = 0;

	for (int regnum = 0; regnum < 32; regnum++)
	{
		if (reglist[REG_G0 + regnum])
		{
			if (count++)
				stream << ',';
			stream << 'G' << regnum;
			if (regnostarlist && !(*regnostarlist)[REG_G0 + regnum])
				stream << '*';
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

	log_bit(REG_C,  "C");
	log_bit(REG_Z,  "Z");
	log_bit(REG_N,  "N");
	log_bit(REG_V,  "V");
	log_bit(REG_FP, "FP");
}


hyperstone_device::frontend::frontend(hyperstone_device &cpu, uint32_t window_start, uint32_t window_end, uint32_t max_sequence)
	: drc_frontend_base(cpu.space_config(AS_PROGRAM)->page_shift(), window_start, window_end, max_sequence)
	, m_cpu(cpu)
{
}

hyperstone_device::frontend::~frontend()
{
}

hyperstone_device::opcode_desc const *hyperstone_device::frontend::describe_code(offs_t startpc)
{
	return do_describe_code(
			[this] (opcode_desc &desc, opcode_desc const *prev) { return describe(desc, prev); },
			startpc);
}


inline void hyperstone_device::frontend::read_op(opcode_desc &desc)
{
	desc.opptr[0] = m_cpu.m_pr16(desc.pc);
}

inline void hyperstone_device::frontend::read_imm1(opcode_desc &desc)
{
	desc.opptr[1] = m_cpu.m_pr16(desc.pc + 2);
}

inline void hyperstone_device::frontend::read_imm2(opcode_desc &desc)
{
	desc.opptr[2] = m_cpu.m_pr16(desc.pc + 4);
}

inline void hyperstone_device::frontend::decode_const(opcode_desc &desc)
{
	read_imm1(desc);
	int32_t const const1 = util::sext(desc.opptr[1], 15);
	if (!E_BIT(desc.opptr[1]))
	{
		desc.length = 4;
		desc.imm = uint32_t(const1);
	}
	else
	{
		read_imm2(desc);
		desc.length = 6;
		desc.imm = uint32_t(const1 << 16) | desc.opptr[2];
	}
}


inline void hyperstone_device::frontend::decode_ll(opcode_desc &desc)
{
	uint16_t const op = desc.opptr[0];
	desc.dst_code = BIT(op, 4, 4);
	desc.src_code = BIT(op, 0, 4);
	desc.dst_local = true;
	desc.src_local = true;
}

inline void hyperstone_device::frontend::decode_llext(opcode_desc &desc)
{
	decode_ll(desc);
	read_imm1(desc);
	desc.length = 4;
}

inline void hyperstone_device::frontend::decode_lr(opcode_desc &desc)
{
	uint16_t const op = desc.opptr[0];
	desc.dst_code = BIT(op, 4, 4);
	desc.src_code = BIT(op, 0, 4);
	desc.dst_local = true;
	desc.src_local = BIT(op, 8);
}

inline void hyperstone_device::frontend::decode_rr(opcode_desc &desc)
{
	uint16_t const op = desc.opptr[0];
	desc.dst_code = BIT(op, 4, 4);
	desc.src_code = BIT(op, 0, 4);
	desc.dst_local = BIT(op, 9);
	desc.src_local = BIT(op, 8);
}

inline void hyperstone_device::frontend::decode_ln(opcode_desc &desc)
{
	uint16_t const op = desc.opptr[0];
	desc.dst_code = BIT(op, 4, 4);
	desc.dst_local = true;
	desc.imm = (BIT(op, 8) << 4) | BIT(op, 0, 4);
}

inline void hyperstone_device::frontend::decode_rn(opcode_desc &desc)
{
	uint16_t const op = desc.opptr[0];
	desc.dst_code = BIT(op, 4, 4);
	desc.dst_local = BIT(op, 9);
	desc.imm = (BIT(op, 8) << 4) | BIT(op, 0, 4);
}

inline void hyperstone_device::frontend::decode_pcrel(opcode_desc &desc)
{
	if (!BIT(desc.opptr[0], 7))
	{
		uint32_t const low_rel = BIT(desc.opptr[0], 1, 6);
		int32_t const s = util::sext(desc.opptr[0], 1);
		desc.imm = uint32_t(s << 7) | (low_rel << 1);
	}
	else
	{
		read_imm1(desc);
		desc.length = 4;
		uint32_t const high_rel = BIT(desc.opptr[0], 0, 7);
		uint32_t const low_rel = BIT(desc.opptr[1], 1, 15);
		int32_t const s = util::sext(desc.opptr[1], 1);
		desc.imm = uint32_t(s << 23) | (high_rel << 16) | (low_rel << 1);
	}
}

inline void hyperstone_device::frontend::decode_lrconst(opcode_desc &desc)
{
	decode_lr(desc);
	decode_const(desc);
}

inline void hyperstone_device::frontend::decode_rrconst(opcode_desc &desc)
{
	decode_rr(desc);
	decode_const(desc);
}

inline void hyperstone_device::frontend::decode_rrdis(opcode_desc &desc)
{
	decode_rr(desc);
	read_imm1(desc);
	int32_t const dis1 = util::sext((BIT(desc.opptr[1], 14) << 12) | BIT(desc.opptr[1], 0, 12), 13);
	if (!E_BIT(desc.opptr[1]))
	{
		desc.length = 4;
		desc.imm = uint32_t(dis1);
	}
	else
	{
		read_imm2(desc);
		desc.length = 6;
		desc.imm = uint32_t(dis1 << 16) | desc.opptr[2];
	}
}

inline void hyperstone_device::frontend::decode_rimm(opcode_desc &desc, bool cmpbi_andni)
{
	constexpr int32_t table[32] = {
			0,      1,      2,      3,      4,      5,      6,      7,
			8,      9,      10,     11,     12,     13,     14,     15,
			16,     -1,     -1,     -1,     32,     64,     128,    int32_t(uint32_t(0x80000000)),
			-8,     -7,     -6,     -5,     -4,     -3,     -2,     -1 };

	uint16_t const op = desc.opptr[0];
	desc.dst_code = BIT(op, 4, 4);
	desc.dst_local = BIT(op, 9);

	uint16_t const n = (BIT(op, 8) << 4) | BIT(op, 0, 4);
	switch (n)
	{
	case 17:
		read_imm1(desc);
		read_imm2(desc);
		desc.length = 6;
		desc.imm = (uint32_t(desc.opptr[1]) << 16) | desc.opptr[2];
		break;
	case 18:
	case 19:
		read_imm1(desc);
		desc.length = 4;
		desc.imm = (BIT(n, 0) ? 0xffff0000U : 0U) | desc.opptr[1];
		break;
	case 31:
		desc.imm = cmpbi_andni ? 0x7fffffff : uint32_t(table[n]);
		break;
	default:
		desc.imm = uint32_t(table[n]);
	}
}

inline void hyperstone_device::frontend::decode_rrlim(opcode_desc &desc)
{
	decode_rr(desc);
	read_imm1(desc);
	uint32_t const lim1 = BIT(desc.opptr[1], 0, 12);
	if (!E_BIT(desc.opptr[1]))
	{
		desc.length = 4;
		desc.imm = lim1;
	}
	else
	{
		read_imm2(desc);
		desc.length = 6;
		desc.imm = (lim1 << 16) | desc.opptr[2];
	}
}


/*-------------------------------------------------
    describe - build a description of a single
    instruction
-------------------------------------------------*/

bool hyperstone_device::frontend::describe(opcode_desc &desc, const opcode_desc *prev)
{
	read_op(desc);
	uint16_t const op = desc.opptr[0];

	// most instructions are 2 bytes and a single cycle
	desc.length = 2;
	desc.delayslots = 0;

	switch (op >> 8)
	{
	case 0x00: // chk global,global
	case 0x01: // chk global,local
	case 0x02: // chk local,global
	case 0x03: // chk local,local
		decode_rr(desc);
		if (!desc.dst_is_src() || desc.src_is_sr())
		{
			desc.set_can_cause_exception();
			if (desc.dst_local || desc.src_local)
				desc.set_fp_used();
			if (!desc.dst_local)
				desc.set_g_used(desc.dst_code);
			if (!desc.src_local && (desc.src_code != SR_REGISTER))
				desc.set_g_used(desc.src_code);
		}
		else if (desc.dst_is_pc())
		{
			desc.set_will_cause_exception();
			desc.set_end_sequence();
		}
		break;
	case 0x04: // movd global,global
	case 0x05: // movd global,local
	case 0x06: // movd local,global
	case 0x07: // movd local,local
		decode_rr(desc);
		if (desc.dst_local || desc.src_local)
			desc.set_fp_used();
		if (!desc.src_local && (desc.dst_is_pc() || !desc.src_is_sr()))
		{
			desc.set_g_used(desc.src_code);
			desc.set_g_used(desc.src_code + 1);
		}
		if (!desc.dst_local)
		{
			desc.set_g_modified(desc.dst_code);
			desc.set_g_modified(desc.dst_code + 1);
		}
		desc.set_znv_modified();
		if (desc.dst_is_pc())
		{
			desc.targetpc = BRANCH_TARGET_DYNAMIC;
			desc.set_g_used(SP_REGISTER);
			desc.set_g_modified(SP_REGISTER);
			desc.set_fp_modified();
			desc.set_is_unconditional_branch();
			desc.set_can_cause_exception();
			desc.set_end_sequence();
			desc.set_can_change_modes();
			desc.set_reads_memory();
		}
		break;
	case 0x08: // divu global,global
	case 0x09: // divu global,local
	case 0x0a: // divu local,global
	case 0x0b: // divu local,local
	case 0x0c: // divs global,global
	case 0x0d: // divs global,local
	case 0x0e: // divs local,global
	case 0x0f: // divs local,local
		decode_rr(desc);
		desc.set_can_cause_exception();
		if (desc.dst_local || desc.src_local)
			desc.set_fp_used();
		if (!desc.src_local)
			desc.set_g_used(desc.src_code);
		if (!desc.dst_local)
		{
			desc.set_g_used(desc.dst_code);
			desc.set_g_used(desc.dst_code + 1);
			desc.set_g_modified(desc.dst_code);
			desc.set_g_modified(desc.dst_code + 1);
		}
		desc.set_znv_modified();
		break;
	case 0x10: // xm global,global
	case 0x11: // xm global,local
	case 0x12: // xm local,global
	case 0x13: // xm local,local
		decode_rrlim(desc);
		if (desc.imm != 0U)
			desc.set_can_cause_exception();
		if (desc.dst_local || desc.src_local)
			desc.set_fp_used();
		if (!desc.src_local)
			desc.set_g_used(desc.src_code);
		if (!desc.dst_local)
			desc.set_g_modified(desc.dst_code);
		break;
	case 0x14: // mask global,global
	case 0x15: // mask global,local
	case 0x16: // mask local,global
	case 0x17: // mask local,local
		decode_rrconst(desc);
		if (desc.dst_local || desc.src_local)
			desc.set_fp_used();
		if (!desc.src_local)
			desc.set_g_used(desc.src_code);
		if (!desc.dst_local)
			desc.set_g_modified(desc.dst_code);
		desc.set_z_modified();
		if (desc.dst_is_pc())
		{
			if (!desc.src_is_pc() || desc.pc_value_unknown())
				desc.targetpc = BRANCH_TARGET_DYNAMIC;
			else
				desc.targetpc = desc.pc_value() & desc.imm;
			desc.set_is_unconditional_branch();
			desc.set_end_sequence();
		}
		break;
	case 0x18: // sum global,global
	case 0x19: // sum global,local
	case 0x1a: // sum local,global
	case 0x1b: // sum local,local
	case 0x1c: // sums global,global
	case 0x1d: // sums global,local
	case 0x1e: // sums local,global
	case 0x1f: // sums local,local
		decode_rrconst(desc);
		if ((op & 0x0400) && !desc.src_is_sr())
			desc.set_can_cause_exception(); // sums
		if (desc.dst_local || desc.src_local)
			desc.set_fp_used();
		if (desc.src_is_sr())
			desc.set_c_used();
		else if (!desc.src_local)
			desc.set_g_used(desc.src_code);
		if (!desc.dst_local)
			desc.set_g_modified(desc.dst_code);
		if (op & 0x0400)
			desc.set_znv_modified(); // sums
		else
			desc.set_cznv_modified(); // sum
		if (desc.dst_is_pc())
		{
			if (!desc.src_is_pc() || desc.pc_value_unknown())
				desc.targetpc = BRANCH_TARGET_DYNAMIC;
			else
				desc.targetpc = (desc.pc_value() + desc.imm) & ~uint32_t(1);
			desc.set_is_unconditional_branch();
			desc.set_end_sequence();
		}
		break;
	case 0x20: // cmp global,global
	case 0x21: // cmp global,local
	case 0x22: // cmp local,global
	case 0x23: // cmp local,local
	case 0x30: // cmpb global,global
	case 0x31: // cmpb global,local
	case 0x32: // cmpb local,global
	case 0x33: // cmpb local,local
		decode_rr(desc);
		if (desc.dst_local || desc.src_local)
			desc.set_fp_used();
		if (!(op & 0x1000) && desc.src_is_sr())
			desc.set_c_used();
		else if (!desc.src_local)
			desc.set_g_used(desc.src_code);
		if (!desc.dst_local)
			desc.set_g_used(desc.dst_code);
		if (op & 0x1000)
			desc.set_z_modified(); // cmpb
		else
			desc.set_cznv_modified(); // cmp
		break;
	case 0x24: // mov global,global
	case 0x25: // mov global,local
	case 0x26: // mov local,global
	case 0x27: // mov local,local
		decode_rr(desc);
		if (desc.dst_local || desc.src_local)
			desc.set_fp_used();
		if (!desc.dst_local || !desc.src_local)
		{
			if (!prev || prev->regout[SR_REGISTER])
				desc.set_check_h();
			desc.regin.set(SR_REGISTER); // only the H bit is used
		}
		if (!desc.src_local)
		{
			desc.set_g_used(desc.src_code);
			desc.set_g_used(desc.src_code + 16);
		}
		if (!desc.dst_local)
		{
			desc.set_can_cause_exception();
			desc.set_g_used(desc.dst_code);
			desc.set_g_used(desc.dst_code + 16);
			desc.set_g_modified(desc.dst_code);
			desc.set_g_modified(desc.dst_code + 16);
			if (desc.dst_code == PC_REGISTER)
			{
				if (!desc.src_is_pc() || desc.pc_value_unknown())
					desc.targetpc = BRANCH_TARGET_DYNAMIC;
				else
					desc.targetpc = desc.pc_value();
				desc.set_is_conditional_branch(); // technically it won't branch if H is set
				desc.set_end_sequence();
			}
			else if (desc.dst_code == 5) // TPR_REGISTER & 0xf
			{
				desc.set_end_sequence();
			}
		}
		desc.set_znv_modified();
		break;
	case 0x28: // add global,global
	case 0x29: // add global,local
	case 0x2a: // add local,global
	case 0x2b: // add local,local
	case 0x2c: // adds global,global
	case 0x2d: // adds global,local
	case 0x2e: // adds local,global
	case 0x2f: // adds local,local
	case 0x48: // sub global,global
	case 0x49: // sub global,local
	case 0x4a: // sub local,global
	case 0x4b: // sub local,local
	case 0x4c: // subs global,global
	case 0x4d: // subs global,local
	case 0x4e: // subs local,global
	case 0x4f: // subs local,local
		decode_rr(desc);
		if (op & 0x0400)
			desc.set_can_cause_exception(); // adds, subs
		if (desc.dst_local || desc.src_local)
			desc.set_fp_used();
		if (desc.src_is_sr())
			desc.set_c_used();
		else if (!desc.src_local)
			desc.set_g_used(desc.src_code);
		if (!desc.dst_local)
		{
			desc.set_g_used(desc.dst_code);
			desc.set_g_modified(desc.dst_code);
			if (desc.dst_code == PC_REGISTER)
			{
				if (!desc.src_is_pc())
					desc.targetpc = BRANCH_TARGET_DYNAMIC;
				else if ((op & 0xf800) == 0x4800)
					desc.targetpc = 0; // sub, subs
				else if (!desc.pc_value_unknown())
					desc.targetpc = desc.pc_value() << 1; // add, adds
				else
					desc.targetpc = BRANCH_TARGET_DYNAMIC;
				desc.set_is_unconditional_branch();
				desc.set_end_sequence();
			}
		}
		if (op & 0x0400)
			desc.set_znv_modified(); // adds, subs
		else
			desc.set_cznv_modified(); // add, sub
		break;
	case 0x34: // andn global,global
	case 0x35: // andn global,local
	case 0x36: // andn local,global
	case 0x37: // andn local,local
	case 0x38: // or global,global
	case 0x39: // or global,local
	case 0x3a: // or local,global
	case 0x3b: // or local,local
	case 0x3c: // xor global,global
	case 0x3d: // xor global,local
	case 0x3e: // xor local,global
	case 0x3f: // xor local,local
	case 0x54: // and global,global
	case 0x55: // and global,local
	case 0x56: // and local,global
	case 0x57: // and local,local
		decode_rr(desc);
		if (desc.dst_local || desc.src_local)
			desc.set_fp_used();
		if (!desc.src_local)
			desc.set_g_used(desc.src_code);
		if (!desc.dst_local)
		{
			desc.set_g_used(desc.dst_code);
			desc.set_g_modified(desc.dst_code);
			if (desc.dst_code == PC_REGISTER)
			{
				if (!desc.src_is_pc())
					desc.targetpc = BRANCH_TARGET_DYNAMIC;
				else if ((op & 0xf400) == 0x3400)
					desc.targetpc = 0; // andn, xor
				else if (!desc.pc_value_unknown())
					desc.targetpc = desc.pc_value(); // or, and
				else
					desc.targetpc = BRANCH_TARGET_DYNAMIC;
				desc.set_is_unconditional_branch();
				desc.set_end_sequence();
			}
		}
		desc.set_z_modified();
		break;
	case 0x40: // subc global,global
	case 0x41: // subc global,local
	case 0x42: // subc local,global
	case 0x43: // subc local,local
	case 0x50: // addc global,global
	case 0x51: // addc global,local
	case 0x52: // addc local,global
	case 0x53: // addc local,local
		decode_rr(desc);
		if (desc.dst_local || desc.src_local)
			desc.set_fp_used();
		desc.set_cz_used();
		if (!desc.src_local && !desc.src_is_sr())
			desc.set_g_used(desc.src_code);
		if (!desc.dst_local)
		{
			desc.set_g_used(desc.dst_code);
			desc.set_g_modified(desc.dst_code);
			if (desc.dst_code == PC_REGISTER)
			{
				if (!desc.src_is_sr() || desc.pc_value_unknown() || ((op & 0xfc00) == 0x4000))
					desc.targetpc = BRANCH_TARGET_DYNAMIC;
				else
					desc.targetpc = desc.pc_value();
				desc.set_is_unconditional_branch();
				desc.set_end_sequence();
			}
		}
		desc.set_cznv_modified();
		break;
	case 0x44: // not global,global
	case 0x45: // not global,local
	case 0x46: // not local,global
	case 0x47: // not local,local
	case 0x58: // neg global,global
	case 0x59: // neg global,local
	case 0x5a: // neg local,global
	case 0x5b: // neg local,local
	case 0x5c: // negs global,global
	case 0x5d: // negs global,local
	case 0x5e: // negs local,global
	case 0x5f: // negs local,local
		decode_rr(desc);
		if (((op & 0xfc00) == 0x5c00) && !desc.src_is_sr())
			desc.set_can_cause_exception(); // negs
		if (desc.dst_local || desc.src_local)
			desc.set_fp_used();
		if (((op & 0xf800) == 0x5800) & desc.src_is_sr())
			desc.set_c_used();
		else if (!desc.src_local)
			desc.set_g_used(desc.src_code);
		if (!desc.dst_local)
		{
			desc.set_g_modified(desc.dst_code);
			if (desc.dst_code == PC_REGISTER)
			{
				if (!desc.src_is_pc() || desc.pc_value_unknown())
					desc.targetpc = BRANCH_TARGET_DYNAMIC;
				else if ((op & 0xfc00) == 0x4400)
					desc.targetpc = ~desc.pc_value() & ~uint32_t(1); // not
				else
					desc.targetpc = uint32_t(-int32_t(desc.pc_value())) & ~uint32_t(1); // neg, negs
				desc.set_is_unconditional_branch();
				desc.set_end_sequence();
			}
		}
		if (!(op & 0x1000))
			desc.set_z_modified(); // not
		else if (!(op & 0x0400))
			desc.set_cznv_modified(); // neg
		else
			desc.set_znv_modified(); // negs
		break;
	case 0x60: // cmpi global
	case 0x61: // cmpi global
	case 0x62: // cmpi local
	case 0x63: // cmpi local
	case 0x70: // cmpbi global
	case 0x71: // cmpbi global
	case 0x72: // cmpbi local
	case 0x73: // cmpbi local
		decode_rimm(desc, op & 0x1000);
		if (desc.dst_local)
			desc.set_fp_used();
		else
			desc.set_g_used(desc.dst_code);
		if (op & 0x1000)
			desc.set_z_modified(); // cmpbi
		else
			desc.set_cznv_modified(); // cmpi
		break;
	case 0x64: // movi global
	case 0x65: // movi global
		decode_rimm(desc, false);
		desc.set_can_cause_exception();
		if (!prev || prev->regout[SR_REGISTER])
			desc.set_check_h();
		desc.set_g_used(desc.dst_code);
		desc.set_g_used(desc.dst_code + 16);
		desc.regin.set(SR_REGISTER); // only the H bit is used
		desc.set_g_modified(desc.dst_code);
		desc.set_g_modified(desc.dst_code + 16);
		if (desc.dst_code == PC_REGISTER)
		{
			desc.targetpc = desc.imm & ~uint32_t(1);
			desc.set_is_unconditional_branch();
			desc.set_end_sequence();
		}
		else if (desc.dst_code == 5) // TPR_REGISTER & 0xf
		{
			desc.set_end_sequence();
		}
		break;
	case 0x66: // movi local
	case 0x67: // movi local
		decode_rimm(desc, false);
		desc.set_fp_used();
		desc.set_znv_modified();
		break;
	case 0x68: // addi global
	case 0x69: // addi global
	case 0x6a: // addi local
	case 0x6b: // addi local
	case 0x6c: // addsi global
	case 0x6d: // addsi global
	case 0x6e: // addsi local
	case 0x6f: // addsi local
		decode_rimm(desc, false);
		if (op & 0x0400)
			desc.set_can_cause_exception(); // addsi
		if (desc.dst_local)
			desc.set_fp_used();
		if (!(op & 0x010f))
			desc.set_cz_used();
		if (!desc.dst_local)
		{
			desc.set_g_used(desc.dst_code);
			desc.set_g_modified(desc.dst_code);
			if (desc.dst_code == PC_REGISTER)
			{
				if (desc.pc_value_unknown())
					desc.targetpc = BRANCH_TARGET_DYNAMIC;
				else
					desc.targetpc = (desc.pc_value() + desc.imm) & ~uint32_t(1);
				desc.set_is_unconditional_branch();
				desc.set_end_sequence();
			}
		}
		if (op & 0x0400)
			desc.set_znv_modified(); // addsi
		else
			desc.set_cznv_modified(); // addi
		break;
	case 0x74: // andni global
	case 0x75: // andni global
	case 0x78: // ori global
	case 0x79: // ori global
	case 0x7c: // xori global
	case 0x7d: // xori global
		decode_rimm(desc, !(op & 0x0800));
		desc.set_g_used(desc.dst_code);
		desc.set_g_modified(desc.dst_code);
		if (desc.dst_code == PC_REGISTER)
		{
			if (desc.pc_value_unknown())
				desc.targetpc = BRANCH_TARGET_DYNAMIC;
			else if (!(op & 0x0800))
				desc.targetpc = desc.pc_value() & ~desc.imm; // andni
			else if (!(op & 0x0400))
				desc.targetpc = (desc.pc_value() | desc.imm) & ~uint32_t(1); // ori
			else
				desc.targetpc = (desc.pc_value() ^ desc.imm) & ~uint32_t(1); // xori
			desc.set_is_unconditional_branch();
			desc.set_end_sequence();
		}
		desc.set_z_modified();
		break;
	case 0x76: // andni local
	case 0x77: // andni local
	case 0x7a: // ori local
	case 0x7b: // ori local
	case 0x7e: // xori local
	case 0x7f: // xori local
		decode_rimm(desc, !(op & 0x0800));
		desc.set_fp_used();
		desc.set_z_modified();
		break;
	case 0x80: case 0x81: // shrdi
	case 0x84: case 0x85: // sardi
	case 0x88: case 0x89: // shldi
		decode_ln(desc);
		desc.set_fp_used();
		if (op & 0x0800)
			desc.set_cznv_modified(); // shldi
		else
			desc.set_czn_modified(); // shrdi, sardi
		break;
	case 0x82: // shrd
	case 0x83: // shr
	case 0x86: // sard
	case 0x87: // sar
	case 0x8a: // shld
	case 0x8b: // shl
	case 0x8f: // rol
		decode_ll(desc);
		desc.set_fp_used();
		if (op & 0x0800)
			desc.set_cznv_modified(); // shld, shl, rol
		else
			desc.set_czn_modified(); // shrd, shr, sard, sar
		break;
	case 0x8c: case 0x8d: // reserved
		return false;
	case 0x8e: // testlz
		decode_ll(desc);
		desc.set_fp_used();
		break;
	case 0x90: // ldxx.d/a global,global
	case 0x91: // ldxx.d/a global,local
	case 0x92: // ldxx.d/a local,global
	case 0x93: // ldxx.d/a local,local
	case 0x94: // ldxx.n/s global,global
	case 0x95: // ldxx.n/s global,local
	case 0x96: // ldxx.n/s local,global
	case 0x97: // ldxx.n/s local,local
		decode_rrdis(desc);
		if (desc.dst_local || desc.src_local)
			desc.set_fp_used();
		if (!desc.dst_local && !desc.dst_is_sr())
			desc.set_g_used(desc.dst_code);
		if ((op & 0x0400) && !desc.dst_local)
			desc.set_g_modified(desc.dst_code);
		if (!desc.src_local)
		{
			desc.set_g_modified(desc.src_code);
			if ((desc.opptr[1] & 0x3000) == 0x3000)
			{
				if ((desc.imm & ((op & 0x0400) ? 0x3 : 0x1)) == 0x1)
					desc.set_g_modified(desc.src_code + 1);
			}
		}
		desc.set_reads_memory();
		break;
	case 0x98: // stxx.d/a global,global
	case 0x99: // stxx.d/a global,local
	case 0x9a: // stxx.d/a local,global
	case 0x9b: // stxx.d/a local,local
	case 0x9c: // stxx.n/s global,global
	case 0x9d: // stxx.n/s global,local
	case 0x9e: // stxx.n/s local,global
	case 0x9f: // stxx.n/s local,local
		decode_rrdis(desc);
		if (((desc.opptr[1] & 0x3000) == 0x0000) || (((desc.opptr[1] & 0x3000) == 0x0000) && (desc.imm & 0x1)))
			desc.set_can_cause_exception();
		if (desc.dst_local || desc.src_local)
			desc.set_fp_used();
		if (!desc.dst_local && !desc.dst_is_sr())
			desc.set_g_used(desc.dst_code);
		if (!desc.src_local)
		{
			desc.set_g_used(desc.src_code);
			if ((desc.opptr[1] & 0x3000) == 0x3000)
			{
				if ((desc.imm & ((op & 0x0400) ? 0x3 : 0x1)) == 0x1)
					desc.set_g_used(desc.src_code + 1);
			}
		}
		if ((op & 0x0400) && !desc.dst_local)
			desc.set_g_modified(desc.dst_code);
		desc.set_writes_memory();
		break;
	case 0xa0: // shri global
	case 0xa1: // shri global
	case 0xa2: // shri local
	case 0xa3: // shri local
	case 0xa4: // sari global
	case 0xa5: // sari global
	case 0xa6: // sari local
	case 0xa7: // sari local
	case 0xa8: // shli global
	case 0xa9: // shli global
	case 0xaa: // shli local
	case 0xab: // shli local
		decode_rn(desc);
		if (desc.dst_local)
			desc.set_fp_used();
		if (!desc.dst_local)
		{
			desc.set_g_used(desc.dst_code);
			desc.set_g_modified(desc.dst_code);
		}
		if (op & 0x0800)
			desc.set_cznv_modified(); // shli
		else
			desc.set_czn_modified(); // shri, sari
		break;
	case 0xac: case 0xad: case 0xae: case 0xaf: // reserved
		return false;
	case 0xb0: // mulu global,global
	case 0xb1: // mulu global,local
	case 0xb2: // mulu local,global
	case 0xb3: // mulu local,local
	case 0xb4: // muls global,global
	case 0xb5: // muls global,local
	case 0xb6: // muls local,global
	case 0xb7: // muls local,local
	case 0xbc: // mul global,global
	case 0xbd: // mul global,local
	case 0xbe: // mul local,global
	case 0xbf: // mul local,local
		decode_rr(desc);
		if (desc.dst_local || desc.src_local)
			desc.set_fp_used();
		if (!desc.src_local)
			desc.set_g_used(desc.src_code);
		if (!desc.dst_local)
		{
			desc.set_g_used(desc.dst_code);
			desc.set_g_modified(desc.dst_code);
			if (!(op & 0x0800))
				desc.set_g_modified(desc.dst_code + 1); // mulu, muls
		}
		desc.set_cznv_modified();
		break;
	case 0xb8: // set global
	case 0xb9: // set global
	case 0xba: // set local
	case 0xbb: // set local
		decode_rn(desc);
		if (desc.dst_local)
			desc.set_fp_used();
		switch (desc.imm)
		{
		case 4: case 5: case 20: case 21: // le, gt, lem, gtm
			desc.set_z_used();
			desc.set_n_used();
			break;
		case 6: case 7: case 22: case 23: // lt/n, ge/nn, ltm/nm, gem/nnm
			desc.set_n_used();
			break;
		case 8: case 9: case 24: case 25: // se, ht, sem, htm
			desc.set_cz_used();
			break;
		case 10: case 11: case 26: case 27: // st/c, he/nc, stm/cm, hem/ncm
			desc.set_c_used();
			break;
		case 12: case 13: case 28: case 29: // e/z, ne/nz, em/zm, nem/nzm
			desc.set_z_used();
			break;
		case 14: case 15: case 30: case 31: // v, nv, vm, nvm
			desc.set_v_used();
			break;
		}
		if (!desc.dst_local)
			desc.set_g_modified(desc.dst_code);
		break;
	case 0xc0: case 0xc1: case 0xc2: case 0xc3: // software
	case 0xc4: case 0xc5: case 0xc6: case 0xc7: // software
	case 0xc8: case 0xc9: case 0xca: case 0xcb: // software
	case 0xcc: case 0xcd: // software
	case 0xcf: // do
		decode_ll(desc);
		desc.set_fp_used();
		desc.set_g_used(PC_REGISTER);
		desc.set_g_used(SR_REGISTER);
		desc.targetpc = BRANCH_TARGET_DYNAMIC;
		desc.set_is_unconditional_branch();
		desc.set_end_sequence();
		desc.set_can_change_modes();
		break;
	case 0xce: // extend
		decode_llext(desc);
		desc.set_fp_used();
		desc.set_g_used(14);
		desc.set_g_used(15);
		desc.set_g_modified(14);
		desc.set_g_modified(15);
		break;
	case 0xd0: // ldwr global
	case 0xd1: // ldwr local
	case 0xd2: // lddr global
	case 0xd3: // lddr local
	case 0xd4: // ldwp global
	case 0xd5: // ldwp local
	case 0xd6: // lddp global
	case 0xd7: // lddp local
		decode_lr(desc);
		desc.set_fp_used();
		if (!desc.src_local)
		{
			desc.set_g_modified(desc.src_code);
			if (op & 0x0200)
				desc.set_g_modified(desc.src_code + 1);
		}
		desc.set_reads_memory();
		break;
	case 0xd8: // stwr global
	case 0xd9: // stwr local
	case 0xda: // stdr global
	case 0xdb: // stdr local
	case 0xdc: // stwp global
	case 0xdd: // stwp local
	case 0xde: // stdp global
	case 0xdf: // stdp local
		decode_lr(desc);
		desc.set_fp_used();
		if (!desc.src_local && !desc.src_is_sr())
		{
			desc.set_g_used(desc.src_code);
			if (op & 0x0200)
				desc.set_g_used(desc.src_code + 1);
		}
		desc.set_writes_memory();
		break;
	case 0xe0: case 0xe1: case 0xe2: case 0xe3: // dbv, dbnv, dbe, dbne
	case 0xe4: case 0xe5: case 0xe6: case 0xe7: // dbc, dbnc, dbse, dbht
	case 0xe8: case 0xe9: case 0xea: case 0xeb: // dbn, dbnn, dble, dbgt
	case 0xec: // dbr
	case 0xf0: case 0xf1: case 0xf2: case 0xf3: // bv, bnv, be, bne
	case 0xf4: case 0xf5: case 0xf6: case 0xf7: // bc, bnc, bse, bht
	case 0xf8: case 0xf9: case 0xfa: case 0xfb: // bn, bnn, ble, bgt
	case 0xfc: // br
		decode_pcrel(desc);
		switch (op & 0x0f00)
		{
		case 0x0000: case 0x0100: // bv, bnv
			desc.set_v_used();
			break;
		case 0x0200: case 0x0300: // be/bz, bne/bnz
			desc.set_z_used();
			break;
		case 0x0400: case 0x0500: // bst/bc, bhe/bnc
			desc.set_c_used();
			break;
		case 0x0600: case 0x0700: // bse, bht
			desc.set_cz_used();
			break;
		case 0x0800: case 0x0900: // blt/bn, bge/bnn
			desc.set_n_used();
			break;
		case 0x0a00: case 0x0b00: // ble, bgt
			desc.set_z_used();
			desc.set_n_used();
			break;
		case 0x0c00: // br
			desc.set_is_unconditional_branch();
			desc.set_end_sequence();
			break;
		}
		if ((op & 0x0f00) != 0x0c00)
			desc.set_is_conditional_branch();
		desc.delayslots = (op & 0x1000) ? 0 : 1;
		if (desc.pc_value_unknown())
			desc.targetpc = BRANCH_TARGET_DYNAMIC;
		else
			desc.targetpc = desc.pc_value() + desc.imm;
		break;
	case 0xed: // frame
		desc.set_can_cause_exception();
		desc.set_fp_used();
		desc.regin.set(SR_REGISTER); // uses FL
		desc.set_g_used(SP_REGISTER);
		desc.set_g_used(UB_REGISTER);
		desc.set_g_modified(SP_REGISTER);
		desc.set_fp_modified();
		desc.set_writes_memory();
		break;
	case 0xee: // call global
	case 0xef: // call local
		decode_lrconst(desc);
		if (!desc.src_local && !desc.src_is_sr())
			desc.set_g_used(desc.src_code);
		desc.set_g_used(PC_REGISTER);
		desc.set_g_used(SR_REGISTER);
		if (desc.src_is_sr())
			desc.targetpc = desc.imm & ~uint32_t(1);
		else if (desc.src_is_pc() && !desc.pc_value_unknown())
			desc.targetpc = (desc.pc_value() + desc.imm) & ~uint32_t(1);
		else
			desc.targetpc = BRANCH_TARGET_DYNAMIC;
		desc.set_is_unconditional_branch();
		desc.set_end_sequence();
		break;
	case 0xfd: case 0xfe: case 0xff: // trap
		switch (bitswap<4>(op, 9, 8, 1, 0))
		{
		case 4: case 5: // traple, trapgt
			desc.set_z_used();
			desc.set_n_used();
			break;
		case 6: case 7: // traplt, trapge
			desc.set_n_used();
			break;
		case 8: case 9: // trapse, trapht
			desc.set_cz_used();
			break;
		case 10: case 11: // trapst, traphe
			desc.set_c_used();
			break;
		case 12: case 13: // trape, trapne
			desc.set_z_used();
			break;
		case 14: // trapv
			desc.set_z_used();
			break;
		case 15: // trap
			desc.set_will_cause_exception();
			desc.set_end_sequence();
			break;
		}
		if (bitswap<4>(op, 9, 8, 1, 0) != 15)
			desc.set_can_cause_exception();
		break;
	}
	return true;
}
