// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    WE|AT&T DSP16 series instruction analyser

***************************************************************************/

#include "emu.h"
#include "dsp16fe.h"


/***********************************************************************
    construction/destruction
***********************************************************************/

dsp16_device_base::frontend::frontend(dsp16_device_base &host, u32 window_start, u32 window_end, u32 max_sequence)
	: drc_frontend(host, window_start, window_end, max_sequence)
	, m_host(host)
{
}

/***********************************************************************
    drc_frontend implementation
***********************************************************************/

bool dsp16_device_base::frontend::describe(opcode_desc &desc, opcode_desc const *prev)
{
	// most instructions are one word long and run in one machine cycle
	desc.length = 1U;
	desc.cycles = 1U;

	u16 const op(m_host.m_pcache->read_word(desc.physpc));
	switch (op >> 11)
	{
	case 0x00: // goto JA
	case 0x01:
	case 0x10: // call JA
	case 0x11:
		desc.cycles = 2U;
		desc.targetpc = (desc.physpc & XAAU_I_EXT) | (op_ja(op) & XAAU_I_MASK);
		desc.flags |= OPFLAG_IS_UNCONDITIONAL_BRANCH;
		if (BIT(op, 15))
			flag_output_reg(desc, REG_BIT_XAAU_PR);
		return true;

	case 0x02: // R = M
	case 0x03:
		flag_output_reg(desc, REG_BIT_YAAU_R0 + (((op >> 9) & 0x0007U) ^ 0x0004U));
		return true;

	case 0x04: // F1 ; Y = a1[l]
	case 0x1c: // F1 ; Y = a0[l]
		desc.cycles = 2U;
		flag_input_reg(desc, BIT(op, 14) ? REG_BIT_DAU_A0 : REG_BIT_DAU_A1, REG_BIT_DAU_AUC);
		describe_f1(desc, op);
		describe_y(desc, op, false, true);
		return true;

	case 0x05: // F1 ; Z : aT[l]
		desc.cycles = 2U;
		flag_input_reg(desc, op_d(op) ? REG_BIT_DAU_A0 : REG_BIT_DAU_A1, REG_BIT_DAU_AUC);
		flag_output_reg(desc, op_d(op) ? REG_BIT_DAU_A0 : REG_BIT_DAU_A1, REG_BIT_DAU_PSW);
		describe_f1(desc, op);
		describe_z(desc, op);
		return true;

	case 0x06: // F1 ; Y
		describe_f1(desc, op);
		describe_y(desc, op, bool(m_host.machine().debug_flags & DEBUG_FLAG_ENABLED), false); // only read memory for watchpoints
		return true;

	case 0x07: // F1 ; aT[l] = Y
		flag_output_reg(desc, op_d(op) ? REG_BIT_DAU_A0 : REG_BIT_DAU_A1, REG_BIT_DAU_PSW);
		describe_f1(desc, op);
		describe_y(desc, op, true, false);
		return true;

	case 0x08: // aT = R
		desc.cycles = 2U;
		if (op & 0x000fU) // reserved field?
			desc.flags |= OPFLAG_INVALID_OPCODE;
		flag_output_reg(desc, op_d(op) ? REG_BIT_DAU_A0 : REG_BIT_DAU_A1, REG_BIT_DAU_PSW);
		describe_r(desc, op, true, false);
		return true;

	case 0x09: // R = a0
	case 0x0b: // R = a1
		desc.cycles = 2U;
		if (op & 0x000fU) // reserved field?
			desc.flags |= OPFLAG_INVALID_OPCODE;
		flag_input_reg(desc, BIT(op, 12) ? REG_BIT_DAU_A1 : REG_BIT_DAU_A0, REG_BIT_DAU_AUC);
		describe_r(desc, op, false, true);
		return true;

	case 0x0a: // R = N
		desc.length = 2U;
		desc.cycles = 2U;
		if (op & 0x000fU) // reserved field?
			desc.flags |= OPFLAG_INVALID_OPCODE;
		describe_r(desc, op, false, true);
		return true;

	case 0x0c: // Y = R
		desc.cycles = 2U;
		if (op & 0x0400U) // reserved field?
			desc.flags |= OPFLAG_INVALID_OPCODE;
		describe_r(desc, op, true, false);
		describe_y(desc, op, false, true);
		return true;

	case 0x0d: // Z : R
		desc.cycles = 2U;
		describe_r(desc, op, true, true);
		describe_z(desc, op);
		return true;

	case 0x0e: // do K { instre1...instrNI } # redo K
		if (op_ni(op))
			return describe_do(desc, op);
		else
			return describe_redo(desc, op);

	case 0x0f: // R = Y
		desc.cycles = 2U;
		if (op & 0x0400U) // reserved field?
			desc.flags |= OPFLAG_INVALID_OPCODE;
		describe_r(desc, op, false, true);
		describe_y(desc, op, true, false);
		return true;

	case 0x12: // ifc CON F2
		flag_input_reg(desc, REG_BIT_DAU_C1);
		flag_output_reg(desc, REG_BIT_DAU_C1, REG_BIT_DAU_C2);
		describe_con(desc, op, false);
		describe_f2(desc, op);
		return true;

	case 0x13: // if CON F2
		describe_con(desc, op, true);
		describe_f2(desc, op);
		return true;

	case 0x14: // F1 ; Y = y[l]
		desc.cycles = 2U;
		flag_input_reg(desc, REG_BIT_DAU_Y);
		describe_f1(desc, op);
		describe_y(desc, op, false, true);
		return true;

	case 0x15: // F1 ; Z : y[l]
		desc.cycles = 2U;
		flag_input_reg(desc, REG_BIT_DAU_Y);
		flag_output_reg(desc, REG_BIT_DAU_Y);
		describe_f1(desc, op);
		describe_z(desc, op);
		return true;

	case 0x16: // F1 ; x = Y
		flag_output_reg(desc, REG_BIT_DAU_X);
		describe_f1(desc, op);
		describe_y(desc, op, true, false);
		return true;

	case 0x17: // F1 ; y[l] = Y
		flag_output_reg(desc, REG_BIT_DAU_Y);
		describe_f1(desc, op);
		describe_y(desc, op, true, false);
		return true;

	case 0x18: // goto B
		return describe_goto_b(desc, op);

	case 0x19: // F1 ; y = a0 ; x = *pt++[i]
	case 0x1b: // F1 ; y = a1 ; x = *pt++[i]
		desc.cycles = 2U;
		if (op & 0x000fU)
			desc.flags |= OPFLAG_INVALID_OPCODE;
		flag_input_reg(desc, BIT(op, 12) ? REG_BIT_DAU_A1 : REG_BIT_DAU_A0, REG_BIT_DAU_AUC);
		flag_output_reg(desc, REG_BIT_DAU_Y);
		describe_f1(desc, op);
		describe_x(desc, op);
		return true;

	case 0x1a: // if CON # icall
		if (op & 0x03e0U) // reserved field?
			desc.flags |= OPFLAG_INVALID_OPCODE;
		if (BIT(op, 10))
			return describe_icall(desc, op);
		else
			return describe_if_con(desc, op);

	case 0x1d: // F1 ; Z : y ; x = *pt++[i]
		desc.cycles = 2U;
		flag_input_reg(desc, REG_BIT_DAU_Y);
		flag_output_reg(desc, REG_BIT_DAU_Y);
		describe_f1(desc, op);
		describe_x(desc, op);
		describe_z(desc, op);
		return true;

	case 0x1e: // Reserved
		desc.flags |= OPFLAG_INVALID_OPCODE;
		return true;

	case 0x1f: // F1 ; y = Y ; x = *pt++[i]
		desc.cycles = 2U;
		flag_output_reg(desc, REG_BIT_DAU_Y);
		describe_f1(desc, op);
		describe_x(desc, op);
		describe_y(desc, op, true, false);
		return true;
	};

	return false;
}

