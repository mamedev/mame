// license:BSD-3-Clause
// copyright-holders:Ville Linde

/******************************************************************************

    Front-end for SHARC recompiler

******************************************************************************/

#include "emu.h"
#include "sharcfe.h"

#define REG_USED(desc,x)			do { (desc).regin[0] |= 1 << (x); } while(0)
#define REG_MODIFIED(desc,x)		do { (desc).regout[0] |= 1 << (x); } while(0)

#define AZ_USED(desc)				do { (desc).regin[0] |= 1 << 16; } while(0)
#define AZ_MODIFIED(desc)			do { (desc).regout[0] |= 1 << 16; } while(0)
#define AV_USED(desc)				do { (desc).regin[0] |= 1 << 17; } while(0)
#define AV_MODIFIED(desc)			do { (desc).regout[0] |= 1 << 17; } while(0)
#define AN_USED(desc)				do { (desc).regin[0] |= 1 << 18; } while(0)
#define AN_MODIFIED(desc)			do { (desc).regout[0] |= 1 << 18; } while(0)
#define AC_USED(desc)				do { (desc).regin[0] |= 1 << 19; } while(0)
#define AC_MODIFIED(desc)			do { (desc).regout[0] |= 1 << 19; } while(0)
#define AS_USED(desc)				do { (desc).regin[0] |= 1 << 20; } while(0)
#define AS_MODIFIED(desc)			do { (desc).regout[0] |= 1 << 20; } while(0)
#define AI_USED(desc)				do { (desc).regin[0] |= 1 << 21; } while(0)
#define AI_MODIFIED(desc)			do { (desc).regout[0] |= 1 << 21; } while(0)
#define MN_USED(desc)				do { (desc).regin[0] |= 1 << 22; } while(0)
#define MN_MODIFIED(desc)			do { (desc).regout[0] |= 1 << 22; } while(0)
#define MV_USED(desc)				do { (desc).regin[0] |= 1 << 23; } while(0)
#define MV_MODIFIED(desc)			do { (desc).regout[0] |= 1 << 23; } while(0)
#define MU_USED(desc)				do { (desc).regin[0] |= 1 << 24; } while(0)
#define MU_MODIFIED(desc)			do { (desc).regout[0] |= 1 << 24; } while(0)
#define MI_USED(desc)				do { (desc).regin[0] |= 1 << 25; } while(0)
#define MI_MODIFIED(desc)			do { (desc).regout[0] |= 1 << 25; } while(0)
#define SV_USED(desc)				do { (desc).regin[0] |= 1 << 26; } while(0)
#define SV_MODIFIED(desc)			do { (desc).regout[0] |= 1 << 26; } while(0)
#define SZ_USED(desc)				do { (desc).regin[0] |= 1 << 27; } while(0)
#define SZ_MODIFIED(desc)			do { (desc).regout[0] |= 1 << 27; } while(0)
#define SS_USED(desc)				do { (desc).regin[0] |= 1 << 28; } while(0)
#define SS_MODIFIED(desc)			do { (desc).regout[0] |= 1 << 28; } while(0)
#define BTF_USED(desc)				do { (desc).regin[0] |= 1 << 29; } while(0)
#define BTF_MODIFIED(desc)			do { (desc).regout[0] |= 1 << 29; } while(0)
#define AF_USED(desc)				do { (desc).regin[0] |= 1 << 30; } while(0)
#define AF_MODIFIED(desc)			do { (desc).regout[0] |= 1 << 30; } while(0)

#define ALU_FLAGS_MODIFIED(desc)	do { AZ_MODIFIED(desc);AN_MODIFIED(desc);AV_MODIFIED(desc);AC_MODIFIED(desc);AS_MODIFIED(desc);AI_MODIFIED(desc); } while(0)
#define MULT_FLAGS_MODIFIED(desc)	do { MN_MODIFIED(desc);MV_MODIFIED(desc);MU_MODIFIED(desc);MI_MODIFIED(desc); } while(0)
#define SHIFT_FLAGS_MODIFIED(desc)	do { SZ_MODIFIED(desc);SV_MODIFIED(desc);SS_MODIFIED(desc); } while(0)


#define PM_I_USED(desc,x)			do { (desc).regin[1] |= 1 << (x); } while(0)
#define PM_I_MODIFIED(desc,x)		do { (desc).regout[1] |= 1 << (x); } while(0)
#define PM_M_USED(desc,x)			do { (desc).regin[1] |= 1 << ((x) + 8); } while(0)
#define PM_M_MODIFIED(desc,x)		do { (desc).regout[1] |= 1 << ((x) + 8); } while(0)
#define PM_B_USED(desc,x)			do { (desc).regin[1] |= 1 << ((x) + 16); } while(0)
#define PM_B_MODIFIED(desc,x)		do { (desc).regout[1] |= 1 << ((x) + 16); } while(0)
#define PM_L_USED(desc,x)			do { (desc).regin[1] |= 1 << ((x) + 24); } while(0)
#define PM_L_MODIFIED(desc,x)		do { (desc).regout[1] |= 1 << ((x) + 24); } while(0)

#define DM_I_USED(desc,x)			do { (desc).regin[2] |= 1 << (x); } while(0)
#define DM_I_MODIFIED(desc,x)		do { (desc).regout[2] |= 1 << (x); } while(0)
#define DM_M_USED(desc,x)			do { (desc).regin[2] |= 1 << ((x) + 8); } while(0)
#define DM_M_MODIFIED(desc,x)		do { (desc).regout[2] |= 1 << ((x) + 8); } while(0)
#define DM_B_USED(desc,x)			do { (desc).regin[2] |= 1 << ((x) + 16); } while(0)
#define DM_B_MODIFIED(desc,x)		do { (desc).regout[2] |= 1 << ((x) + 16); } while(0)
#define DM_L_USED(desc,x)			do { (desc).regin[2] |= 1 << ((x) + 24); } while(0)
#define DM_L_MODIFIED(desc,x)		do { (desc).regout[2] |= 1 << ((x) + 24); } while(0)


sharc_frontend::sharc_frontend(adsp21062_device *sharc, UINT32 window_start, UINT32 window_end, UINT32 max_sequence)
	: drc_frontend(*sharc, window_start, window_end, max_sequence),
		m_sharc(sharc)
{
	m_loopmap = std::make_unique<LOOP_ENTRY[]>(0x20000);
}



void sharc_frontend::flush()
{
	LOOP_ENTRY* map = m_loopmap.get();

	memset(map, 0, sizeof(LOOP_ENTRY) * 0x20000);
}

void sharc_frontend::add_loop_entry(UINT32 pc, UINT8 type, UINT32 start_pc, UINT8 looptype, UINT8 condition)
{
	UINT32 l2 = pc >> 17;
	UINT32 l1 = pc & 0x1ffff;

	if (l2 != 0x1)
		fatalerror("sharc_frontend::add_loop_entry: PC = %08X", pc);

	LOOP_ENTRY* map = m_loopmap.get();
	UINT32 current_type = map[l1].entrytype;
	if (current_type & type)
	{
		// check for mismatch if the entry is already used
		if (map[l1].start_pc != start_pc ||
			map[l1].looptype != looptype ||
			map[l1].condition != condition)
		{
			fatalerror("sharc_frontend::add_loop_entry: existing entry does not match: start_pc %08X/%08X, looptype %02X/%02X, cond %02X/%02X", start_pc, map[l1].start_pc, looptype, map[l1].looptype, condition, map[l1].condition);
		}
	}

	current_type |= type;

	map[l1].entrytype = current_type;
	map[l1].looptype = looptype;
	map[l1].condition = condition;
	map[l1].start_pc = start_pc;
}

void sharc_frontend::insert_loop(const LOOP_DESCRIPTOR &loopdesc)
{
	add_loop_entry(loopdesc.start_pc, LOOP_ENTRY_START, loopdesc.start_pc, loopdesc.type, loopdesc.condition);
	add_loop_entry(loopdesc.end_pc, LOOP_ENTRY_EVALUATION, loopdesc.start_pc, loopdesc.type, loopdesc.condition);
	if (loopdesc.astat_check_pc != 0xffffffff)
		add_loop_entry(loopdesc.astat_check_pc, LOOP_ENTRY_ASTAT_CHECK, loopdesc.start_pc, loopdesc.type, loopdesc.condition);
}

bool sharc_frontend::is_loop_evaluation(UINT32 pc)
{
	UINT32 l2 = pc >> 17;
	UINT32 l1 = pc & 0x1ffff;

	if (l2 != 0x1)
		return false;

	LOOP_ENTRY* map = m_loopmap.get();
	if (map[l1].entrytype & LOOP_ENTRY_EVALUATION)
		return true;

	return false;
}

bool sharc_frontend::is_loop_start(UINT32 pc)
{
	UINT32 l2 = pc >> 17;
	UINT32 l1 = pc & 0x1ffff;

	if (l2 != 0x1)
		return false;

	LOOP_ENTRY* map = m_loopmap.get();
	if (map[l1].entrytype & LOOP_ENTRY_START)
		return true;

	return false;
}

