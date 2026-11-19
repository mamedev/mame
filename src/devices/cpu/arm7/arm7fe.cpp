// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*
    ARM dynamic recompiler frontend
    Emulation by R. Belmont
*/

#include "emu.h"
#include "arm7fe.h"

#include "arm7core.h"

#include "cpu/drcfe.ipp"

#include <bit>

namespace {

// the condition flags each condition reads (opcode_desc::NZCV_*): EQ NE CS CC MI PL VS VC HI LS GE LT GT LE AL NV
constexpr uint8_t COND_FLAGS[16] = { 4, 4, 2, 2, 8, 8, 1, 1, 6, 6, 9, 9, 13, 13, 0, 0 };

} // anonymous namespace


arm7_cpu_device::frontend::frontend(arm7_cpu_device &arm, uint32_t window_start, uint32_t window_end, uint32_t max_sequence)
	: drc_frontend_base(arm.space_config(AS_PROGRAM)->page_shift(), window_start, window_end, max_sequence)
	, m_arm(arm)
{
}

arm7_cpu_device::frontend::~frontend()
{
}

const arm7_cpu_device::opcode_desc *arm7_cpu_device::frontend::describe_code(offs_t startpc)
{
	const opcode_desc *const desclist = do_describe_code(
			[this] (opcode_desc &desc, const opcode_desc *prev) { return describe(desc, prev); },
			startpc);
	compute_flag_liveness(desclist);
	return desclist;
}


// a written flag is live if a later instruction reads it; branches, traps and the interpreter read them all
void arm7_cpu_device::frontend::compute_flag_liveness(const opcode_desc *desclist)
{
	if (!(m_arm.m_drc_opt & DRC_OPT_FLAGS))
	{
		return;     // every flag written stays live (reset())
	}

	std::vector<opcode_desc *> sequence;
	for (const opcode_desc *seqhead = desclist; seqhead != nullptr; seqhead = sequence.back()->next())
	{
		sequence.clear();
		for (const opcode_desc *desc = seqhead; ; desc = desc->next())
		{
			sequence.push_back(const_cast<opcode_desc *>(desc));
			if (desc->end_sequence() || desc->next() == nullptr)
			{
				break;
			}
		}

		uint8_t live = opcode_desc::NZCV_ALL;
		for (auto it = sequence.rbegin(); it != sequence.rend(); ++it)
		{
			opcode_desc &desc = **it;
			desc.flags_live = live & desc.flags_written;
			const uint8_t killed = (desc.length == 4 && (desc.opptr >> INSN_COND_SHIFT) < COND_AL) ? 0 : desc.flags_written;
			live = (live & ~killed) | desc.flags_used;
			if (desc.is_branch() || desc.reads_memory() || desc.writes_memory() || desc.can_cause_exception() || desc.validate_tlb()
				|| desc.compiler_page_fault() || !m_arm.drc_generates(desc))
			{
				live = opcode_desc::NZCV_ALL;
			}
		}
	}
}

bool arm7_cpu_device::frontend::describe(opcode_desc &desc, const opcode_desc *prev)
{
	const bool thumb = T_IS_SET(m_arm.m_r[eCPSR]);

	// the fetch address as the interpreter forms it from R15
	desc.physpc = desc.pc & (thumb ? ~uint32_t(1) : ~uint32_t(3));
	desc.length = thumb ? 2 : 4;
	if (m_arm.m_control & COPRO_CTRL_MMU_EN)
	{
		address_space *tspace;
		if (!m_arm.memory_translate(AS_PROGRAM, device_memory_interface::TR_FETCH, desc.physpc, tspace))
		{
			// not fetchable now: the block takes the prefetch abort if it still isn't when it runs
			desc.set_validate_tlb();
			desc.set_can_cause_exception();
			desc.set_compiler_page_fault();
			desc.set_virtual_noop();
			desc.set_end_sequence();
			return true;
		}
	}

	if (thumb)
	{
		const uint32_t word = m_arm.insn_fetch_word(desc.physpc & ~uint32_t(3));
		const uint16_t op = uint16_t(word >> ((desc.physpc & 2) ? m_arm.m_prefetch_word1_shift : m_arm.m_prefetch_word0_shift));
		desc.opptr = op;
		desc.opword = word;
		desc.cycles = m_arm.thumb_insn_cycles();
		describe_thumb(desc, op, prev);
	}
	else
	{
		const uint32_t op = m_arm.insn_fetch_word(desc.physpc);
		desc.opptr = op;
		desc.opword = op;
		desc.cycles = m_arm.arm_insn_cycles(op);
		describe_arm(desc, op);
	}

	// the interpreter can leave the PC and the mode anywhere
	if (!m_arm.drc_generates(desc))
	{
		desc.set_can_change_modes();
		desc.set_end_sequence();
	}
	return true;
}