/***********************************************************************
    program fetch helpers
***********************************************************************/

u16 dsp16_device_base::frontend::read_op(opcode_desc const &desc, u16 offset) const
{
	return m_host.m_pcache->read_word((desc.physpc & XAAU_I_EXT) | ((desc.physpc + offset) & XAAU_I_MASK));
}

/***********************************************************************
    non-trivial instruction helpers
***********************************************************************/

bool dsp16_device_base::frontend::describe_goto_b(opcode_desc &desc, u16 op)
{
	desc.cycles = 2U;
	if (op & 0x00ffU)
		desc.flags |= OPFLAG_INVALID_OPCODE;
	desc.targetpc = BRANCH_TARGET_DYNAMIC;
	desc.flags |= OPFLAG_IS_UNCONDITIONAL_BRANCH;
	switch (op_b(op))
	{
	case 0x0: // return
		flag_input_reg(desc, REG_BIT_XAAU_PR);
		break;
	case 0x1: // ireturn
		desc.flags |= OPFLAG_CAN_CHANGE_MODES; // FIXME: is this flag useful for a switch in/out of interrupt service mode?
		flag_input_reg(desc, REG_BIT_XAAU_PI);
		break;
	case 0x2: // goto pt
		flag_input_reg(desc, REG_BIT_XAAU_PT);
		break;
	case 0x3: // call pt
		flag_input_reg(desc, REG_BIT_XAAU_PT);
		flag_output_reg(desc, REG_BIT_XAAU_PR);
		break;
	case 0x4: // Reserved
	case 0x5:
	case 0x6:
	case 0x7:
		desc.flags |= OPFLAG_INVALID_OPCODE;
		break;
	}
	return true;
}

