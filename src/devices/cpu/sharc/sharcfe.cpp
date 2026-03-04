// license:BSD-3-Clause
// copyright-holders:Ville Linde

/******************************************************************************

    Front-end for SHARC recompiler

******************************************************************************/

#include "emu.h"
#include "sharcfe.h"

#include "sharcinternal.ipp"

#include "cpu/drcfe.ipp"


adsp21062_device::frontend::frontend(adsp21062_device *sharc, uint32_t window_start, uint32_t window_end, uint32_t max_sequence)
	: drc_frontend_base(sharc->space_config(AS_PROGRAM)->page_shift(), window_start, window_end, max_sequence)
	, m_sharc(sharc)
{
	m_loopmap = std::make_unique<LOOP_ENTRY[]>(0x20000);
}

adsp21062_device::frontend::~frontend()
{
}


adsp21062_device::opcode_desc const *adsp21062_device::frontend::describe_code(offs_t startpc)
{
	return do_describe_code(
			[this] (opcode_desc &desc, opcode_desc const *prev) { return describe(desc, prev); },
			startpc);
}


void adsp21062_device::frontend::flush()
{
	for (unsigned i = 0; 0x20000 > i; ++i)
		m_loopmap[i].clear();
}

void adsp21062_device::frontend::add_loop_entry(uint32_t pc, uint8_t type, opcode_desc::extra_flags const &userflags)
{
	uint32_t const l2 = pc >> 17;
	uint32_t const l1 = pc & 0x1ffff;

	if (l2 != 0x1)
		fatalerror("adsp21062_device::frontend::add_loop_entry: PC = %08X", pc);

	LOOP_ENTRY* map = m_loopmap.get();
	uint32_t current_type = map[l1].entrytype;
	opcode_desc::extra_flags current_userflags = map[l1].userflags;

	current_type |= type;
	current_userflags |= userflags;

	map[l1].entrytype = current_type;
	map[l1].userflags = current_userflags;
}

void adsp21062_device::frontend::insert_loop(const LOOP_DESCRIPTOR &loopdesc)
{
	add_loop_entry(loopdesc.start_pc, LOOP_ENTRY_START, 0);
	add_loop_entry(loopdesc.end_pc, LOOP_ENTRY_EVALUATION, 0);
	if (loopdesc.astat_check_pc != 0xffffffff)
	{
		uint32_t flags = m_sharc->do_condition_astat_bits(loopdesc.condition);
		opcode_desc::extra_flags userflags;
		userflags.reset();
		if (flags & AZ) { userflags.set(opcode_desc::ASTAT_DELAY_COPY_AZ); }
		if (flags & AN) { userflags.set(opcode_desc::ASTAT_DELAY_COPY_AN); }
		if (flags & AV) { userflags.set(opcode_desc::ASTAT_DELAY_COPY_AV); }
		if (flags & AC) { userflags.set(opcode_desc::ASTAT_DELAY_COPY_AC); }
		if (flags & MN) { userflags.set(opcode_desc::ASTAT_DELAY_COPY_MN); }
		if (flags & MV) { userflags.set(opcode_desc::ASTAT_DELAY_COPY_MV); }
		if (flags & SV) { userflags.set(opcode_desc::ASTAT_DELAY_COPY_SV); }
		if (flags & SZ) { userflags.set(opcode_desc::ASTAT_DELAY_COPY_SZ); }
		if (flags & BTF) { userflags.set(opcode_desc::ASTAT_DELAY_COPY_BTF); }
		add_loop_entry(loopdesc.astat_check_pc, LOOP_ENTRY_ASTAT_CHECK, userflags);
	}
}

bool adsp21062_device::frontend::is_loop_evaluation(uint32_t pc)
{
	uint32_t l2 = pc >> 17;
	uint32_t l1 = pc & 0x1ffff;

	if (l2 != 0x1)
		return false;

	LOOP_ENTRY* map = m_loopmap.get();
	if (map[l1].entrytype & LOOP_ENTRY_EVALUATION)
		return true;

	return false;
}

bool adsp21062_device::frontend::is_loop_start(uint32_t pc)
{
	uint32_t l2 = pc >> 17;
	uint32_t l1 = pc & 0x1ffff;

	if (l2 != 0x1)
		return false;

	LOOP_ENTRY* map = m_loopmap.get();
	if (map[l1].entrytype & LOOP_ENTRY_START)
		return true;

	return false;
}

bool adsp21062_device::frontend::is_astat_delay_check(uint32_t pc)
{
	uint32_t l2 = pc >> 17;
	uint32_t l1 = pc & 0x1ffff;

	if (l2 != 0x1)
		return false;

	LOOP_ENTRY* map = m_loopmap.get();
	if (map[l1].entrytype & LOOP_ENTRY_ASTAT_CHECK)
		return true;

	return false;
}