bool sharc_frontend::is_astat_delay_check(UINT32 pc)
{
	UINT32 l2 = pc >> 17;
	UINT32 l1 = pc & 0x1ffff;

	if (l2 != 0x1)
		return false;

	LOOP_ENTRY* map = m_loopmap.get();
	if (map[l1].entrytype & LOOP_ENTRY_ASTAT_CHECK)
		return true;

	return false;
}


bool sharc_frontend::describe(opcode_desc &desc, const opcode_desc *prev)
{
	UINT64 opcode = desc.opptr.q[0] = m_sharc->pm_read48(desc.physpc);

	desc.length = 1;
	desc.cycles = 1;

	// handle looping
	if (is_astat_delay_check(desc.pc))
	{
		LOOP_ENTRY* map = m_loopmap.get();
		int index = desc.pc & 0x1ffff;

		if (map[index].looptype == LOOP_TYPE_CONDITIONAL)
		{
			UINT32 flags = m_sharc->do_condition_astat_bits(map[index].condition);
			if (flags & adsp21062_device::ASTAT_FLAGS::AZ) { desc.userflags |= OP_USERFLAG_ASTAT_DELAY_COPY_AZ; AZ_USED(desc); }
			if (flags & adsp21062_device::ASTAT_FLAGS::AN) { desc.userflags |= OP_USERFLAG_ASTAT_DELAY_COPY_AN; AN_USED(desc); }
			if (flags & adsp21062_device::ASTAT_FLAGS::AV) { desc.userflags |= OP_USERFLAG_ASTAT_DELAY_COPY_AV; AV_USED(desc); }
			if (flags & adsp21062_device::ASTAT_FLAGS::AC) { desc.userflags |= OP_USERFLAG_ASTAT_DELAY_COPY_AC; AC_USED(desc); }
			if (flags & adsp21062_device::ASTAT_FLAGS::MN) { desc.userflags |= OP_USERFLAG_ASTAT_DELAY_COPY_MN; MN_USED(desc); }
			if (flags & adsp21062_device::ASTAT_FLAGS::MV) { desc.userflags |= OP_USERFLAG_ASTAT_DELAY_COPY_MV; MV_USED(desc); }
			if (flags & adsp21062_device::ASTAT_FLAGS::SV) { desc.userflags |= OP_USERFLAG_ASTAT_DELAY_COPY_SV; SV_USED(desc); }
			if (flags & adsp21062_device::ASTAT_FLAGS::SZ) { desc.userflags |= OP_USERFLAG_ASTAT_DELAY_COPY_SZ; SZ_USED(desc); }
			if (flags & adsp21062_device::ASTAT_FLAGS::BTF) { desc.userflags |= OP_USERFLAG_ASTAT_DELAY_COPY_BTF; BTF_USED(desc); }
		}
	}

	if (is_loop_start(desc.pc))
	{
		desc.flags |= OPFLAG_IS_BRANCH_TARGET;
	}

	if (is_loop_evaluation(desc.pc))
	{
		LOOP_ENTRY* map = m_loopmap.get();
		int index = desc.pc & 0x1ffff;

		desc.flags |= OPFLAG_IS_CONDITIONAL_BRANCH;
		desc.targetpc = map[index].start_pc;
		if (map[index].looptype == LOOP_TYPE_COUNTER)
		{
			desc.userflags |= OP_USERFLAG_COUNTER_LOOP;
		}
		else if (map[index].looptype == LOOP_TYPE_CONDITIONAL)
		{
			desc.userflags |= OP_USERFLAG_COND_LOOP;
			desc.userflags |= (map[index].condition << 2) & OP_USERFLAG_COND_FIELD;
		}
	}


	switch ((opcode >> 45) & 7)
	{
		case 0:				// subops
		{
			UINT32 subop = (opcode >> 40) & 0x1f;
			switch (subop)
			{
				case 0x00:			// NOP / idle						|000|00000|
					if (opcode & U64(0x008000000000))
					{
						// IDLE
						desc.flags |= OPFLAG_END_SEQUENCE;
					}
					else
					{
						// NOP
					}
					break;				

				case 0x01:			// compute								|000|00001|
				{
					int cond = (opcode >> 33) & 0x1f;
					describe_if_condition(desc, cond);

					if (!describe_compute(desc, opcode))
						return false;
					break;
				}

				case 0x02:			// immediate shift						|000|00010|
				{
					int shiftop = (opcode >> 16) & 0x3f;
					int rn = (opcode >> 4) & 0xf;
					int rx = (opcode & 0xf);
					int cond = (opcode >> 33) & 0x1f;

					describe_if_condition(desc, cond);

					if (!describe_shiftop_imm(desc, shiftop, rn, rx))
						return false;
					break;
				}
					
				case 0x04:			// compute / modify						|000|00100|
				{
					int g = (opcode >> 38) & 0x1;
					int m = (opcode >> 27) & 0x7;
					int i = (opcode >> 30) & 0x7;
					int cond = (opcode >> 33) & 0x1f;

					describe_if_condition(desc, cond);

					if (!describe_compute(desc, opcode))
						return false;

					if (g)
					{
						// PM
						PM_I_USED(desc, i);
						PM_I_MODIFIED(desc, i);
						PM_M_USED(desc, m);
					}
					else
					{
						// DM
						DM_I_USED(desc, i);
						DM_I_MODIFIED(desc, i);
						DM_M_USED(desc, m);
					}
					break;
				}

				case 0x06:			// direct jump|call						|000|00110|
				{					
					int j = (opcode >> 26) & 0x1;
					int cond = (opcode >> 33) & 0x1f;
					UINT32 address = opcode & 0xffffff;

					if (m_sharc->if_condition_always_true(cond))
						desc.flags |= OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_END_SEQUENCE;
					else
						desc.flags |= OPFLAG_IS_CONDITIONAL_BRANCH;

					describe_if_condition(desc, cond);

					desc.targetpc = address;
					desc.delayslots = (j) ? 2 : 0;
					break;
				}

				case 0x07:			// direct jump|call						|000|00111|
				{
					int j = (opcode >> 26) & 0x1;
					int cond = (opcode >> 33) & 0x1f;
					UINT32 address = opcode & 0xffffff;

					if (m_sharc->if_condition_always_true(cond))
						desc.flags |= OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_END_SEQUENCE;
					else
						desc.flags |= OPFLAG_IS_CONDITIONAL_BRANCH;

					describe_if_condition(desc, cond);

					desc.targetpc = desc.pc + SIGN_EXTEND24(address);
					desc.delayslots = (j) ? 2 : 0;
					break;
				}

				case 0x08:			// indirect jump|call / compute			|000|01000|
				{					
					int j = (opcode >> 26) & 0x1;
					int pmi = (opcode >> 30) & 0x7;
					int pmm = (opcode >> 27) & 0x7;
					int cond = (opcode >> 33) & 0x1f;

					if (!describe_compute(desc, opcode))
						return false;

					if (m_sharc->if_condition_always_true(cond))
						desc.flags |= OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_END_SEQUENCE;
					else
						desc.flags |= OPFLAG_IS_CONDITIONAL_BRANCH;

					describe_if_condition(desc, cond);

					PM_I_USED(desc, pmi);
					PM_M_USED(desc, pmm);

					desc.targetpc = BRANCH_TARGET_DYNAMIC;
					desc.delayslots = (j) ? 2 : 0;
					break;
				}

				case 0x09:			// indirect jump|call / compute			|000|01001|
				{
					int j = (opcode >> 26) & 0x1;;
					int cond = (opcode >> 33) & 0x1f;
					
					if (!describe_compute(desc, opcode))
						return false;

					if (m_sharc->if_condition_always_true(cond))
						desc.flags |= OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_END_SEQUENCE;
					else
						desc.flags |= OPFLAG_IS_CONDITIONAL_BRANCH;

					describe_if_condition(desc, cond);

					desc.targetpc = desc.pc + SIGN_EXTEND6((opcode >> 27) & 0x3f);
					desc.delayslots = (j) ? 2 : 0;

					break;
				}

				case 0x0a:			// return from subroutine / compute		|000|01010|
				{
					int cond = (opcode >> 33) & 0x1f;
					int j = (opcode >> 26) & 0x1;

					if (!describe_compute(desc, opcode))
						return false;

					if (m_sharc->if_condition_always_true(cond))
						desc.flags |= OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_END_SEQUENCE;
					else
						desc.flags |= OPFLAG_IS_CONDITIONAL_BRANCH;

					describe_if_condition(desc, cond);

					desc.targetpc = BRANCH_TARGET_DYNAMIC;
					desc.delayslots = (j) ? 2 : 0;
					break;
				}

				case 0x0b:			// return from interrupt / compute		|000|01011|
				{
					int cond = (opcode >> 33) & 0x1f;
					int j = (opcode >> 26) & 0x1;

					if (!describe_compute(desc, opcode))
						return false;

					if (m_sharc->if_condition_always_true(cond))
						desc.flags |= OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_END_SEQUENCE;
					else
						desc.flags |= OPFLAG_IS_CONDITIONAL_BRANCH;

					describe_if_condition(desc, cond);

					desc.targetpc = BRANCH_TARGET_DYNAMIC;
					desc.delayslots = (j) ? 2 : 0;
					break;
				}

				case 0x0c:			// do until counter expired				|000|01100|
				{
					int offset = SIGN_EXTEND24(opcode & 0xffffff);

					LOOP_DESCRIPTOR loop;
					loop.start_pc = desc.pc + 1;
					loop.end_pc = desc.pc + offset;
					loop.astat_check_pc = 0xffffffff;
					loop.type = LOOP_TYPE_COUNTER;
					loop.condition = 0;

					insert_loop(loop);
					break;
				}

				case 0x0d:			// do until counter expired				|000|01101|
				{
					int ureg = (opcode >> 32) & 0xff;
					if (!describe_ureg_access(desc, ureg, UREG_READ))
						return false;

					int offset = SIGN_EXTEND24(opcode & 0xffffff);

					LOOP_DESCRIPTOR loop;
					loop.start_pc = desc.pc + 1;
					loop.end_pc = desc.pc + offset;
					loop.astat_check_pc = 0xffffffff;
					loop.type = LOOP_TYPE_COUNTER;
					loop.condition = 0;

					insert_loop(loop);
					break;
				}

				case 0x0e:			// do until								|000|01110|
				{
					int offset = SIGN_EXTEND24(opcode & 0xffffff);
					int cond = (opcode >> 33) & 0x1f;

					LOOP_DESCRIPTOR loop;
					loop.start_pc = desc.pc + 1;
					loop.end_pc = desc.pc + offset;
					loop.type = LOOP_TYPE_CONDITIONAL;
					loop.condition = cond;

					loop.astat_check_pc = loop.end_pc - 2;
					if (loop.astat_check_pc < loop.start_pc)
						fatalerror("describe_compute: conditional loop < 2 at %08X", desc.pc);

					insert_loop(loop);
					break;
				}

				case 0x0f:			// immediate data -> ureg				|000|01111|
				{
					int ureg = (opcode >> 32) & 0xff;
					if (!describe_ureg_access(desc, ureg, UREG_WRITE))
						return false;
					break;
				}

				case 0x10:			// ureg <-> DM|PM (direct)				|000|100|G|D|
				case 0x11:
				case 0x12:
				case 0x13:
				{
					int ureg = (opcode >> 32) & 0xff;
					int d = (opcode >> 40) & 1;
					if (d)
					{
						if (!describe_ureg_access(desc, ureg, UREG_READ))
							return false;
						desc.flags |= OPFLAG_WRITES_MEMORY;
					}
					else
					{
						if (!describe_ureg_access(desc, ureg, UREG_WRITE))
							return false;
						desc.flags |= OPFLAG_READS_MEMORY;
					}
					break;
				}

				case 0x14:			// system register bit manipulation		|000|10100|
				{
					int bop = (opcode >> 37) & 0x7;
					int sreg = (opcode >> 32) & 0xf;
					UINT32 data = (UINT32)(opcode);

					switch (bop)
					{
						case 0:		// SET
						case 1:		// CLEAR
						case 2:		// TOGGLE
							if (sreg == 0x7c)	// ASTAT
							{
								if (data & adsp21062_device::AZ)
								{
									AZ_USED(desc); AZ_MODIFIED(desc);
								}
								if (data & adsp21062_device::AV)
								{
									AV_USED(desc); AV_MODIFIED(desc);
								}
								if (data & adsp21062_device::AN)
								{
									AN_USED(desc); AN_MODIFIED(desc);
								}
								if (data & adsp21062_device::AC)
								{
									AC_USED(desc); AC_MODIFIED(desc);
								}
								if (data & adsp21062_device::AS)
								{
									AS_USED(desc); AS_MODIFIED(desc);
								}
								if (data & adsp21062_device::AI)
								{
									AI_USED(desc); AI_MODIFIED(desc);
								}
								if (data & adsp21062_device::MN)
								{
									MN_USED(desc); MN_MODIFIED(desc);
								}
								if (data & adsp21062_device::MV)
								{
									MV_USED(desc); MV_MODIFIED(desc);
								}
								if (data & adsp21062_device::MU)
								{
									MU_USED(desc); MU_MODIFIED(desc);
								}
								if (data & adsp21062_device::MI)
								{
									MI_USED(desc); MI_MODIFIED(desc);
								}
								if (data & adsp21062_device::SV)
								{
									SV_USED(desc); SV_MODIFIED(desc);
								}
								if (data & adsp21062_device::SZ)
								{
									SZ_USED(desc); SZ_MODIFIED(desc);
								}
								if (data & adsp21062_device::SS)
								{
									SS_USED(desc); SS_MODIFIED(desc);
								}
								if (data & adsp21062_device::BTF)
								{
									BTF_USED(desc); BTF_MODIFIED(desc);
								}
								if (data & adsp21062_device::AF)
								{
									AF_USED(desc); AF_MODIFIED(desc);
								}
							}
							break;

						case 4:		// TEST
						case 5:		// XOR
							BTF_MODIFIED(desc);
							break;

						default:
							fatalerror("sharc_frontend::describe: system reg bit manipulation %d", bop);
							return false;
					}
					break;
				}

				case 0x16:			// I register modify / bit-reverse		|000|10110|
				{
					if (opcode & U64(0x008000000000))	// bit reverse
					{
						fatalerror("sharc_frontend::describe: bit reverse unimplemented");
					}
					else			// modify
					{
						int g = (opcode >> 38) & 0x1;
						int i = (opcode >> 32) & 0x7;

						if (g)
							PM_I_USED(desc, i);
						else
							DM_I_USED(desc, i);
					}
					break;
				}

				case 0x17:			// push|pop stacks / flush cache		|000|10111|					
					break;

				case 0x18:			// cjump								|000|11000|
					fatalerror("sharc_frontend::describe: cjump unimplemented");
					break;

				case 0x19:			// rframe								|000|11001|
					fatalerror("sharc_frontend::describe: rframe unimplemented");
					break;
				
				default:
					fatalerror("sharc_frontend::describe: unknown subop %02X in opcode %04X%08X", subop, (UINT16)(opcode >> 32), (UINT32)(opcode));
					return false;
			}
			break;
		}

		case 1:				// compute / dreg <-> DM / dreg <-> PM									|001|
		{
			if (!describe_compute(desc, opcode))
				return false;

			int pm_dreg = (opcode >> 23) & 0xf;
			int pmm = (opcode >> 27) & 0x7;
			int pmi = (opcode >> 30) & 0x7;
			int dm_dreg = (opcode >> 33) & 0xf;
			int dmm = (opcode >> 38) & 0x7;
			int dmi = (opcode >> 41) & 0x7;
			int pmd = (opcode >> 37) & 0x1;
			int dmd = (opcode >> 44) & 0x1;

			PM_I_USED(desc, pmi);
			PM_I_MODIFIED(desc, pmi);
			PM_M_USED(desc, pmm);

			if (pmd)
			{
				REG_USED(desc, pm_dreg);
				desc.flags |= OPFLAG_WRITES_MEMORY;
			}
			else
			{
				REG_MODIFIED(desc, pm_dreg);
				desc.flags |= OPFLAG_READS_MEMORY;
			}

			DM_I_USED(desc, dmi);
			DM_I_MODIFIED(desc, dmi);
			DM_M_USED(desc, dmm);

			if (dmd)
			{
				REG_USED(desc, dm_dreg);
				desc.flags |= OPFLAG_WRITES_MEMORY;
			}
			else
			{
				REG_MODIFIED(desc, dm_dreg);
				desc.flags |= OPFLAG_READS_MEMORY;
			}
			break;
		}

		case 2:				// compute / ureg <-> DM|PM, register modify							|010|
		{
			if (!describe_compute(desc, opcode))
				return false;

			int i = (opcode >> 41) & 0x7;
			int m = (opcode >> 38) & 0x7;
			int cond = (opcode >> 33) & 0x1f;
			int g = (opcode >> 32) & 0x1;
			int d = (opcode >> 31) & 0x1;
			int ureg = (opcode >> 23) & 0xff;

			describe_if_condition(desc, cond);

			if (d)
			{
				if (!describe_ureg_access(desc, ureg, UREG_READ))
					return false;
				desc.flags |= OPFLAG_WRITES_MEMORY;
			}
			else
			{
				if (!describe_ureg_access(desc, ureg, UREG_WRITE))
					return false;
				desc.flags |= OPFLAG_READS_MEMORY;
			}

			if (g)
			{
				// PM
				PM_I_USED(desc, i);
				PM_M_USED(desc, m);
			}
			else
			{
				// DM
				DM_I_USED(desc, i);
				DM_M_USED(desc, m);
			}

			break;
		}

		case 3:
		{
			if (!describe_compute(desc, opcode))
				return false;

			if (opcode & U64(0x100000000000))	// compute / ureg <-> ureg							|011|1|
			{
				int src_ureg = (opcode >> 36) & 0xff;
				int dst_ureg = (opcode >> 23) & 0xff;
				int cond = (opcode >> 31) & 0x1f;

				describe_if_condition(desc, cond);

				if (!describe_ureg_access(desc, src_ureg, UREG_READ))
					return false;
				if (!describe_ureg_access(desc, dst_ureg, UREG_WRITE))
					return false;
			}
			else								// compute / dreg <-> DM|PM, immediate modify		|011|0|
			{
				int u = (opcode >> 38) & 0x1;
				int d = (opcode >> 39) & 0x1;
				int g = (opcode >> 40) & 0x1;
				int dreg = (opcode >> 23) & 0xf;
				int i = (opcode >> 41) & 0x7;
				int cond = (opcode >> 33) & 0x1f;

				describe_if_condition(desc, cond);

				if (d)
				{
					REG_USED(desc, dreg);
					desc.flags |= OPFLAG_WRITES_MEMORY;
				}
				else
				{
					REG_MODIFIED(desc, dreg);
					desc.flags |= OPFLAG_READS_MEMORY;
				}
				
				if (g)
				{
					// PM
					PM_I_USED(desc, i);

					if (u)	// post-modify with update
					{
						PM_I_MODIFIED(desc, i);
					}
				}
				else
				{
					// DM
					DM_I_USED(desc, i);

					if (u)	// post-modify with update
					{
						DM_I_MODIFIED(desc, i);
					}
				}
			}
			break;
		}

		case 4:
		{
			if (opcode & U64(0x100000000000))	// immediate data -> DM|PM							|100|1|
			{
				int i = (opcode >> 41) & 0x7;
				int m = (opcode >> 38) & 0x7;
				int g = (opcode >> 37) & 0x1;

				desc.flags |= OPFLAG_WRITES_MEMORY;

				if (g)
				{
					// PM
					PM_I_USED(desc, i);
					PM_I_MODIFIED(desc, i);
					PM_M_USED(desc, m);
				}
				else
				{
					// DM
					DM_I_USED(desc, i);
					DM_I_MODIFIED(desc, i);
					DM_M_USED(desc, m);
				}
			}
			else								// immediate shift / dreg <-> DM|PM					|100|0|
			{
				int i = (opcode >> 41) & 0x7;
				int m = (opcode >> 38) & 0x7;
				int g = (opcode >> 32) & 0x1;
				int d = (opcode >> 31) & 0x1;
				int dreg = (opcode >> 23) & 0xf;
				int shiftop = (opcode >> 16) & 0x3f;
				int rn = (opcode >> 4) & 0xf;
				int rx = (opcode & 0xf);
				int cond = (opcode >> 33) & 0x1f;

				describe_if_condition(desc, cond);

				if (!describe_shiftop_imm(desc, shiftop, rn, rx))
					return false;

				if (d)
				{
					REG_USED(desc, dreg);
					desc.flags |= OPFLAG_WRITES_MEMORY;
				}
				else
				{
					REG_MODIFIED(desc, dreg);
					desc.flags |= OPFLAG_READS_MEMORY;
				}

				if (g)
				{
					// PM
					PM_I_USED(desc, i);
					PM_I_MODIFIED(desc, i);
					PM_M_USED(desc, m);
				}
				else
				{
					// DM
					DM_I_USED(desc, i);
					DM_I_MODIFIED(desc, i);
					DM_M_USED(desc, m);
				}
			}
			break;
		}

		case 5:								// ureg <-> DM|PM (indirect)							|101|
		{
			int ureg = (opcode >> 32) & 0xff;
			int d = (opcode >> 40) & 1;
			int i = (opcode >> 41) & 0x7;
			int g = (opcode >> 44) & 1;

			if (d)
			{
				if (!describe_ureg_access(desc, ureg, UREG_READ))
					return false;
				desc.flags |= OPFLAG_WRITES_MEMORY;
			}
			else
			{
				if (!describe_ureg_access(desc, ureg, UREG_WRITE))
					return false;
				desc.flags |= OPFLAG_READS_MEMORY;
			}

			if (g)
				PM_I_USED(desc, i);
			else
				DM_I_USED(desc, i);			
			break;
		}

		case 6:								// indirect jump / compute / dreg <-> DM				|110|
		{
			int d = (opcode >> 44) & 0x1;
			int dmi = (opcode >> 41) & 0x7;
			int dmm = (opcode >> 38) & 0x7;
			int pmi = (opcode >> 30) & 0x7;
			int pmm = (opcode >> 27) & 0x7;
			int cond = (opcode >> 33) & 0x1f;
			int dreg = (opcode >> 23) & 0xf;

			if (!describe_compute(desc, opcode))
				return false;

			if (m_sharc->if_condition_always_true(cond))
				desc.flags |= OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_END_SEQUENCE;
			else
				desc.flags |= OPFLAG_IS_CONDITIONAL_BRANCH;

			describe_if_condition(desc, cond);

			PM_I_USED(desc, pmi);
			PM_M_USED(desc, pmm);
			DM_I_USED(desc, dmi);
			DM_I_MODIFIED(desc, dmi);
			DM_M_USED(desc, dmm);

			if (d)
			{
				REG_USED(desc, dreg);
				desc.flags |= OPFLAG_WRITES_MEMORY;
			}
			else
			{
				REG_MODIFIED(desc, dreg);
				desc.flags |= OPFLAG_READS_MEMORY;
			}

			desc.targetpc = BRANCH_TARGET_DYNAMIC;
			desc.delayslots = 0;
			break;
		}

		case 7:								// indirect jump / compute / dreg <-> DM				|111|
		{
			int d = (opcode >> 44) & 0x1;
			int dmi = (opcode >> 41) & 0x7;
			int dmm = (opcode >> 38) & 0x7;
			int cond = (opcode >> 33) & 0x1f;
			int dreg = (opcode >> 23) & 0xf;

			if (!describe_compute(desc, opcode))
				return false;

			if (m_sharc->if_condition_always_true(cond))
				desc.flags |= OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_END_SEQUENCE;
			else
				desc.flags |= OPFLAG_IS_CONDITIONAL_BRANCH;

			describe_if_condition(desc, cond);

			DM_I_USED(desc, dmi);
			DM_I_MODIFIED(desc, dmi);
			DM_M_USED(desc, dmm);

			if (d)
			{
				REG_USED(desc, dreg);
				desc.flags |= OPFLAG_WRITES_MEMORY;
			}
			else
			{
				REG_MODIFIED(desc, dreg);
				desc.flags |= OPFLAG_READS_MEMORY;
			}

			desc.targetpc = desc.pc + SIGN_EXTEND6((opcode >> 27) & 0x3f);
			desc.delayslots = 0;
			break;
		}
	}

	return true;
}