bool dsp16_device_base::frontend::describe_if_con(opcode_desc &desc, u16 op)
{
	// look ahead and see if the next instruction can be predicated
	u16 const next(read_imm(desc));
	switch (next >> 11)
	{
	case 0x00: // goto JA
	case 0x01:
	case 0x10: // call JA
	case 0x11:
		desc.cycles = 3U;
		desc.length = 2U;
		switch (op_con(op))
		{
		case 0xe: // true
			desc.targetpc = (desc.physpc & XAAU_I_EXT) | (op_ja(next) & XAAU_I_MASK);
			desc.flags |= OPFLAG_IS_UNCONDITIONAL_BRANCH;
			if (BIT(next, 15))
				flag_output_reg(desc, REG_BIT_XAAU_PR);
			break;
		case 0xf: // false
			break;
		default:
			desc.targetpc = (desc.physpc & XAAU_I_EXT) | (op_ja(next) & XAAU_I_MASK);
			desc.flags |= OPFLAG_IS_CONDITIONAL_BRANCH;
			if (BIT(next, 15))
				flag_output_reg(desc, REG_BIT_XAAU_PR);
		}
		break;

	case 0x18: // goto B
		desc.cycles = 3U;
		desc.length = 2U;
		if (op_b(next) == 0x1)
		switch (op_b(next))
		{
		case 0x0: // return
		case 0x2: // goto pt
		case 0x3: // call pt
			break;
		case 0x1: // can't predicate ireturn?
		case 0x4: // Reserved
		case 0x5:
		case 0x6:
		case 0x7:
			desc.flags |= OPFLAG_INVALID_OPCODE;
			break;
		}
		if (op_b(next) == 0x1) // can't predicate ireturn?
		{
			desc.targetpc = BRANCH_TARGET_DYNAMIC;
			desc.flags |= OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_CAN_CHANGE_MODES; // FIXME: confirm appropriate flags
			flag_input_reg(desc, REG_BIT_XAAU_PI);
		}
		else
		{
			switch (op_con(op))
			{
			case 0xe: // true
				desc.targetpc = BRANCH_TARGET_DYNAMIC;
				desc.flags |= OPFLAG_IS_UNCONDITIONAL_BRANCH;
				switch (op_b(next))
				{
				case 0x0: // return
					flag_input_reg(desc, REG_BIT_XAAU_PR);
					break;
				case 0x2: // goto pt
					flag_input_reg(desc, REG_BIT_XAAU_PT);
					break;
				case 0x3: // call pt
					flag_input_reg(desc, REG_BIT_XAAU_PT);
					flag_output_reg(desc, REG_BIT_XAAU_PR);
					break;
				}
				break;
			case 0xf: // false
				break;
			default:
				desc.targetpc = BRANCH_TARGET_DYNAMIC;
				desc.flags |= OPFLAG_IS_CONDITIONAL_BRANCH;
				switch (op_b(next))
				{
				case 0x0: // return
					flag_input_reg(desc, REG_BIT_XAAU_PR);
					break;
				case 0x2: // goto pt
					flag_input_reg(desc, REG_BIT_XAAU_PT);
					break;
				case 0x3: // call pt
					flag_input_reg(desc, REG_BIT_XAAU_PT);
					flag_output_reg(desc, REG_BIT_XAAU_PR);
					break;
				}
			}
		}
		break;

	default:
		desc.flags |= OPFLAG_INVALID_OPCODE; // can't predicate things that aren't branches?
	}
	describe_con(desc, op, true);
	return true;
}