bool adsp21062_device::frontend::describe(opcode_desc &desc, const opcode_desc *prev)
{
	uint64_t opcode = desc.opptr = m_sharc->pm_read48(desc.pc);

	desc.length = 1;

	// handle looping
	if (is_astat_delay_check(desc.pc))
	{
		LOOP_ENTRY *map = m_loopmap.get();
		int index = desc.pc & 0x1ffff;
		desc.set_extra_flags(map[index].userflags);
		if (map[index].userflags[opcode_desc::ASTAT_DELAY_COPY_AZ]) { desc.set_az_used(); }
		if (map[index].userflags[opcode_desc::ASTAT_DELAY_COPY_AN]) { desc.set_an_used(); }
		if (map[index].userflags[opcode_desc::ASTAT_DELAY_COPY_AV]) { desc.set_av_used(); }
		if (map[index].userflags[opcode_desc::ASTAT_DELAY_COPY_AC]) { desc.set_ac_used(); }
		if (map[index].userflags[opcode_desc::ASTAT_DELAY_COPY_MN]) { desc.set_mn_used(); }
		if (map[index].userflags[opcode_desc::ASTAT_DELAY_COPY_MV]) { desc.set_mv_used(); }
		if (map[index].userflags[opcode_desc::ASTAT_DELAY_COPY_SV]) { desc.set_sv_used(); }
		if (map[index].userflags[opcode_desc::ASTAT_DELAY_COPY_SZ]) { desc.set_sz_used(); }
		if (map[index].userflags[opcode_desc::ASTAT_DELAY_COPY_BTF]) { desc.set_btf_used(); }
	}

	if (is_loop_start(desc.pc))
	{
		desc.set_is_branch_target();
	}

	if (is_loop_evaluation(desc.pc))
	{
		desc.set_is_conditional_branch();
		desc.set_loop();
	}


	switch ((opcode >> 45) & 7)
	{
		case 0:             // subops
		{
			uint32_t const subop = op_get_subop(opcode);
			switch (subop)
			{
				case 0x00:          // NOP / idle                       |000|00000|
					if (opcode & 0x008000000000U)
					{
						// IDLE
						desc.set_end_sequence();
					}
					else
					{
						// NOP
					}
					break;

				case 0x01:          // compute                              |000|00001|
				{
					int cond = op_get_cond(opcode);
					describe_if_condition(desc, cond);

					if (!describe_compute(desc, opcode))
						return false;
					break;
				}

				case 0x02:          // immediate shift                      |000|00010|
				{
					int const shiftop = (opcode >> 16) & 0x3f;
					int const rn = (opcode >> 4) & 0xf;
					int const rx = (opcode & 0xf);
					int const cond = op_get_cond(opcode);

					describe_if_condition(desc, cond);

					if (!describe_shiftop_imm(desc, shiftop, rn, rx))
						return false;
					break;
				}

				case 0x04:          // compute / modify                     |000|00100|
				{
					int const g = (opcode >> 38) & 0x1;
					int const m = (opcode >> 27) & 0x7;
					int const i = (opcode >> 30) & 0x7;
					int const cond = op_get_cond(opcode);

					describe_if_condition(desc, cond);

					if (!describe_compute(desc, opcode))
						return false;

					if (g)
					{
						// PM
						desc.set_pm_i_used(i);
						desc.set_pm_i_modified(i);
						desc.set_pm_m_used(m);
					}
					else
					{
						// DM
						desc.set_dm_i_used(i);
						desc.set_dm_i_modified(i);
						desc.set_dm_m_used(m);
					}
					break;
				}

				case 0x06:          // direct jump|call                     |000|00110|
				{
					int const j = op_get_jump_j(opcode);
					int const b = op_get_jump_b(opcode);
					int const cond = op_get_cond(opcode);
					uint32_t const address = opcode & 0xffffff;

					if (m_sharc->if_condition_always_true(cond))
					{
						desc.set_is_unconditional_branch();
						desc.set_end_sequence();
					}
					else
					{
						desc.set_is_conditional_branch();
					}

					describe_if_condition(desc, cond);

					desc.targetpc = address;
					desc.delayslots = j ? 2 : 0;

					if (b) desc.set_call();
					break;
				}

				case 0x07:          // direct jump|call                     |000|00111|
				{
					int const j = op_get_jump_j(opcode);
					int const b = op_get_jump_b(opcode);
					int const cond = op_get_cond(opcode);
					uint32_t const address = opcode & 0xffffff;

					if (m_sharc->if_condition_always_true(cond))
					{
						desc.set_is_unconditional_branch();
						desc.set_end_sequence();
					}
					else
					{
						desc.set_is_conditional_branch();
					}

					describe_if_condition(desc, cond);

					desc.targetpc = desc.pc + util::sext(address, 24);
					desc.delayslots = j ? 2 : 0;

					if (b) desc.set_call();
					break;
				}

				case 0x08:          // indirect jump|call / compute         |000|01000|
				{
					int const j = op_get_jump_j(opcode);
					int const b = op_get_jump_b(opcode);
					int const pmi = op_get_pmi(opcode);
					int const pmm = op_get_pmm(opcode);
					int const cond = op_get_cond(opcode);

					if (!describe_compute(desc, opcode))
						return false;

					if (m_sharc->if_condition_always_true(cond))
					{
						desc.set_is_unconditional_branch();
						desc.set_end_sequence();
					}
					else
					{
						desc.set_is_conditional_branch();
					}

					describe_if_condition(desc, cond);

					desc.set_pm_i_used(pmi);
					desc.set_pm_m_used(pmm);

					desc.targetpc = BRANCH_TARGET_DYNAMIC;
					desc.delayslots = j ? 2 : 0;

					if (b) desc.set_call();
					break;
				}

				case 0x09:          // indirect jump|call / compute         |000|01001|
				{
					int const j = op_get_jump_j(opcode);
					int const b = op_get_jump_b(opcode);
					int const cond = op_get_cond(opcode);

					if (!describe_compute(desc, opcode))
						return false;

					if (m_sharc->if_condition_always_true(cond))
					{
						desc.set_is_unconditional_branch();
						desc.set_end_sequence();
					}
					else
					{
						desc.set_is_conditional_branch();
					}

					describe_if_condition(desc, cond);

					desc.targetpc = desc.pc + op_get_reladdr(opcode);
					desc.delayslots = j ? 2 : 0;

					if (b) desc.set_call();
					break;
				}

				case 0x0a:          // return from subroutine / compute     |000|01010|
				{
					int const cond = op_get_cond(opcode);
					int const j = op_get_jump_j(opcode);

					if (!describe_compute(desc, opcode))
						return false;

					if (m_sharc->if_condition_always_true(cond))
					{
						desc.set_is_unconditional_branch();
						desc.set_end_sequence();
					}
					else
					{
						desc.set_is_conditional_branch();
					}

					describe_if_condition(desc, cond);

					desc.targetpc = BRANCH_TARGET_DYNAMIC;
					desc.delayslots = j ? 2 : 0;
					break;
				}

				case 0x0b:          // return from interrupt / compute      |000|01011|
				{
					int const cond = op_get_cond(opcode);
					int const j = op_get_jump_j(opcode);

					if (!describe_compute(desc, opcode))
						return false;

					if (m_sharc->if_condition_always_true(cond))
					{
						desc.set_is_unconditional_branch();
						desc.set_end_sequence();
					}
					else
					{
						desc.set_is_conditional_branch();
					}

					describe_if_condition(desc, cond);

					desc.targetpc = BRANCH_TARGET_DYNAMIC;
					desc.delayslots = j ? 2 : 0;
					break;
				}

				case 0x0c:          // do until counter expired             |000|01100|
				{
					int const offset = util::sext(opcode & 0xffffff, 24);

					LOOP_DESCRIPTOR loop;
					loop.start_pc = desc.pc + 1;
					loop.end_pc = desc.pc + offset;
					loop.astat_check_pc = 0xffffffff;
					loop.type = LOOP_TYPE_COUNTER;
					loop.condition = 0;

					insert_loop(loop);
					break;
				}

				case 0x0d:          // do until counter expired             |000|01101|
				{
					int const ureg = (opcode >> 32) & 0xff;
					if (!describe_ureg_access(desc, ureg, UREG_READ))
						return false;

					int const offset = util::sext(opcode & 0xffffff, 24);

					LOOP_DESCRIPTOR loop;
					loop.start_pc = desc.pc + 1;
					loop.end_pc = desc.pc + offset;
					loop.astat_check_pc = 0xffffffff;
					loop.type = LOOP_TYPE_COUNTER;
					loop.condition = 0;

					insert_loop(loop);
					break;
				}

				case 0x0e:          // do until                             |000|01110|
				{
					int const offset = util::sext(opcode & 0xffffff, 24);
					int const cond = op_get_cond(opcode);

					LOOP_DESCRIPTOR loop;
					loop.start_pc = desc.pc + 1;
					loop.end_pc = desc.pc + offset;
					loop.type = LOOP_TYPE_CONDITIONAL;
					loop.condition = cond;

					/*
					loop.astat_check_pc = loop.end_pc - 2;
					if (loop.astat_check_pc < loop.start_pc)
					    fatalerror("describe_compute: conditional loop < 2 at %08X", desc.pc);
					    */

					int jump_diff = loop.end_pc - loop.start_pc;
					if (jump_diff >= 2)
						loop.astat_check_pc = loop.end_pc - 2;
					else if (jump_diff == 1)
						loop.astat_check_pc = loop.end_pc - 1;
					else
						loop.astat_check_pc = loop.end_pc;

					insert_loop(loop);
					break;
				}

				case 0x0f:          // immediate data -> ureg               |000|01111|
				{
					int const ureg = (opcode >> 32) & 0xff;
					if (!describe_ureg_access(desc, ureg, UREG_WRITE))
						return false;
					break;
				}

				case 0x10:          // ureg <-> DM|PM (direct)              |000|100|G|D|
				case 0x11:
				case 0x12:
				case 0x13:
				{
					int const ureg = (opcode >> 32) & 0xff;
					int const d = (opcode >> 40) & 1;
					if (d)
					{
						if (!describe_ureg_access(desc, ureg, UREG_READ))
							return false;
						desc.set_writes_memory();
					}
					else
					{
						if (!describe_ureg_access(desc, ureg, UREG_WRITE))
							return false;
						desc.set_reads_memory();
					}
					break;
				}

				case 0x14:          // system register bit manipulation     |000|10100|
				{
					int const bop = (opcode >> 37) & 0x7;
					int const sreg = (opcode >> 32) & 0xf;
					uint32_t const data = uint32_t(opcode);

					switch (bop)
					{
						case 0:     // SET
						case 1:     // CLEAR
							if (sreg == 0x7c)   // ASTAT
							{
								if (data & AZ) desc.set_az_modified();
								if (data & AV) desc.set_av_modified();
								if (data & AN) desc.set_an_modified();
								if (data & AC) desc.set_ac_modified();
								if (data & AS) desc.set_as_modified();
								if (data & AI) desc.set_ai_modified();
								if (data & MN) desc.set_mn_modified();
								if (data & MV) desc.set_mv_modified();
								if (data & MU) desc.set_mu_modified();
								if (data & MI) desc.set_mi_modified();
								if (data & SV) desc.set_sv_modified();
								if (data & SZ) desc.set_sz_modified();
								if (data & SS) desc.set_ss_modified();
								if (data & BTF) desc.set_btf_modified();
								if (data & AF) desc.set_af_modified();
							}
							break;
						case 2:     // TOGGLE
							if (sreg == 0x7c)   // ASTAT
							{
								if (data & AZ)
								{
									desc.set_az_used(); desc.set_az_modified();
								}
								if (data & AV)
								{
									desc.set_av_used(); desc.set_av_modified();
								}
								if (data & AN)
								{
									desc.set_an_used(); desc.set_an_modified();
								}
								if (data & AC)
								{
									desc.set_ac_used(); desc.set_ac_modified();
								}
								if (data & AS)
								{
									desc.set_as_used(); desc.set_as_modified();
								}
								if (data & AI)
								{
									desc.set_ai_used(); desc.set_ai_modified();
								}
								if (data & MN)
								{
									desc.set_mn_used(); desc.set_mn_modified();
								}
								if (data & MV)
								{
									desc.set_mv_used(); desc.set_mv_modified();
								}
								if (data & MU)
								{
									desc.set_mu_used(); desc.set_mu_modified();
								}
								if (data & MI)
								{
									desc.set_mi_used(); desc.set_mi_modified();
								}
								if (data & SV)
								{
									desc.set_sv_used(); desc.set_sv_modified();
								}
								if (data & SZ)
								{
									desc.set_sz_used(); desc.set_sz_modified();
								}
								if (data & SS)
								{
									desc.set_ss_used(); desc.set_ss_modified();
								}
								if (data & BTF)
								{
									desc.set_btf_used(); desc.set_btf_modified();
								}
								if (data & AF)
								{
									desc.set_af_used(); desc.set_af_modified();
								}
							}
							break;

						case 4:     // TEST
						case 5:     // XOR
							desc.set_btf_modified();
							break;

						default:
							fatalerror("adsp21062_device::frontend::describe: system reg bit manipulation %d", bop);
							return false;
					}
					break;
				}

				case 0x16:          // I register modify / bit-reverse      |000|10110|
				{
					if (opcode & 0x008000000000U)   // bit reverse
					{
						fatalerror("adsp21062_device::frontend::describe: bit reverse unimplemented");
					}
					else            // modify
					{
						int const g = (opcode >> 38) & 0x1;
						int const i = (opcode >> 32) & 0x7;

						if (g)
							desc.set_pm_i_used(i);
						else
							desc.set_dm_i_used(i);
					}
					break;
				}

				case 0x17:          // push|pop stacks / flush cache        |000|10111|
					break;

				case 0x18:          // cjump                                |000|11000|
					fatalerror("adsp21062_device::frontend::describe: cjump unimplemented");
					break;

				case 0x19:          // rframe                               |000|11001|
					fatalerror("adsp21062_device::frontend::describe: rframe unimplemented");
					break;

				default:
					fatalerror("adsp21062_device::frontend::describe: unknown subop %02X in opcode %04X%08X", subop, (uint16_t)(opcode >> 32), (uint32_t)(opcode));
					return false;
			}
			break;
		}

		case 1:             // compute / dreg <-> DM / dreg <-> PM                                  |001|
		{
			if (!describe_compute(desc, opcode))
				return false;

			int const pm_dreg = (opcode >> 23) & 0xf;
			int const pmm = op_get_pmm(opcode);
			int const pmi = op_get_pmi(opcode);
			int const dm_dreg = (opcode >> 33) & 0xf;
			int const dmm = op_get_dmm(opcode);
			int const dmi = op_get_dmi(opcode);
			int const pmd = (opcode >> 37) & 0x1;
			int const dmd = (opcode >> 44) & 0x1;

			desc.set_pm_i_used(pmi);
			desc.set_pm_i_modified(pmi);
			desc.set_pm_m_used(pmm);

			if (pmd)
			{
				desc.set_reg_used(pm_dreg);
				desc.set_writes_memory();
			}
			else
			{
				desc.set_reg_modified(pm_dreg);
				desc.set_reads_memory();
			}

			desc.set_dm_i_used(dmi);
			desc.set_dm_i_modified(dmi);
			desc.set_dm_m_used(dmm);

			if (dmd)
			{
				desc.set_reg_used(dm_dreg);
				desc.set_writes_memory();
			}
			else
			{
				desc.set_reg_modified(dm_dreg);
				desc.set_reads_memory();
			}
			break;
		}

		case 2:             // compute / ureg <-> DM|PM, register modify                            |010|
		{
			if (!describe_compute(desc, opcode))
				return false;

			int const i = (opcode >> 41) & 0x7;
			int const m = (opcode >> 38) & 0x7;
			int const cond = op_get_cond(opcode);
			int const g = (opcode >> 32) & 0x1;
			int const d = (opcode >> 31) & 0x1;
			int const ureg = (opcode >> 23) & 0xff;

			describe_if_condition(desc, cond);

			if (d)
			{
				if (!describe_ureg_access(desc, ureg, UREG_READ))
					return false;
				desc.set_writes_memory();
			}
			else
			{
				if (!describe_ureg_access(desc, ureg, UREG_WRITE))
					return false;
				desc.set_writes_memory();
			}

			if (g)
			{
				// PM
				desc.set_pm_i_used(i);
				desc.set_pm_m_used(m);
			}
			else
			{
				// DM
				desc.set_dm_i_used(i);
				desc.set_dm_m_used(m);
			}

			break;
		}

		case 3:
		{
			if (!describe_compute(desc, opcode))
				return false;

			if (opcode & 0x100000000000U)   // compute / ureg <-> ureg                          |011|1|
			{
				int const src_ureg = op_get_ureg_src(opcode);
				int const dst_ureg = op_get_ureg_dst(opcode);
				int const cond = op_get_cond_ureg(opcode);

				describe_if_condition(desc, cond);

				if (!describe_ureg_access(desc, src_ureg, UREG_READ))
					return false;
				if (!describe_ureg_access(desc, dst_ureg, UREG_WRITE))
					return false;
			}
			else                                // compute / dreg <-> DM|PM, immediate modify       |011|0|
			{
				int const u = (opcode >> 38) & 0x1;
				int const d = (opcode >> 39) & 0x1;
				int const g = (opcode >> 40) & 0x1;
				int const dreg = (opcode >> 23) & 0xf;
				int const i = (opcode >> 41) & 0x7;
				int const cond = op_get_cond(opcode);

				describe_if_condition(desc, cond);

				if (d)
				{
					desc.set_reg_used(dreg);
					desc.set_writes_memory();
				}
				else
				{
					desc.set_reg_modified(dreg);
					desc.set_reads_memory();
				}

				if (g)
				{
					// PM
					desc.set_pm_i_used(i);

					if (u)  // post-modify with update
					{
						desc.set_pm_i_modified(i);
					}
				}
				else
				{
					// DM
					desc.set_dm_i_used(i);

					if (u)  // post-modify with update
					{
						desc.set_dm_i_modified(i);
					}
				}
			}
			break;
		}

		case 4:
		{
			if (opcode & 0x100000000000U)   // immediate data -> DM|PM                          |100|1|
			{
				int const i = (opcode >> 41) & 0x7;
				int const m = (opcode >> 38) & 0x7;
				int const g = (opcode >> 37) & 0x1;

				desc.set_writes_memory();

				if (g)
				{
					// PM
					desc.set_pm_i_used(i);
					desc.set_pm_i_modified(i);
					desc.set_pm_m_used(m);
				}
				else
				{
					// DM
					desc.set_dm_i_used(i);
					desc.set_dm_i_modified(i);
					desc.set_dm_m_used(m);
				}
			}
			else                                // immediate shift / dreg <-> DM|PM                 |100|0|
			{
				int const i = (opcode >> 41) & 0x7;
				int const m = (opcode >> 38) & 0x7;
				int const g = (opcode >> 32) & 0x1;
				int const d = (opcode >> 31) & 0x1;
				int const dreg = (opcode >> 23) & 0xf;
				int const shiftop = (opcode >> 16) & 0x3f;
				int const rn = (opcode >> 4) & 0xf;
				int const rx = (opcode & 0xf);
				int const cond = op_get_cond(opcode);

				describe_if_condition(desc, cond);

				if (!describe_shiftop_imm(desc, shiftop, rn, rx))
					return false;

				if (d)
				{
					desc.set_reg_used(dreg);
					desc.set_writes_memory();
				}
				else
				{
					desc.set_reg_modified(dreg);
					desc.set_reads_memory();
				}

				if (g)
				{
					// PM
					desc.set_pm_i_used(i);
					desc.set_pm_i_modified(i);
					desc.set_pm_m_used(m);
				}
				else
				{
					// DM
					desc.set_dm_i_used(i);
					desc.set_dm_i_modified(i);
					desc.set_dm_m_used(m);
				}
			}
			break;
		}

		case 5:                             // ureg <-> DM|PM (indirect)                            |101|
		{
			int const ureg = (opcode >> 32) & 0xff;
			int const d = (opcode >> 40) & 1;
			int const i = (opcode >> 41) & 0x7;
			int const g = (opcode >> 44) & 1;

			if (d)
			{
				if (!describe_ureg_access(desc, ureg, UREG_READ))
					return false;
				desc.set_writes_memory();
			}
			else
			{
				if (!describe_ureg_access(desc, ureg, UREG_WRITE))
					return false;
				desc.set_reads_memory();
			}

			if (g)
				desc.set_pm_i_used(i);
			else
				desc.set_dm_i_used(i);
			break;
		}

		case 6:                             // indirect jump / compute / dreg <-> DM                |110|
		{
			int const d = (opcode >> 44) & 0x1;
			int const dmi = op_get_dmi(opcode);
			int const dmm = op_get_dmm(opcode);
			int const pmi = op_get_pmi(opcode);
			int const pmm = op_get_pmm(opcode);
			int const cond = op_get_cond(opcode);
			int const dreg = (opcode >> 23) & 0xf;

			if (!describe_compute(desc, opcode))
				return false;

			if (m_sharc->if_condition_always_true(cond))
			{
				desc.set_is_unconditional_branch();
				desc.set_end_sequence();
			}
			else
			{
				desc.set_is_conditional_branch();
			}

			describe_if_condition(desc, cond);

			desc.set_pm_i_used(pmi);
			desc.set_pm_m_used(pmm);
			desc.set_dm_i_used(dmi);
			desc.set_dm_i_modified(dmi);
			desc.set_dm_m_used(dmm);

			if (d)
			{
				desc.set_reg_used(dreg);
				desc.set_writes_memory();
			}
			else
			{
				desc.set_reg_modified(dreg);
				desc.set_reads_memory();
			}

			desc.targetpc = BRANCH_TARGET_DYNAMIC;
			desc.delayslots = 0;
			break;
		}

		case 7:                             // indirect jump / compute / dreg <-> DM                |111|
		{
			int const d = (opcode >> 44) & 0x1;
			int const dmi = op_get_dmi(opcode);
			int const dmm = op_get_dmm(opcode);
			int const cond = op_get_cond(opcode);
			int const dreg = (opcode >> 23) & 0xf;

			if (!describe_compute(desc, opcode))
				return false;

			if (m_sharc->if_condition_always_true(cond))
			{
				desc.set_is_unconditional_branch();
				desc.set_end_sequence();
			}
			else
			{
				desc.set_is_conditional_branch();
			}

			describe_if_condition(desc, cond);

			desc.set_dm_i_used(dmi);
			desc.set_dm_i_modified(dmi);
			desc.set_dm_m_used(dmm);

			if (d)
			{
				desc.set_reg_used(dreg);
				desc.set_writes_memory();
			}
			else
			{
				desc.set_reg_modified(dreg);
				desc.set_reads_memory();
			}

			desc.targetpc = desc.pc + op_get_reladdr(opcode);
			desc.delayslots = 0;
			break;
		}
	}

	return true;
}