bool sharc_frontend::describe_compute(opcode_desc &desc, UINT64 opcode)
{
	// skip if no-op
	if ((opcode & 0x7fffff) == 0)
		return true;

	int rs = (opcode >> 12) & 0xf;
	int rn = (opcode >> 8) & 0xf;
	int ra = rn;
	int rx = (opcode >> 4) & 0xf;
	int ry = (opcode >> 0) & 0xf;

	if (opcode & 0x400000)		// multi-function operation
	{
		UINT32 multiop = (opcode >> 16) & 0x3f;
		int fm = rs;
		int fa = rn;
		int fxm = (opcode >> 6) & 0x3;          // registers 0 - 3
		int fym = ((opcode >> 4) & 0x3) + 4;    // registers 4 - 7
		int fxa = ((opcode >> 2) & 0x3) + 8;    // registers 8 - 11
		int fya = (opcode & 0x3) + 12;          // registers 12 - 15

		switch (multiop)
		{
			case 0x00:			// Rn = MRxx
				REG_MODIFIED(desc, rn);
				break;
			case 0x01:			// MRxx = Rn
				REG_USED(desc, rn);
				break;

			case 0x07:			// Ra = Rx + Ry,   Rs = Rx - Ry
			case 0x0f:			// Fa = Fx + Fy,   Fs = Fx - Fy
				REG_USED(desc, rx);
				REG_USED(desc, ry);
				REG_MODIFIED(desc, ra);
				REG_MODIFIED(desc, rs);
				ALU_FLAGS_MODIFIED(desc);
				break;

			case 0x04:			// Rm = R3-0 * R7-4 (SSFR),   Ra = R11-8 + R15-12
			case 0x05:			// Rm = R3-0 * R7-4 (SSFR),   Ra = R11-8 - R15-12
			case 0x06:			// Rm = R3-0 * R7-4 (SSFR),   Ra = (R11-8 + R15-12) / 2
				REG_USED(desc, fxm);
				REG_USED(desc, fym);
				REG_USED(desc, fxa);
				REG_USED(desc, fya);
				REG_MODIFIED(desc, fm);
				REG_MODIFIED(desc, fa);
				ALU_FLAGS_MODIFIED(desc);
				MULT_FLAGS_MODIFIED(desc);
				break;

			case 0x08:			// MRF = MRF + R3-0 * R7-4 (SSF),   Ra = R11-8 + R15-12
			case 0x09:			// MRF = MRF + R3-0 * R7-4 (SSF),   Ra = R11-8 - R15-12
			case 0x0a:			// MRF = MRF + R3-0 * R7-4 (SSF),   Ra = (R11-8 + R15-12) / 2
				REG_USED(desc, fxm);
				REG_USED(desc, fym);
				REG_USED(desc, fxa);
				REG_USED(desc, fya);
				ALU_FLAGS_MODIFIED(desc);
				MULT_FLAGS_MODIFIED(desc);
				break;

			case 0x0c:			// Rm = MRF + R3-0 * R7-4 (SSFR),   Ra = R11-8 + R15-12
			case 0x0d:			// Rm = MRF + R3-0 * R7-4 (SSFR),   Ra = R11-8 - R15-12
			case 0x0e:			// Rm = MRF + R3-0 * R7-4 (SSFR),   Ra = (R11-8 + R15-12) / 2
				REG_USED(desc, fxm);
				REG_USED(desc, fym);
				REG_USED(desc, fxa);
				REG_USED(desc, fya);
				REG_MODIFIED(desc, fm);
				REG_MODIFIED(desc, fa);
				ALU_FLAGS_MODIFIED(desc);
				MULT_FLAGS_MODIFIED(desc);
				break;

			case 0x10:			// MRF = MRF - R3-0 * R7-4 (SSF),   Ra = R11-8 + R15-12
			case 0x11:			// MRF = MRF - R3-0 * R7-4 (SSF),   Ra = R11-8 - R15-12
			case 0x12:			// MRF = MRF - R3-0 * R7-4 (SSF),   Ra = (R11-8 + R15-12) / 2
				REG_USED(desc, fxm);
				REG_USED(desc, fym);
				REG_USED(desc, fxa);
				REG_USED(desc, fya);
				ALU_FLAGS_MODIFIED(desc);
				MULT_FLAGS_MODIFIED(desc);
				break;

			case 0x14:			// Rm = MRF - R3-0 * R7-4 (SSFR),   Ra = R11-8 + R15-12
			case 0x15:			// Rm = MRF - R3-0 * R7-4 (SSFR),   Ra = R11-8 - R15-12
			case 0x16:			// Rm = MRF - R3-0 * R7-4 (SSFR),   Ra = (R11-8 + R15-12) / 2
				REG_USED(desc, fxm);
				REG_USED(desc, fym);
				REG_USED(desc, fxa);
				REG_USED(desc, fya);
				REG_MODIFIED(desc, fm);
				REG_MODIFIED(desc, fa);
				ALU_FLAGS_MODIFIED(desc);
				MULT_FLAGS_MODIFIED(desc);
				break;

			case 0x18:			// Fm = F3-0 * F7-4,   Fa = F11-8 + F15-12
			case 0x19:			// Fm = F3-0 * F7-4,   Fa = F11-8 - F15-12
			case 0x1a:			// Fm = F3-0 * F7-4,   Fa = FLOAT F11-8 BY R15-12
			case 0x1b:			// Fm = F3-0 * F7-4,   Fa = FIX F11-8 BY R15-12
			case 0x1c:			// Fm = F3-0 * F7-4,   Fa = (F11-8 + F15-12) / 2
			case 0x1e:			// Fm = F3-0 * F7-4,   Fa = MAX(F11-8, F15-12)
			case 0x1f:			// Fm = F3-0 * F7-4,   Fa = MIN(F11-8, F15-12)
				REG_USED(desc, fxm);
				REG_USED(desc, fym);
				REG_USED(desc, fxa);
				REG_USED(desc, fya);
				REG_MODIFIED(desc, fm);
				REG_MODIFIED(desc, fa);
				ALU_FLAGS_MODIFIED(desc);
				MULT_FLAGS_MODIFIED(desc);
				break;

			case 0x1d:			// Fm = F3-0 * F7-4,   Fa = ABS F11-8
				REG_USED(desc, fxm);
				REG_USED(desc, fym);
				REG_USED(desc, fxa);
				REG_MODIFIED(desc, fm);
				REG_MODIFIED(desc, fa);
				ALU_FLAGS_MODIFIED(desc);
				MULT_FLAGS_MODIFIED(desc);
				break;

			case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27:
			case 0x28: case 0x29: case 0x2a: case 0x2b: case 0x2c: case 0x2d: case 0x2e: case 0x2f:
								// Rm = R3-0 * R7-4 (SSFR),   Ra = R11-8 + R15-12,   Rs = R11-8 - R15-12
				REG_USED(desc, fxm);
				REG_USED(desc, fym);
				REG_USED(desc, fxa);
				REG_USED(desc, fya);
				REG_MODIFIED(desc, fm);
				REG_MODIFIED(desc, fa);
				REG_MODIFIED(desc, (opcode >> 16) & 0xf);
				ALU_FLAGS_MODIFIED(desc);
				MULT_FLAGS_MODIFIED(desc);
				break;

			case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37:
			case 0x38: case 0x39: case 0x3a: case 0x3b: case 0x3c: case 0x3d: case 0x3e: case 0x3f:
								// Fm = F3-0 * F7-4,   Fa = F11-8 + F15-12,   Fs = F11-8 - F15-12
				REG_USED(desc, fxm);
				REG_USED(desc, fym);
				REG_USED(desc, fxa);
				REG_USED(desc, fya);
				REG_MODIFIED(desc, fm);
				REG_MODIFIED(desc, fa);
				REG_MODIFIED(desc, (opcode >> 16) & 0xf);
				ALU_FLAGS_MODIFIED(desc);
				MULT_FLAGS_MODIFIED(desc);
				break;

			default:
				fatalerror("sharc_frontend::describe_compute: unknown multiop %02X in opcode %04X%08X at %08X", multiop, (UINT16)(opcode >> 32), (UINT32)(opcode), desc.pc);
				return false;
		}
	}
	else							// single-function operation
	{
		UINT32 operation = (opcode >> 12) & 0xff;

		switch ((opcode >> 20) & 3)
		{
			case 0:				// ALU operations
			{
				switch (operation)
				{
					case 0x01:		// Rn = Rx + Ry
					case 0x02:		// Rn = Rx - Ry
					case 0x09:		// Rn = (Rx + Ry) / 2
					case 0x40:		// Rn = Rx AND Ry						
					case 0x41:		// Rn = Rx OR Ry
					case 0x42:		// Rn = Rx XOR Ry
					case 0x61:		// Rn = MIN(Rx, Ry)
					case 0x62:		// Rn = MAX(Rx, Ry)
					case 0x63:		// Rn = CLIP Rx BY Ry
					case 0x81:		// Fn = Fx + Fy
					case 0x82:		// Fn = Fx - Fy
					case 0x91:		// Fn = ABS(Fx + Fy)
					case 0x92:		// Fn = ABS(Fx - Fy)
					case 0x89:		// Fn = (Fx + Fy) / 2
					case 0xbd:		// Fn = SCALB Fx BY Ry
					case 0xd9:		// Rn = FIX Fx BY Ry
					case 0xdd:		// Rn = TRUNC Fx BY Ry
					case 0xda:		// Fn = FLOAT Rx BY Ry
					case 0xe1:		// Fn = MIN(Fx, Fy)
					case 0xe2:		// Fn = MAX(Fx, Fy)
					case 0xe3:		// Fn = CLIP Fx BY Fy
					case 0xe0:		// Fn = Fx COPYSIGN Fy
						REG_USED(desc, rx);
						REG_USED(desc, ry);
						REG_MODIFIED(desc, rn);
						ALU_FLAGS_MODIFIED(desc);
						break;

					case 0x05:		// Rn = Rx + Ry + CI
					case 0x06:		// Rn = Rx - Ry + CI - 1
					case 0x25:		// Rn = Rx + CI
					case 0x26:		// Rn = Rx + CI - 1
						REG_USED(desc, rx);
						REG_USED(desc, ry);
						REG_MODIFIED(desc, rn);
						AC_USED(desc);
						ALU_FLAGS_MODIFIED(desc);
						break;

					case 0x0a:		// COMP(Rx, Ry)
					case 0x8a:		// COMP(Fx, Fy)
						REG_USED(desc, rx);
						REG_USED(desc, ry);
						ALU_FLAGS_MODIFIED(desc);
						break;

					case 0x29:		// Rn = Rx + 1
					case 0x2a:		// Rn = Rx - 1
					case 0x22:		// Rn = -Rx
					case 0x30:		// Rn = ABS Rx
					case 0x21:		// Rn = PASS Rx
					case 0x43:		// Rn = NOT Rx
					case 0xb0:		// Fn = ABS(Fx)
					case 0xa1:		// Fn = PASS Fx
					case 0xa2:		// Fn = -Fx
					case 0xa5:		// Fn = RND Fx
					case 0xad:		// Rn = MANT Fx
					case 0xc1:		// Rn = LOGB Fx
					case 0xc9:		// Rn = FIX Fx
					case 0xcd:		// Rn = TRUNC Fx
					case 0xca:		// Fn = FLOAT Rx
					case 0xc4:		// Fn = RECIPS Fx
					case 0xc5:		// Fn = RSQRTS Fx
						REG_USED(desc, rx);
						REG_MODIFIED(desc, rn);
						ALU_FLAGS_MODIFIED(desc);
						break;

					case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77:
					case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7e: case 0x7f:
					{
						/* Fixed-point Dual Add/Subtract */
						REG_USED(desc, rx);
						REG_USED(desc, ry);
						REG_MODIFIED(desc, rn);
						REG_MODIFIED(desc, ra);
						ALU_FLAGS_MODIFIED(desc);
						break;
					}

					case 0xf0: case 0xf1: case 0xf2: case 0xf3: case 0xf4: case 0xf5: case 0xf6: case 0xf7:
					case 0xf8: case 0xf9: case 0xfa: case 0xfb: case 0xfc: case 0xfd: case 0xfe: case 0xff:
					{
						/* Floating-point Dual Add/Subtract */
						REG_USED(desc, rx);
						REG_USED(desc, ry);
						REG_MODIFIED(desc, rn);
						REG_MODIFIED(desc, ra);
						ALU_FLAGS_MODIFIED(desc);
						break;
					}

					default:
						fatalerror("sharc_frontend::describe_compute: unknown ALU op %02X in opcode %04X%08X at %08X", operation, (UINT16)(opcode >> 32), (UINT32)(opcode), desc.pc);
						return false;
				}
				break;
			}

			case 1:				// multiplier operations
			{
				switch (operation)
				{
					case 0x40:		// Rn = Rx * Ry (UUI)
					case 0x48:		// Rn = Rx * Ry (UUF)
					case 0x49:		// Rn = Rx * Ry (UUFR)
					case 0x50:		// Rn = Rx * Ry (SUI)
					case 0x58:		// Rn = Rx * Ry (SUF)
					case 0x59:		// Rn = Rx * Ry (SUFR)
					case 0x60:		// Rn = Rx * Ry (USI)
					case 0x68:		// Rn = Rx * Ry (USF)
					case 0x69:		// Rn = Rx * Ry (USFR)
					case 0x70:		// Rn = Rx * Ry (SSI)
					case 0x78:		// Rn = Rx * Ry (SSF)
					case 0x79:		// Rn = Rx * Ry (SSFR)
						REG_USED(desc, rx);
						REG_USED(desc, ry);
						REG_MODIFIED(desc, rn);
						MULT_FLAGS_MODIFIED(desc);
						break;

					case 0x44:		// MRF = Rx * Ry (UUI)
					case 0x4c:		// MRF = Rx * Ry (UUF)
					case 0x4d:		// MRF = Rx * Ry (UUFR)
					case 0x54:		// MRF = Rx * Ry (SUI)
					case 0x5c:		// MRF = Rx * Ry (SUF)
					case 0x5d:		// MRF = Rx * Ry (SUFR)
					case 0x64:		// MRF = Rx * Ry (USI)
					case 0x6c:		// MRF = Rx * Ry (USF)
					case 0x6d:		// MRF = Rx * Ry (USFR)
					case 0x74:		// MRF = Rx * Ry (SSI)
					case 0x7c:		// MRF = Rx * Ry (SSF)
					case 0x7d:		// MRF = Rx * Ry (SSFR)
						// TODO: track MRF?
						REG_USED(desc, rx);
						REG_USED(desc, ry);
						MULT_FLAGS_MODIFIED(desc);
						break;

					case 0x46:		// MRB = Rx * Ry (UUI)
					case 0x4e:		// MRB = Rx * Ry (UUF)
					case 0x4f:		// MRB = Rx * Ry (UUFR)
					case 0x56:		// MRB = Rx * Ry (SUI)
					case 0x5e:		// MRB = Rx * Ry (SUF)
					case 0x5f:		// MRB = Rx * Ry (SUFR)
					case 0x66:		// MRB = Rx * Ry (USI)
					case 0x6e:		// MRB = Rx * Ry (USF)
					case 0x6f:		// MRB = Rx * Ry (USFR)
					case 0x76:		// MRB = Rx * Ry (SSI)
					case 0x7e:		// MRB = Rx * Ry (SSF)
					case 0x7f:		// MRB = Rx * Ry (SSFR)
						// TODO: track MRB?
						REG_USED(desc, rx);
						REG_USED(desc, ry);
						MULT_FLAGS_MODIFIED(desc);
						break;

					case 0x80:		// Rn = MRF + Rx * Ry (UUI)
					case 0x88:		// Rn = MRF + Rx * Ry (UUF)
					case 0x89:		// Rn = MRF + Rx * Ry (UUFR)
					case 0x90:		// Rn = MRF + Rx * Ry (SUI)
					case 0x98:		// Rn = MRF + Rx * Ry (SUF)
					case 0x99:		// Rn = MRF + Rx * Ry (SUFR)
					case 0xa0:		// Rn = MRF + Rx * Ry (USI)
					case 0xa8:		// Rn = MRF + Rx * Ry (USF)
					case 0xa9:		// Rn = MRF + Rx * Ry (USFR)
					case 0xb0:		// Rn = MRF + Rx * Ry (SSI)
					case 0xb8:		// Rn = MRF + Rx * Ry (SSF)
					case 0xb9:		// Rn = MRF + Rx * Ry (SSFR)
						// TODO: track MRF?
						REG_USED(desc, rx);
						REG_USED(desc, ry);
						REG_MODIFIED(desc, rn);
						MULT_FLAGS_MODIFIED(desc);
						break;

					case 0x82:		// Rn = MRB + Rx * Ry (UUI)
					case 0x8a:		// Rn = MRB + Rx * Ry (UUF)
					case 0x8b:		// Rn = MRB + Rx * Ry (UUFR)
					case 0x92:		// Rn = MRB + Rx * Ry (SUI)
					case 0x9a:		// Rn = MRB + Rx * Ry (SUF)
					case 0x9b:		// Rn = MRB + Rx * Ry (SUFR)
					case 0xa2:		// Rn = MRB + Rx * Ry (USI)
					case 0xaa:		// Rn = MRB + Rx * Ry (USF)
					case 0xab:		// Rn = MRB + Rx * Ry (USFR)
					case 0xb2:		// Rn = MRB + Rx * Ry (SSI)
					case 0xba:		// Rn = MRB + Rx * Ry (SSF)
					case 0xbb:		// Rn = MRB + Rx * Ry (SSFR)
						// TODO: track MRB?
						REG_USED(desc, rx);
						REG_USED(desc, ry);
						REG_MODIFIED(desc, rn);
						MULT_FLAGS_MODIFIED(desc);
						break;

					case 0x84:		// MRF = MRF + Rx * Ry (UUI)
					case 0x8c:		// MRF = MRF + Rx * Ry (UUF)
					case 0x8d:		// MRF = MRF + Rx * Ry (UUFR)
					case 0x94:		// MRF = MRF + Rx * Ry (SUI)
					case 0x9c:		// MRF = MRF + Rx * Ry (SUF)
					case 0x9d:		// MRF = MRF + Rx * Ry (SUFR)
					case 0xa4:		// MRF = MRF + Rx * Ry (USI)
					case 0xac:		// MRF = MRF + Rx * Ry (USF)
					case 0xad:		// MRF = MRF + Rx * Ry (USFR)
					case 0xb4:		// MRF = MRF + Rx * Ry (SSI)
					case 0xbc:		// MRF = MRF + Rx * Ry (SSF)
					case 0xbd:		// MRF = MRF + Rx * Ry (SSFR)
						// TODO: track MRF?
						REG_USED(desc, rx);
						REG_USED(desc, ry);
						MULT_FLAGS_MODIFIED(desc);
						break;

					case 0x86:		// MRB = MRB + Rx * Ry (UUI)
					case 0x8e:		// MRB = MRB + Rx * Ry (UUF)
					case 0x8f:		// MRB = MRB + Rx * Ry (UUFR)
					case 0x96:		// MRB = MRB + Rx * Ry (SUI)
					case 0x9e:		// MRB = MRB + Rx * Ry (SUF)
					case 0x9f:		// MRB = MRB + Rx * Ry (SUFR)
					case 0xa6:		// MRB = MRB + Rx * Ry (USI)
					case 0xae:		// MRB = MRB + Rx * Ry (USF)
					case 0xaf:		// MRB = MRB + Rx * Ry (USFR)
					case 0xb6:		// MRB = MRB + Rx * Ry (SSI)
					case 0xbe:		// MRB = MRB + Rx * Ry (SSF)
					case 0xbf:		// MRB = MRB + Rx * Ry (SSFR)
						break;
						// TODO: track MRB?
						REG_USED(desc, rx);
						REG_USED(desc, ry);
						MULT_FLAGS_MODIFIED(desc);
						break;
					
					case 0xc0:		// Rn = MRF - Rx * Ry (UUI)
					case 0xc8:		// Rn = MRF - Rx * Ry (UUF)
					case 0xc9:		// Rn = MRF - Rx * Ry (UUFR)
					case 0xd0:		// Rn = MRF - Rx * Ry (SUI)
					case 0xd8:		// Rn = MRF - Rx * Ry (SUF)
					case 0xd9:		// Rn = MRF - Rx * Ry (SUFR)
					case 0xe0:		// Rn = MRF - Rx * Ry (USI)
					case 0xe8:		// Rn = MRF - Rx * Ry (USF)
					case 0xe9:		// Rn = MRF - Rx * Ry (USFR)
					case 0xf0:		// Rn = MRF - Rx * Ry (SSI)
					case 0xf8:		// Rn = MRF - Rx * Ry (SSF)
					case 0xf9:		// Rn = MRF - Rx * Ry (SSFR)
						// TODO: track MRF?
						REG_USED(desc, rx);
						REG_USED(desc, ry);
						REG_MODIFIED(desc, rn);
						MULT_FLAGS_MODIFIED(desc);
						break;

					case 0xc2:		// Rn = MRB - Rx * Ry (UUI)
					case 0xca:		// Rn = MRB - Rx * Ry (UUF)
					case 0xcb:		// Rn = MRB - Rx * Ry (UUFR)
					case 0xd2:		// Rn = MRB - Rx * Ry (SUI)
					case 0xda:		// Rn = MRB - Rx * Ry (SUF)
					case 0xdb:		// Rn = MRB - Rx * Ry (SUFR)
					case 0xe2:		// Rn = MRB - Rx * Ry (USI)
					case 0xea:		// Rn = MRB - Rx * Ry (USF)
					case 0xeb:		// Rn = MRB - Rx * Ry (USFR)
					case 0xf2:		// Rn = MRB - Rx * Ry (SSI)
					case 0xfa:		// Rn = MRB - Rx * Ry (SSF)
					case 0xfb:		// Rn = MRB - Rx * Ry (SSFR)
						// TODO: track MRB?
						REG_USED(desc, rx);
						REG_USED(desc, ry);
						REG_MODIFIED(desc, rn);
						MULT_FLAGS_MODIFIED(desc);
						break;

					case 0xc4:		// MRF = MRF - Rx * Ry (UUI)
					case 0xcc:		// MRF = MRF - Rx * Ry (UUF)
					case 0xcd:		// MRF = MRF - Rx * Ry (UUFR)
					case 0xd4:		// MRF = MRF - Rx * Ry (SUI)
					case 0xdc:		// MRF = MRF - Rx * Ry (SUF)
					case 0xdd:		// MRF = MRF - Rx * Ry (SUFR)
					case 0xe4:		// MRF = MRF - Rx * Ry (USI)
					case 0xec:		// MRF = MRF - Rx * Ry (USF)
					case 0xed:		// MRF = MRF - Rx * Ry (USFR)
					case 0xf4:		// MRF = MRF - Rx * Ry (SSI)
					case 0xfc:		// MRF = MRF - Rx * Ry (SSF)
					case 0xfd:		// MRF = MRF - Rx * Ry (SSFR)
						// TODO: track MRF?
						REG_USED(desc, rx);
						REG_USED(desc, ry);
						MULT_FLAGS_MODIFIED(desc);
						break;

					case 0xc6:		// MRB = MRB - Rx * Ry (UUI)
					case 0xce:		// MRB = MRB - Rx * Ry (UUF)
					case 0xcf:		// MRB = MRB - Rx * Ry (UUFR)
					case 0xd6:		// MRB = MRB - Rx * Ry (SUI)
					case 0xde:		// MRB = MRB - Rx * Ry (SUF)
					case 0xdf:		// MRB = MRB - Rx * Ry (SUFR)
					case 0xe6:		// MRB = MRB - Rx * Ry (USI)
					case 0xee:		// MRB = MRB - Rx * Ry (USF)
					case 0xef:		// MRB = MRB - Rx * Ry (USFR)
					case 0xf6:		// MRB = MRB - Rx * Ry (SSI)
					case 0xfe:		// MRB = MRB - Rx * Ry (SSF)
					case 0xff:		// MRB = MRB - Rx * Ry (SSFR)
						// TODO: track MRB?
						REG_USED(desc, rx);
						REG_USED(desc, ry);
						MULT_FLAGS_MODIFIED(desc);
						break;
					
					case 0x00:		// Rn = SAT MRF (UI)
					case 0x01:		// Rn = SAT MRF (SI)
					case 0x08:		// Rn = SAT MRF (UF)
					case 0x09:		// Rn = SAT MRF (SF)
						// TODO: track MRF?
						REG_MODIFIED(desc, rn);
						MULT_FLAGS_MODIFIED(desc);
						break;

					case 0x02:		// Rn = SAT MRB (UI)
					case 0x03:		// Rn = SAT MRB (SI)
					case 0x0a:		// Rn = SAT MRB (UF)
					case 0x0b:		// Rn = SAT MRB (SF)
						// TODO: track MRB?
						REG_MODIFIED(desc, rn);
						MULT_FLAGS_MODIFIED(desc);
						break;

					case 0x04:		// MRF = SAT MRF (UI)
					case 0x05:		// MRF = SAT MRF (SI)
					case 0x0c:		// MRF = SAT MRF (UF)
					case 0x0d:		// MRF = SAT MRF (SF)
						// TODO: track MRF?
						MULT_FLAGS_MODIFIED(desc);
						break;

					case 0x06:		// MRB = SAT MRB (UI)
					case 0x07:		// MRB = SAT MRB (SI)
					case 0x0e:		// MRB = SAT MRB (UF)
					case 0x0f:		// MRB = SAT MRB (SF)
						// TODO: track MRB?
						MULT_FLAGS_MODIFIED(desc);
						break;

					case 0x18:		// Rn = RND MRF (U)
					case 0x19:		// Rn = RND MRF (S)
						REG_MODIFIED(desc, rn);
						MULT_FLAGS_MODIFIED(desc);
						break;

					case 0x1a:		// Rn = RND MRB (U)
					case 0x1b:		// Rn = RND MRB (S)
						REG_MODIFIED(desc, rn);
						MULT_FLAGS_MODIFIED(desc);
						break;

					case 0x1c:		// MRF = RND MRF (U)
					case 0x1d:		// MRF = RND MRF (S)
						MULT_FLAGS_MODIFIED(desc);
						break;

					case 0x1e:		// MRB = RND MRB (U)
					case 0x1f:		// MRB = RND MRB (S)
						MULT_FLAGS_MODIFIED(desc);
						break;

					case 0x14:		// MRF = 0
						MULT_FLAGS_MODIFIED(desc);
						break;
					case 0x16:		// MRB = 0
						MULT_FLAGS_MODIFIED(desc);
						break;

					case 0x30:		// Fn = Fx * Fy
						REG_USED(desc, rx);
						REG_USED(desc, ry);
						REG_MODIFIED(desc, rn);
						MULT_FLAGS_MODIFIED(desc);
						break;

					default:
						fatalerror("sharc_frontend::describe_compute: unknown mult op %02X in opcode %04X%08X at %08X", operation, (UINT16)(opcode >> 32), (UINT32)(opcode), desc.pc);
				}
				break;
			}

			case 2:				// shifter operations
			{
				switch (operation)
				{
					case 0x00:		// Rn = LSHIFT Rx BY Ry | <data8>
					case 0x04:		// Rn = ASHIFT Rx BY Ry | <data8>
					case 0x08:		// Rn = ROT Rx BY Ry | <data8>
					case 0xc4:		// Rn = BCLR Rx BY Ry | <data8>
					case 0xc0:		// Rn = BSET Rx BY Ry | <data8>
					case 0x44:		// Rn = FDEP Rx BY Ry | <bit6>:<len6>
					case 0x4c:		// Rn = FDEP Rx BY Ry | <bit6>:<len6> (SE)
					case 0x40:		// Rn = FEXT Rx BY Ry | <bit6>:<len6>
					case 0x48:		// Rn = FEXT Rx BY Ry | <bit6>:<len6> (SE)
						REG_USED(desc, rx);
						REG_USED(desc, ry);
						REG_MODIFIED(desc, rn);
						SHIFT_FLAGS_MODIFIED(desc);
						break;

					case 0x20:		// Rn = Rn OR LSHIFT Rx BY Ry | <data8>
					case 0x24:		// Rn = Rn OR ASHIFT Rx BY Ry | <data8>
					case 0x64:		// Rn = Rn OR FDEP Rx BY Ry | <bit6>:<len6>
					case 0x6c:		// Rn = Rn OR FDEP Rx BY Ry | <bit6>:<len6> (SE)
						REG_USED(desc, rx);
						REG_USED(desc, ry);
						REG_USED(desc, rn);
						REG_MODIFIED(desc, rn);
						SHIFT_FLAGS_MODIFIED(desc);
						break;
					
					case 0xcc:		// BTST Rx BY Ry | <data8>
						REG_USED(desc, rx);
						REG_USED(desc, ry);
						SHIFT_FLAGS_MODIFIED(desc);
						break;
				
					case 0x80:		// Rn = EXP Rx
					case 0x84:		// Rn = EXP Rx (EX)
					case 0x88:		// Rn = LEFTZ Rx
					case 0x8c:		// Rn = LEFTO Rx
					case 0x90:		// Rn = FPACK Fx
					case 0x94:		// Fn = FUNPACK Rx
						REG_USED(desc, rx);
						REG_MODIFIED(desc, rn);
						SHIFT_FLAGS_MODIFIED(desc);
						break;

					default:
						fatalerror("sharc_frontend::describe_compute: unknown shift op %02X in opcode %04X%08X at %08X", operation, (UINT16)(opcode >> 32), (UINT32)(opcode), desc.pc);
				}
				break;
			}

			default:
				fatalerror("sharc_frontend::describe_compute: unknown operation type in opcode %04X%08X at %08X", (UINT16)(opcode >> 32), (UINT32)(opcode), desc.pc);
				return false;
		}
	}

	return true;
}