bool dsp16_device_base::frontend::describe_icall(opcode_desc &desc, u16 op)
{
	desc.cycles = 3U;
	if (0x000eU != op_con(op)) // CON must be true for icall?
		desc.flags |= OPFLAG_INVALID_OPCODE;
	switch (op_con(op))
	{
	case 0xe: // true
		desc.targetpc = 0x0002U;
		desc.flags |= OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_CAN_CHANGE_MODES; // FIXME: confirm appropriate flags
		break;
	case 0xf: // false
		break;
	default:
		desc.targetpc = 0x0002U;
		desc.flags |= OPFLAG_IS_CONDITIONAL_BRANCH | OPFLAG_CAN_CHANGE_MODES; // FIXME: confirm appropriate flags
	}
	describe_con(desc, op, true);
	return true;
}

bool dsp16_device_base::frontend::describe_do(opcode_desc &desc, u16 op)
{
	u16 const ni(op_ni(op)), k(op_k(op));
	if (2U > k) // p3-25: "The iteration count can be between 2 and 127, inclusive"
		desc.flags |= OPFLAG_INVALID_OPCODE;
	desc.length = ni + 1;
	u32 cycles(0U), romcycles(0U), cachecycles(0U);
	for (u16 i = 0; i < ni; ++i)
	{
		u16 const next(read_op(desc, i + 1));
		switch (next >> 11)
		{
		case 0x02: // R = M
		case 0x03:
			romcycles = cachecycles = 1U;
			flag_output_reg(desc, REG_BIT_YAAU_R0 + (((next >> 9) & 0x0007U) ^ 0x0004U));
			break;

		case 0x04: // F1 ; Y = a1[l]
		case 0x1c: // F1 ; Y = a0[l]
			romcycles = cachecycles = 2U;
			flag_input_reg(desc, BIT(op, 14) ? REG_BIT_DAU_A0 : REG_BIT_DAU_A1, REG_BIT_DAU_AUC);
			describe_f1(desc, op);
			describe_y(desc, op, false, true);
			break;

		case 0x05: // F1 ; Z : aT[l]
			romcycles = cachecycles = 2U;
			flag_input_reg(desc, op_d(op) ? REG_BIT_DAU_A0 : REG_BIT_DAU_A1, REG_BIT_DAU_AUC);
			flag_output_reg(desc, op_d(op) ? REG_BIT_DAU_A0 : REG_BIT_DAU_A1, REG_BIT_DAU_PSW);
			describe_f1(desc, op);
			describe_z(desc, op);
			break;

		case 0x06: // F1 ; Y
			romcycles = cachecycles = 1U;
			describe_f1(desc, op);
			describe_y(desc, op, bool(m_host.machine().debug_flags & DEBUG_FLAG_ENABLED), false); // only read memory for watchpoints
			break;

		case 0x07: // F1 ; aT[l] = Y
			romcycles = cachecycles = 1U;
			flag_output_reg(desc, op_d(op) ? REG_BIT_DAU_A0 : REG_BIT_DAU_A1, REG_BIT_DAU_PSW);
			describe_f1(desc, op);
			describe_y(desc, op, true, false);
			break;

		case 0x08: // aT = R
			romcycles = cachecycles = 2U;
			if (op & 0x000fU) // reserved field?
				desc.flags |= OPFLAG_INVALID_OPCODE;
			flag_output_reg(desc, op_d(op) ? REG_BIT_DAU_A0 : REG_BIT_DAU_A1, REG_BIT_DAU_PSW);
			describe_r(desc, op, true, false);
			break;

		case 0x09: // R = a0
		case 0x0b: // R = a1
			romcycles = cachecycles = 2U;
			if (op & 0x000fU) // reserved field?
				desc.flags |= OPFLAG_INVALID_OPCODE;
			flag_input_reg(desc, BIT(op, 12) ? REG_BIT_DAU_A1 : REG_BIT_DAU_A0, REG_BIT_DAU_AUC);
			describe_r(desc, op, false, true);
			break;

		case 0x0c: // Y = R
			romcycles = cachecycles = 2U;
			if (op & 0x0400U) // reserved field?
				desc.flags |= OPFLAG_INVALID_OPCODE;
			describe_r(desc, op, true, false);
			describe_y(desc, op, false, true);
			break;

		case 0x0d: // Z : R
			romcycles = cachecycles = 2U;
			describe_r(desc, op, true, true);
			describe_z(desc, op);
			break;

		case 0x0f: // R = Y
			romcycles = cachecycles = 2U;
			if (op & 0x0400U) // reserved field?
				desc.flags |= OPFLAG_INVALID_OPCODE;
			describe_r(desc, op, false, true);
			describe_y(desc, op, true, false);
			break;

		case 0x12: // ifc CON F2
			romcycles = cachecycles = 1U;
			flag_input_reg(desc, REG_BIT_DAU_C1);
			flag_output_reg(desc, REG_BIT_DAU_C1, REG_BIT_DAU_C2);
			describe_con(desc, op, false);
			describe_f2(desc, op);
			break;

		case 0x13: // if CON F2
			romcycles = cachecycles = 1U;
			describe_con(desc, op, true);
			describe_f2(desc, op);
			break;

		case 0x14: // F1 ; Y = y[l]
			romcycles = cachecycles = 2U;
			flag_input_reg(desc, REG_BIT_DAU_Y);
			describe_f1(desc, op);
			describe_y(desc, op, false, true);
			break;

		case 0x15: // F1 ; Z : y[l]
			romcycles = cachecycles = 2U;
			flag_input_reg(desc, REG_BIT_DAU_Y);
			flag_output_reg(desc, REG_BIT_DAU_Y);
			describe_f1(desc, op);
			describe_z(desc, op);
			break;

		case 0x16: // F1 ; x = Y
			romcycles = cachecycles = 1U;
			flag_output_reg(desc, REG_BIT_DAU_X);
			describe_f1(desc, op);
			describe_y(desc, op, true, false);
			break;

		case 0x17: // F1 ; y[l] = Y
			romcycles = cachecycles = 1U;
			flag_output_reg(desc, REG_BIT_DAU_Y);
			describe_f1(desc, op);
			describe_y(desc, op, true, false);
			break;

		case 0x19: // F1 ; y = a0 ; x = *pt++[i]
		case 0x1b: // F1 ; y = a1 ; x = *pt++[i]
			romcycles = 2U;
			cachecycles = 1U;
			if (op & 0x000fU)
				desc.flags |= OPFLAG_INVALID_OPCODE;
			flag_input_reg(desc, BIT(op, 12) ? REG_BIT_DAU_A1 : REG_BIT_DAU_A0, REG_BIT_DAU_AUC);
			flag_output_reg(desc, REG_BIT_DAU_Y);
			describe_f1(desc, op);
			describe_x(desc, op);
			break;

		case 0x1d: // F1 ; Z : y ; x = *pt++[i]
			romcycles = cachecycles = 2U;
			flag_input_reg(desc, REG_BIT_DAU_Y);
			flag_output_reg(desc, REG_BIT_DAU_Y);
			describe_f1(desc, op);
			describe_x(desc, op);
			describe_z(desc, op);
			break;

		case 0x1f: // F1 ; y = Y ; x = *pt++[i]
			romcycles = 2U;
			cachecycles = 1U;
			flag_output_reg(desc, REG_BIT_DAU_Y);
			describe_f1(desc, op);
			describe_x(desc, op);
			describe_y(desc, op, true, false);
			break;

		case 0x00: // goto JA
		case 0x01:
		case 0x10: // call JA
		case 0x11:
		case 0x0a: // R = N
		case 0x0e: // do K { instre1...instrNI } # redo K
		case 0x18: // goto B
		case 0x1a: // if CON # icall
		case 0x1e: // Reserved
			desc.flags |= OPFLAG_INVALID_OPCODE;
			m_cache_valid = false;
			return false; // not going to get into what happens if you cache ineligible instructions
		}
		desc.cycles += romcycles;
		cycles += cachecycles;
	}
	m_cache_cycles = cycles;
	m_cache_last_cycles = cycles + (romcycles - cachecycles);
	m_cache_flags = desc.flags & (OPFLAG_READS_MEMORY | OPFLAG_WRITES_MEMORY);
	std::copy_n(desc.regin, ARRAY_LENGTH(desc.regin), m_cache_regin);
	std::copy_n(desc.regout, ARRAY_LENGTH(desc.regout), m_cache_regout);
	std::copy_n(desc.regreq, ARRAY_LENGTH(desc.regreq), m_cache_regreq);
	m_cache_valid = true;
	desc.cycles += (2U - romcycles) + ((k - 1) * cycles) + (romcycles - cachecycles);
	return true;
}