void arm7_cpu_device::frontend::describe_branch(opcode_desc &desc, bool conditional, offs_t target)
{
	desc.targetpc = target;
	if (conditional)
	{
		desc.set_is_conditional_branch();
	}
	else
	{
		desc.set_is_unconditional_branch();
		desc.set_end_sequence();
	}
}

void arm7_cpu_device::frontend::describe_pc_write(opcode_desc &desc, bool conditional, bool changes_modes)
{
	desc.targetpc = BRANCH_TARGET_DYNAMIC;
	if (conditional)
	{
		desc.set_is_conditional_branch();
	}
	else
	{
		desc.set_is_unconditional_branch();
		desc.set_end_sequence();
	}
	if (changes_modes)
	{
		desc.set_can_change_modes();
		desc.set_cpsr_modified();
	}
}

void arm7_cpu_device::frontend::describe_undefined(opcode_desc &desc)
{
	desc.set_will_cause_exception();
	desc.set_can_change_modes();
	desc.set_end_sequence();
}

void arm7_cpu_device::frontend::describe_arm(opcode_desc &desc, uint32_t op)
{
	const uint32_t cond = op >> INSN_COND_SHIFT;
	if (cond == COND_NV)
	{
		if (m_arm.m_archRev >= 5)
		{
			describe_arm_unconditional(desc, op);
			return;
		}
		return;     // never executed before ARMv5
	}

	const bool conditional = (cond != COND_AL);
	if (conditional)
	{
		desc.set_flags_used(COND_FLAGS[cond]);
	}

	switch ((op >> 24) & 0xf)
	{
		case 0x0: case 0x1: case 0x2: case 0x3:
			describe_arm_misc(desc, op);
			return;

		case 0x4: case 0x5: case 0x6: case 0x7:
			describe_arm_mem_single(desc, op);
			return;

		case 0x8: case 0x9:
			describe_arm_mem_block(desc, op);
			return;

		case 0xa: case 0xb:
			if (op & INSN_BL)
			{
				desc.set_r_modified(14);
			}
			describe_branch(desc, conditional, (desc.pc + 8 + util::sext(op << 2, 26)) & pc_mask());
			return;

		case 0xc: case 0xd: case 0xe:
			describe_coprocessor(desc, op);
			return;

		case 0xf:
		default:
			// SWI
			desc.set_will_cause_exception();
			desc.set_can_change_modes();
			desc.set_end_sequence();
			return;
	}
}

void arm7_cpu_device::frontend::describe_arm_unconditional(opcode_desc &desc, uint32_t op)
{
	switch ((op >> 24) & 0xf)
	{
		case 0x1:
			// CPS, SETEND (ARMv6)
			if (m_arm.m_archRev >= 6 && ((op & 0x00f10020) == 0x00000000 || (op & 0x00ff00f0) == 0x00010000))
			{
				desc.set_cpsr_used();
				desc.set_cpsr_modified();
				desc.set_can_change_modes();
				desc.set_end_sequence();
				return;
			}
			break;

		case 0x5: case 0x7:
			// CLREX (ARMv6K), and PLD (a no-op without a cache model)
			if (op == 0xf57ff01f && (m_arm.m_archFlags & ARCHFLAG_K))
			{
				return;
			}
			if ((op & 0x0070f000) == 0x0050f000)
			{
				desc.set_r_used((op >> 16) & 0xf);
				if (op & INSN_I)
				{
					desc.set_r_used(op & 0xf);
				}
				return;
			}
			break;

		case 0x8: case 0x9:
			// SRS, RFE (ARMv6)
			if (m_arm.m_archRev >= 6)
			{
				const bool srs = (op & 0x005fffe0) == 0x004d0500;
				const bool rfe = (op & 0x0050ffff) == 0x00100a00;
				if (srs || rfe)
				{
					desc.set_cpsr_used();
					desc.set_r_used((op >> 16) & 0xf);
					desc.set_can_cause_exception();
					if (BIT(op, 21))
					{
						desc.set_r_modified((op >> 16) & 0xf);
					}
					if (srs)
					{
						desc.set_r_used(14);
						desc.set_writes_memory();
					}
					else
					{
						desc.set_reads_memory();
						describe_pc_write(desc, false, true);
					}
					desc.set_can_change_modes();
					desc.set_end_sequence();
					return;
				}
			}
			break;

		case 0xa: case 0xb:
			// BLX to Thumb: the target is in the other instruction set, so it isn't followed
			desc.set_r_modified(14);
			describe_pc_write(desc, false, true);
			return;

		case 0xc:
			// MCRR2/MRRC2 for a device that implements them (ARM1176); undefined otherwise
			describe_coprocessor(desc, op);
			return;

		default:
			break;
	}

	describe_undefined(desc);
}