bool sharc_frontend::describe_ureg_access(opcode_desc &desc, int reg, UREG_ACCESS access)
{
	switch (reg)
	{
		case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06: case 0x07:
		case 0x08: case 0x09: case 0x0a: case 0x0b: case 0x0c: case 0x0d: case 0x0e: case 0x0f:
			if (access == UREG_READ)
				REG_USED(desc, reg);
			else
				REG_MODIFIED(desc, reg);
			break;

		case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
			if (access == UREG_READ)
				DM_I_USED(desc, reg & 7);
			else
				DM_I_MODIFIED(desc, reg & 7);
			break;
		case 0x18: case 0x19: case 0x1a: case 0x1b: case 0x1c: case 0x1d: case 0x1e: case 0x1f:
			if (access == UREG_READ)
				PM_I_USED(desc, reg & 7);
			else
				PM_I_MODIFIED(desc, reg & 7);
			break;

		case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27:
			if (access == UREG_READ)
				DM_M_USED(desc, reg & 7);
			else
				DM_M_MODIFIED(desc, reg & 7);
			break;
		case 0x28: case 0x29: case 0x2a: case 0x2b: case 0x2c: case 0x2d: case 0x2e: case 0x2f:
			if (access == UREG_READ)
				PM_M_USED(desc, reg & 7);
			else
				PM_M_MODIFIED(desc, reg & 7);
			break;

		case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37:
			if (access == UREG_READ)
				DM_L_USED(desc, reg & 7);
			else
				DM_L_MODIFIED(desc, reg & 7);
			break;
		case 0x38: case 0x39: case 0x3a: case 0x3b: case 0x3c: case 0x3d: case 0x3e: case 0x3f:
			if (access == UREG_READ)
				PM_L_USED(desc, reg & 7);
			else
				PM_L_MODIFIED(desc, reg & 7);
			break;

		case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x46: case 0x47:
			if (access == UREG_READ)
				DM_B_USED(desc, reg & 7);
			else
				DM_B_MODIFIED(desc, reg & 7);
			break;
		case 0x48: case 0x49: case 0x4a: case 0x4b: case 0x4c: case 0x4d: case 0x4e: case 0x4f:
			if (access == UREG_READ)
				PM_B_USED(desc, reg & 7);
			else
				PM_B_MODIFIED(desc, reg & 7);
			break;

		case 0x60:		// FADDR
			break;
		case 0x61:		// DADDR
			break;
		case 0x63:		// PC
			break;
		case 0x64:		// PCSTK
			break;
		case 0x65:		// PCSTKP
			break;
		case 0x66:		// LADDR
			break;
		case 0x67:		// CURLCNTR
			break;
		case 0x68:		// LCNTR
			break;
		case 0x70:		// USTAT1
			break;
		case 0x71:		// USTAT2
			break;
		case 0x79:		// IRPTL
			break;
		case 0x7a:		// MODE1
			break;
		case 0x7b:		// MODE2
			break;

		case 0x7c:		// ASTAT
			if (access == UREG_READ)
			{
				AZ_USED(desc);
				AN_USED(desc);
				AV_USED(desc);
				AC_USED(desc);
				AS_USED(desc);
				AI_USED(desc);
				MN_USED(desc);
				MV_USED(desc);
				MU_USED(desc);
				MI_USED(desc);
				SV_USED(desc);
				SZ_USED(desc);
				SS_USED(desc);
				BTF_USED(desc);
				AF_USED(desc);
			}
			else
			{
				ALU_FLAGS_MODIFIED(desc);
				MULT_FLAGS_MODIFIED(desc);
				SHIFT_FLAGS_MODIFIED(desc);
				BTF_MODIFIED(desc);
				AF_MODIFIED(desc);
			}
			break;

		case 0x7d:		// IMASK
			break;
		case 0x7e:		// STKY
			break;
		case 0x7f:		// IMASKP
			break;
		case 0xdb:		// PX
			break;
		case 0xdc:		// PX1
			break;
		case 0xdd:		// PX2
			break;

		default:
			fatalerror("sharc_frontend::describe_ureg_access: unknown UREG %02X", reg);
			return false;
	}

	return true;
}