bool dsp16_device_base::frontend::describe_redo(opcode_desc &desc, u16 op)
{
	desc.cycles = 2U;
	u16 const k(op_k(op));
	if (2U > k) // p3-25: "The iteration count can be between 2 and 127, inclusive"
		desc.flags |= OPFLAG_INVALID_OPCODE;

	// assume that the block follwoing the last do K { ... } we examined is still cached - we can check properly when we actually run
	if (!m_cache_valid)
	{
		desc.flags |= OPFLAG_INVALID_OPCODE;
		return false;
	}
	else
	{
		desc.cycles += ((k - 1) * m_cache_cycles) + m_cache_last_cycles;
		desc.flags |= m_cache_flags;
		std::copy_n(m_cache_regin, ARRAY_LENGTH(desc.regin), desc.regin);
		std::copy_n(m_cache_regout, ARRAY_LENGTH(desc.regout), desc.regout);
		std::copy_n(m_cache_regreq, ARRAY_LENGTH(desc.regreq), desc.regreq);
		return true;
	}
}

/***********************************************************************
    sub-operation helpers
***********************************************************************/

void dsp16_device_base::frontend::describe_r(opcode_desc &desc, u16 op, bool read, bool write)
{
	u8 const r(op_r(op));
	switch (r)
	{
	case 0x00: // r0 (u)
	case 0x01: // r1 (u)
	case 0x02: // r2 (u)
	case 0x03: // r3 (u)
	case 0x04: // j (s)
	case 0x05: // k (s)
	case 0x06: // rb (u)
	case 0x07: // re (u)
	case 0x08: // pt
	case 0x09: // pr
	case 0x0b: // i (s)
	case 0x10: // x
	case 0x11: // y
	case 0x13: // auc (u)
	case 0x15: // c0 (s)
	case 0x16: // c1 (s)
	case 0x17: // c2 (s)
		if (read)
			flag_input_reg(desc, r);
		if (write)
			flag_output_reg(desc, r);
		break;
	case 0x0a: // pi
		// FIXME: mode-sensitive and resets PRNG
		if (read)
			flag_input_reg(desc, REG_BIT_XAAU_PI);
		if (write)
			flag_output_reg(desc, REG_BIT_XAAU_PI);
		break;
	case 0x12: // yl
		if (read)
			flag_input_reg(desc, REG_BIT_DAU_Y);
		if (write)
			flag_output_reg(desc, REG_BIT_DAU_Y);
		break;
	case 0x14: // psw
		if (read)
			flag_input_reg(desc, REG_BIT_DAU_PSW);
		if (write)
			flag_output_reg(desc, REG_BIT_DAU_A0, REG_BIT_DAU_A1, REG_BIT_DAU_PSW);
		break;
	case 0x18: // sioc
	case 0x19: // srta
	case 0x1b: // tdms
		if (read)
			flag_input_reg(desc, r);
		if (write)
			flag_required_output_reg(desc, r);
		break;
	case 0x1a: // sdx
		if (write)
			flag_required_output_reg(desc, r);
		break;
	case 0x1c: // pioc
	case 0x1d: // pdx0
	case 0x1e: // pdx1
		break;
	default:
		desc.flags |= OPFLAG_INVALID_OPCODE;
	}
}