bool adsp21062_device::frontend::describe_compute(opcode_desc &desc, uint64_t opcode)
{
	// skip if no-op
	if ((opcode & 0x7fffff) == 0)
		return true;

	int const rs = (opcode >> 12) & 0xf;
	int const rn = (opcode >> 8) & 0xf;
	int const ra = rn;
	int const rx = (opcode >> 4) & 0xf;
	int const ry = (opcode >> 0) & 0xf;

	if (opcode & 0x400000)      // multi-function operation
	{
		uint32_t const multiop = (opcode >> 16) & 0x3f;
		int const fm = rs;
		int const fa = rn;
		int const fxm = (opcode >> 6) & 0x3;          // registers 0 - 3
		int const fym = ((opcode >> 4) & 0x3) + 4;    // registers 4 - 7
		int const fxa = ((opcode >> 2) & 0x3) + 8;    // registers 8 - 11
		int const fya = (opcode & 0x3) + 12;          // registers 12 - 15

		switch (multiop)
		{
			case 0x00:          // Rn = MRxx
				desc.set_reg_modified(rn);
				break;
			case 0x01:          // MRxx = Rn
				desc.set_reg_used(rn);
				break;

			case 0x07:          // Ra = Rx + Ry,   Rs = Rx - Ry
			case 0x0f:          // Fa = Fx + Fy,   Fs = Fx - Fy
				desc.set_reg_used(rx);
				desc.set_reg_used(ry);
				desc.set_reg_modified(ra);
				desc.set_reg_modified(rs);
				desc.set_alu_flags_modified();
				break;

			case 0x04:          // Rm = R3-0 * R7-4 (SSFR),   Ra = R11-8 + R15-12
			case 0x05:          // Rm = R3-0 * R7-4 (SSFR),   Ra = R11-8 - R15-12
			case 0x06:          // Rm = R3-0 * R7-4 (SSFR),   Ra = (R11-8 + R15-12) / 2
				desc.set_reg_used(fxm);
				desc.set_reg_used(fym);
				desc.set_reg_used(fxa);
				desc.set_reg_used(fya);
				desc.set_reg_modified(fm);
				desc.set_reg_modified(fa);
				desc.set_alu_flags_modified();
				desc.set_mult_flags_modified();
				break;

			case 0x08:          // MRF = MRF + R3-0 * R7-4 (SSF),   Ra = R11-8 + R15-12
			case 0x09:          // MRF = MRF + R3-0 * R7-4 (SSF),   Ra = R11-8 - R15-12
			case 0x0a:          // MRF = MRF + R3-0 * R7-4 (SSF),   Ra = (R11-8 + R15-12) / 2
				desc.set_reg_used(fxm);
				desc.set_reg_used(fym);
				desc.set_reg_used(fxa);
				desc.set_reg_used(fya);
				desc.set_alu_flags_modified();
				desc.set_mult_flags_modified();
				break;

			case 0x0c:          // Rm = MRF + R3-0 * R7-4 (SSFR),   Ra = R11-8 + R15-12
			case 0x0d:          // Rm = MRF + R3-0 * R7-4 (SSFR),   Ra = R11-8 - R15-12
			case 0x0e:          // Rm = MRF + R3-0 * R7-4 (SSFR),   Ra = (R11-8 + R15-12) / 2
				desc.set_reg_used(fxm);
				desc.set_reg_used(fym);
				desc.set_reg_used(fxa);
				desc.set_reg_used(fya);
				desc.set_reg_modified(fm);
				desc.set_reg_modified(fa);
				desc.set_alu_flags_modified();
				desc.set_mult_flags_modified();
				break;

			case 0x10:          // MRF = MRF - R3-0 * R7-4 (SSF),   Ra = R11-8 + R15-12
			case 0x11:          // MRF = MRF - R3-0 * R7-4 (SSF),   Ra = R11-8 - R15-12
			case 0x12:          // MRF = MRF - R3-0 * R7-4 (SSF),   Ra = (R11-8 + R15-12) / 2
				desc.set_reg_used(fxm);
				desc.set_reg_used(fym);
				desc.set_reg_used(fxa);
				desc.set_reg_used(fya);
				desc.set_alu_flags_modified();
				desc.set_mult_flags_modified();
				break;

			case 0x14:          // Rm = MRF - R3-0 * R7-4 (SSFR),   Ra = R11-8 + R15-12
			case 0x15:          // Rm = MRF - R3-0 * R7-4 (SSFR),   Ra = R11-8 - R15-12
			case 0x16:          // Rm = MRF - R3-0 * R7-4 (SSFR),   Ra = (R11-8 + R15-12) / 2
				desc.set_reg_used(fxm);
				desc.set_reg_used(fym);
				desc.set_reg_used(fxa);
				desc.set_reg_used(fya);
				desc.set_reg_modified(fm);
				desc.set_reg_modified(fa);
				desc.set_alu_flags_modified();
				desc.set_mult_flags_modified();
				break;

			case 0x18:          // Fm = F3-0 * F7-4,   Fa = F11-8 + F15-12
			case 0x19:          // Fm = F3-0 * F7-4,   Fa = F11-8 - F15-12
			case 0x1a:          // Fm = F3-0 * F7-4,   Fa = FLOAT F11-8 BY R15-12
			case 0x1b:          // Fm = F3-0 * F7-4,   Fa = FIX F11-8 BY R15-12
			case 0x1c:          // Fm = F3-0 * F7-4,   Fa = (F11-8 + F15-12) / 2
			case 0x1e:          // Fm = F3-0 * F7-4,   Fa = MAX(F11-8, F15-12)
			case 0x1f:          // Fm = F3-0 * F7-4,   Fa = MIN(F11-8, F15-12)
				desc.set_reg_used(fxm);
				desc.set_reg_used(fym);
				desc.set_reg_used(fxa);
				desc.set_reg_used(fya);
				desc.set_reg_modified(fm);
				desc.set_reg_modified(fa);
				desc.set_alu_flags_modified();
				desc.set_mult_flags_modified();
				break;

			case 0x1d:          // Fm = F3-0 * F7-4,   Fa = ABS F11-8
				desc.set_reg_used(fxm);
				desc.set_reg_used(fym);
				desc.set_reg_used(fxa);
				desc.set_reg_modified(fm);
				desc.set_reg_modified(fa);
				desc.set_alu_flags_modified();
				desc.set_mult_flags_modified();
				break;

			case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27:
			case 0x28: case 0x29: case 0x2a: case 0x2b: case 0x2c: case 0x2d: case 0x2e: case 0x2f:
								// Rm = R3-0 * R7-4 (SSFR),   Ra = R11-8 + R15-12,   Rs = R11-8 - R15-12
				desc.set_reg_used(fxm);
				desc.set_reg_used(fym);
				desc.set_reg_used(fxa);
				desc.set_reg_used(fya);
				desc.set_reg_modified(fm);
				desc.set_reg_modified(fa);
				desc.set_reg_modified((opcode >> 16) & 0xf);
				desc.set_alu_flags_modified();
				desc.set_mult_flags_modified();
				break;

			case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37:
			case 0x38: case 0x39: case 0x3a: case 0x3b: case 0x3c: case 0x3d: case 0x3e: case 0x3f:
								// Fm = F3-0 * F7-4,   Fa = F11-8 + F15-12,   Fs = F11-8 - F15-12
				desc.set_reg_used(fxm);
				desc.set_reg_used(fym);
				desc.set_reg_used(fxa);
				desc.set_reg_used(fya);
				desc.set_reg_modified(fm);
				desc.set_reg_modified(fa);
				desc.set_reg_modified((opcode >> 16) & 0xf);
				desc.set_alu_flags_modified();
				desc.set_mult_flags_modified();
				break;

			default:
				fatalerror("adsp21062_device::frontend::describe_compute: unknown multiop %02X in opcode %04X%08X at %08X", multiop, (uint16_t)(opcode >> 32), (uint32_t)(opcode), desc.pc);
				return false;
		}
	}
	else                            // single-function operation
	{
		uint32_t const operation = (opcode >> 12) & 0xff;

		switch ((opcode >> 20) & 3)
		{
			case 0:             // ALU operations
			{
				switch (operation)
				{
					case 0x01:      // Rn = Rx + Ry
					case 0x02:      // Rn = Rx - Ry
					case 0x09:      // Rn = (Rx + Ry) / 2
					case 0x40:      // Rn = Rx AND Ry
					case 0x41:      // Rn = Rx OR Ry
					case 0x42:      // Rn = Rx XOR Ry
					case 0x61:      // Rn = MIN(Rx, Ry)
					case 0x62:      // Rn = MAX(Rx, Ry)
					case 0x63:      // Rn = CLIP Rx BY Ry
					case 0x81:      // Fn = Fx + Fy
					case 0x82:      // Fn = Fx - Fy
					case 0x91:      // Fn = ABS(Fx + Fy)
					case 0x92:      // Fn = ABS(Fx - Fy)
					case 0x89:      // Fn = (Fx + Fy) / 2
					case 0xbd:      // Fn = SCALB Fx BY Ry
					case 0xd9:      // Rn = FIX Fx BY Ry
					case 0xdd:      // Rn = TRUNC Fx BY Ry
					case 0xda:      // Fn = FLOAT Rx BY Ry
					case 0xe1:      // Fn = MIN(Fx, Fy)
					case 0xe2:      // Fn = MAX(Fx, Fy)
					case 0xe3:      // Fn = CLIP Fx BY Fy
					case 0xe0:      // Fn = Fx COPYSIGN Fy
						desc.set_reg_used(rx);
						desc.set_reg_used(ry);
						desc.set_reg_modified(rn);
						desc.set_alu_flags_modified();
						break;

					case 0x05:      // Rn = Rx + Ry + CI
					case 0x06:      // Rn = Rx - Ry + CI - 1
					case 0x25:      // Rn = Rx + CI
					case 0x26:      // Rn = Rx + CI - 1
						desc.set_reg_used(rx);
						desc.set_reg_used(ry);
						desc.set_reg_modified(rn);
						desc.set_ac_used();
						desc.set_alu_flags_modified();
						break;

					case 0x0a:      // COMP(Rx, Ry)
					case 0x8a:      // COMP(Fx, Fy)
						desc.set_reg_used(rx);
						desc.set_reg_used(ry);
						desc.set_alu_flags_modified();
						break;

					case 0x29:      // Rn = Rx + 1
					case 0x2a:      // Rn = Rx - 1
					case 0x22:      // Rn = -Rx
					case 0x30:      // Rn = ABS Rx
					case 0x21:      // Rn = PASS Rx
					case 0x43:      // Rn = NOT Rx
					case 0xb0:      // Fn = ABS(Fx)
					case 0xa1:      // Fn = PASS Fx
					case 0xa2:      // Fn = -Fx
					case 0xa5:      // Fn = RND Fx
					case 0xad:      // Rn = MANT Fx
					case 0xc1:      // Rn = LOGB Fx
					case 0xc9:      // Rn = FIX Fx
					case 0xcd:      // Rn = TRUNC Fx
					case 0xca:      // Fn = FLOAT Rx
					case 0xc4:      // Fn = RECIPS Fx
					case 0xc5:      // Fn = RSQRTS Fx
						desc.set_reg_used(rx);
						desc.set_reg_modified(rn);
						desc.set_alu_flags_modified();
						break;

					case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77:
					case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7e: case 0x7f:
					{
						/* Fixed-point Dual Add/Subtract */
						desc.set_reg_used(rx);
						desc.set_reg_used(ry);
						desc.set_reg_modified(rn);
						desc.set_reg_modified(ra);
						desc.set_alu_flags_modified();
						break;
					}

					case 0xf0: case 0xf1: case 0xf2: case 0xf3: case 0xf4: case 0xf5: case 0xf6: case 0xf7:
					case 0xf8: case 0xf9: case 0xfa: case 0xfb: case 0xfc: case 0xfd: case 0xfe: case 0xff:
					{
						/* Floating-point Dual Add/Subtract */
						desc.set_reg_used(rx);
						desc.set_reg_used(ry);
						desc.set_reg_modified(rn);
						desc.set_reg_modified(ra);
						desc.set_alu_flags_modified();
						break;
					}

					default:
						fatalerror("adsp21062_device::frontend::describe_compute: unknown ALU op %02X in opcode %04X%08X at %08X", operation, (uint16_t)(opcode >> 32), (uint32_t)(opcode), desc.pc);
						return false;
				}
				break;
			}

			case 1:             // multiplier operations
			{
				switch (operation)
				{
					case 0x40:      // Rn = Rx * Ry (UUI)
					case 0x48:      // Rn = Rx * Ry (UUF)
					case 0x49:      // Rn = Rx * Ry (UUFR)
					case 0x50:      // Rn = Rx * Ry (SUI)
					case 0x58:      // Rn = Rx * Ry (SUF)
					case 0x59:      // Rn = Rx * Ry (SUFR)
					case 0x60:      // Rn = Rx * Ry (USI)
					case 0x68:      // Rn = Rx * Ry (USF)
					case 0x69:      // Rn = Rx * Ry (USFR)
					case 0x70:      // Rn = Rx * Ry (SSI)
					case 0x78:      // Rn = Rx * Ry (SSF)
					case 0x79:      // Rn = Rx * Ry (SSFR)
						desc.set_reg_used(rx);
						desc.set_reg_used(ry);
						desc.set_reg_modified(rn);
						desc.set_mult_flags_modified();
						break;

					case 0x44:      // MRF = Rx * Ry (UUI)
					case 0x4c:      // MRF = Rx * Ry (UUF)
					case 0x4d:      // MRF = Rx * Ry (UUFR)
					case 0x54:      // MRF = Rx * Ry (SUI)
					case 0x5c:      // MRF = Rx * Ry (SUF)
					case 0x5d:      // MRF = Rx * Ry (SUFR)
					case 0x64:      // MRF = Rx * Ry (USI)
					case 0x6c:      // MRF = Rx * Ry (USF)
					case 0x6d:      // MRF = Rx * Ry (USFR)
					case 0x74:      // MRF = Rx * Ry (SSI)
					case 0x7c:      // MRF = Rx * Ry (SSF)
					case 0x7d:      // MRF = Rx * Ry (SSFR)
						// TODO: track MRF?
						desc.set_reg_used(rx);
						desc.set_reg_used(ry);
						desc.set_mult_flags_modified();
						break;

					case 0x46:      // MRB = Rx * Ry (UUI)
					case 0x4e:      // MRB = Rx * Ry (UUF)
					case 0x4f:      // MRB = Rx * Ry (UUFR)
					case 0x56:      // MRB = Rx * Ry (SUI)
					case 0x5e:      // MRB = Rx * Ry (SUF)
					case 0x5f:      // MRB = Rx * Ry (SUFR)
					case 0x66:      // MRB = Rx * Ry (USI)
					case 0x6e:      // MRB = Rx * Ry (USF)
					case 0x6f:      // MRB = Rx * Ry (USFR)
					case 0x76:      // MRB = Rx * Ry (SSI)
					case 0x7e:      // MRB = Rx * Ry (SSF)
					case 0x7f:      // MRB = Rx * Ry (SSFR)
						// TODO: track MRB?
						desc.set_reg_used(rx);
						desc.set_reg_used(ry);
						desc.set_mult_flags_modified();
						break;

					case 0x80:      // Rn = MRF + Rx * Ry (UUI)
					case 0x88:      // Rn = MRF + Rx * Ry (UUF)
					case 0x89:      // Rn = MRF + Rx * Ry (UUFR)
					case 0x90:      // Rn = MRF + Rx * Ry (SUI)
					case 0x98:      // Rn = MRF + Rx * Ry (SUF)
					case 0x99:      // Rn = MRF + Rx * Ry (SUFR)
					case 0xa0:      // Rn = MRF + Rx * Ry (USI)
					case 0xa8:      // Rn = MRF + Rx * Ry (USF)
					case 0xa9:      // Rn = MRF + Rx * Ry (USFR)
					case 0xb0:      // Rn = MRF + Rx * Ry (SSI)
					case 0xb8:      // Rn = MRF + Rx * Ry (SSF)
					case 0xb9:      // Rn = MRF + Rx * Ry (SSFR)
						// TODO: track MRF?
						desc.set_reg_used(rx);
						desc.set_reg_used(ry);
						desc.set_reg_modified(rn);
						desc.set_mult_flags_modified();
						break;

					case 0x82:      // Rn = MRB + Rx * Ry (UUI)
					case 0x8a:      // Rn = MRB + Rx * Ry (UUF)
					case 0x8b:      // Rn = MRB + Rx * Ry (UUFR)
					case 0x92:      // Rn = MRB + Rx * Ry (SUI)
					case 0x9a:      // Rn = MRB + Rx * Ry (SUF)
					case 0x9b:      // Rn = MRB + Rx * Ry (SUFR)
					case 0xa2:      // Rn = MRB + Rx * Ry (USI)
					case 0xaa:      // Rn = MRB + Rx * Ry (USF)
					case 0xab:      // Rn = MRB + Rx * Ry (USFR)
					case 0xb2:      // Rn = MRB + Rx * Ry (SSI)
					case 0xba:      // Rn = MRB + Rx * Ry (SSF)
					case 0xbb:      // Rn = MRB + Rx * Ry (SSFR)
						// TODO: track MRB?
						desc.set_reg_used(rx);
						desc.set_reg_used(ry);
						desc.set_reg_modified(rn);
						desc.set_mult_flags_modified();
						break;

					case 0x84:      // MRF = MRF + Rx * Ry (UUI)
					case 0x8c:      // MRF = MRF + Rx * Ry (UUF)
					case 0x8d:      // MRF = MRF + Rx * Ry (UUFR)
					case 0x94:      // MRF = MRF + Rx * Ry (SUI)
					case 0x9c:      // MRF = MRF + Rx * Ry (SUF)
					case 0x9d:      // MRF = MRF + Rx * Ry (SUFR)
					case 0xa4:      // MRF = MRF + Rx * Ry (USI)
					case 0xac:      // MRF = MRF + Rx * Ry (USF)
					case 0xad:      // MRF = MRF + Rx * Ry (USFR)
					case 0xb4:      // MRF = MRF + Rx * Ry (SSI)
					case 0xbc:      // MRF = MRF + Rx * Ry (SSF)
					case 0xbd:      // MRF = MRF + Rx * Ry (SSFR)
						// TODO: track MRF?
						desc.set_reg_used(rx);
						desc.set_reg_used(ry);
						desc.set_mult_flags_modified();
						break;

					case 0x86:      // MRB = MRB + Rx * Ry (UUI)
					case 0x8e:      // MRB = MRB + Rx * Ry (UUF)
					case 0x8f:      // MRB = MRB + Rx * Ry (UUFR)
					case 0x96:      // MRB = MRB + Rx * Ry (SUI)
					case 0x9e:      // MRB = MRB + Rx * Ry (SUF)
					case 0x9f:      // MRB = MRB + Rx * Ry (SUFR)
					case 0xa6:      // MRB = MRB + Rx * Ry (USI)
					case 0xae:      // MRB = MRB + Rx * Ry (USF)
					case 0xaf:      // MRB = MRB + Rx * Ry (USFR)
					case 0xb6:      // MRB = MRB + Rx * Ry (SSI)
					case 0xbe:      // MRB = MRB + Rx * Ry (SSF)
					case 0xbf:      // MRB = MRB + Rx * Ry (SSFR)
						break;
						// TODO: track MRB?
						desc.set_reg_used(rx);
						desc.set_reg_used(ry);
						desc.set_mult_flags_modified();
						break;

					case 0xc0:      // Rn = MRF - Rx * Ry (UUI)
					case 0xc8:      // Rn = MRF - Rx * Ry (UUF)
					case 0xc9:      // Rn = MRF - Rx * Ry (UUFR)
					case 0xd0:      // Rn = MRF - Rx * Ry (SUI)
					case 0xd8:      // Rn = MRF - Rx * Ry (SUF)
					case 0xd9:      // Rn = MRF - Rx * Ry (SUFR)
					case 0xe0:      // Rn = MRF - Rx * Ry (USI)
					case 0xe8:      // Rn = MRF - Rx * Ry (USF)
					case 0xe9:      // Rn = MRF - Rx * Ry (USFR)
					case 0xf0:      // Rn = MRF - Rx * Ry (SSI)
					case 0xf8:      // Rn = MRF - Rx * Ry (SSF)
					case 0xf9:      // Rn = MRF - Rx * Ry (SSFR)
						// TODO: track MRF?
						desc.set_reg_used(rx);
						desc.set_reg_used(ry);
						desc.set_reg_modified(rn);
						desc.set_mult_flags_modified();
						break;

					case 0xc2:      // Rn = MRB - Rx * Ry (UUI)
					case 0xca:      // Rn = MRB - Rx * Ry (UUF)
					case 0xcb:      // Rn = MRB - Rx * Ry (UUFR)
					case 0xd2:      // Rn = MRB - Rx * Ry (SUI)
					case 0xda:      // Rn = MRB - Rx * Ry (SUF)
					case 0xdb:      // Rn = MRB - Rx * Ry (SUFR)
					case 0xe2:      // Rn = MRB - Rx * Ry (USI)
					case 0xea:      // Rn = MRB - Rx * Ry (USF)
					case 0xeb:      // Rn = MRB - Rx * Ry (USFR)
					case 0xf2:      // Rn = MRB - Rx * Ry (SSI)
					case 0xfa:      // Rn = MRB - Rx * Ry (SSF)
					case 0xfb:      // Rn = MRB - Rx * Ry (SSFR)
						// TODO: track MRB?
						desc.set_reg_used(rx);
						desc.set_reg_used(ry);
						desc.set_reg_modified(rn);
						desc.set_mult_flags_modified();
						break;

					case 0xc4:      // MRF = MRF - Rx * Ry (UUI)
					case 0xcc:      // MRF = MRF - Rx * Ry (UUF)
					case 0xcd:      // MRF = MRF - Rx * Ry (UUFR)
					case 0xd4:      // MRF = MRF - Rx * Ry (SUI)
					case 0xdc:      // MRF = MRF - Rx * Ry (SUF)
					case 0xdd:      // MRF = MRF - Rx * Ry (SUFR)
					case 0xe4:      // MRF = MRF - Rx * Ry (USI)
					case 0xec:      // MRF = MRF - Rx * Ry (USF)
					case 0xed:      // MRF = MRF - Rx * Ry (USFR)
					case 0xf4:      // MRF = MRF - Rx * Ry (SSI)
					case 0xfc:      // MRF = MRF - Rx * Ry (SSF)
					case 0xfd:      // MRF = MRF - Rx * Ry (SSFR)
						// TODO: track MRF?
						desc.set_reg_used(rx);
						desc.set_reg_used(ry);
						desc.set_mult_flags_modified();
						break;

					case 0xc6:      // MRB = MRB - Rx * Ry (UUI)
					case 0xce:      // MRB = MRB - Rx * Ry (UUF)
					case 0xcf:      // MRB = MRB - Rx * Ry (UUFR)
					case 0xd6:      // MRB = MRB - Rx * Ry (SUI)
					case 0xde:      // MRB = MRB - Rx * Ry (SUF)
					case 0xdf:      // MRB = MRB - Rx * Ry (SUFR)
					case 0xe6:      // MRB = MRB - Rx * Ry (USI)
					case 0xee:      // MRB = MRB - Rx * Ry (USF)
					case 0xef:      // MRB = MRB - Rx * Ry (USFR)
					case 0xf6:      // MRB = MRB - Rx * Ry (SSI)
					case 0xfe:      // MRB = MRB - Rx * Ry (SSF)
					case 0xff:      // MRB = MRB - Rx * Ry (SSFR)
						// TODO: track MRB?
						desc.set_reg_used(rx);
						desc.set_reg_used(ry);
						desc.set_mult_flags_modified();
						break;

					case 0x00:      // Rn = SAT MRF (UI)
					case 0x01:      // Rn = SAT MRF (SI)
					case 0x08:      // Rn = SAT MRF (UF)
					case 0x09:      // Rn = SAT MRF (SF)
						// TODO: track MRF?
						desc.set_reg_modified(rn);
						desc.set_mult_flags_modified();
						break;

					case 0x02:      // Rn = SAT MRB (UI)
					case 0x03:      // Rn = SAT MRB (SI)
					case 0x0a:      // Rn = SAT MRB (UF)
					case 0x0b:      // Rn = SAT MRB (SF)
						// TODO: track MRB?
						desc.set_reg_modified(rn);
						desc.set_mult_flags_modified();
						break;

					case 0x04:      // MRF = SAT MRF (UI)
					case 0x05:      // MRF = SAT MRF (SI)
					case 0x0c:      // MRF = SAT MRF (UF)
					case 0x0d:      // MRF = SAT MRF (SF)
						// TODO: track MRF?
						desc.set_mult_flags_modified();
						break;

					case 0x06:      // MRB = SAT MRB (UI)
					case 0x07:      // MRB = SAT MRB (SI)
					case 0x0e:      // MRB = SAT MRB (UF)
					case 0x0f:      // MRB = SAT MRB (SF)
						// TODO: track MRB?
						desc.set_mult_flags_modified();
						break;

					case 0x18:      // Rn = RND MRF (U)
					case 0x19:      // Rn = RND MRF (S)
						desc.set_reg_modified(rn);
						desc.set_mult_flags_modified();
						break;

					case 0x1a:      // Rn = RND MRB (U)
					case 0x1b:      // Rn = RND MRB (S)
						desc.set_reg_modified(rn);
						desc.set_mult_flags_modified();
						break;

					case 0x1c:      // MRF = RND MRF (U)
					case 0x1d:      // MRF = RND MRF (S)
						desc.set_mult_flags_modified();
						break;

					case 0x1e:      // MRB = RND MRB (U)
					case 0x1f:      // MRB = RND MRB (S)
						desc.set_mult_flags_modified();
						break;

					case 0x14:      // MRF = 0
						desc.set_mult_flags_modified();
						break;
					case 0x16:      // MRB = 0
						desc.set_mult_flags_modified();
						break;

					case 0x30:      // Fn = Fx * Fy
						desc.set_reg_used(rx);
						desc.set_reg_used(ry);
						desc.set_reg_modified(rn);
						desc.set_mult_flags_modified();
						break;

					default:
						fatalerror("adsp21062_device::frontend::describe_compute: unknown mult op %02X in opcode %04X%08X at %08X", operation, (uint16_t)(opcode >> 32), (uint32_t)(opcode), desc.pc);
				}
				break;
			}

			case 2:             // shifter operations
			{
				switch (operation)
				{
					case 0x00:      // Rn = LSHIFT Rx BY Ry | <data8>
					case 0x04:      // Rn = ASHIFT Rx BY Ry | <data8>
					case 0x08:      // Rn = ROT Rx BY Ry | <data8>
					case 0xc4:      // Rn = BCLR Rx BY Ry | <data8>
					case 0xc0:      // Rn = BSET Rx BY Ry | <data8>
					case 0x44:      // Rn = FDEP Rx BY Ry | <bit6>:<len6>
					case 0x4c:      // Rn = FDEP Rx BY Ry | <bit6>:<len6> (SE)
					case 0x40:      // Rn = FEXT Rx BY Ry | <bit6>:<len6>
					case 0x48:      // Rn = FEXT Rx BY Ry | <bit6>:<len6> (SE)
						desc.set_reg_used(rx);
						desc.set_reg_used(ry);
						desc.set_reg_modified(rn);
						desc.set_shift_flags_modified();
						break;

					case 0x20:      // Rn = Rn OR LSHIFT Rx BY Ry | <data8>
					case 0x24:      // Rn = Rn OR ASHIFT Rx BY Ry | <data8>
					case 0x64:      // Rn = Rn OR FDEP Rx BY Ry | <bit6>:<len6>
					case 0x6c:      // Rn = Rn OR FDEP Rx BY Ry | <bit6>:<len6> (SE)
						desc.set_reg_used(rx);
						desc.set_reg_used(ry);
						desc.set_reg_used(rn);
						desc.set_reg_modified(rn);
						desc.set_shift_flags_modified();
						break;

					case 0xcc:      // BTST Rx BY Ry | <data8>
						desc.set_reg_used(rx);
						desc.set_reg_used(ry);
						desc.set_shift_flags_modified();
						break;

					case 0x80:      // Rn = EXP Rx
					case 0x84:      // Rn = EXP Rx (EX)
					case 0x88:      // Rn = LEFTZ Rx
					case 0x8c:      // Rn = LEFTO Rx
					case 0x90:      // Rn = FPACK Fx
					case 0x94:      // Fn = FUNPACK Rx
						desc.set_reg_used(rx);
						desc.set_reg_modified(rn);
						desc.set_shift_flags_modified();
						break;

					default:
						fatalerror("adsp21062_device::frontend::describe_compute: unknown shift op %02X in opcode %04X%08X at %08X", operation, (uint16_t)(opcode >> 32), (uint32_t)(opcode), desc.pc);
				}
				break;
			}

			default:
				fatalerror("adsp21062_device::frontend::describe_compute: unknown operation type in opcode %04X%08X at %08X", (uint16_t)(opcode >> 32), (uint32_t)(opcode), desc.pc);
				return false;
		}
	}

	return true;
}