bool sharc_frontend::describe_shiftop_imm(opcode_desc &desc, int shiftop, int rn, int rx)
{
	switch (shiftop)
	{
		case 0x00:		// LSHIFT Rx BY <data8>
		case 0x01:		// ASHIFT Rx BY <data8>
		case 0x02:		// ROT Rx BY <data8>
		case 0x10:		// FEXT Rx BY <data8>
		case 0x11:		// FDEP Rx BY <data8>
		case 0x12:		// FEXT Rx BY <bit6>:<len6> (SE)
		case 0x13:		// FDEP Rx BY <bit6>:<len6> (SE)
		case 0x30:		// BSET Rx BY <data8>
		case 0x31:		// BCLR Rx By <data8>
		case 0x32:		// BTGL Rx BY <data8>
			REG_USED(desc, rx);
			REG_MODIFIED(desc, rn);
			SHIFT_FLAGS_MODIFIED(desc);
			break;

		case 0x08:		// Rn = Rn OR LSHIFT Rx BY <data8>
		case 0x19:		// Rn = Rn OR FDEP Rx BY <bit6>:<len6>
		case 0x1b:		// Rn = Rn OR FDEP Rx BY <bit6>:<len6> (SE)
			REG_USED(desc, rx);
			REG_USED(desc, rn);
			REG_MODIFIED(desc, rn);
			SHIFT_FLAGS_MODIFIED(desc);
			break;

		case 0x33:		// BTST Rx BY <data8>
			REG_USED(desc, rx);
			SHIFT_FLAGS_MODIFIED(desc);
			break;

		default:
			fatalerror("sharc_frontend::describe_shiftop_imm: unknown op %02X at %08X", shiftop, desc.pc);
			return false;
	}

	return true;
}