void dsp16_device_base::frontend::describe_con(opcode_desc &desc, u16 op, bool inc)
{
	u16 const con(op_con(op));
	switch (con >> 1)
	{
	case 0x0: // mi/pl
	case 0x1: // eq/ne
	case 0x2: // lvs/lvc
	case 0x3: // mvs/mvc
	case 0x8: // gt/le
		flag_input_reg(desc, REG_BIT_DAU_PSW);
		break;
	case 0x4: // heads/tails
		throw emu_fatalerror("DSP16: unimplemented CON value %02X (PC = %04X)\n", con, desc.physpc);
	case 0x5: // c0ge/c0lt
	case 0x6: // c1ge/c1lt
		{

			u8 const c(REG_BIT_DAU_C0 + (con >> 1) - 0x05);
			flag_input_reg(desc, c);
			if (inc)
				flag_output_reg(desc, c);
		}
		break;
	case 0x7: // true/false
		break;
	default:
		desc.flags |= OPFLAG_INVALID_OPCODE;
	}
}

void dsp16_device_base::frontend::describe_f1(opcode_desc &desc, u16 op)
{
	u32 const s(op_s(op) ? REG_BIT_DAU_A1 : REG_BIT_DAU_A0);
	u32 const d(op_d(op) ? REG_BIT_DAU_A1 : REG_BIT_DAU_A0);
	switch (op_f1(op))
	{
	case 0x0: // aD = p ; p = x*y
		flag_input_reg(desc, REG_BIT_DAU_X, REG_BIT_DAU_Y, REG_BIT_DAU_P, REG_BIT_DAU_AUC);
		flag_output_reg(desc, d, REG_BIT_DAU_P, REG_BIT_DAU_PSW);
		break;
	case 0x1: // aD = aS + p ; p = x*y
		flag_input_reg(desc, s, REG_BIT_DAU_X, REG_BIT_DAU_Y, REG_BIT_DAU_P, REG_BIT_DAU_AUC);
		flag_output_reg(desc, d, REG_BIT_DAU_P, REG_BIT_DAU_PSW);
		break;
	case 0x2: // p = x*y
		flag_input_reg(desc, REG_BIT_DAU_X, REG_BIT_DAU_Y);
		flag_output_reg(desc, REG_BIT_DAU_P);
		break;
	case 0x3: // aD = aS - p ; p = x*y
		flag_input_reg(desc, s, REG_BIT_DAU_X, REG_BIT_DAU_Y, REG_BIT_DAU_P, REG_BIT_DAU_AUC);
		flag_output_reg(desc, d, REG_BIT_DAU_P, REG_BIT_DAU_PSW);
		break;
	case 0x4: // aD = p
		flag_input_reg(desc, REG_BIT_DAU_P, REG_BIT_DAU_AUC);
		flag_output_reg(desc, d, REG_BIT_DAU_PSW);
		break;
	case 0x5: // aD = aS + p
		flag_input_reg(desc, s, REG_BIT_DAU_P, REG_BIT_DAU_AUC);
		flag_output_reg(desc, d, REG_BIT_DAU_PSW);
		break;
	case 0x6: // NOP
		break;
	case 0x7: // aD = aS - p
		flag_input_reg(desc, s, REG_BIT_DAU_P, REG_BIT_DAU_AUC);
		flag_output_reg(desc, d, REG_BIT_DAU_PSW);
		break;
	case 0x8: // aD = aS | y
		flag_input_reg(desc, s, REG_BIT_DAU_Y);
		flag_output_reg(desc, d, REG_BIT_DAU_PSW);
		break;
	case 0x9: // aD = aS ^ y
		flag_input_reg(desc, s, REG_BIT_DAU_Y);
		flag_output_reg(desc, d, REG_BIT_DAU_PSW);
		break;
	case 0xa: // aS & y
		flag_input_reg(desc, s, REG_BIT_DAU_Y);
		flag_output_reg(desc, REG_BIT_DAU_PSW);
		break;
	case 0xb: // aS - y
		flag_input_reg(desc, s, REG_BIT_DAU_Y);
		flag_output_reg(desc, REG_BIT_DAU_PSW);
		break;
	case 0xc: // aD = y
		flag_input_reg(desc, REG_BIT_DAU_Y);
		flag_output_reg(desc, d, REG_BIT_DAU_PSW);
		break;
	case 0xd: // aD = aS + y
		flag_input_reg(desc, s, REG_BIT_DAU_Y);
		flag_output_reg(desc, d, REG_BIT_DAU_PSW);
		break;
	case 0xe: // aD = aS & y
		flag_input_reg(desc, s, REG_BIT_DAU_Y);
		flag_output_reg(desc, d, REG_BIT_DAU_PSW);
		break;
	case 0xf: // aD = aS - y
		flag_input_reg(desc, s, REG_BIT_DAU_Y);
		flag_output_reg(desc, d, REG_BIT_DAU_PSW);
		break;
	}
}