bool adsp21062_device::frontend::describe_ureg_access(opcode_desc &desc, int reg, UREG_ACCESS access)
{
	switch (reg)
	{
		case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06: case 0x07:
		case 0x08: case 0x09: case 0x0a: case 0x0b: case 0x0c: case 0x0d: case 0x0e: case 0x0f:
			if (access == UREG_READ)
				desc.set_reg_used(reg);
			else
				desc.set_reg_modified(reg);
			break;

		case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
			if (access == UREG_READ)
				desc.set_dm_i_used(reg & 7);
			else
				desc.set_dm_i_modified(reg & 7);
			break;
		case 0x18: case 0x19: case 0x1a: case 0x1b: case 0x1c: case 0x1d: case 0x1e: case 0x1f:
			if (access == UREG_READ)
				desc.set_pm_i_used(reg & 7);
			else
				desc.set_pm_i_modified(reg & 7);
			break;

		case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27:
			if (access == UREG_READ)
				desc.set_dm_m_used(reg & 7);
			else
				desc.set_dm_m_modified(reg & 7);
			break;
		case 0x28: case 0x29: case 0x2a: case 0x2b: case 0x2c: case 0x2d: case 0x2e: case 0x2f:
			if (access == UREG_READ)
				desc.set_pm_m_used(reg & 7);
			else
				desc.set_pm_m_modified(reg & 7);
			break;

		case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37:
			if (access == UREG_READ)
				desc.set_dm_l_used(reg & 7);
			else
				desc.set_dm_l_modified(reg & 7);
			break;
		case 0x38: case 0x39: case 0x3a: case 0x3b: case 0x3c: case 0x3d: case 0x3e: case 0x3f:
			if (access == UREG_READ)
				desc.set_pm_l_used(reg & 7);
			else
				desc.set_pm_l_modified(reg & 7);
			break;

		case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x46: case 0x47:
			if (access == UREG_READ)
				desc.set_dm_b_used(reg & 7);
			else
				desc.set_dm_b_modified(reg & 7);
			break;
		case 0x48: case 0x49: case 0x4a: case 0x4b: case 0x4c: case 0x4d: case 0x4e: case 0x4f:
			if (access == UREG_READ)
				desc.set_pm_b_used(reg & 7);
			else
				desc.set_pm_b_modified(reg & 7);
			break;

		case 0x60:      // FADDR
			break;
		case 0x61:      // DADDR
			break;
		case 0x63:      // PC
			break;
		case 0x64:      // PCSTK
			break;
		case 0x65:      // PCSTKP
			break;
		case 0x66:      // LADDR
			break;
		case 0x67:      // CURLCNTR
			break;
		case 0x68:      // LCNTR
			break;
		case 0x70:      // USTAT1
			break;
		case 0x71:      // USTAT2
			break;
		case 0x79:      // IRPTL
			break;
		case 0x7a:      // MODE1
			break;
		case 0x7b:      // MODE2
			break;

		case 0x7c:      // ASTAT
			if (access == UREG_READ)
			{
				desc.set_az_used();
				desc.set_an_used();
				desc.set_av_used();
				desc.set_ac_used();
				desc.set_as_used();
				desc.set_ai_used();
				desc.set_mn_used();
				desc.set_mv_used();
				desc.set_mu_used();
				desc.set_mi_used();
				desc.set_sv_used();
				desc.set_sz_used();
				desc.set_ss_used();
				desc.set_btf_used();
				desc.set_af_used();
			}
			else
			{
				desc.set_alu_flags_modified();
				desc.set_mult_flags_modified();
				desc.set_shift_flags_modified();
				desc.set_btf_modified();
				desc.set_af_modified();
			}
			break;

		case 0x7d:      // IMASK
			break;
		case 0x7e:      // STKY
			break;
		case 0x7f:      // IMASKP
			break;
		case 0xdb:      // PX
			break;
		case 0xdc:      // PX1
			break;
		case 0xdd:      // PX2
			break;

		default:
			fatalerror("adsp21062_device::frontend::describe_ureg_access: unknown UREG %02X", reg);
			return false;
	}

	return true;
}