// groups 0-3: multiplies, swap, halfword transfers, the miscellaneous space, PSR transfers and data processing
void arm7_cpu_device::frontend::describe_arm_misc(opcode_desc &desc, uint32_t op)
{
	const bool conditional = (op >> INSN_COND_SHIFT) != COND_AL;
	const uint32_t rm = op & 0xf;
	const uint32_t rs = (op >> 8) & 0xf;
	const uint32_t rd = (op >> 12) & 0xf;
	const uint32_t rn = (op >> 16) & 0xf;

	if ((op & 0x0ff000f0) == 0x00400090)
	{
		// UMAAL
		if (m_arm.m_archRev < 6)
		{
			describe_undefined(desc);
			return;
		}
		desc.set_r_used(rm);
		desc.set_r_used(rs);
		desc.set_r_used(rd);
		desc.set_r_used(rn);
		desc.set_r_modified(rd);
		desc.set_r_modified(rn);
		return;
	}

	if ((op & 0x0f800ff0) == 0x01800f90)
	{
		// the exclusive loads and stores
		if (m_arm.m_archRev < 6)
		{
			describe_undefined(desc);
			return;
		}
		const bool load = BIT(op, 20);
		const bool dword = ((op >> 21) & 3) == 1;
		desc.set_r_used(rn);
		desc.set_r_modified(rd);
		if (load)
		{
			desc.set_reads_memory();
			if (dword)
			{
				desc.set_r_modified((rd + 1) & 0xf);
			}
		}
		else
		{
			desc.set_writes_memory();
			desc.set_r_used(rm);
			if (dword)
			{
				desc.set_r_used((rm + 1) & 0xf);
			}
		}
		desc.set_can_cause_exception();
		return;
	}

	if (m_arm.m_archRev >= 3 && (op & 0x0ffffff0) == 0x012fff10)
	{
		// BX
		desc.set_r_used(rm);
		describe_pc_write(desc, conditional, true);
		return;
	}

	if (m_arm.m_archRev >= 5)
	{
		if ((op & 0x0ff000f0) == 0x01200030)
		{
			// BLX Rm
			desc.set_r_used(rm);
			desc.set_r_modified(14);
			describe_pc_write(desc, conditional, true);
			return;
		}
		if ((op & 0x0ff000f0) == 0x01200070)
		{
			// BKPT: a prefetch abort
			desc.set_will_cause_exception();
			desc.set_can_change_modes();
			desc.set_end_sequence();
			return;
		}
		if ((op & 0x0ff000f0) == 0x01600010)
		{
			// CLZ
			desc.set_r_used(rm);
			desc.set_r_modified(rd);
			return;
		}
		if ((op & 0x0f900090) == 0x01000080 || (op & 0x0f9000f0) == 0x01000050)
		{
			// DSP extension: halfword multiplies and saturating adds and subtracts
			if (!(m_arm.m_archFlags & ARCHFLAG_E))
			{
				describe_undefined(desc);
				return;
			}
			desc.set_r_used(rm);
			if ((op & 0x0f9000f0) == 0x01000050)
			{
				// QADD, QSUB, QDADD, QDSUB: Rd = Rm op Rn
				desc.set_r_used(rn);
				desc.set_r_modified(rd);
			}
			else
			{
				// SMLAxy, SMLAWy, SMULxy, SMULWy, SMLALxy: Rd is in the Rn field, the accumulator in the Rd field
				desc.set_r_used(rs);
				if ((op & 0x0ff00090) == 0x01400080)
				{
					desc.set_r_used(rd);
					desc.set_r_used(rn);
					desc.set_r_modified(rd);
				}
				else if ((op & 0x0ff00090) == 0x01000080 || (op & 0x0ff000b0) == 0x01200080)
				{
					desc.set_r_used(rd);
				}
				desc.set_r_modified(rn);
			}
			desc.set_cpsr_used();
			desc.set_cpsr_modified();        // Q
			return;
		}
	}

	if ((op & 0x0e000000) == 0 && (op & 0x90) == 0x90)
	{
		// the multiply hole: halfword transfers, swap, the multiplies
		if (op & 0x60)
		{
			if (m_arm.m_archRev < 3)
			{
				describe_undefined(desc);
				return;
			}
			describe_arm_mem_half(desc, op);
			return;
		}
		if (op & 0x01000000)
		{
			// SWP, SWPB
			if (m_arm.m_archRev < 3 && !(m_arm.m_archFlags & ARCHFLAG_V2A))
			{
				describe_undefined(desc);
				return;
			}
			desc.set_r_used(rn);
			desc.set_r_used(rm);
			desc.set_r_modified(rd);
			desc.set_reads_memory();
			desc.set_writes_memory();
			desc.set_can_cause_exception();
			return;
		}
		if (op & 0x00800000)
		{
			// UMULL, UMLAL, SMULL, SMLAL
			if (m_arm.m_archRev < 3)
			{
				describe_undefined(desc);
				return;
			}
			desc.set_r_used(rm);
			desc.set_r_used(rs);
			if (op & INSN_MUL_A)
			{
				desc.set_r_used(rd);
				desc.set_r_used(rn);
			}
			desc.set_r_modified(rd);
			desc.set_r_modified(rn);
			if (op & INSN_S)
			{
				desc.set_flags_modified(opcode_desc::NZCV_N | opcode_desc::NZCV_Z);
			}
			return;
		}
		// MUL, MLA
		if (m_arm.m_archRev < 2)
		{
			describe_undefined(desc);
			return;
		}
		desc.set_r_used(rm);
		desc.set_r_used(rs);
		if (op & INSN_MUL_A)
		{
			desc.set_r_used(rd);
		}
		desc.set_r_modified(rn);
		if (op & INSN_S)
		{
			desc.set_flags_modified(opcode_desc::NZCV_N | opcode_desc::NZCV_Z);
		}
		return;
	}

	if (m_arm.m_archRev >= 3 && !(op & 0x00100000) && (op & 0x01800000) == 0x01000000)
	{
		// MRS, MSR; the rest of the miscellaneous space is undefined
		if (!(op & INSN_I) && (op & 0xf0))
		{
			describe_undefined(desc);
			return;
		}
		if (op & 0x00200000)
		{
			if (!(op & INSN_I))
			{
				desc.set_r_used(rm);
			}
			desc.set_flags_used();
			desc.set_cpsr_used();
			if (op & 0x00080000)
			{
				desc.set_flags_modified();
			}
			if (op & 0x00070000)
			{
				desc.set_cpsr_modified();
				if (!(op & 0x00400000))
				{
					// the control field of the CPSR: mode, I, F
					desc.set_can_change_modes();
					desc.set_end_sequence();
				}
			}
		}
		else
		{
			desc.set_flags_used();
			desc.set_cpsr_used();
			desc.set_r_modified(rd);
		}
		return;
	}

	describe_arm_alu(desc, op);
}