void dsp16_device_base::frontend::describe_f2(opcode_desc &desc, u16 op)
{
	u32 const s(op_s(op) ? REG_BIT_DAU_A1 : REG_BIT_DAU_A0);
	u32 const d(op_d(op) ? REG_BIT_DAU_A1 : REG_BIT_DAU_A0);
	switch (op_f2(op))
	{
	case 0x0: // aD = aS >> 1
	case 0x1: // aD = aS << 1
	case 0x2: // aD = aS >> 4
	case 0x3: // aD = aS << 4
	case 0x4: // aD = aS >> 8
	case 0x5: // aD = aS << 8
	case 0x6: // aD = aS >> 16
	case 0x7: // aD = aS << 16
	case 0x9: // aDh = aSh + 1
	case 0xb: // aD = rnd(aS)
	case 0xd: // aD = aS + 1
	case 0xe: // aD = aS
	case 0xf: // aD = -aS
		flag_input_reg(desc, s);
		flag_output_reg(desc, d, REG_BIT_DAU_PSW);
		break;
	case 0x8: // aD = p
		flag_input_reg(desc, REG_BIT_DAU_P, REG_BIT_DAU_AUC);
		flag_output_reg(desc, d, REG_BIT_DAU_PSW);
		break;
	case 0xa: // Reserved
		desc.flags |= OPFLAG_INVALID_OPCODE;
		break;
	case 0xc: // aD = y
		flag_input_reg(desc, REG_BIT_DAU_Y);
		flag_output_reg(desc, d, REG_BIT_DAU_PSW);
		break;
	}
}