bool adsp21062_device::frontend::describe_shiftop_imm(opcode_desc &desc, int shiftop, int rn, int rx)
{
	switch (shiftop)
	{
		case 0x00:      // LSHIFT Rx BY <data8>
		case 0x01:      // ASHIFT Rx BY <data8>
		case 0x02:      // ROT Rx BY <data8>
		case 0x10:      // FEXT Rx BY <data8>
		case 0x11:      // FDEP Rx BY <data8>
		case 0x12:      // FEXT Rx BY <bit6>:<len6> (SE)
		case 0x13:      // FDEP Rx BY <bit6>:<len6> (SE)
		case 0x30:      // BSET Rx BY <data8>
		case 0x31:      // BCLR Rx By <data8>
		case 0x32:      // BTGL Rx BY <data8>
			desc.set_reg_used(rx);
			desc.set_reg_modified(rn);
			desc.set_shift_flags_modified();
			break;

		case 0x08:      // Rn = Rn OR LSHIFT Rx BY <data8>
		case 0x19:      // Rn = Rn OR FDEP Rx BY <bit6>:<len6>
		case 0x1b:      // Rn = Rn OR FDEP Rx BY <bit6>:<len6> (SE)
			desc.set_reg_used(rx);
			desc.set_reg_used(rn);
			desc.set_reg_modified(rn);
			desc.set_shift_flags_modified();
			break;

		case 0x33:      // BTST Rx BY <data8>
			desc.set_reg_used(rx);
			desc.set_shift_flags_modified();
			break;

		default:
			fatalerror("adsp21062_device::frontend::describe_shiftop_imm: unknown op %02X at %08X", shiftop, desc.pc);
			return false;
	}

	return true;
}