void arm7_cpu_device::frontend::describe_arm_alu(opcode_desc &desc, uint32_t op)
{
	const bool conditional = (op >> INSN_COND_SHIFT) != COND_AL;
	const uint32_t opcode = (op & INSN_OPCODE) >> INSN_OPCODE_SHIFT;
	const uint32_t rd = (op & INSN_RD) >> INSN_RD_SHIFT;
	const bool compare = (opcode & 0xc) == 0x8;      // TST, TEQ, CMP, CMN
	const bool sets_flags = (op & INSN_S) != 0;

	if ((opcode & 0xd) != 0xd)
	{
		desc.set_r_used((op & INSN_RN) >> INSN_RN_SHIFT);       // everything but MOV and MVN
	}
	if (!(op & INSN_I))
	{
		desc.set_r_used(op & INSN_OP2_RM);
		if (op & 0x10)
		{
			desc.set_r_used((op >> 8) & 0xf);
		}

		// in the 26-bit modes R15 as Rm reads with the PSR
		if ((op & INSN_OP2_RM) == eR15 && (m_arm.m_core->mode & 1))
		{
			desc.set_flags_used();
			desc.set_cpsr_used();
		}
	}
	// logical ops take C from the shifter, except for an unrotated immediate or LSL #0; a register amount can be 0
	const bool logical = alu_is_logical(opcode);
	uint8_t shifter_c = 0;
	bool shifter_c_may = false;
	if (op & INSN_I)
	{
		shifter_c = ((op >> 8) & 0xf) ? opcode_desc::NZCV_C : 0;
	}
	else if (op & 0x10)
	{
		shifter_c = opcode_desc::NZCV_C;
		shifter_c_may = true;
	}
	else if (((op >> 5) & 3) == 3 && !((op >> 7) & 0x1f))
	{
		shifter_c = opcode_desc::NZCV_C;        // RRX reads C as well
		desc.set_flags_used(opcode_desc::NZCV_C);
	}
	else if (((op >> 5) & 3) != 0 || ((op >> 7) & 0x1f) != 0)
	{
		shifter_c = opcode_desc::NZCV_C;
	}
	if (opcode == OPCODE_ADC || opcode == OPCODE_SBC || opcode == OPCODE_RSC)
	{
		desc.set_flags_used(opcode_desc::NZCV_C);
	}
	if (sets_flags)
	{
		if (logical)
		{
			desc.set_flags_modified(opcode_desc::NZCV_N | opcode_desc::NZCV_Z | shifter_c);
			if (shifter_c_may)
			{
				desc.set_flags_used(opcode_desc::NZCV_C);
			}
		}
		else
		{
			desc.set_flags_modified();
		}
	}

	if (!compare)
	{
		if (rd == eR15)
		{
			// a PC write; with S the SPSR is copied to the CPSR
			describe_pc_write(desc, conditional, sets_flags);
		}
		else
		{
			desc.set_r_modified(rd);
		}
	}
	else if (rd == eR15 && sets_flags)
	{
		// TEQP and friends: the SPSR goes to the CPSR in the privileged modes
		desc.set_cpsr_used();
		desc.set_cpsr_modified();
		desc.set_can_change_modes();
		desc.set_end_sequence();
	}
}