void dsp16_device_base::frontend::describe_x(opcode_desc &desc, u16 op)
{
	desc.flags |= OPFLAG_READS_MEMORY; // TODO: is reading ROM the same as memory for dependency purposes?
	flag_input_reg(desc, REG_BIT_XAAU_PT);
	flag_output_reg(desc, REG_BIT_XAAU_PT, REG_BIT_DAU_X);
	if (op_x(op))
		flag_input_reg(desc, REG_BIT_XAAU_I);
}

void dsp16_device_base::frontend::describe_y(opcode_desc &desc, u16 op, bool read, bool write)
{
	u32 const r(REG_BIT_YAAU_R0 + ((op >> 2) & 0x0003U));
	if (read)
		desc.flags |= OPFLAG_READS_MEMORY;
	if (write)
		desc.flags |= OPFLAG_WRITES_MEMORY;
	switch (op & 0x0003U)
	{
	case 0x0: // *rN
		if (read || write)
			flag_input_reg(desc, r);
		break;
	case 0x1: // *rN++
		flag_input_reg(desc, r, REG_BIT_YAAU_RB, REG_BIT_YAAU_RE);
		flag_output_reg(desc, r);
		break;
	case 0x2: // *rN--
		flag_input_reg(desc, r);
		flag_output_reg(desc, r);
		break;
	case 0x3: // *rN++j
		flag_input_reg(desc, r, REG_BIT_YAAU_J);
		flag_output_reg(desc, r);
		break;
	}
}

void dsp16_device_base::frontend::describe_z(opcode_desc &desc, u16 op)
{
	u32 const r(REG_BIT_YAAU_R0 + ((op >> 2) & 0x0003U));
	desc.flags |= OPFLAG_READS_MEMORY | OPFLAG_WRITES_MEMORY;
	flag_output_reg(desc, r);
	switch (op & 0x0003U)
	{
	case 0x0: // *rNzp
	case 0x1: // *rNpz
		flag_input_reg(desc, r, REG_BIT_YAAU_RB, REG_BIT_YAAU_RE);
		break;
	case 0x2: // *rNm2
		flag_input_reg(desc, r);
		break;
	case 0x3: // *rNjk
		flag_input_reg(desc, r, REG_BIT_YAAU_J, REG_BIT_YAAU_K);
		break;
	}
}