void adsp21062_device::frontend::describe_if_condition(opcode_desc &desc, int condition)
{
	switch (condition)
	{
		case 0x00:  desc.set_az_used(); break;                     // EQ
		case 0x01:  desc.set_az_used(); desc.set_an_used(); break; // LT
		case 0x02:  desc.set_az_used(); desc.set_an_used(); break; // LE
		case 0x03:  desc.set_ac_used(); break;                     // AC
		case 0x04:  desc.set_av_used(); break;                     // AV
		case 0x05:  desc.set_mv_used(); break;                     // MV
		case 0x06:  desc.set_mn_used(); break;                     // MS
		case 0x07:  desc.set_sv_used(); break;                     // SV
		case 0x08:  desc.set_sz_used(); break;                     // SZ
		case 0x0d:  desc.set_btf_used(); break;                    // TF
		case 0x10:  desc.set_az_used(); break;                     // NOT EQUAL
		case 0x11:  desc.set_az_used(); desc.set_an_used(); break; // GE
		case 0x12:  desc.set_az_used(); desc.set_an_used(); break; // GT
		case 0x13:  desc.set_ac_used(); break;                     // NOT AC
		case 0x14:  desc.set_av_used(); break;                     // NOT AV
		case 0x15:  desc.set_mv_used(); break;                     // NOT MV
		case 0x16:  desc.set_mn_used(); break;                     // NOT MS
		case 0x17:  desc.set_sv_used(); break;                     // NOT SV
		case 0x18:  desc.set_sz_used(); break;                     // NOT SZ
		case 0x1d:  desc.set_btf_used(); break;                    // NOT TF
	}
}