void arm7_cpu_device::frontend::describe_arm_mem_single(opcode_desc &desc, uint32_t op)
{
	const bool conditional = (op >> INSN_COND_SHIFT) != COND_AL;
	const uint32_t rn = (op & INSN_RN) >> INSN_RN_SHIFT;
	const uint32_t rd = (op & INSN_RD) >> INSN_RD_SHIFT;

	if ((op & INSN_I) && (op & 0x10))
	{
		// the media instructions
		if (m_arm.m_archRev < 6)
		{
			describe_undefined(desc);
			return;
		}
		desc.set_r_used(op & 0xf);
		desc.set_r_used((op >> 8) & 0xf);
		desc.set_r_used(rd);
		desc.set_r_used(rn);
		desc.set_r_modified(rd);
		desc.set_r_modified(rn);
		desc.set_cpsr_used();
		desc.set_cpsr_modified();        // GE, Q
		desc.set_can_cause_exception();  // the undefined forms
		return;
	}

	desc.set_r_used(rn);
	if (op & INSN_I)
	{
		desc.set_r_used(op & 0xf);
		if ((op & 0xff0) == 0x060)
		{
			desc.set_flags_used(opcode_desc::NZCV_C);       // an RRX offset
		}
	}
	if (op & INSN_SDT_L)
	{
		desc.set_reads_memory();
		if (rd == eR15 && !(op & INSN_SDT_B))
		{
			// a word-aligned PC-relative literal is a static target; the generator folds the load
			const void *host = nullptr;
			uint32_t word = 0;
			const offs_t address = desc.pc + 8 + ((op & INSN_SDT_U) ? (op & INSN_SDT_IMM) : -(op & INSN_SDT_IMM));
			if (rn == eR15 && !(op & INSN_I) && (op & INSN_SDT_P) && !(op & INSN_SDT_W) && m_arm.drc_literal(desc, address, host, word)
				&& !(std::rotr(word, 8 * (address & 3)) & 3))
			{
				// in the 26-bit modes only the word's address bits count
				describe_branch(desc, conditional, std::rotr(word, 8 * (address & 3)) & pc_mask());
			}
			else
			{
				describe_pc_write(desc, conditional, m_arm.m_archRev >= 5);     // ARMv5: LDR PC interworks
			}
		}
		else
		{
			desc.set_r_modified(rd);
		}
	}
	else
	{
		desc.set_writes_memory();
		desc.set_r_used(rd);
	}
	if (!(op & INSN_SDT_P) || (op & INSN_SDT_W))
	{
		desc.set_r_modified(rn);
	}
	desc.set_can_cause_exception();
}

void arm7_cpu_device::frontend::describe_arm_mem_half(opcode_desc &desc, uint32_t op)
{
	const bool conditional = (op >> INSN_COND_SHIFT) != COND_AL;
	const uint32_t rn = (op & INSN_RN) >> INSN_RN_SHIFT;
	const uint32_t rd = (op & INSN_RD) >> INSN_RD_SHIFT;
	const bool load = (op & INSN_SDT_L) || ((op & 0x60) == 0x40);
	const bool dword = !(op & INSN_SDT_L) && (op & 0x60) != 0x20;

	desc.set_r_used(rn);
	if (!(op & 0x400000))
	{
		desc.set_r_used(op & 0xf);
	}
	if (dword && !(m_arm.m_archFlags & ARCHFLAG_E))
	{
		describe_undefined(desc);
		return;
	}
	if (load)
	{
		desc.set_reads_memory();
		if (rd == eR15 && !dword)
		{
			describe_pc_write(desc, conditional, false);
		}
		else
		{
			desc.set_r_modified(rd);
			if (dword)
			{
				desc.set_r_modified((rd + 1) & 0xf);
			}
		}
	}
	else
	{
		desc.set_writes_memory();
		desc.set_r_used(rd);
		if (dword)
		{
			desc.set_r_used((rd + 1) & 0xf);
		}
	}
	if (!(op & INSN_SDT_P) || (op & INSN_SDT_W))
	{
		desc.set_r_modified(rn);
	}
	desc.set_can_cause_exception();
}