void sharc_frontend::describe_if_condition(opcode_desc &desc, int condition)
{
	switch (condition)
	{
		case 0x00:  AZ_USED(desc); break;                  /* EQ */
		case 0x01:  AZ_USED(desc); AN_USED(desc); break;   /* LT */
		case 0x02:  AZ_USED(desc); AN_USED(desc); break;   /* LE */
		case 0x03:  AC_USED(desc); break;                  /* AC */
		case 0x04:  AV_USED(desc); break;                  /* AV */
		case 0x05:  MV_USED(desc); break;                  /* MV */
		case 0x06:  MN_USED(desc); break;                  /* MS */
		case 0x07:  SV_USED(desc); break;                  /* SV */
		case 0x08:  SZ_USED(desc); break;                  /* SZ */
		case 0x0d:  BTF_USED(desc); break;                 /* TF */
		case 0x10:  AZ_USED(desc); break;                  /* NOT EQUAL */
		case 0x11:  AZ_USED(desc); AN_USED(desc); break;   /* GE */
		case 0x12:  AZ_USED(desc); AN_USED(desc); break;   /* GT */
		case 0x13:  AC_USED(desc); break;                  /* NOT AC */
		case 0x14:  AV_USED(desc); break;                  /* NOT AV */
		case 0x15:  MV_USED(desc); break;                  /* NOT MV */
		case 0x16:  MN_USED(desc); break;                  /* NOT MS */
		case 0x17:  SV_USED(desc); break;                  /* NOT SV */
		case 0x18:  SZ_USED(desc); break;                  /* NOT SZ */
		case 0x1d:  BTF_USED(desc); break;                 /* NOT TF */
	}
}