void arm7_cpu_device::frontend::describe_arm_mem_block(opcode_desc &desc, uint32_t op)
{
	const bool conditional = (op >> INSN_COND_SHIFT) != COND_AL;
	const uint32_t rn = (op & INSN_RN) >> INSN_RN_SHIFT;
	const uint32_t rlist = op & 0xffff;

	desc.set_r_used(rn);
	if (op & INSN_BDT_S)
	{
		desc.set_cpsr_used();       // the user bank, or the SPSR when R15 is loaded
	}
	if (op & INSN_BDT_L)
	{
		desc.set_reads_memory();
		for (uint32_t i = 0; i < 15; i++)
		{
			if (BIT(rlist, i))
			{
				desc.set_r_modified(i);
			}
		}
		if (BIT(rlist, 15))
		{
			describe_pc_write(desc, conditional, (op & INSN_BDT_S) || (m_arm.m_archRev >= 5));
		}
	}
	else
	{
		desc.set_writes_memory();
		for (uint32_t i = 0; i < 16; i++)
		{
			if (BIT(rlist, i))
			{
				desc.set_r_used(i);
			}
		}
	}
	if (op & INSN_BDT_W)
	{
		desc.set_r_modified(rn);
	}
	desc.set_can_cause_exception();
}

// the coprocessor callbacks can trap or change the MMU state, so the sequence ends
void arm7_cpu_device::frontend::describe_coprocessor(opcode_desc &desc, uint32_t op)
{
	const uint32_t rn = (op >> 16) & 0xf;
	const uint32_t rd = (op >> 12) & 0xf;

	switch ((op >> 24) & 0xf)
	{
		case 0xc: case 0xd:
			if ((op & 0x0fe00000) == 0x0c400000)
			{
				// MCRR, MRRC
				if (BIT(op, 20))
				{
					desc.set_r_modified(rd);
					desc.set_r_modified(rn);
				}
				else
				{
					desc.set_r_used(rd);
					desc.set_r_used(rn);
				}
			}
			else
			{
				// LDC, STC
				desc.set_r_used(rn);
				if (BIT(op, 21))
				{
					desc.set_r_modified(rn);
				}
				if (BIT(op, 20))
				{
					desc.set_reads_memory();
				}
				else
				{
					desc.set_writes_memory();
				}
			}
			break;

		case 0xe:
		default:
			if (op & 0x10)
			{
				// MRC, MCR; MRC to R15 sets the flags
				if (BIT(op, 20))
				{
					if (rd == eR15)
					{
						desc.set_flags_modified();
					}
					else
					{
						desc.set_r_modified(rd);
					}
				}
				else
				{
					desc.set_r_used(rd);
				}
			}
			break;
	}
	desc.set_cpsr_used();
	desc.set_can_cause_exception();
	desc.set_can_change_modes();
	desc.set_end_sequence();
}

void arm7_cpu_device::frontend::describe_thumb(opcode_desc &desc, uint16_t op, const opcode_desc *prev)
{
	const uint32_t rd = op & 7;
	const uint32_t rs = (op >> 3) & 7;

	switch (op >> 12)
	{
		case 0x0: case 0x1:
			if ((op & 0x1800) != 0x1800)
			{
				// LSL, LSR, ASR Rd, Rs, #imm: N, Z and C, but LSL #0 keeps C
				desc.set_r_used(rs);
				desc.set_r_modified(rd);
				desc.set_flags_modified(opcode_desc::NZCV_N | opcode_desc::NZCV_Z | ((op & 0x1fc0) ? opcode_desc::NZCV_C : 0));
			}
			else
			{
				// ADD, SUB Rd, Rs, Rn / #imm3
				desc.set_r_used(rs);
				if (!(op & 0x0400))
				{
					desc.set_r_used((op >> 6) & 7);
				}
				desc.set_r_modified(rd);
				desc.set_flags_modified();
			}
			return;

		case 0x2: case 0x3:
			{
				// MOV, CMP, ADD, SUB Rd, #imm8
				const uint32_t reg = (op >> 8) & 7;
				const uint32_t kind = (op >> 11) & 3;
				if (kind != 0)
				{
					desc.set_r_used(reg);
				}
				if (kind != 1)
				{
					desc.set_r_modified(reg);
				}
				desc.set_flags_modified((kind == 0) ? (opcode_desc::NZCV_N | opcode_desc::NZCV_Z) : opcode_desc::NZCV_ALL);
			}
			return;

		case 0x4:
			if (!(op & 0x0800))
			{
				if (!(op & 0x0400))
				{
					// the ALU operations on low registers
					const uint32_t alu = (op >> 6) & 0xf;
					desc.set_r_used(rd);
					desc.set_r_used(rs);
					if (alu != 0x8 && alu != 0xa && alu != 0xb)
					{
						desc.set_r_modified(rd); // all but TST, CMP, CMN
					}
					switch (alu)
					{
						case 0x2: case 0x3: case 0x4: case 0x7:
							// LSL, LSR, ASR, ROR by a register: N, Z, and C unless the amount is 0
							desc.set_flags_used(opcode_desc::NZCV_C);
							desc.set_flags_modified(opcode_desc::NZCV_N | opcode_desc::NZCV_Z | opcode_desc::NZCV_C);
							break;
						case 0x5: case 0x6:
							// ADC, SBC
							desc.set_flags_used(opcode_desc::NZCV_C);
							desc.set_flags_modified();
							break;
						case 0x9: case 0xa: case 0xb:
							// NEG, CMP, CMN
							desc.set_flags_modified();
							break;
						default:
							// AND, EOR, TST, ORR, MUL, BIC, MVN: N and Z
							desc.set_flags_modified(opcode_desc::NZCV_N | opcode_desc::NZCV_Z);
							break;
					}
				}
				else
				{
					// ADD, CMP, MOV with high registers; BX/BLX
					const uint32_t hd = rd | ((op >> 4) & 8);
					const uint32_t hs = rs | ((op >> 3) & 8);
					switch ((op >> 8) & 3)
					{
						case 0:
							desc.set_r_used(hd);
							desc.set_r_used(hs);
							if (hd == 15)
							{
								describe_pc_write(desc, false, false);
							}
							else
							{
								desc.set_r_modified(hd);
							}
							break;
						case 1:
							desc.set_r_used(hd);
							desc.set_r_used(hs);
							desc.set_flags_modified();
							break;
						case 2:
							desc.set_r_used(hs);
							if (hd == 15)
							{
								describe_pc_write(desc, false, false);
							}
							else
							{
								desc.set_r_modified(hd);
							}
							break;
						case 3:
						default:
							// BX, and BLX on ARMv5
							desc.set_r_used(hs);
							if (op & 0x80)
							{
								if (m_arm.m_archRev < 5)
								{
									describe_undefined(desc);
									return;
								}
								desc.set_r_modified(14);
							}
							describe_pc_write(desc, false, true);
							break;
					}
				}
			}
			else
			{
				// LDR Rd, [PC, #imm]
				desc.set_r_modified((op >> 8) & 7);
				desc.set_reads_memory();
				desc.set_can_cause_exception();
			}
			return;

		case 0x5:
			{
				// loads and stores with a register offset
				const uint32_t kind = (op >> 9) & 7;
				desc.set_r_used(rs);
				desc.set_r_used((op >> 6) & 7);
				if (kind == 0 || kind == 1 || kind == 2)
				{
					desc.set_r_used(rd);
					desc.set_writes_memory();
				}
				else
				{
					desc.set_r_modified(rd);
					desc.set_reads_memory();
				}
				desc.set_can_cause_exception();
			}
			return;

		case 0x6: case 0x7: case 0x8:
			// loads and stores with an immediate offset
			desc.set_r_used(rs);
			if (op & 0x0800)
			{
				desc.set_r_modified(rd);
				desc.set_reads_memory();
			}
			else
			{
				desc.set_r_used(rd);
				desc.set_writes_memory();
			}
			desc.set_can_cause_exception();
			return;

		case 0x9:
			// loads and stores relative to SP
			desc.set_r_used(13);
			if (op & 0x0800)
			{
				desc.set_r_modified((op >> 8) & 7);
				desc.set_reads_memory();
			}
			else
			{
				desc.set_r_used((op >> 8) & 7);
				desc.set_writes_memory();
			}
			desc.set_can_cause_exception();
			return;

		case 0xa:
			// ADD Rd, PC/SP, #imm
			if (op & 0x0800)
			{
				desc.set_r_used(13);
			}
			desc.set_r_modified((op >> 8) & 7);
			return;

		case 0xb:
			switch ((op >> 8) & 0xf)
			{
				case 0x0:
					// ADD SP, #imm
					desc.set_r_used(13);
					desc.set_r_modified(13);
					return;

				case 0x2: case 0xa:
					// SXTH, SXTB, UXTH, UXTB; REV, REV16, REVSH (ARMv6)
					if (m_arm.m_archRev < 6)
					{
						describe_undefined(desc);
						return;
					}
					desc.set_r_used(rs);
					desc.set_r_modified(rd);
					return;

				case 0x4: case 0x5:
					// PUSH
					desc.set_r_used(13);
					desc.set_r_modified(13);
					for (uint32_t i = 0; i < 8; i++)
					{
						if (BIT(op, i))
						{
							desc.set_r_used(i);
						}
					}
					if (op & 0x0100)
					{
						desc.set_r_used(14);
					}
					desc.set_writes_memory();
					desc.set_can_cause_exception();
					return;

				case 0x6:
					// SETEND, CPS (ARMv6)
					if (m_arm.m_archRev < 6)
					{
						describe_undefined(desc);
						return;
					}
					desc.set_cpsr_used();
					desc.set_cpsr_modified();
					desc.set_can_change_modes();
					desc.set_end_sequence();
					return;

				case 0xc: case 0xd:
					// POP
					desc.set_r_used(13);
					desc.set_r_modified(13);
					for (uint32_t i = 0; i < 8; i++)
					{
						if (BIT(op, i))
						{
							desc.set_r_modified(i);
						}
					}
					desc.set_reads_memory();
					desc.set_can_cause_exception();
					if (op & 0x0100)
					{
						describe_pc_write(desc, false, m_arm.m_archRev >= 5);
					}
					return;

				case 0xe:
					// BKPT (ARMv5)
					if (m_arm.m_archRev < 5)
					{
						describe_undefined(desc);
						return;
					}
					desc.set_will_cause_exception();
					desc.set_can_change_modes();
					desc.set_end_sequence();
					return;

				default:
					describe_undefined(desc);
					return;
			}

		case 0xc:
			{
				// STMIA, LDMIA
				const uint32_t rb = (op >> 8) & 7;
				desc.set_r_used(rb);
				desc.set_r_modified(rb);
				for (uint32_t i = 0; i < 8; i++)
				{
					if (BIT(op, i))
					{
						if (op & 0x0800)
						{
							desc.set_r_modified(i);
						}
						else
						{
							desc.set_r_used(i);
						}
					}
				}
				if (op & 0x0800)
				{
					desc.set_reads_memory();
				}
				else
				{
					desc.set_writes_memory();
				}
				desc.set_can_cause_exception();
			}
			return;

		case 0xd:
			{
				const uint32_t cond = (op >> 8) & 0xf;
				if (cond == 0xe)
				{
					describe_undefined(desc);
				}
				else if (cond == 0xf)
				{
					// SWI
					desc.set_will_cause_exception();
					desc.set_can_change_modes();
					desc.set_end_sequence();
				}
				else
				{
					desc.set_flags_used(COND_FLAGS[cond]);
					describe_branch(desc, true, desc.pc + 4 + util::sext((op & 0xff) << 1, 9));
				}
			}
			return;

		case 0xe:
			if (!(op & 0x0800))
			{
				// B
				describe_branch(desc, false, desc.pc + 4 + util::sext((op & 0x7ff) << 1, 12));
			}
			else
			{
				// the second half of BLX (ARMv5)
				if (m_arm.m_archRev < 5)
				{
					describe_undefined(desc);
					return;
				}
				desc.set_r_used(14);
				desc.set_r_modified(14);
				describe_pc_write(desc, false, true);
			}
			return;

		case 0xf:
		default:
			if (!(op & 0x0800))
			{
				// the first half of BL/BLX: LR takes the high part of the offset
				desc.set_r_modified(14);
			}
			else
			{
				// the second half of BL: with the first half right before it the target is static
				desc.set_r_used(14);
				desc.set_r_modified(14);
				if ((m_arm.m_drc_opt & DRC_OPT_BLFUSE) && prev != nullptr && prev->length == 2 && prev->pc + 2 == desc.pc && (prev->opptr & 0xf800) == 0xf000)
				{
					describe_branch(desc, false, prev->pc + 4 + util::sext((prev->opptr & 0x7ff) << 12, 23) + ((op & 0x7ff) << 1));
				}
				else
				{
					describe_pc_write(desc, false, false);
				}
			}
			return;
	}
}
