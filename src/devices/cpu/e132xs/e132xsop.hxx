// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli
void hyperstone_device::hyperstone_chk_global_global()
{
	check_delay_PC();

	const uint32_t src_code = SRC_CODE;
	const uint32_t dst_code = DST_CODE;
	const uint32_t dreg = m_global_regs[dst_code];

	if (src_code == SR_REGISTER)
	{
		if (dreg == 0)
			execute_exception(get_trap_addr(TRAPNO_RANGE_ERROR));
	}
	else
	{
		const uint32_t sreg = m_global_regs[src_code];
		if (src_code == PC_REGISTER)
		{
			if (dreg >= sreg)
				execute_exception(get_trap_addr(TRAPNO_RANGE_ERROR));
		}
		else
		{
			if (dreg > sreg)
				execute_exception(get_trap_addr(TRAPNO_RANGE_ERROR));
		}
	}

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_chk_global_local()
{
	check_delay_PC();

	const uint32_t sreg = m_local_regs[(SRC_CODE + GET_FP) & 0x3f];
	const uint32_t dreg = m_global_regs[DST_CODE];
	if (dreg > sreg)
		execute_exception(get_trap_addr(TRAPNO_RANGE_ERROR));

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_chk_local_global()
{
	check_delay_PC();

	const uint32_t src_code = SRC_CODE;
	const uint32_t dreg = m_local_regs[(DST_CODE + GET_FP) & 0x3f];

	if (src_code == SR_REGISTER)
	{
		if (dreg == 0)
			execute_exception(get_trap_addr(TRAPNO_RANGE_ERROR));
	}
	else
	{
		const uint32_t sreg = m_global_regs[src_code];
		if (src_code == PC_REGISTER)
		{
			if (dreg >= sreg)
				execute_exception(get_trap_addr(TRAPNO_RANGE_ERROR));
		}
		else
		{
			if (dreg > sreg)
				execute_exception(get_trap_addr(TRAPNO_RANGE_ERROR));
		}
	}

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_chk_local_local()
{
	check_delay_PC();

	const uint32_t fp = GET_FP;
	const uint32_t sreg = m_local_regs[(SRC_CODE + fp) & 0x3f];
	const uint32_t dreg = m_local_regs[(DST_CODE + fp) & 0x3f];
	if (dreg > sreg)
		execute_exception(get_trap_addr(TRAPNO_RANGE_ERROR));

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_movd_global_global()
{
	check_delay_PC();

	const uint32_t src_code = SRC_CODE;
	const uint32_t dst_code = DST_CODE;

	const uint32_t sreg = m_global_regs[src_code];
	const uint32_t sregf = m_global_regs[src_code + 1];

	if (dst_code == PC_REGISTER)
	{
		// RET instruction
		if (src_code < 2)
		{
			DEBUG_PRINTF(("Denoted PC or SR in RET instruction. PC = %08X\n", PC));
			m_icount -= m_clock_cycles_1;
			return;
		}

		const uint32_t old_s = SR & S_MASK;
		const uint32_t old_l = SR & L_MASK;
		PPC = PC;
		PC = sreg & ~1;
		SR = (sregf & 0xffe00000) | ((sreg & 0x01) << 18 ) | (sregf & 0x3ffff);
		if (m_intblock < 1)
			m_intblock = 1;

		m_instruction_length = 0; // undefined

		const uint32_t new_s = SR & S_MASK;
		const uint32_t new_l = SR & L_MASK;
		if( (!old_s && new_s) || (!new_s && !old_l && new_l))
			execute_exception(get_trap_addr(TRAPNO_PRIVILEGE_ERROR));

		int8_t difference = GET_FP - ((SP & 0x1fc) >> 2);

		/* convert to 8 bits */
		if(difference > 63)
			difference = (int8_t)(difference|0x80);
		else if( difference < -64 )
			difference = difference & 0x7f;

		for (; difference < 0; difference++)
		{
			SP -= 4;
			m_local_regs[(SP & 0xfc) >> 2] = READ_W(SP);
		}

		//TODO: no 1!
		m_icount -= m_clock_cycles_1;
	}
	else if (src_code == SR_REGISTER) // Rd doesn't denote PC and Rs denotes SR
	{
		set_global_register(dst_code, 0);
		set_global_register(dst_code + 1, 0);
		SR |= Z_MASK;
		SR &= ~N_MASK;

		m_icount -= m_clock_cycles_2;
	}
	else // Rd doesn't denote PC and Rs doesn't denote SR
	{
		set_global_register(dst_code, sreg);
		set_global_register(dst_code + 1, sregf);

		SR &= ~(Z_MASK | N_MASK);
		if (concat_64(sreg, sregf) == 0)
			SR |= Z_MASK;
		SR |= SIGN_TO_N(sreg);

		m_icount -= m_clock_cycles_2;
	}
}

void hyperstone_device::hyperstone_movd_global_local()
{
	check_delay_PC();
	const uint32_t src_code = SRC_CODE + GET_FP;
	const uint32_t dst_code = DST_CODE;

	const uint32_t sreg = m_local_regs[src_code & 0x3f];
	const uint32_t sregf = m_local_regs[(src_code + 1) & 0x3f];

	if (dst_code == PC_REGISTER)
	{
		uint32_t old_s = SR & S_MASK;
		uint32_t old_l = SR & L_MASK;
		PPC = PC;

		SET_PC(sreg);
		SR = (sregf & 0xffe00000) | ((sreg & 0x01) << 18 ) | (sregf & 0x3ffff);
		if (!m_intblock)
			m_intblock = 1;

		m_instruction_length = 0; // undefined

		uint32_t new_s = SR & S_MASK;
		uint32_t new_l = SR & L_MASK;
		if ((!old_s && new_s) || (!new_s && !old_l && new_l))
		{
			uint32_t addr = get_trap_addr(TRAPNO_PRIVILEGE_ERROR);
			execute_exception(addr);
		}

		int8_t difference = GET_FP - ((SP & 0x1fc) >> 2);

		/* convert to 8 bits */
		if (difference > 63)
			difference |= 0x80;
		else if ( difference < -64 )
			difference &= 0x7f;

		for (; difference < 0; difference++)
		{
			SP -= 4;
			m_local_regs[(SP & 0xfc) >> 2] = READ_W(SP);
		}
	}
	else
	{
		set_global_register(dst_code, sreg);
		set_global_register(dst_code + 1, sregf);

		uint64_t tmp = concat_64(sreg, sregf);
		SR &= ~(Z_MASK | N_MASK);
		if (tmp == 0)
			SR |= Z_MASK;
		SR |= SIGN_TO_N(sreg);

		m_icount -= m_clock_cycles_2;
	}
}

void hyperstone_device::hyperstone_movd_local_global()
{
	check_delay_PC();

	const uint32_t fp = GET_FP;
	const uint32_t src_code = SRC_CODE;
	const uint32_t dst_code = DST_CODE;

	if (src_code == SR_REGISTER) // Rd doesn't denote PC and Rs denotes SR
	{
		m_local_regs[(dst_code + fp) & 0x3f] = 0;
		m_local_regs[(dst_code + 1 + fp) & 0x3f] = 0;
		SR |= Z_MASK;
		SR &= ~N_MASK;

		m_icount -= m_clock_cycles_2;
	}
	else // Rd doesn't denote PC and Rs doesn't denote SR
	{
		const uint32_t sreg = m_global_regs[src_code];
		const uint32_t sregf = m_global_regs[src_code + 1];
		m_local_regs[(dst_code + fp) & 0x3f] = sreg;
		m_local_regs[(dst_code + 1 + fp) & 0x3f] = sregf;

		SR &= ~(Z_MASK | N_MASK);

		if (sreg == 0 && sregf == 0)
			SR |= Z_MASK;
		SR |= SIGN_TO_N(sreg);

		m_icount -= m_clock_cycles_2;
	}
}

void hyperstone_device::hyperstone_movd_local_local()
{
	check_delay_PC();

	const uint32_t src_code = SRC_CODE;
	const uint32_t dst_code = DST_CODE;

	const uint32_t fp = GET_FP;
	const uint32_t sreg = m_local_regs[(src_code + fp) & 0x3f];
	const uint32_t sregf = m_local_regs[(src_code + 1 + fp) & 0x3f];

	m_local_regs[(dst_code + fp) & 0x3f] = sreg;
	m_local_regs[(dst_code + 1 + fp) & 0x3f] = sregf;

	SR &= ~(Z_MASK | N_MASK);

	if (sreg == 0 && sregf == 0)
		SR |= Z_MASK;
	SR |= SIGN_TO_N(sreg);

	m_icount -= m_clock_cycles_2;
}

void hyperstone_device::hyperstone_divu_global_global()
{
	check_delay_PC();

	const uint32_t dst_code = DST_CODE;
	const uint32_t dstf_code = dst_code + 1;

	const uint32_t src_code = SRC_CODE;
	const uint32_t sreg = m_global_regs[src_code];

	if (src_code == dst_code || src_code == dstf_code || src_code < 2)
	{
		DEBUG_PRINTF(("Denoted the same register code or PC/SR as source in hyperstone_divu instruction. PC = %08X\n", PC));
		m_icount -= 36 << m_clck_scale;
		return;
	}

	if (src_code == 0)
	{
		//Rd//Rdf -> undefined
		//Z -> undefined
		//N -> undefined
		SR |= V_MASK;
		uint32_t addr = get_trap_addr(TRAPNO_RANGE_ERROR);
		execute_exception(addr);
	}
	else
	{
		const uint64_t dividend = concat_64(m_global_regs[dst_code], m_global_regs[dstf_code]);

		/* TODO: add quotient overflow */
		uint32_t quotient = dividend / sreg;
		set_global_register(dst_code, dividend % sreg);
		set_global_register(dst_code + 1, quotient);

		SR &= ~(V_MASK | Z_MASK | N_MASK);
		if (quotient == 0)
			SR |= Z_MASK;
		SR |= SIGN_TO_N(quotient);
	}

	m_icount -= 36 << m_clck_scale;
}

void hyperstone_device::hyperstone_divu_global_local()
{
	check_delay_PC();

	const uint32_t dst_code = DST_CODE;
	const uint32_t dstf_code = dst_code + 1;
	const uint32_t sreg = m_local_regs[(SRC_CODE + GET_FP) & 0x3f];

	if (sreg == 0)
	{
		//Rd//Rdf -> undefined
		//Z -> undefined
		//N -> undefined
		SR |= V_MASK;
		execute_exception(get_trap_addr(TRAPNO_RANGE_ERROR));
	}
	else
	{
		const uint64_t dividend = concat_64(m_global_regs[dst_code], m_global_regs[dstf_code]);

		/* TODO: add quotient overflow */
		uint32_t quotient = dividend / sreg;
		set_global_register(dst_code, dividend % sreg);
		set_global_register(dstf_code, quotient);

		SR &= ~(V_MASK | Z_MASK | N_MASK);
		if (quotient == 0)
			SR |= Z_MASK;
		SR |= SIGN_TO_N(quotient);
	}

	m_icount -= 36 << m_clck_scale;
}

void hyperstone_device::hyperstone_divu_local_global()
{
	check_delay_PC();

	const uint32_t src_code = SRC_CODE;
	const uint32_t sreg = m_global_regs[src_code];

	const uint32_t dst_code = (DST_CODE + GET_FP) & 0x3f;
	const uint32_t dstf_code = (dst_code + 1) & 0x3f;

	if (src_code < 2)
	{
		DEBUG_PRINTF(("Denoted the same register code or PC/SR as source in hyperstone_divu instruction. PC = %08X\n", PC));
		m_icount -= 36 << m_clck_scale;
		return;
	}

	if (sreg == 0)
	{
		//Rd//Rdf -> undefined
		//Z -> undefined
		//N -> undefined
		SR |= V_MASK;
		execute_exception(get_trap_addr(TRAPNO_RANGE_ERROR));
	}
	else
	{
		const uint64_t dividend = concat_64(m_local_regs[dst_code], m_local_regs[dstf_code]);

		/* TODO: add quotient overflow */
		uint32_t quotient = dividend / sreg;
		m_local_regs[dst_code] = dividend % sreg;
		m_local_regs[dstf_code] = quotient;

		SR &= ~(V_MASK | Z_MASK | N_MASK);
		if (quotient == 0)
			SR |= Z_MASK;
		SR |= SIGN_TO_N(quotient);
	}

	m_icount -= 36 << m_clck_scale;
}

void hyperstone_device::hyperstone_divu_local_local()
{
	check_delay_PC();

	const uint32_t fp = GET_FP;
	const uint32_t src_code = (SRC_CODE + fp) & 0x3f;
	const uint32_t d_code = DST_CODE + fp;
	const uint32_t dst_code = d_code & 0x3f;
	const uint32_t dstf_code = (d_code + 1) & 0x3f;

	if (src_code == dst_code || src_code == dstf_code)
	{
		m_icount -= 36 << m_clck_scale;
		return;
	}

	const uint32_t sreg = m_local_regs[src_code];
	const uint64_t dividend = concat_64(m_local_regs[dst_code], m_local_regs[dstf_code]);

	if (sreg == 0)
	{
		//Rd//Rdf -> undefined
		//Z -> undefined
		//N -> undefined
		SR |= V_MASK;
		execute_exception(get_trap_addr(TRAPNO_RANGE_ERROR));
	}
	else
	{
		/* TODO: add quotient overflow */
		const uint32_t quotient = dividend / sreg;
		m_local_regs[dst_code] = dividend % sreg;
		m_local_regs[dstf_code] = quotient;

		SR &= ~(V_MASK | Z_MASK | N_MASK);
		if (quotient == 0)
			SR |= Z_MASK;
		SR |= SIGN_TO_N(quotient);
	}

	m_icount -= 36 << m_clck_scale;
}

void hyperstone_device::hyperstone_divs_global_global()
{
	check_delay_PC();

	const uint32_t src_code = SRC_CODE;
	const uint32_t dst_code = DST_CODE;
	const uint32_t dstf_code = dst_code + 1;
	const int32_t sreg = (int32_t)m_global_regs[src_code];

	if(src_code == dst_code || src_code == dstf_code || src_code < 2)
	{
		DEBUG_PRINTF(("Denoted invalid register code in hyperstone_divs instruction. PC = %08X\n", PC));
		m_icount -= 36 << m_clck_scale;
		return;
	}

	const int64_t dividend = (int64_t) concat_64(m_global_regs[dst_code], m_global_regs[dstf_code]);

	if (sreg == 0 || (dividend & 0x8000000000000000U))
	{
		//Rd//Rdf -> undefined
		//Z -> undefined
		//N -> undefined
		SR |= V_MASK;
		execute_exception(get_trap_addr(TRAPNO_RANGE_ERROR));
	}
	else
	{
		/* TODO: add quotient overflow */
		const int32_t quotient = dividend / sreg;
		set_global_register(dst_code, dividend % sreg);
		set_global_register(dst_code + 1, quotient);

		SR &= ~(V_MASK | Z_MASK | N_MASK);
		if (quotient == 0)
			SR |= Z_MASK;
		SR |= SIGN_TO_N(quotient);
	}

	m_icount -= 36 << m_clck_scale;
}

void hyperstone_device::hyperstone_divs_global_local()
{
	check_delay_PC();

	const uint32_t dst_code = DST_CODE;
	const int32_t sreg = (int32_t)m_local_regs[(SRC_CODE + GET_FP) & 0x3f];
	const int64_t dividend = (int64_t) concat_64(m_global_regs[dst_code], m_global_regs[dst_code + 1]);

	if (sreg == 0 || (dividend & 0x8000000000000000U))
	{
		//Rd//Rdf -> undefined
		//Z -> undefined
		//N -> undefined
		SR |= V_MASK;
		execute_exception(get_trap_addr(TRAPNO_RANGE_ERROR));
	}
	else
	{
		/* TODO: add quotient overflow */
		const int32_t quotient = dividend / sreg;
		set_global_register(dst_code, dividend % sreg);
		set_global_register(dst_code + 1, quotient);

		SR &= ~(V_MASK | Z_MASK | N_MASK);
		if (quotient == 0)
			SR |= Z_MASK;
		SR |= SIGN_TO_N(quotient);
	}

	m_icount -= 36 << m_clck_scale;
}

void hyperstone_device::hyperstone_divs_local_global()
{
	check_delay_PC();

	const uint32_t fp = GET_FP;
	const uint32_t dst_code = (DST_CODE + fp) & 0x3f;
	const uint32_t dstf_code = (dst_code + 1) & 0x3f;
	const uint32_t src_code = SRC_CODE;
	const int32_t sreg = (int32_t)m_global_regs[src_code];

	if (src_code < 2)
	{
		DEBUG_PRINTF(("Denoted PC or SR as source register in hyperstone_divs instruction. PC = %08X\n", PC));
		m_icount -= 36 << m_clck_scale;
		return;
	}

	const int64_t dividend = (int64_t) concat_64(m_local_regs[dst_code], m_local_regs[dstf_code]);

	if (sreg == 0 || (dividend & 0x8000000000000000U))
	{
		//Rd//Rdf -> undefined
		//Z -> undefined
		//N -> undefined
		SR |= V_MASK;
		execute_exception(get_trap_addr(TRAPNO_RANGE_ERROR));
	}
	else
	{
		/* TODO: add quotient overflow */
		const int32_t quotient = dividend / sreg;
		m_local_regs[dst_code] = (int32_t)(dividend % sreg);
		m_local_regs[dstf_code] = quotient;

		SR &= ~(V_MASK | Z_MASK | N_MASK);

		if (quotient == 0)
			SR |= Z_MASK;
		SR |= SIGN_TO_N(quotient);
	}

	m_icount -= 36 << m_clck_scale;
}

void hyperstone_device::hyperstone_divs_local_local()
{
	check_delay_PC();

	const uint32_t fp = GET_FP;
	const uint32_t src_code = (SRC_CODE + fp) & 0x3f;
	const uint32_t d_code = DST_CODE;
	const uint32_t dst_code = (d_code + fp) & 0x3f;
	const uint32_t dstf_code = (d_code + 1 + fp) & 0x3f;

	if (src_code == dst_code || src_code == dstf_code)
	{
		m_icount -= 36 << m_clck_scale;
		return;
	}

	const int32_t sreg = (int32_t)m_local_regs[src_code];
	const int64_t dividend = (int64_t) concat_64(m_local_regs[dst_code], m_local_regs[dstf_code]);

	if (sreg == 0 || (dividend & 0x8000000000000000U))
	{
		//Rd//Rdf -> undefined
		//Z -> undefined
		//N -> undefined
		SR |= V_MASK;
		execute_exception(get_trap_addr(TRAPNO_RANGE_ERROR));
	}
	else
	{
		/* TODO: add quotient overflow */
		const int32_t quotient = dividend / sreg;
		m_local_regs[dst_code] = dividend % sreg;
		m_local_regs[dstf_code] = quotient;

		SR &= ~(V_MASK | Z_MASK | N_MASK);
		if (quotient == 0)
			SR |= Z_MASK;
		SR |= SIGN_TO_N(quotient);
	}

	m_icount -= 36 << m_clck_scale;
}



void hyperstone_device::hyperstone_xm_global_global()
{
	const uint32_t next = READ_OP(PC);
	PC += 2;

	const uint8_t sub_type = (next & 0x7000) >> 12;

	uint32_t extra_u = next & 0xfff;
	if (next & 0x8000)
	{
		extra_u = ((extra_u & 0xfff) << 16) | READ_OP(PC);
		PC += 2;
		m_instruction_length = (3<<19);
	}
	else
	{
		m_instruction_length = (2<<19);
	}

	check_delay_PC();

	const uint32_t src_code = SRC_CODE;
	const uint32_t dst_code = DST_CODE;

	if (src_code == SR_REGISTER || dst_code < 2)
	{
		DEBUG_PRINTF(("Denoted PC or SR in hyperstone_xm. PC = %08X\n", PC));
		m_icount -= m_clock_cycles_1;
		return;
	}

	uint32_t sreg = m_global_regs[src_code];

	if (sub_type < 4)
	{
		if (src_code != PC_REGISTER && sreg > extra_u)
			execute_exception(get_trap_addr(TRAPNO_RANGE_ERROR));
		else if (src_code == PC_REGISTER && sreg >= extra_u)
			execute_exception(get_trap_addr(TRAPNO_RANGE_ERROR));
		else
			sreg <<= sub_type;
	}
	else
	{
		sreg <<= (sub_type - 4);
	}

	set_global_register(dst_code, sreg);

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_xm_global_local()
{
	const uint32_t next = READ_OP(PC);
	PC += 2;

	const uint8_t sub_type = (next & 0x7000) >> 12;

	uint32_t extra_u = next & 0xfff;
	if (next & 0x8000)
	{
		extra_u = ((extra_u & 0xfff) << 16) | READ_OP(PC);
		PC += 2;
		m_instruction_length = (3<<19);
	}
	else
	{
		m_instruction_length = (2<<19);
	}

	check_delay_PC();

	const uint32_t dst_code = DST_CODE;

	if (dst_code < 2)
	{
		DEBUG_PRINTF(("Denoted PC or SR in hyperstone_xm. PC = %08X\n", PC));
		m_icount -= m_clock_cycles_1;
		return;
	}

	uint32_t sreg = m_local_regs[(SRC_CODE + GET_FP) & 0x3f];

	if (sub_type < 4)
	{
		if (sreg > extra_u)
			execute_exception(get_trap_addr(TRAPNO_RANGE_ERROR));
		else
			sreg <<= sub_type;
	}
	else
	{
		sreg <<= (sub_type - 4);
	}

	set_global_register(dst_code, sreg);

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_xm_local_global()
{
	const uint32_t next = READ_OP(PC);
	PC += 2;

	const uint8_t sub_type = (next & 0x7000) >> 12;

	uint32_t extra_u = next & 0xfff;
	if (next & 0x8000)
	{
		extra_u = ((extra_u & 0xfff) << 16) | READ_OP(PC);
		PC += 2;
		m_instruction_length = (3<<19);
	}
	else
	{
		m_instruction_length = (2<<19);
	}

	check_delay_PC();

	const uint32_t src_code = SRC_CODE;

	if (src_code == SR_REGISTER)
	{
		DEBUG_PRINTF(("Denoted SR in hyperstone_xm. PC = %08X\n", PC));
		m_icount -= m_clock_cycles_1;
		return;
	}

	uint32_t sreg = m_global_regs[src_code];

	if (sub_type < 4)
	{
		if (src_code != PC_REGISTER && sreg > extra_u)
			execute_exception(get_trap_addr(TRAPNO_RANGE_ERROR));
		else if (src_code == PC_REGISTER && sreg >= extra_u)
			execute_exception(get_trap_addr(TRAPNO_RANGE_ERROR));
		else
			sreg <<= sub_type;
	}
	else
	{
		sreg <<= (sub_type - 4);
	}

	m_local_regs[(DST_CODE + GET_FP) & 0x3f] = sreg;
}

void hyperstone_device::hyperstone_xm_local_local()
{
	const uint32_t next = READ_OP(PC);
	PC += 2;

	const uint8_t sub_type = (next & 0x7000) >> 12;

	uint32_t extra_u = next & 0xfff;
	if (next & 0x8000)
	{
		extra_u = ((extra_u & 0xfff) << 16) | READ_OP(PC);
		PC += 2;
		m_instruction_length = (3<<19);
	}
	else
	{
		m_instruction_length = (2<<19);
	}

	check_delay_PC();

	const uint32_t fp = GET_FP;
	uint32_t sreg = m_local_regs[(SRC_CODE + fp) & 0x3f];

	if (sub_type < 4)
	{
		if(sreg > extra_u)
			execute_exception(get_trap_addr(TRAPNO_RANGE_ERROR));
		else
			sreg <<= sub_type;
	}
	else
	{
		sreg <<= (sub_type - 4);
	}

	m_local_regs[(DST_CODE + fp) & 0x3f] = sreg;
}

void hyperstone_device::hyperstone_mask_global_global()
{
	const uint32_t extra_u = decode_const();

	check_delay_PC();

	const uint32_t dreg = m_global_regs[SRC_CODE] & extra_u;
	set_global_register(DST_CODE, dreg);

	if (dreg == 0)
		SR |= Z_MASK;
	else
		SR &= ~Z_MASK;

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_mask_global_local()
{
	const uint32_t extra_u = decode_const();

	check_delay_PC();

	const uint32_t dreg = m_local_regs[(SRC_CODE + GET_FP) & 0x3f] & extra_u;
	set_global_register(DST_CODE, dreg);

	if (dreg == 0)
		SR |= Z_MASK;
	else
		SR &= ~Z_MASK;

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_mask_local_global()
{
	const uint32_t extra_u = decode_const();

	check_delay_PC();

	const uint32_t dreg = m_global_regs[SRC_CODE] & extra_u;
	m_local_regs[(DST_CODE + GET_FP) & 0x3f] = dreg;

	if (dreg == 0)
		SR |= Z_MASK;
	else
		SR &= ~Z_MASK;

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_mask_local_local()
{
	const uint32_t extra_u = decode_const();

	check_delay_PC();

	const uint32_t fp = GET_FP;
	const uint32_t dreg = m_local_regs[(SRC_CODE + fp) & 0x3f] & extra_u;
	m_local_regs[(DST_CODE + fp) & 0x3f] = dreg;

	if (dreg == 0)
		SR |= Z_MASK;
	else
		SR &= ~Z_MASK;

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_sum_global_global()
{
	const uint32_t extra_u = decode_const();

	check_delay_PC();

	const uint32_t sreg = m_global_regs[SRC_CODE];
	const uint64_t tmp = (uint64_t)sreg + (uint64_t)extra_u;

	SR &= ~(C_MASK | V_MASK | Z_MASK | N_MASK);

	SR |= (tmp & 0x100000000L) >> 32;
	SR |= ((sreg ^ tmp) & (extra_u ^ tmp) & 0x80000000) >> 28;

	const uint32_t dreg = sreg + extra_u;

	set_global_register(DST_CODE, dreg);

	if (dreg == 0)
		SR |= Z_MASK;
	SR |= SIGN_TO_N(dreg);

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_sum_global_local()
{
	const uint32_t extra_u = decode_const();

	check_delay_PC();

	const uint32_t sreg = m_local_regs[(SRC_CODE + GET_FP) & 0x3f];
	const uint64_t tmp = (uint64_t)sreg + (uint64_t)extra_u;

	SR &= ~(C_MASK | V_MASK | Z_MASK | N_MASK);

	SR |= (tmp & 0x100000000L) >> 32;
	SR |= ((sreg ^ tmp) & (extra_u ^ tmp) & 0x80000000) >> 28;

	const uint32_t dreg = sreg + extra_u;

	set_global_register(DST_CODE, dreg);

	if (dreg == 0)
		SR |= Z_MASK;
	SR |= SIGN_TO_N(dreg);

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_sum_local_global()
{
	const uint32_t extra_u = decode_const();

	check_delay_PC();

	const uint32_t sreg = m_global_regs[SRC_CODE];
	const uint64_t tmp = (uint64_t)sreg + (uint64_t)extra_u;

	SR &= ~(C_MASK | V_MASK | Z_MASK | N_MASK);

	SR |= (tmp & 0x100000000L) >> 32;
	SR |= ((sreg ^ tmp) & (extra_u ^ tmp) & 0x80000000) >> 28;

	const uint32_t dreg = sreg + extra_u;

	m_local_regs[(DST_CODE + GET_FP) & 0x3f] = dreg;

	if (dreg == 0)
		SR |= Z_MASK;
	SR |= SIGN_TO_N(dreg);

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_sum_local_local()
{
	const uint32_t extra_u = decode_const();

	check_delay_PC();

	const uint32_t fp = GET_FP;
	const uint32_t sreg = m_local_regs[(SRC_CODE + fp) & 0x3f];

	const uint64_t tmp = (uint64_t)sreg + (uint64_t)extra_u;

	SR &= ~(C_MASK | V_MASK | Z_MASK | N_MASK);

	SR |= (tmp & 0x100000000L) >> 32;
	SR |= ((sreg ^ tmp) & (extra_u ^ tmp) & 0x80000000) >> 28;

	const uint32_t dreg = sreg + extra_u;

	m_local_regs[(DST_CODE + fp) & 0x3f] = dreg;

	if (dreg == 0)
		SR |= Z_MASK;
	SR |= SIGN_TO_N(dreg);

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_sums_global_global()
{
	const int32_t extra_s = decode_const();

	check_delay_PC();

	const uint32_t src_code = SRC_CODE;
	const int32_t sreg = (src_code == SR_REGISTER) ? (SR & C_MASK) : m_global_regs[src_code];

	const int64_t tmp = (int64_t)sreg + (int64_t)extra_s;
	SR |= ((sreg ^ tmp) & (extra_s ^ tmp) & 0x80000000) >> 28;

//#if SETCARRYS
//  CHECK_C(tmp);
//#endif

	const int32_t res = (int32_t)sreg + extra_s;
	set_global_register(DST_CODE, res);

	if (res == 0)
		SR |= Z_MASK;
	SR |= SIGN_TO_N(res);

	m_icount -= m_clock_cycles_1;

	if ((SR & V_MASK) && src_code != SR_REGISTER)
		execute_exception(get_trap_addr(TRAPNO_RANGE_ERROR));
}

void hyperstone_device::hyperstone_sums_global_local()
{
	const int32_t extra_s = decode_const();

	check_delay_PC();

	const int32_t sreg = m_local_regs[(SRC_CODE + GET_FP) & 0x3f];
	const int64_t tmp = (int64_t)sreg + (int64_t)extra_s;
	SR |= ((sreg ^ tmp) & (extra_s ^ tmp) & 0x80000000) >> 28;

//#if SETCARRYS
//  CHECK_C(tmp);
//#endif

	const int32_t res = (int32_t)sreg + extra_s;
	set_global_register(DST_CODE, res);

	if (res == 0)
		SR |= Z_MASK;
	SR |= SIGN_TO_N(res);

	m_icount -= m_clock_cycles_1;

	if (SR & V_MASK)
		execute_exception(get_trap_addr(TRAPNO_RANGE_ERROR));
}

void hyperstone_device::hyperstone_sums_local_global()
{
	const int32_t extra_s = decode_const();

	check_delay_PC();

	const uint32_t src_code = SRC_CODE;
	const int32_t sreg = (src_code == SR_REGISTER) ? (SR & C_MASK) : m_global_regs[src_code];
	const int64_t tmp = (int64_t)sreg + (int64_t)extra_s;
	SR |= ((sreg ^ tmp) & (extra_s ^ tmp) & 0x80000000) >> 28;

//#if SETCARRYS
//  CHECK_C(tmp);
//#endif

	const int32_t res = (int32_t)sreg + extra_s;
	m_local_regs[(DST_CODE + GET_FP) & 0x3f] = res;

	if (res == 0)
		SR |= Z_MASK;
	SR |= SIGN_TO_N(res);

	m_icount -= m_clock_cycles_1;

	if ((SR & V_MASK) && src_code != SR_REGISTER)
		execute_exception(get_trap_addr(TRAPNO_RANGE_ERROR));
}

void hyperstone_device::hyperstone_sums_local_local()
{
	const int32_t extra_s = decode_const();

	check_delay_PC();

	const uint32_t fp = GET_FP;
	const int32_t sreg = m_local_regs[(SRC_CODE + fp) & 0x3f];
	const int64_t tmp = (int64_t)sreg + (int64_t)extra_s;
	SR |= ((sreg ^ tmp) & (extra_s ^ tmp) & 0x80000000) >> 28;

//#if SETCARRYS
//  CHECK_C(tmp);
//#endif

	const int32_t res = (int32_t)sreg + extra_s;
	m_local_regs[(DST_CODE + fp) & 0x3f] = res;

	if (res == 0)
		SR |= Z_MASK;
	SR |= SIGN_TO_N(res);

	m_icount -= m_clock_cycles_1;

	if (SR & V_MASK)
		execute_exception(get_trap_addr(TRAPNO_RANGE_ERROR));
}


void hyperstone_device::hyperstone_cmp_global_global()
{
	check_delay_PC();

	const uint32_t src_code = SRC_CODE;
	const uint32_t sreg = ((src_code == SR_REGISTER) ? GET_C : m_global_regs[src_code]);
	const uint32_t dreg = m_global_regs[DST_CODE];

	const uint64_t tmp = (uint64_t)dreg - (uint64_t)sreg;

	SR &= ~(Z_MASK | N_MASK | V_MASK | C_MASK);
	SR |= ((tmp ^ dreg) & (dreg ^ sreg) & 0x80000000) >> 28;

	if (dreg == sreg)
		SR |= Z_MASK;

	if ((int32_t)dreg < (int32_t)sreg)
		SR |= N_MASK;

	if (dreg < sreg)
		SR |= C_MASK;

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_cmp_global_local()
{
	check_delay_PC();

	const uint32_t sreg = m_local_regs[(SRC_CODE + GET_FP) & 0x3f];
	const uint32_t dreg = m_global_regs[DST_CODE];

	const uint64_t tmp = (uint64_t)dreg - (uint64_t)sreg;

	SR &= ~(Z_MASK | N_MASK | V_MASK | C_MASK);
	SR |= ((tmp ^ dreg) & (dreg ^ sreg) & 0x80000000) >> 28;

	if (dreg == sreg)
		SR |= Z_MASK;

	if ((int32_t)dreg < (int32_t)sreg)
		SR |= N_MASK;

	if (dreg < sreg)
		SR |= C_MASK;

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_cmp_local_global()
{
	check_delay_PC();

	const uint32_t src_code = SRC_CODE;
	const uint32_t sreg = ((src_code == SR_REGISTER) ? GET_C : m_global_regs[src_code]);
	const uint32_t dreg = m_local_regs[(DST_CODE + GET_FP) & 0x3f];

	const uint64_t tmp = (uint64_t)dreg - (uint64_t)sreg;

	SR &= ~(Z_MASK | N_MASK | V_MASK | C_MASK);
	SR |= ((tmp ^ dreg) & (dreg ^ sreg) & 0x80000000) >> 28;

	if (dreg == sreg)
		SR |= Z_MASK;

	if ((int32_t)dreg < (int32_t)sreg)
		SR |= N_MASK;

	if (dreg < sreg)
		SR |= C_MASK;

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_cmp_local_local()
{
	check_delay_PC();

	const uint32_t fp = GET_FP;
	const uint32_t sreg = m_local_regs[(SRC_CODE + fp) & 0x3f];
	const uint32_t dreg = m_local_regs[(DST_CODE + fp) & 0x3f];

	const uint64_t tmp = (uint64_t)dreg - (uint64_t)sreg;

 	SR &= ~(Z_MASK | N_MASK | V_MASK | C_MASK);
	SR |= ((tmp ^ dreg) & (dreg ^ sreg) & 0x80000000) >> 28;

	if (dreg == sreg)
		SR |= Z_MASK;

	if ((int32_t)dreg < (int32_t)sreg)
		SR |= N_MASK;

	if (dreg < sreg)
		SR |= C_MASK;

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_mov_global_global()
{
	check_delay_PC();

	const uint32_t dst_code = DST_CODE;

	uint32_t sreg;
	if (SR & H_MASK)
	{
		if (!(SR & S_MASK))
			execute_exception(get_trap_addr(TRAPNO_PRIVILEGE_ERROR));

		const uint32_t src_code = SRC_CODE + 16;
		sreg = (WRITE_ONLY_REGMASK & (1 << src_code)) ? 0 : get_global_register(src_code);
		set_global_register(dst_code + 16, sreg);
	}
	else
	{
		sreg = m_global_regs[SRC_CODE];
		m_global_regs[dst_code] = sreg;

		if (dst_code == PC_REGISTER)
			SR &= ~M_MASK;
	}

	SR &= ~(Z_MASK | N_MASK);
	if (sreg == 0)
		SR |= Z_MASK;
	SR |= SIGN_TO_N(sreg);

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_mov_global_local()
{
	check_delay_PC();

	const uint32_t dst_code = DST_CODE;
	const uint32_t sreg = m_local_regs[(SRC_CODE + GET_FP) & 0x3f];

	if (SR & H_MASK)
	{
		if (!(SR & S_MASK))
			execute_exception(get_trap_addr(TRAPNO_PRIVILEGE_ERROR));

		set_global_register(DST_CODE + 16, sreg);
	}
	else
	{
		m_global_regs[dst_code] = sreg;

		if (dst_code == PC_REGISTER)
			SR &= ~M_MASK;
	}

	SR &= ~(Z_MASK | N_MASK);
	if (sreg == 0)
		SR |= Z_MASK;
	SR |= SIGN_TO_N(sreg);

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_mov_local_global()
{
	check_delay_PC();
	uint32_t src_code = SRC_CODE;

	uint32_t sreg;
	if (SR & H_MASK)
	{
		src_code += 16;
		sreg = (WRITE_ONLY_REGMASK & (1 << src_code)) ? 0 : get_global_register(src_code);
	}
	else
	{
		sreg = m_global_regs[src_code];                                     \
	}

	m_local_regs[(DST_CODE + GET_FP) & 0x3f] = sreg;

	SR &= ~(Z_MASK | N_MASK);
	if (sreg == 0)
		SR |= Z_MASK;
	SR |= SIGN_TO_N(sreg);

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_mov_local_local()
{
	check_delay_PC();

	const uint32_t fp = GET_FP;
	const uint32_t sreg = m_local_regs[(SRC_CODE + fp) & 0x3f];
	m_local_regs[(DST_CODE + fp) & 0x3f] = sreg;

	SR &= ~(Z_MASK | N_MASK);
	if (sreg == 0)
		SR |= Z_MASK;
	SR |= SIGN_TO_N(sreg);

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_add_global_global()
{
	check_delay_PC();

	const uint32_t src_code = SRC_CODE;
	const uint32_t dst_code = DST_CODE;

	const uint32_t sreg = (src_code == SR_REGISTER) ? (SR & C_MASK) : m_global_regs[src_code];
	uint32_t dreg = m_global_regs[dst_code];

	const uint64_t tmp = (uint64_t)sreg + (uint64_t)dreg;

	SR &= ~(C_MASK | V_MASK | Z_MASK | N_MASK);

	SR |= (tmp & 0x100000000L) >> 32;
	SR |= ((sreg ^ tmp) & (dreg ^ tmp) & 0x80000000) >> 28;

	dreg += sreg;
	set_global_register(dst_code, dreg);

	if (dst_code == 0)
		SR &= ~M_MASK;

	if (dreg == 0)
		SR |= Z_MASK;
	SR |= SIGN_TO_N(dreg);

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_add_global_local()
{
	check_delay_PC();

	const uint32_t dst_code = DST_CODE;

	const uint32_t sreg = m_local_regs[(SRC_CODE + GET_FP) & 0x3f];
	uint32_t dreg = m_global_regs[dst_code];

	const uint64_t tmp = (uint64_t)sreg + (uint64_t)dreg;

	SR &= ~(C_MASK | V_MASK | Z_MASK | N_MASK);

	SR |= (tmp & 0x100000000L) >> 32;
	SR |= ((sreg ^ tmp) & (dreg ^ tmp) & 0x80000000) >> 28;

	dreg += sreg;
	set_global_register(dst_code, dreg);

	if (dst_code == 0)
		SR &= ~M_MASK;

	if (dreg == 0)
		SR |= Z_MASK;
	SR |= SIGN_TO_N(dreg);

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_add_local_global()
{
	check_delay_PC();

	const uint32_t src_code = SRC_CODE;
	const uint32_t dst_code = (DST_CODE + GET_FP) & 0x3f;
	uint32_t sreg = ((src_code == SR_REGISTER) ? GET_C : m_global_regs[src_code]);
	uint32_t dreg = m_local_regs[dst_code];

	if (src_code == SR_REGISTER)
		sreg = GET_C;

	const uint64_t tmp = (uint64_t)sreg + (uint64_t)dreg;

	SR &= ~(C_MASK | V_MASK | Z_MASK | N_MASK);

	SR |= (tmp & 0x100000000L) >> 32;
	SR |= ((sreg ^ tmp) & (dreg ^ tmp) & 0x80000000) >> 28;

	dreg += sreg;
	m_local_regs[dst_code] = dreg;

	if (dreg == 0)
		SR |= Z_MASK;
	SR |= SIGN_TO_N(dreg);

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_add_local_local()
{
	check_delay_PC();

	const uint32_t fp = GET_FP;
	const uint32_t dst_code = (DST_CODE + fp) & 0x3f;

	const uint32_t sreg = m_local_regs[(SRC_CODE + fp) & 0x3f];
	uint32_t dreg = m_local_regs[dst_code];

	const uint64_t tmp = (uint64_t)sreg + (uint64_t)dreg;

	SR &= ~(C_MASK | V_MASK | Z_MASK | N_MASK);

	SR |= (tmp & 0x100000000L) >> 32;
	SR |= ((sreg ^ tmp) & (dreg ^ tmp) & 0x80000000) >> 28;

	dreg += sreg;
	m_local_regs[dst_code] = dreg;

	if (dreg == 0)
		SR |= Z_MASK;
	SR |= SIGN_TO_N(dreg);

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_adds_global_global()
{
	const uint32_t src_code = SRC_CODE;
	const uint32_t dst_code = DST_CODE;
	const int32_t sreg = (src_code == SR_REGISTER) ? (SR & C_MASK) : (int32_t)m_global_regs[src_code];
	const int32_t dreg = m_global_regs[dst_code];
	const int64_t tmp = (int64_t)sreg + (int64_t)dreg;

	SR &= ~(V_MASK | Z_MASK | N_MASK);
	SR |= ((sreg ^ tmp) & (dreg ^ tmp) & 0x80000000) >> 28;

//#if SETCARRYS
//  CHECK_C(tmp);
//#endif

	const int32_t res = sreg + dreg;
	m_global_regs[dst_code] = res;

	if (res == 0)
		SR |= Z_MASK;
	SR |= SIGN_TO_N(res);

	m_icount -= m_clock_cycles_1;

	if (SR & V_MASK)
		execute_exception(get_trap_addr(TRAPNO_RANGE_ERROR));
}

void hyperstone_device::hyperstone_adds_global_local()
{
	const uint32_t dst_code = DST_CODE;
	const int32_t sreg = (int32_t)m_local_regs[(SRC_CODE + GET_FP) & 0x3f];
	const int32_t dreg = m_global_regs[dst_code];
	const int64_t tmp = (int64_t)sreg + (int64_t)dreg;

	SR &= ~(V_MASK | Z_MASK | N_MASK);
	SR |= ((sreg ^ tmp) & (dreg ^ tmp) & 0x80000000) >> 28;

//#if SETCARRYS
//  CHECK_C(tmp);
//#endif

	const int32_t res = sreg + dreg;
	m_global_regs[dst_code] = res;

	if (res == 0)
		SR |= Z_MASK;
	SR |= SIGN_TO_N(res);

	m_icount -= m_clock_cycles_1;

	if (SR & V_MASK)
		execute_exception(get_trap_addr(TRAPNO_RANGE_ERROR));
}

void hyperstone_device::hyperstone_adds_local_global()
{
	const uint32_t src_code = SRC_CODE;
	const uint32_t dst_code = (DST_CODE + GET_FP) & 0x3f;
	const int32_t sreg = (src_code == SR_REGISTER) ? (SR & C_MASK) : (int32_t)m_global_regs[src_code];
	const int32_t dreg = m_local_regs[dst_code];
	const int64_t tmp = (int64_t)sreg + (int64_t)dreg;

	SR &= ~(V_MASK | Z_MASK | N_MASK);
	SR |= ((sreg ^ tmp) & (dreg ^ tmp) & 0x80000000) >> 28;

//#if SETCARRYS
//  CHECK_C(tmp);
//#endif

	const int32_t res = sreg + dreg;
	m_local_regs[dst_code] = res;

	if (res == 0)
		SR |= Z_MASK;
	SR |= SIGN_TO_N(res);

	m_icount -= m_clock_cycles_1;

	if (SR & V_MASK)
		execute_exception(get_trap_addr(TRAPNO_RANGE_ERROR));
}

void hyperstone_device::hyperstone_adds_local_local()
{
	const uint32_t fp = GET_FP;
	const uint32_t dst_code = (DST_CODE + fp) & 0x3f;
	const int32_t sreg = (int32_t)m_local_regs[(SRC_CODE + fp) & 0x3f];
	const int32_t dreg = m_local_regs[dst_code];
	const int64_t tmp = (int64_t)sreg + (int64_t)dreg;

	SR &= ~(V_MASK | Z_MASK | N_MASK);
	SR |= ((sreg ^ tmp) & (dreg ^ tmp) & 0x80000000) >> 28;

//#if SETCARRYS
//  CHECK_C(tmp);
//#endif

	const int32_t res = sreg + dreg;
	m_local_regs[dst_code] = res;

	if (res == 0)
		SR |= Z_MASK;
	SR |= SIGN_TO_N(res);

	m_icount -= m_clock_cycles_1;

	if (SR & V_MASK)
		execute_exception(get_trap_addr(TRAPNO_RANGE_ERROR));
}



void hyperstone_device::hyperstone_cmpb_global_global()
{
	check_delay_PC();

	uint32_t sreg = m_global_regs[SRC_CODE];
	uint32_t dreg = m_global_regs[DST_CODE];

	if (dreg & sreg)
		SR &= ~Z_MASK;
	else
		SR |= Z_MASK;

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_cmpb_global_local()
{
	check_delay_PC();

	uint32_t sreg = m_local_regs[(SRC_CODE + GET_FP) & 0x3f];
	uint32_t dreg = m_global_regs[DST_CODE];

	if (dreg & sreg)
		SR &= ~Z_MASK;
	else
		SR |= Z_MASK;

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_cmpb_local_global()
{
	check_delay_PC();

	uint32_t sreg = m_global_regs[SRC_CODE];
	uint32_t dreg = m_local_regs[(DST_CODE + GET_FP) & 0x3f]; /* registers offset by frame pointer */

	if (dreg & sreg)
		SR &= ~Z_MASK;
	else
		SR |= Z_MASK;

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_cmpb_local_local()
{
	check_delay_PC();

	uint32_t sreg = m_local_regs[(SRC_CODE + GET_FP) & 0x3f];
	uint32_t dreg = m_local_regs[(DST_CODE + GET_FP) & 0x3f]; /* registers offset by frame pointer */

	if (dreg & sreg)
		SR &= ~Z_MASK;
	else
		SR |= Z_MASK;

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_andn_global_global()
{
	check_delay_PC();

	const uint32_t dst_code = DST_CODE;
	const uint32_t dreg = m_global_regs[dst_code] & ~m_global_regs[SRC_CODE];
	set_global_register(dst_code, dreg);

	if (dreg == 0)
		SR |= Z_MASK;
	else
		SR &= ~Z_MASK;

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_andn_global_local()
{
	check_delay_PC();

	const uint32_t dst_code = DST_CODE;
	const uint32_t dreg = m_global_regs[dst_code] & ~m_local_regs[(SRC_CODE + GET_FP) & 0x3f];
	set_global_register(dst_code, dreg);

	if (dreg == 0)
		SR |= Z_MASK;
	else
		SR &= ~Z_MASK;

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_andn_local_global()
{
	check_delay_PC();

	const uint32_t dst_code = (DST_CODE + GET_FP) & 0x3f;
	const uint32_t dreg = m_local_regs[dst_code] & ~m_global_regs[SRC_CODE];
	m_local_regs[dst_code] = dreg;

	if (dreg == 0)
		SR |= Z_MASK;
	else
		SR &= ~Z_MASK;

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_andn_local_local()
{
	check_delay_PC();

	const uint32_t fp = GET_FP;
	uint32_t dst_code = (DST_CODE + fp) & 0x3f;
	const uint32_t dreg = m_local_regs[dst_code] & ~m_local_regs[(SRC_CODE + fp) & 0x3f];
	m_local_regs[dst_code] = dreg;

	if (dreg == 0)
		SR |= Z_MASK;
	else
		SR &= ~Z_MASK;

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_or_global_global()
{
	check_delay_PC();

	const uint32_t dst_code = DST_CODE;
	const uint32_t dreg = m_global_regs[dst_code] | m_global_regs[SRC_CODE];
	set_global_register(dst_code, dreg);

	if (dreg == 0)
		SR |= Z_MASK;
	else
		SR &= ~Z_MASK;

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_or_global_local()
{
	check_delay_PC();

	const uint32_t dst_code = DST_CODE;
	const uint32_t dreg = m_global_regs[dst_code] | m_local_regs[(SRC_CODE + GET_FP) & 0x3f];
	set_global_register(dst_code, dreg);

	if (dreg == 0)
		SR |= Z_MASK;
	else
		SR &= ~Z_MASK;

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_or_local_global()
{
	check_delay_PC();

	const uint32_t dst_code = (DST_CODE + GET_FP) & 0x3f;
	const uint32_t dreg = m_local_regs[dst_code] | m_global_regs[SRC_CODE];
	m_local_regs[dst_code] = dreg;

	if (dreg == 0)
		SR |= Z_MASK;
	else
		SR &= ~Z_MASK;

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_or_local_local()
{
	check_delay_PC();

	const uint32_t dst_code = (DST_CODE + GET_FP) & 0x3f;
	const uint32_t dreg = m_local_regs[dst_code] | m_local_regs[(SRC_CODE + GET_FP) & 0x3f];
	m_local_regs[dst_code] = dreg;

	if (dreg == 0)
		SR |= Z_MASK;
	else
		SR &= ~Z_MASK;

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_xor_global_global()
{
	check_delay_PC();

	const uint32_t dst_code = DST_CODE;
	const uint32_t dreg = m_global_regs[dst_code] ^ m_global_regs[SRC_CODE];
	set_global_register(dst_code, dreg);

	if (dreg == 0)
		SR |= Z_MASK;
	else
		SR &= ~Z_MASK;

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_xor_global_local()
{
	check_delay_PC();

	const uint32_t dst_code = DST_CODE;
	const uint32_t dreg = m_global_regs[dst_code] ^ m_local_regs[(SRC_CODE + GET_FP) & 0x3f];
	set_global_register(dst_code, dreg);

	if (dreg == 0)
		SR |= Z_MASK;
	else
		SR &= ~Z_MASK;

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_xor_local_global()
{
	check_delay_PC();

	const uint32_t dst_code = (DST_CODE + GET_FP) & 0x3f;
	const uint32_t dreg = m_local_regs[dst_code] ^ m_global_regs[SRC_CODE];
	m_local_regs[dst_code] = dreg;

	if (dreg == 0)
		SR |= Z_MASK;
	else
		SR &= ~Z_MASK;

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_xor_local_local()
{
	check_delay_PC();

	const uint32_t fp = GET_FP;
	const uint32_t dst_code = (DST_CODE + fp) & 0x3f;
	const uint32_t dreg = m_local_regs[dst_code] ^ m_local_regs[(SRC_CODE + fp) & 0x3f];
	m_local_regs[dst_code] = dreg;

	if (dreg == 0)
		SR |= Z_MASK;
	else
		SR &= ~Z_MASK;

	m_icount -= m_clock_cycles_1;
}



void hyperstone_device::hyperstone_subc_global_global()
{
	const uint32_t src_code = SRC_CODE;
	const uint32_t dst_code = DST_CODE;
	uint32_t dreg = m_global_regs[dst_code];

	if (src_code == SR_REGISTER)
	{
		const uint32_t c = SR & C_MASK;
		const uint64_t tmp = (uint64_t)dreg - (uint64_t)c;
		SR &= ~(C_MASK | V_MASK | Z_MASK | N_MASK);
		SR |= ((tmp ^ dreg) & dreg & 0x80000000);
		SR |= (tmp & 0x100000000L) >> 32;
		dreg -= c;
	}
	else
	{
		const uint32_t sreg = m_global_regs[src_code];
		const uint32_t c = SR & C_MASK;
		const uint64_t tmp = (uint64_t)dreg - ((uint64_t)sreg + (uint64_t)c);
		SR &= ~(C_MASK | V_MASK | Z_MASK | N_MASK);
		//CHECK!
		const uint32_t sreg_c = sreg + c;
		SR |= ((tmp ^ dreg) & (dreg ^ sreg_c) & 0x80000000);
		SR |= (tmp & 0x100000000L) >> 32;
		dreg -= sreg_c;
	}

	set_global_register(DST_CODE, dreg);

	if (dreg == 0)
		SR |= Z_MASK;
	SR |= SIGN_TO_N(dreg);

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_subc_global_local()
{
	const uint32_t dst_code = DST_CODE;
	const uint32_t sreg = m_local_regs[(SRC_CODE + GET_FP) & 0x3f];
	uint32_t dreg = m_global_regs[dst_code];

	const uint32_t c = SR & C_MASK;
	const uint64_t tmp = (uint64_t)dreg - ((uint64_t)sreg + (uint64_t)c);

	//CHECK!
	const uint32_t sreg_c = sreg + c;

	SR &= ~(C_MASK | V_MASK | Z_MASK | N_MASK);
	SR |= ((tmp ^ dreg) & (dreg ^ sreg_c) & 0x80000000);
	SR |= (tmp & 0x100000000L) >> 32;
	dreg -= sreg_c;

	set_global_register(dst_code, dreg);

	if (dreg == 0)
		SR |= Z_MASK;
	SR |= SIGN_TO_N(dreg);

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_subc_local_global()
{
	const uint32_t src_code = SRC_CODE;
	const uint32_t dst_code = (DST_CODE + GET_FP) & 0x3f;
	uint32_t dreg = m_local_regs[dst_code];

	if (src_code == SR_REGISTER)
	{
		const uint32_t c = SR & C_MASK;
		const uint64_t tmp = (uint64_t)dreg - (uint64_t)c;
		SR &= ~(C_MASK | V_MASK | Z_MASK | N_MASK);
		SR |= ((tmp ^ dreg) & dreg & 0x80000000);
		SR |= (tmp & 0x100000000L) >> 32;
		dreg -= c;
	}
	else
	{
		const uint32_t sreg = m_global_regs[src_code];
		const uint32_t c = SR & C_MASK;
		const uint64_t tmp = (uint64_t)dreg - ((uint64_t)sreg + (uint64_t)c);
		SR &= ~(C_MASK | V_MASK | Z_MASK | N_MASK);
		//CHECK!
		const uint32_t sreg_c = sreg + c;
		SR |= ((tmp ^ dreg) & (dreg ^ sreg_c) & 0x80000000);
		SR |= (tmp & 0x100000000L) >> 32;
		dreg -= sreg_c;
	}

	m_local_regs[dst_code] = dreg;

	if (dreg == 0)
		SR |= Z_MASK;
	SR |= SIGN_TO_N(dreg);

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_subc_local_local()
{
	const uint32_t fp = GET_FP;
	const uint32_t dst_code = (DST_CODE + fp) & 0x3f;
	const uint32_t sreg = m_local_regs[(SRC_CODE + fp) & 0x3f];
	uint32_t dreg = m_local_regs[dst_code];

	const uint32_t c = SR & C_MASK;
	const uint64_t tmp = (uint64_t)dreg - ((uint64_t)sreg + (uint64_t)c);

	//CHECK!
	const uint32_t sreg_c = sreg + c;

	SR &= ~(C_MASK | V_MASK | Z_MASK | N_MASK);
	SR |= ((tmp ^ dreg) & (dreg ^ sreg_c) & 0x80000000);
	SR |= (tmp & 0x100000000L) >> 32;
	dreg -= sreg_c;

	m_local_regs[dst_code] = dreg;

	if (dreg == 0)
		SR |= Z_MASK;
	SR |= SIGN_TO_N(dreg);

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_not_global_global()
{
	check_delay_PC();

	const uint32_t dreg = ~m_global_regs[SRC_CODE];
	set_global_register(DST_CODE, dreg);

	if (dreg == 0)
		SR |= Z_MASK;
	else
		SR &= ~Z_MASK;

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_not_global_local()
{
	check_delay_PC();

	const uint32_t dreg = ~m_local_regs[(SRC_CODE + GET_FP) & 0x3f];
	set_global_register(DST_CODE, dreg);

	if (dreg == 0)
		SR |= Z_MASK;
	else
		SR &= ~Z_MASK;

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_not_local_global()
{
	check_delay_PC();

	const uint32_t dst_code = (DST_CODE + GET_FP) & 0x3f;
	const uint32_t dreg = ~m_global_regs[SRC_CODE];
	m_local_regs[dst_code] = dreg;

	if (dreg == 0)
		SR |= Z_MASK;
	else
		SR &= ~Z_MASK;

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_not_local_local()
{
	check_delay_PC();

	const uint32_t dst_code = (DST_CODE + GET_FP) & 0x3f;
	const uint32_t dreg = ~m_local_regs[(SRC_CODE + GET_FP) & 0x3f];
	m_local_regs[dst_code] = dreg;

	if (dreg == 0)
		SR |= Z_MASK;
	else
		SR &= ~Z_MASK;

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_sub_global_global()
{
	check_delay_PC();

	const uint32_t src_code = SRC_CODE;
	const uint32_t sreg = (src_code == SR_REGISTER) ? (SR & C_MASK) : m_global_regs[src_code];
	const uint32_t dst_code = DST_CODE;
	uint32_t dreg = m_global_regs[dst_code];

	const uint64_t tmp = (uint64_t)dreg - (uint64_t)sreg;

	SR &= ~(C_MASK | V_MASK | Z_MASK | N_MASK);

	SR |= (tmp & 0x100000000L) >> 32;
	SR |= ((tmp ^ dreg) & (dreg ^ sreg) & 0x80000000) >> 28;

	dreg -= sreg;
	set_global_register(dst_code, dreg);

	if (dst_code == PC_REGISTER)
		SR &= ~M_MASK;

	if (dreg == 0)
		SR |= Z_MASK;
	SR |= SIGN_TO_N(dreg);

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_sub_global_local()
{
	check_delay_PC();

	const uint32_t sreg = m_global_regs[(SRC_CODE + GET_FP) & 0x3f];
	const uint32_t dst_code = DST_CODE;
	uint32_t dreg = m_global_regs[dst_code];

	const uint64_t tmp = (uint64_t)dreg - (uint64_t)sreg;

	SR &= ~(C_MASK | V_MASK | Z_MASK | N_MASK);

	SR |= (tmp & 0x100000000L) >> 32;
	SR |= ((tmp ^ dreg) & (dreg ^ sreg) & 0x80000000) >> 28;

	dreg -= sreg;
	set_global_register(dst_code, dreg);

	if (dst_code == PC_REGISTER)
		SR &= ~M_MASK;

	if (dreg == 0)
		SR |= Z_MASK;
	SR |= SIGN_TO_N(dreg);

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_sub_local_global()
{
	check_delay_PC();

	const uint32_t src_code = SRC_CODE;
	const uint32_t sreg = (src_code == SR_REGISTER) ? (SR & C_MASK) : m_global_regs[src_code];
	const uint32_t dst_code = (DST_CODE + GET_FP) & 0x3f;
	uint32_t dreg = m_local_regs[dst_code];

	const uint64_t tmp = (uint64_t)dreg - (uint64_t)sreg;

	SR &= ~(C_MASK | V_MASK | Z_MASK | N_MASK);

	SR |= (tmp & 0x100000000L) >> 32;
	SR |= ((tmp ^ dreg) & (dreg ^ sreg) & 0x80000000) >> 28;

	dreg -= sreg;
	m_local_regs[dst_code] = dreg;

	if (dreg == 0)
		SR |= Z_MASK;
	SR |= SIGN_TO_N(dreg);

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_sub_local_local()
{
	check_delay_PC();

	const uint32_t fp = GET_FP;
	const uint32_t sreg = m_local_regs[(SRC_CODE + fp) & 0x3f];
	const uint32_t dst_code = (DST_CODE + fp) & 0x3f;
	uint32_t dreg = m_local_regs[dst_code];

	const uint64_t tmp = (uint64_t)dreg - (uint64_t)sreg;

	SR &= ~(C_MASK | V_MASK | Z_MASK | N_MASK);

	SR |= (tmp & 0x100000000L) >> 32;
	SR |= ((tmp ^ dreg) & (dreg ^ sreg) & 0x80000000) >> 28;

	dreg -= sreg;
	m_local_regs[dst_code] = dreg;

	if (dreg == 0)
		SR |= Z_MASK;
	SR |= SIGN_TO_N(dreg);

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_subs_global_global()
{
	const uint32_t src_code = SRC_CODE;
	const uint32_t dst_code = DST_CODE;
	const int32_t sreg = (src_code == SR_REGISTER) ? (SR & C_MASK) : (int32_t)m_global_regs[dst_code];
	const int32_t dreg = (int32_t)m_global_regs[dst_code];
	const int64_t tmp = (int64_t)dreg - (int64_t)sreg;

	SR &= ~(V_MASK | Z_MASK | N_MASK);

//#ifdef SETCARRYS
//  CHECK_C(tmp);
//#endif

	SR |= ((tmp ^ dreg) & (dreg ^ sreg) & 0x80000000) >> 28;

	const int32_t res = dreg - sreg;
	set_global_register(dst_code, res);

	if (res == 0)
		SR |= Z_MASK;
	SR |= SIGN_TO_N(res);

	m_icount -= m_clock_cycles_1;

	if (SR & V_MASK)
		execute_exception(get_trap_addr(TRAPNO_RANGE_ERROR));
}

void hyperstone_device::hyperstone_subs_global_local()
{
	const uint32_t dst_code = DST_CODE;
	const int32_t sreg = (int32_t)m_local_regs[(SRC_CODE + GET_FP) & 0x3f];
	const int32_t dreg = (int32_t)m_global_regs[dst_code];
	const int64_t tmp = (int64_t)dreg - (int64_t)sreg;

	SR &= ~(V_MASK | Z_MASK | N_MASK);

//#ifdef SETCARRYS
//  CHECK_C(tmp);
//#endif

	SR |= ((tmp ^ dreg) & (dreg ^ sreg) & 0x80000000) >> 28;

	const int32_t res = dreg - sreg;
	set_global_register(dst_code, res);

	if (res == 0)
		SR |= Z_MASK;
	SR |= SIGN_TO_N(res);

	m_icount -= m_clock_cycles_1;

	if (SR & V_MASK)
		execute_exception(get_trap_addr(TRAPNO_RANGE_ERROR));
}

void hyperstone_device::hyperstone_subs_local_global()
{
	const uint32_t src_code = SRC_CODE;
	const uint32_t dst_code = (DST_CODE + GET_FP) & 0x3f;
	const int32_t sreg = (src_code == SR_REGISTER) ? (SR & C_MASK) : (int32_t)m_global_regs[dst_code];
	const int32_t dreg = (int32_t)m_local_regs[dst_code];
	const int64_t tmp = (int64_t)dreg - (int64_t)sreg;

	SR &= ~(V_MASK | Z_MASK | N_MASK);

//#ifdef SETCARRYS
//  CHECK_C(tmp);
//#endif

	SR |= ((tmp ^ dreg) & (dreg ^ sreg) & 0x80000000) >> 28;

	const int32_t res = dreg - sreg;
	m_local_regs[dst_code] = res;

	if (res == 0)
		SR |= Z_MASK;
	SR |= SIGN_TO_N(res);

	m_icount -= m_clock_cycles_1;

	if (SR & V_MASK)
		execute_exception(get_trap_addr(TRAPNO_RANGE_ERROR));
}

void hyperstone_device::hyperstone_subs_local_local()
{
	const uint32_t fp = GET_FP;
	const uint32_t dst_code = (DST_CODE + fp) & 0x3f;
	const int32_t sreg = (int32_t)m_local_regs[(SRC_CODE + fp) & 0x3f];
	const int32_t dreg = (int32_t)m_local_regs[dst_code];
	const int64_t tmp = (int64_t)dreg - (int64_t)sreg;

	SR &= ~(V_MASK | Z_MASK | N_MASK);

//#ifdef SETCARRYS
//  CHECK_C(tmp);
//#endif

	SR |= ((tmp ^ dreg) & (dreg ^ sreg) & 0x80000000) >> 28;

	const int32_t res = dreg - sreg;
	m_local_regs[dst_code] = res;

	if (res == 0)
		SR |= Z_MASK;
	SR |= SIGN_TO_N(res);

	m_icount -= m_clock_cycles_1;

	if (SR & V_MASK)
		execute_exception(get_trap_addr(TRAPNO_RANGE_ERROR));
}



void hyperstone_device::hyperstone_addc_global_global()
{
	check_delay_PC();

	const uint32_t src_code = SRC_CODE;
	const uint32_t sreg = m_global_regs[src_code];

	const uint32_t dst_code = DST_CODE;
	uint32_t dreg = m_global_regs[dst_code];

	const bool old_z = (SR & Z_MASK) != 0;
	const uint32_t c = SR & C_MASK;

	SR &= ~(C_MASK | V_MASK | Z_MASK | N_MASK);

	uint64_t tmp;
	if (src_code == SR_REGISTER)
	{
		tmp = (uint64_t)dreg + (uint64_t)c;
		SR |= ((dreg ^ tmp) & (c ^ tmp) & 0x80000000) >> 28;
		dreg += c;
	}
	else
	{
		tmp = (uint64_t)sreg + (uint64_t)dreg + (uint64_t)c;
		SR |= ((sreg ^ tmp) & (dreg ^ tmp) & (c ^ tmp) & 0x80000000) >> 28;
		dreg += sreg + c;
	}

	SR |= (tmp & 0x100000000L) >> 32;

	set_global_register(dst_code, dreg);

	if (dreg == 0 && old_z)
		SR |= Z_MASK;
	SR |= SIGN_TO_N(dreg);

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_addc_global_local()
{
	check_delay_PC();

	const uint32_t src_code = (SRC_CODE + GET_FP) & 0x3f;
	const uint32_t sreg = m_local_regs[src_code];

	const uint32_t dst_code = DST_CODE;
	uint32_t dreg = m_global_regs[dst_code];

	const bool old_z = (SR & Z_MASK) != 0;
	const uint32_t c = SR & C_MASK;
	const uint64_t tmp = (uint64_t)sreg + (uint64_t)dreg + (uint64_t)c;

	SR &= ~(C_MASK | V_MASK | Z_MASK | N_MASK);

	SR |= (tmp & 0x100000000L) >> 32;
	SR |= ((sreg ^ tmp) & (dreg ^ tmp) & (c ^ tmp) & 0x80000000) >> 28;

	dreg += sreg + c;

	set_global_register(dst_code, dreg);

	if (dreg == 0 && old_z)
		SR |= Z_MASK;
	SR |= SIGN_TO_N(dreg);

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_addc_local_global()
{
	check_delay_PC();

	const uint32_t src_code = SRC_CODE;
	const uint32_t sreg = m_global_regs[src_code];

	const uint32_t dst_code = (DST_CODE + GET_FP) & 0x3f;
	uint32_t dreg = m_local_regs[dst_code];

	const bool old_z = (SR & Z_MASK) != 0;
	const uint32_t c = SR & C_MASK;

	SR &= ~(V_MASK | C_MASK | Z_MASK | N_MASK);

	uint64_t tmp;
	if (src_code == SR_REGISTER)
	{
		tmp = (uint64_t)dreg + (uint64_t)c;
		SR |= ((dreg ^ tmp) & (c ^ tmp) & 0x80000000) >> 28;
		dreg += c;
	}
	else
	{
		tmp = (uint64_t)sreg + (uint64_t)dreg + (uint64_t)c;
		SR |= ((sreg ^ tmp) & (dreg ^ tmp) & (c ^ tmp) & 0x80000000) >> 28;
		dreg += sreg + c;
	}

	SR |= (tmp & 0x100000000L) >> 32;

	m_local_regs[dst_code] = dreg;

	if (dreg == 0 && old_z)
		SR |= Z_MASK;
	SR |= SIGN_TO_N(dreg);

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_addc_local_local()
{
	check_delay_PC();

	const uint32_t fp = GET_FP;
	const uint32_t sreg = m_local_regs[(SRC_CODE + fp) & 0x3f];
	const uint32_t dst_code = (DST_CODE + fp) & 0x3f;
	uint32_t dreg = m_local_regs[dst_code];

	const bool old_z = (SR & Z_MASK) != 0;
	const uint32_t c = SR & C_MASK;
	const uint64_t tmp = (uint64_t)sreg + (uint64_t)dreg + (uint64_t)c;

	SR &= ~(C_MASK | V_MASK | Z_MASK | N_MASK);

	SR |= (tmp & 0x100000000L) >> 32;
	SR |= ((sreg ^ tmp) & (dreg ^ tmp) & (c ^ tmp) & 0x80000000) >> 28;

	dreg += sreg + c;

	m_local_regs[dst_code] = dreg;

	if (dreg == 0 && old_z)
		SR |= Z_MASK;
	SR |= SIGN_TO_N(dreg);

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_and_global_global()
{
	check_delay_PC();

	const uint32_t dst_code = DST_CODE;
	uint32_t dreg = m_global_regs[dst_code];
	dreg &= m_global_regs[SRC_CODE];
	set_global_register(dst_code, dreg);

	if (m_local_regs[dst_code] == 0)
		SR |= Z_MASK;
	else
		SR &= ~Z_MASK;

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_and_global_local()
{
	check_delay_PC();

	const uint32_t dst_code = DST_CODE;
	uint32_t dreg = m_global_regs[dst_code];
	dreg &= m_local_regs[(SRC_CODE + GET_FP) & 0x3f];
	set_global_register(dst_code, dreg);

	if (m_local_regs[dst_code] == 0)
		SR |= Z_MASK;
	else
		SR &= ~Z_MASK;

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_and_local_global()
{
	check_delay_PC();

	const uint32_t dst_code = (DST_CODE + GET_FP) & 0x3f;
	m_local_regs[dst_code] &= m_global_regs[SRC_CODE];

	if (m_local_regs[dst_code] == 0)
		SR |= Z_MASK;
	else
		SR &= ~Z_MASK;

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_and_local_local()
{
	check_delay_PC();

	const uint32_t fp = GET_FP;
	const uint32_t dst_code = (DST_CODE + fp) & 0x3f;
	m_local_regs[dst_code] &= m_local_regs[(SRC_CODE + fp) & 0x3f];

	if (m_local_regs[dst_code] == 0)
		SR |= Z_MASK;
	else
		SR &= ~Z_MASK;

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_neg_global_global()
{
	check_delay_PC();

	const uint32_t src_code = SRC_CODE;
	const uint32_t sreg = (src_code == SR_REGISTER) ? (SR & C_MASK) : m_global_regs[src_code];
	const uint64_t tmp = -(uint64_t)sreg;

	SR &= ~(C_MASK | V_MASK | Z_MASK | N_MASK);
	SR |= (tmp & 0x100000000L) >> 32;
	SR |= (tmp & sreg & 0x80000000) >> 28;

	const uint32_t dreg = -sreg;
	m_global_regs[DST_CODE] = dreg;

	if (dreg == 0)
		SR |= Z_MASK;
	SR |= SIGN_TO_N(dreg);

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_neg_global_local()
{
	check_delay_PC();

	const uint32_t sreg = m_local_regs[(SRC_CODE + GET_FP) & 0x3f];
	const uint64_t tmp = -(uint64_t)sreg;

	SR &= ~(C_MASK | V_MASK | Z_MASK | N_MASK);

	SR |= (tmp & 0x100000000L) >> 32;
	SR |= (tmp & sreg & 0x80000000) >> 28;

	const uint32_t dreg = -sreg;
	m_global_regs[DST_CODE] = dreg;

	if (dreg == 0)
		SR |= Z_MASK;
	SR |= SIGN_TO_N(dreg);

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_neg_local_global()
{
	check_delay_PC();

	const uint32_t fp = GET_FP;
	const uint32_t src_code = SRC_CODE;
	const uint32_t sreg = (src_code == SR_REGISTER) ? (SR & C_MASK) : m_global_regs[src_code];

	const uint64_t tmp = -(uint64_t)sreg;

	SR &= ~(C_MASK | V_MASK | Z_MASK | N_MASK);

	SR |= (tmp & 0x100000000L) >> 32;
	SR |= (tmp & sreg & 0x80000000) >> 28;

	const uint32_t dreg = -sreg;
	m_local_regs[(DST_CODE + fp) & 0x3f] = dreg;

	if (dreg == 0)
		SR |= Z_MASK;
	SR |= SIGN_TO_N(dreg);

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_neg_local_local()
{
	check_delay_PC();

	const uint32_t fp = GET_FP;
	const uint32_t sreg = m_local_regs[(SRC_CODE + fp) & 0x3f];

	const uint64_t tmp = -(uint64_t)sreg;

	SR &= ~(C_MASK | V_MASK | Z_MASK | N_MASK);

	SR |= (tmp & 0x100000000L) >> 32;
	SR |= (tmp & sreg & 0x80000000) >> 28;

	const uint32_t dreg = -sreg;
	m_local_regs[(DST_CODE + fp) & 0x3f] = dreg;

	if (dreg == 0)
		SR |= Z_MASK;
	SR |= SIGN_TO_N(dreg);

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_negs_global_global()
{
	check_delay_PC();

	const uint32_t src_code = SRC_CODE;
	const int32_t sreg = (src_code == SR_REGISTER) ? (SR & C_MASK) : (int32_t)m_global_regs[src_code];
	const int64_t tmp = -(int64_t)sreg;

	SR &= ~(V_MASK | Z_MASK | N_MASK);

	if (tmp & sreg & 0x80000000)
		SR |= V_MASK;

//#if SETCARRYS
//  CHECK_C(tmp);
//#endif

	const int32_t res = -sreg;
	set_global_register(DST_CODE, res);

	if (res == 0)
		SR |= Z_MASK;
	SR |= SIGN_TO_N(res);

	m_icount -= m_clock_cycles_1;

	if (GET_V && src_code != SR_REGISTER) // trap doesn't occur when source is SR
		execute_exception(get_trap_addr(TRAPNO_RANGE_ERROR));
}

void hyperstone_device::hyperstone_negs_global_local()
{
	check_delay_PC();

	const int32_t sreg = (int32_t)m_local_regs[(SRC_CODE + GET_FP) & 0x3f];
	const int64_t tmp = -(int64_t)sreg;

	SR &= ~(V_MASK | Z_MASK | N_MASK);

	if (tmp & sreg & 0x80000000)
		SR |= V_MASK;

//#if SETCARRYS
//  CHECK_C(tmp);
//#endif

	const int32_t res = -sreg;
	set_global_register(DST_CODE, res);

	if (res == 0)
		SR |= Z_MASK;
	SR |= SIGN_TO_N(res);

	m_icount -= m_clock_cycles_1;

	if (GET_V)
		execute_exception(get_trap_addr(TRAPNO_RANGE_ERROR));
}

void hyperstone_device::hyperstone_negs_local_global()
{
	check_delay_PC();

	const uint32_t src_code = SRC_CODE;
	const int32_t sreg = (src_code == SR_REGISTER) ? (SR & C_MASK) : (int32_t)m_global_regs[src_code];
	const int64_t tmp = -(int64_t)sreg;

	SR &= ~(V_MASK | Z_MASK | N_MASK);

	if (tmp & sreg & 0x80000000)
		SR |= V_MASK;

//#if SETCARRYS
//  CHECK_C(tmp);
//#endif

	const int32_t res = -sreg;
	m_local_regs[(DST_CODE + GET_FP) & 0x3f] = res;

	if (res == 0)
		SR |= Z_MASK;
	SR |= SIGN_TO_N(res);

	m_icount -= m_clock_cycles_1;

	if (GET_V && src_code != SR_REGISTER) // trap doesn't occur when source is SR
		execute_exception(get_trap_addr(TRAPNO_RANGE_ERROR));
}

void hyperstone_device::hyperstone_negs_local_local()
{
	check_delay_PC();

	const uint32_t fp = GET_FP;
	const int32_t sreg = (int32_t)m_local_regs[(SRC_CODE + fp) & 0x3f];
	const int64_t tmp = -(int64_t)sreg;

	SR &= ~(V_MASK | Z_MASK | N_MASK);

	if (tmp & sreg & 0x80000000)
		SR |= V_MASK;

//#if SETCARRYS
//  CHECK_C(tmp);
//#endif

	const int32_t res = -sreg;
	m_local_regs[(DST_CODE + fp) & 0x3f] = res;

	if (res == 0)
		SR |= Z_MASK;
	SR |= SIGN_TO_N(res);

	m_icount -= m_clock_cycles_1;

	if (GET_V)
		execute_exception(get_trap_addr(TRAPNO_RANGE_ERROR));
}



template <hyperstone_device::reg_bank DST_GLOBAL, hyperstone_device::imm_size IMM_LONG>
void hyperstone_device::hyperstone_cmpi()
{
	uint32_t imm;
	if (IMM_LONG)
		imm = decode_immediate_s();

	check_delay_PC();

	if (!IMM_LONG)
		imm = immediate_values[m_op & 0x0f];
	const uint32_t dst_code = DST_GLOBAL ? DST_CODE : ((DST_CODE + GET_FP) & 0x3f);
	const uint32_t dreg = (DST_GLOBAL ? m_global_regs : m_local_regs)[dst_code];

	SR &= ~(V_MASK | Z_MASK | N_MASK | C_MASK);

	uint64_t tmp = (uint64_t)dreg - (uint64_t)imm;
	SR |= ((tmp ^ dreg) & (dreg ^ imm) & 0x80000000) >> 28;

	if (dreg == imm)
		SR |= Z_MASK;

	if ((int32_t)dreg < (int32_t)imm)
		SR |= N_MASK;

	if (dreg < imm)
		SR |= C_MASK;

	m_icount -= m_clock_cycles_1;
}

template <hyperstone_device::reg_bank DST_GLOBAL, hyperstone_device::imm_size IMM_LONG>
void hyperstone_device::hyperstone_movi()
{
	uint32_t imm;
	if (IMM_LONG)
		imm = decode_immediate_s();
	check_delay_PC();

	if (!IMM_LONG)
		imm = immediate_values[m_op & 0x0f];
	if (DST_GLOBAL)
	{
		uint32_t dst_code = DST_CODE;
		if (SR & H_MASK)
		{
			dst_code += 16;
			if (!(SR & S_MASK))
				execute_exception(get_trap_addr(TRAPNO_PRIVILEGE_ERROR));
		}

		set_global_register(dst_code, imm);

		if (dst_code == PC_REGISTER)
			SR &= ~M_MASK;
	}
	else
	{
		m_local_regs[(DST_CODE + GET_FP) & 0x3f] = imm;
	}

	SR &= ~(Z_MASK | N_MASK);
	if (imm == 0)
		SR |= Z_MASK;
	SR |= SIGN_TO_N(imm);

#if MISSIONCRAFT_FLAGS
	SR &= ~V_MASK; // or V undefined ?
#endif

	m_icount -= m_clock_cycles_1;
}

template <hyperstone_device::reg_bank DST_GLOBAL, hyperstone_device::imm_size IMM_LONG>
void hyperstone_device::hyperstone_addi()
{
	uint32_t imm;
	if (IMM_LONG)
		imm = decode_immediate_s();
	check_delay_PC();

	const uint32_t dst_code = DST_GLOBAL ? DST_CODE : ((DST_CODE + GET_FP) & 0x3f);
	uint32_t dreg = (DST_GLOBAL ? m_global_regs : m_local_regs)[dst_code];

	if (!N_OP_MASK)
		imm = GET_C & (((SR & Z_MASK) ? 0 : 1) | (dreg & 0x01));
	else if (!IMM_LONG)
		imm = immediate_values[m_op & 0x0f];

	SR &= ~(C_MASK | V_MASK | Z_MASK | N_MASK);

	const uint64_t tmp = (uint64_t)imm + (uint64_t)dreg;

	SR |= (tmp & 0x100000000L) >> 32;
	SR |= ((imm ^ tmp) & (dreg ^ tmp) & 0x80000000) >> 28;

	dreg += imm;
	if (DST_GLOBAL)
	{
		set_global_register(dst_code, dreg);

		if (dst_code == 0)
			SR &= ~M_MASK;
	}
	else
	{
		m_local_regs[dst_code] = dreg;
	}

	if (dreg == 0)
		SR |= Z_MASK;
	SR |= SIGN_TO_N(dreg);

	m_icount -= m_clock_cycles_1;
}

template <hyperstone_device::reg_bank DST_GLOBAL, hyperstone_device::imm_size IMM_LONG>
void hyperstone_device::hyperstone_addsi()
{
	if (!IMM_LONG)
		check_delay_PC();

	const uint32_t dst_code = DST_GLOBAL ? DST_CODE : ((DST_CODE + GET_FP) & 0x3f);
	const int32_t dreg = (DST_GLOBAL ? m_global_regs : m_local_regs)[dst_code];

	int32_t imm;
	if (N_OP_MASK)
	{
		if (IMM_LONG)
		{
			imm = decode_immediate_s();
			check_delay_PC();
		}
		else
		{
			imm = immediate_values[m_op & 0x0f];
		}
	}
	else
	{
		if (IMM_LONG)
		{
			ignore_immediate_s();
			check_delay_PC();
		}
		imm = SR & (((SR & Z_MASK) ? 0 : 1) | (dreg & 0x01));
	}

	SR &= ~(V_MASK | Z_MASK | N_MASK);

	const int64_t tmp = (int64_t)imm + (int64_t)(int32_t)dreg;
	SR |= ((imm ^ tmp) & (dreg ^ tmp) & 0x80000000) >> 28;

//#if SETCARRYS
//  CHECK_C(tmp);
//#endif

	const int32_t res = imm + (int32_t)dreg;

	if (DST_GLOBAL)
		set_global_register(dst_code, res);
	else
		m_local_regs[dst_code] = res;

	if (res == 0)
		SR |= Z_MASK;
	SR |= SIGN_TO_N(res);

	m_icount -= m_clock_cycles_1;

	if (SR & V_MASK)
		execute_exception(get_trap_addr(TRAPNO_RANGE_ERROR));
}



void hyperstone_device::hyperstone_cmpbi_global_simm()
{
	check_delay_PC();

	const uint32_t dreg = m_global_regs[DST_CODE];

	const uint32_t n = N_VALUE;
	if (n)
	{
		uint32_t imm;
		if (n == 31)
			imm = 0x7fffffff; // bit 31 = 0, others = 1
		else
			imm = immediate_values[m_op & 0x0f];

		if (dreg & imm)
			SR &= ~Z_MASK;
		else
			SR |= Z_MASK;
	}
	else
	{
		if ((dreg & 0xff000000) == 0 || (dreg & 0x00ff0000) == 0 || (dreg & 0x0000ff00) == 0 || (dreg & 0x000000ff) == 0)
			SR |= Z_MASK;
		else
			SR &= ~Z_MASK;
	}

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_cmpbi_global_limm()
{
	const uint32_t dreg = m_global_regs[DST_CODE];

	const uint32_t n = N_VALUE;
	if (n)
	{
		uint32_t imm;
		if (n == 31)
		{
			ignore_immediate_s();
			check_delay_PC();
			imm = 0x7fffffff; // bit 31 = 0, others = 1
		}
		else
		{
			imm = decode_immediate_s();
			check_delay_PC();
		}

		if (dreg & imm)
			SR &= ~Z_MASK;
		else
			SR |= Z_MASK;
	}
	else
	{
		ignore_immediate_s();
		check_delay_PC();
		if ((dreg & 0xff000000) == 0 || (dreg & 0x00ff0000) == 0 || (dreg & 0x0000ff00) == 0 || (dreg & 0x000000ff) == 0)
			SR |= Z_MASK;
		else
			SR &= ~Z_MASK;
	}

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_cmpbi_local_simm()
{
	check_delay_PC();

	const uint32_t dreg = m_local_regs[(DST_CODE + GET_FP) & 0x3f];

	const uint32_t n = N_VALUE;
	if (n)
	{
		uint32_t imm;
		if (n == 31)
		{
			imm = 0x7fffffff; // bit 31 = 0, others = 1
		}
		else
		{
			imm = immediate_values[m_op & 0x0f];
		}

		if (dreg & imm)
			SR &= ~Z_MASK;
		else
			SR |= Z_MASK;
	}
	else
	{
		if ((dreg & 0xff000000) == 0 || (dreg & 0x00ff0000) == 0 || (dreg & 0x0000ff00) == 0 || (dreg & 0x000000ff) == 0)
			SR |= Z_MASK;
		else
			SR &= ~Z_MASK;
	}

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_cmpbi_local_limm()
{
	const uint32_t dreg = m_local_regs[(DST_CODE + GET_FP) & 0x3f];

	const uint32_t n = N_VALUE;
	if (n)
	{
		uint32_t imm;
		if (n == 31)
		{
			ignore_immediate_s();
			check_delay_PC();
			imm = 0x7fffffff; // bit 31 = 0, others = 1
		}
		else
		{
			imm = decode_immediate_s();
			check_delay_PC();
		}

		if (dreg & imm)
			SR &= ~Z_MASK;
		else
			SR |= Z_MASK;
	}
	else
	{
		ignore_immediate_s();
		check_delay_PC();
		if ((dreg & 0xff000000) == 0 || (dreg & 0x00ff0000) == 0 || (dreg & 0x0000ff00) == 0 || (dreg & 0x000000ff) == 0)
			SR |= Z_MASK;
		else
			SR &= ~Z_MASK;
	}

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_andni_global_simm()
{
	check_delay_PC();

	uint32_t imm;
	if (N_OP_MASK == 0x10f)
		imm = 0x7fffffff; // bit 31 = 0, others = 1
	else
		imm = immediate_values[m_op & 0x0f];

	const uint32_t dst_code = DST_CODE;
	const uint32_t dreg = m_global_regs[dst_code] & ~imm;
	set_global_register(dst_code, dreg);

	if (dreg == 0)
		SR |= Z_MASK;
	else
		SR &= ~Z_MASK;

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_andni_global_limm()
{
	const uint32_t extra_u = decode_immediate_s();

	check_delay_PC();

	uint32_t imm;
	if (N_OP_MASK == 0x10f)
		imm = 0x7fffffff; // bit 31 = 0, others = 1
	else
		imm = extra_u;

	const uint32_t dst_code = DST_CODE;
	const uint32_t dreg = m_global_regs[dst_code] & ~imm;
	set_global_register(dst_code, dreg);

	if (dreg == 0)
		SR |= Z_MASK;
	else
		SR &= ~Z_MASK;

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_andni_local_simm()
{
	check_delay_PC();

	uint32_t imm;
	if (N_OP_MASK == 0x10f)
		imm = 0x7fffffff; // bit 31 = 0, others = 1
	else
		imm = immediate_values[m_op & 0x0f];

	const uint32_t dst_code = (DST_CODE + GET_FP) & 0x3f;
	const uint32_t dreg = m_local_regs[dst_code] & ~imm;
	m_local_regs[dst_code] = dreg;

	if (dreg == 0)
		SR |= Z_MASK;
	else
		SR &= ~Z_MASK;

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_andni_local_limm()
{
	const uint32_t extra_u = decode_immediate_s();

	check_delay_PC();

	const uint32_t dst_code = (DST_CODE + GET_FP) & 0x3f;

	uint32_t imm;
	if (N_OP_MASK == 0x10f)
		imm = 0x7fffffff; // bit 31 = 0, others = 1
	else
		imm = extra_u;

	const uint32_t dreg = m_local_regs[dst_code] & ~imm;
	m_local_regs[dst_code] = dreg;

	if (dreg == 0)
		SR |= Z_MASK;
	else
		SR &= ~Z_MASK;

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_ori_global_simm()
{
	check_delay_PC();

	const uint32_t dst_code = DST_CODE;
	const uint32_t dreg = m_global_regs[dst_code] | immediate_values[m_op & 0x0f];
	set_global_register(dst_code, dreg);

	if (dreg)
		SR &= ~Z_MASK;
	else
		SR |= Z_MASK;

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_ori_global_limm()
{
	uint32_t extra_u = decode_immediate_s();

	check_delay_PC();

	const uint32_t dst_code = DST_CODE;
	const uint32_t dreg = m_global_regs[dst_code] | extra_u;
	set_global_register(dst_code, dreg);

	if (dreg)
		SR &= ~Z_MASK;
	else
		SR |= Z_MASK;

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_ori_local_simm()
{
	check_delay_PC();

	const uint32_t dst_code = (DST_CODE + GET_FP) & 0x3f;
	m_local_regs[dst_code] |= immediate_values[m_op & 0x0f];

	if (m_local_regs[dst_code] == 0)
		SR |= Z_MASK;
	else
		SR &= ~Z_MASK;

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_ori_local_limm()
{
	const uint32_t extra_u = decode_immediate_s();
	check_delay_PC();

	const uint32_t dst_code = (DST_CODE + GET_FP) & 0x3f;
	m_local_regs[dst_code] |= extra_u;

	if (m_local_regs[dst_code] == 0)
		SR |= Z_MASK;
	else
		SR &= ~Z_MASK;

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_xori_global_simm()
{
	check_delay_PC();

	const uint32_t dst_code = DST_CODE;
	const uint32_t dreg = m_global_regs[dst_code] ^ immediate_values[m_op & 0x0f];
	set_global_register(dst_code, dreg);

	if (dreg == 0)
		SR |= Z_MASK;
	else
		SR &= ~Z_MASK;

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_xori_global_limm()
{
	const uint32_t extra_u = decode_immediate_s();

	check_delay_PC();

	const uint32_t dst_code = DST_CODE;
	const uint32_t dreg = m_global_regs[dst_code] ^ extra_u;
	set_global_register(dst_code, dreg);

	if (dreg == 0)
		SR |= Z_MASK;
	else
		SR &= ~Z_MASK;

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_xori_local_simm()
{
	check_delay_PC();

	const uint32_t dst_code = (DST_CODE + GET_FP) & 0x3f;
	const uint32_t dreg = m_local_regs[dst_code] ^ immediate_values[m_op & 0x0f];
	m_local_regs[dst_code] = dreg;

	if (dreg == 0)
		SR |= Z_MASK;
	else
		SR &= ~Z_MASK;

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_xori_local_limm()
{
	const uint32_t extra_u = decode_immediate_s();

	check_delay_PC();

	const uint32_t dst_code = (DST_CODE + GET_FP) & 0x3f;
	const uint32_t dreg = m_local_regs[dst_code] ^ extra_u;
	m_local_regs[dst_code] = dreg;

	if (dreg == 0)
		SR |= Z_MASK;
	else
		SR &= ~Z_MASK;

	m_icount -= m_clock_cycles_1;
}



void hyperstone_device::hyperstone_shrdi()
{
	check_delay_PC();

	const uint32_t fp = GET_FP;
	const uint32_t code = DST_CODE;
	const uint32_t dst_code = (code + fp) & 0x3f;
	const uint32_t dstf_code = (code + 1 + fp) & 0x3f;
	uint32_t high_order = m_local_regs[dst_code];
	const uint32_t low_order  = m_local_regs[dstf_code];

	uint64_t val = concat_64(high_order, low_order);

	SR &= ~(C_MASK | Z_MASK | N_MASK);

	const uint32_t n = N_VALUE;
	if (n)
	{
		SR |= (val >> (n - 1)) & 1;

		val >>= n;
	}

	high_order = extract_64hi(val);

	m_local_regs[dst_code] = high_order;
	m_local_regs[dstf_code] = extract_64lo(val);

	if (val == 0)
		SR |= Z_MASK;
	SR |= SIGN_TO_N(high_order);

	m_icount -= m_clock_cycles_2;
}

void hyperstone_device::hyperstone_shrd()
{
	check_delay_PC();

	const uint32_t fp = GET_FP;
	const uint32_t src_code = (SRC_CODE + fp) & 0x3f;
	const uint32_t d_code = DST_CODE;
	const uint32_t dst_code = (d_code + fp) & 0x3f;
	const uint32_t dstf_code = (d_code + 1 + fp) & 0x3f;

	if (src_code == dst_code || src_code == dstf_code)
	{
		m_icount -= m_clock_cycles_2;
		return;
	}

	uint64_t val = concat_64(m_local_regs[dst_code], m_local_regs[dstf_code]);

	SR &= ~(C_MASK | Z_MASK | N_MASK);

	const uint32_t n = m_local_regs[src_code] & 0x1f;

	if (n)
	{
		SR |= (val >> (n - 1)) & 1;
		val >>= n;
	}

	m_local_regs[dst_code] = (uint32_t)(val >> 32);
	m_local_regs[dstf_code] = (uint32_t)val;

	if (val == 0)
		SR |= Z_MASK;
	SR |= SIGN_TO_N(m_local_regs[dst_code]);

	m_icount -= m_clock_cycles_2;
}

void hyperstone_device::hyperstone_shr()
{
	check_delay_PC();

	const uint32_t fp = GET_FP;
	const uint32_t dst_code = (DST_CODE + fp) & 0x3f;
	const uint32_t n = m_local_regs[(SRC_CODE + fp) & 0x3f] & 0x1f;
	uint32_t dreg = m_local_regs[dst_code];

	SR &= ~(C_MASK | Z_MASK | N_MASK);

	if (n && (dreg & (1 << (n - 1))))
		SR |= C_MASK;

	dreg >>= n;

	m_local_regs[dst_code] = dreg;
	if (dreg == 0)
		SR |= Z_MASK;
	SR |= SIGN_TO_N(dreg);

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_sardi()
{
	check_delay_PC();

	const uint32_t dst_code = (DST_CODE + GET_FP) & 0x3f;
	const uint32_t dstf_code = (dst_code + 1) & 0x3f;

	uint64_t val = concat_64(m_local_regs[dst_code], m_local_regs[dstf_code]);

	SR &= ~(C_MASK | Z_MASK | N_MASK);

	const uint32_t n = N_VALUE;
	if (n)
	{
		SR |= (val >> (n - 1)) & 1;

		const uint64_t sign_bit = val >> 63;
		val >>= n;

		if (sign_bit)
			val |= 0xffffffff00000000U << (32 - n);
	}

	m_local_regs[dst_code] = (uint32_t)(val >> 32);
	m_local_regs[dstf_code] = (uint32_t)val;

	if (val == 0)
		SR |= Z_MASK;
	SR |= SIGN_TO_N(m_local_regs[dst_code]);

	m_icount -= m_clock_cycles_2;
}

void hyperstone_device::hyperstone_sard()
{
	check_delay_PC();

	const uint32_t fp = GET_FP;
	const uint32_t src_code = (SRC_CODE + fp) & 0x3f;
	const uint32_t d_code = DST_CODE;
	const uint32_t dst_code = (d_code + fp) & 0x3f;
	const uint32_t dstf_code = (d_code + 1 + fp) & 0x3f;

	if (src_code == dst_code || src_code == dstf_code)
	{
		m_icount -= m_clock_cycles_2;
		return;
	}

	uint64_t val = concat_64(m_local_regs[dst_code], m_local_regs[dstf_code]);

	SR &= ~(C_MASK | Z_MASK | N_MASK);

	const uint32_t n = m_local_regs[src_code] & 0x1f;
	if (n)
	{
		SR |= (val >> (n - 1)) & 1;

		uint32_t sign_bit = val >> 63;

		val >>= n;

		if (sign_bit)
		{
			val |= 0xffffffff00000000L << (32 - n);
		}
	}

	m_local_regs[dst_code] = (uint32_t)(val >> 32);
	m_local_regs[dstf_code] = (uint32_t)val;
	if (val == 0)
		SR |= Z_MASK;
	SR |= SIGN_TO_N(m_local_regs[dst_code]);

	m_icount -= m_clock_cycles_2;
}

void hyperstone_device::hyperstone_sar()
{
	check_delay_PC();

	const uint32_t fp = GET_FP;
	const uint32_t dst_code = (DST_CODE + fp) & 0x3f;

	const uint32_t n = m_local_regs[(SRC_CODE + fp) & 0x3f] & 0x1f;
	uint32_t ret = m_local_regs[dst_code];

	SR &= ~(C_MASK | Z_MASK | N_MASK);

	if (n)
	{
		const uint32_t sign_bit = ret & 0x80000000;

		SR |= (ret >> (n - 1)) & 1;

		ret >>= n;

		if (sign_bit)
			ret |= 0xffffffff << (32 - n);
	}

	m_local_regs[dst_code] = ret;

	if (ret == 0)
		SR |= Z_MASK;
	SR |= SIGN_TO_N(ret);

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_shldi()
{
	check_delay_PC();

	const uint32_t fp = GET_FP;
	const uint32_t code = DST_CODE;
	const uint32_t dst_code = (code + fp) & 0x3f;
	const uint32_t dstf_code = (code + 1 + fp) & 0x3f;
	uint32_t high_order = m_local_regs[dst_code];
	uint32_t low_order  = m_local_regs[dstf_code];

	uint64_t val = concat_64(high_order, low_order);

	SR &= ~(C_MASK | V_MASK | Z_MASK | N_MASK);

	const uint32_t n = N_VALUE;

	if (n && ((val << (n - 1)) & 0x8000000000000000U))
		SR |= C_MASK;

	const uint64_t mask = ((1U << (32 - n)) - 1) ^ 0xffffffff;
	const uint32_t tmp  = high_order << n;

	if (((high_order & mask) && (!(tmp & 0x80000000))) || (((high_order & mask) ^ mask) && (tmp & 0x80000000)))
		SR |= V_MASK;

	val <<= n;

	high_order = extract_64hi(val);

	m_local_regs[dst_code] = high_order;
	m_local_regs[dstf_code] = extract_64lo(val);

	if (val == 0)
		SR |= Z_MASK;
	SR |= SIGN_TO_N(high_order);

	m_icount -= m_clock_cycles_2;
}

void hyperstone_device::hyperstone_shld()
{
	check_delay_PC();

	const uint32_t fp = GET_FP;
	const uint32_t d_code = DST_CODE;
	uint32_t src_code = (SRC_CODE + fp) & 0x3f;
	uint32_t dst_code = (d_code + fp) & 0x3f;
	uint32_t dstf_code = (d_code + fp + 1) & 0x3f;

	// result undefined if Ls denotes the same register as Ld or Ldf
	if (src_code == dst_code || src_code == dstf_code)
	{
		DEBUG_PRINTF(("Denoted same registers in hyperstone_shld. PC = %08X\n", PC));
		m_icount -= m_clock_cycles_2;
		return;
	}

	uint32_t n = m_local_regs[src_code & 0x3f] & 0x1f;
	uint32_t high_order = m_local_regs[dst_code]; /* registers offset by frame pointer */
	uint32_t low_order  = m_local_regs[dstf_code];

	uint64_t mask = ((((uint64_t)1) << (32 - n)) - 1) ^ 0xffffffff;

	uint64_t val = concat_64(high_order, low_order);

	SR &= ~(C_MASK | V_MASK | Z_MASK | N_MASK);
	SR |= (n)?(((val<<(n-1))&0x8000000000000000U)?1:0):0;

	uint32_t tmp = high_order << n;
	if (((high_order & mask) && (!(tmp & 0x80000000))) || (((high_order & mask) ^ mask) && (tmp & 0x80000000)))
		SR |= V_MASK;

	val <<= n;

	m_local_regs[dst_code] = extract_64hi(val);
	m_local_regs[dstf_code] = extract_64lo(val);

	if (val == 0)
		SR |= Z_MASK;
	SR |= SIGN_TO_N(m_local_regs[dst_code]);

	m_icount -= m_clock_cycles_2;
}

void hyperstone_device::hyperstone_shl()
{
	check_delay_PC();

	const uint32_t fp = GET_FP;
	uint32_t src_code = SRC_CODE + fp;
	uint32_t dst_code = DST_CODE + fp;

	uint32_t n    = m_local_regs[src_code & 0x3f] & 0x1f;
	uint32_t base = m_local_regs[dst_code & 0x3f]; /* registers offset by frame pointer */
	uint64_t mask = ((((uint64_t)1) << (32 - n)) - 1) ^ 0xffffffff;

	SR &= ~(C_MASK | V_MASK | Z_MASK | N_MASK);

	SR |= (n)?(((base<<(n-1))&0x80000000)?1:0):0;
	uint32_t ret  = base << n;

	if (((base & mask) && (!(ret & 0x80000000))) || (((base & mask) ^ mask) && (ret & 0x80000000)))
		SR |= V_MASK;

	m_local_regs[dst_code & 0x3f] = ret;

	if (ret == 0)
		SR |= Z_MASK;
	SR |= SIGN_TO_N(ret);

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_testlz()
{
	check_delay_PC();

	const uint32_t fp = GET_FP;
	const uint32_t sreg = m_local_regs[(SRC_CODE + fp) & 0x3f];
	uint32_t zeros = 0;
	for (uint32_t mask = 0x80000000; mask != 0; mask >>= 1 )
	{
		if (sreg & mask)
			break;
		else
			zeros++;
	}

	m_local_regs[(DST_CODE + fp) & 0x3f] = zeros;

	m_icount -= m_clock_cycles_2;
}

void hyperstone_device::hyperstone_rol()
{
	check_delay_PC();

	const uint32_t fp = GET_FP;
	const uint32_t dst_code = (DST_CODE + fp) & 0x3f;

	uint32_t n = m_local_regs[(SRC_CODE + fp) & 0x3f] & 0x1f;
	uint32_t val = m_local_regs[dst_code];

#ifdef MISSIONCRAFT_FLAGS
	const uint32_t base = val;
	const uint64_t mask = ((1U << (32 - n)) - 1) ^ 0xffffffff;
#endif

	if (n)
		val = (val << n) | (val >> (32 - n));

#ifdef MISSIONCRAFT_FLAGS
	SR &= ~(V_MASK | Z_MASK | C_MASK);
	if (((base & mask) && (!(val & 0x80000000))) || (((base & mask) ^ mask) && (val & 0x80000000)))
		SR |= V_MASK;
#else
	SR &= ~(Z_MASK | C_MASK | N_MASK);
#endif

	m_local_regs[dst_code] = val;

	if (val == 0)
		SR |= Z_MASK;
	SR |= SIGN_TO_N(val);

	m_icount -= m_clock_cycles_1;
}



void hyperstone_device::hyperstone_ldxx1_global_global()
{
	uint16_t next_1 = READ_OP(PC);
	PC += 2;

	const uint16_t sub_type = (next_1 & 0x3000) >> 12;

	uint32_t extra_s;
	if (next_1 & 0x8000)
	{
		const uint16_t next_2 = READ_OP(PC);
		PC += 2;
		m_instruction_length = (3<<19);

		extra_s = next_2;
		extra_s |= ((next_1 & 0xfff) << 16);

		if (next_1 & 0x4000)
			extra_s |= 0xf0000000;
	}
	else
	{
		m_instruction_length = (2<<19);
		extra_s = next_1 & 0xfff;

		if (next_1 & 0x4000)
			extra_s |= 0xfffff000;
	}

	check_delay_PC();

	const uint32_t src_code = SRC_CODE;
	const uint32_t dst_code = DST_CODE;
	const uint32_t srcf_code = src_code + 1;

	if (dst_code == SR_REGISTER)
	{
		switch (sub_type)
		{
			case 0: // LDBS.A
				m_global_regs[src_code] = (int32_t)(int8_t)READ_B(extra_s);
				break;

			case 1: // LDBU.A
				m_global_regs[src_code] = READ_B(extra_s);
				break;

			case 2:
				if (extra_s & 1) // LDHS.A
					m_global_regs[src_code] = (int32_t)(int16_t)READ_HW(extra_s);
				else // LDHU.A
					m_global_regs[src_code] = READ_HW(extra_s);
				break;

			case 3:
				switch (extra_s & 3)
				{
					case 0: // LDW.A
						m_global_regs[src_code] = READ_W(extra_s);
						break;
					case 1: // LDD.A
						m_global_regs[src_code] = READ_W(extra_s);
						m_global_regs[srcf_code] = READ_W(extra_s + 4);
						m_icount -= m_clock_cycles_1; // extra cycle
						break;
					case 2: // LDW.IOA
						m_global_regs[src_code] = IO_READ_W(extra_s);
						break;
					case 3: // LDD.IOA
						m_global_regs[src_code] = IO_READ_W(extra_s);
						m_global_regs[srcf_code] = IO_READ_W((extra_s & ~3) + 4);
						m_icount -= m_clock_cycles_1; // extra cycle
						break;
				}
				break;
		}
	}
	else
	{
		const uint32_t dreg = m_global_regs[dst_code];
		switch (sub_type)
		{
			case 0: // LDBS.D
				m_global_regs[src_code] = (int32_t)(int8_t)READ_B(dreg + extra_s);
				break;

			case 1: // LDBU.D
				m_global_regs[src_code] = READ_B(dreg + extra_s);
				break;

			case 2:
				if (extra_s & 1)
					m_global_regs[src_code] = (int32_t)(int16_t)READ_HW(dreg + (extra_s & ~1));
				else
					m_global_regs[src_code] = READ_HW(dreg + (extra_s & ~1));
				break;

			case 3:
				switch (extra_s & 3)
				{
					case 0: // LDW.D
						m_global_regs[src_code] = READ_W(dreg + (extra_s & ~1));
						break;
					case 1: // LDD.D
						m_global_regs[src_code] = READ_W(dreg + (extra_s & ~1));
						m_global_regs[srcf_code] = READ_W(dreg + (extra_s & ~1) + 4);
						m_icount -= m_clock_cycles_1; // extra cycle
						break;
					case 2: // LDW.IOD
						m_global_regs[src_code] = IO_READ_W(dreg + (extra_s & ~3));
						break;
					case 3: // LDD.IOD
						m_global_regs[src_code] = IO_READ_W(dreg + (extra_s & ~3));
						m_global_regs[srcf_code] = IO_READ_W(dreg + (extra_s & ~3) + 4);
						m_icount -= m_clock_cycles_1; // extra cycle
						break;
				}
				break;
		}
	}

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_ldxx1_global_local()
{
	uint16_t next_1 = READ_OP(PC);
	PC += 2;

	const uint16_t sub_type = (next_1 & 0x3000) >> 12;

	uint32_t extra_s;
	if (next_1 & 0x8000)
	{
		const uint16_t next_2 = READ_OP(PC);
		PC += 2;
		m_instruction_length = (3<<19);

		extra_s = next_2;
		extra_s |= ((next_1 & 0xfff) << 16);

		if (next_1 & 0x4000)
			extra_s |= 0xf0000000;
	}
	else
	{
		m_instruction_length = (2<<19);
		extra_s = next_1 & 0xfff;

		if (next_1 & 0x4000)
			extra_s |= 0xfffff000;
	}

	check_delay_PC();

	const uint32_t src_code = (SRC_CODE + GET_FP) & 0x3f;

	if (DST_CODE == SR_REGISTER)
	{
		switch (sub_type)
		{
			case 0: // LDBS.A
				m_local_regs[src_code] = (int32_t)(int8_t)READ_B(extra_s);
				break;

			case 1: // LDBU.A
				m_local_regs[src_code] = READ_B(extra_s);
				break;

			case 2:
				if (extra_s & 1) // LDHS.A
					m_local_regs[src_code] = (int32_t)(int16_t)READ_HW(extra_s);
				else // LDHU.A
					m_local_regs[src_code] = READ_HW(extra_s);
				break;

			case 3:
				switch (extra_s & 3)
				{
					case 0: // LDW.A
						m_local_regs[src_code] = READ_W(extra_s);
						break;
					case 1: // LDD.A
						m_local_regs[src_code] = READ_W(extra_s);
						m_local_regs[(src_code + 1) & 0x3f] = READ_W(extra_s + 4);
						m_icount -= m_clock_cycles_1; // extra cycle
						break;
					case 2: // LDW.IOA
						m_local_regs[src_code] = IO_READ_W(extra_s);
						break;
					case 3: // LDD.IOA
						m_local_regs[src_code] = IO_READ_W(extra_s);
						m_local_regs[(src_code + 1) & 0x3f] = IO_READ_W((extra_s & ~3) + 4);
						m_icount -= m_clock_cycles_1; // extra cycle
						break;
				}
				break;
		}
	}
	else
	{
		const uint32_t dreg = m_global_regs[DST_CODE];
		switch (sub_type)
		{
			case 0: // LDBS.D
				m_local_regs[src_code] = (int32_t)(int8_t)READ_B(dreg + extra_s);
				break;

			case 1: // LDBU.D
				m_local_regs[src_code] = READ_B(dreg + extra_s);
				break;

			case 2:
				if (extra_s & 1)
					m_local_regs[src_code] = (int32_t)(int16_t)READ_HW(dreg + (extra_s & ~1));
				else
					m_local_regs[src_code] = READ_HW(dreg + (extra_s & ~1));
				break;

			case 3:
				switch (extra_s & 3)
				{
					case 0: // LDW.D
						m_local_regs[src_code] = READ_W(dreg + (extra_s & ~1));
						break;
					case 1: // LDD.D
						m_local_regs[src_code] = READ_W(dreg + (extra_s & ~1));
						m_local_regs[(src_code + 1) & 0x3f] = READ_W(dreg + (extra_s & ~1) + 4);
						m_icount -= m_clock_cycles_1; // extra cycle
						break;
					case 2: // LDW.IOD
						m_local_regs[src_code] = IO_READ_W(dreg + (extra_s & ~3));
						break;
					case 3: // LDD.IOD
						m_local_regs[src_code] = IO_READ_W(dreg + (extra_s & ~3));
						m_local_regs[(src_code + 1) & 0x3f] = IO_READ_W(dreg + (extra_s & ~3) + 4);
						m_icount -= m_clock_cycles_1; // extra cycle
						break;
				}
				break;
		}
	}

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_ldxx1_local_global()
{
	uint16_t next_1 = READ_OP(PC);
	PC += 2;

	const uint16_t sub_type = (next_1 & 0x3000) >> 12;

	uint32_t extra_s;
	if (next_1 & 0x8000)
	{
		const uint16_t next_2 = READ_OP(PC);
		PC += 2;
		m_instruction_length = (3<<19);

		extra_s = next_2;
		extra_s |= ((next_1 & 0xfff) << 16);

		if (next_1 & 0x4000)
			extra_s |= 0xf0000000;
	}
	else
	{
		m_instruction_length = (2<<19);
		extra_s = next_1 & 0xfff;

		if (next_1 & 0x4000)
			extra_s |= 0xfffff000;
	}

	check_delay_PC();

	if (DST_CODE == SR_REGISTER)
	{
		switch (sub_type)
		{
			case 0: // LDBS.A
				set_global_register(SRC_CODE, (int32_t)(int8_t)READ_B(extra_s));
				break;

			case 1: // LDBU.A
				set_global_register(SRC_CODE, READ_B(extra_s));
				break;

			case 2:
				if (extra_s & 1) // LDHS.A
					set_global_register(SRC_CODE, (int32_t)(int16_t)READ_HW(extra_s));
				else // LDHU.A
					set_global_register(SRC_CODE, READ_HW(extra_s));
				break;

			case 3:
				switch (extra_s & 3)
				{
					case 0: // LDW.A
						set_global_register(SRC_CODE, READ_W(extra_s));
						break;
					case 1: // LDD.A
						set_global_register(SRC_CODE, READ_W(extra_s));
						set_global_register(SRC_CODE + 1, READ_W(extra_s + 4));
						m_icount -= m_clock_cycles_1; // extra cycle
						break;
					case 2: // LDW.IOA
						set_global_register(SRC_CODE, IO_READ_W(extra_s));
						break;
					case 3: // LDD.IOA
						set_global_register(SRC_CODE, IO_READ_W(extra_s));
						set_global_register(SRC_CODE + 1, IO_READ_W((extra_s & ~3) + 4));
						m_icount -= m_clock_cycles_1; // extra cycle
						break;
				}
				break;
		}
	}
	else
	{
		const uint32_t dreg = m_local_regs[(DST_CODE + GET_FP) & 0x3f];
		switch (sub_type)
		{
			case 0: // LDBS.D
				set_global_register(SRC_CODE, (int32_t)(int8_t)READ_B(dreg + extra_s));
				break;

			case 1: // LDBU.D
				set_global_register(SRC_CODE, READ_B(dreg + extra_s));
				break;

			case 2:
				if (extra_s & 1)
					set_global_register(SRC_CODE, (int32_t)(int16_t)READ_HW(dreg + (extra_s & ~1)));
				else
					set_global_register(SRC_CODE, READ_HW(dreg + (extra_s & ~1)));
				break;

			case 3:
				switch (extra_s & 3)
				{
					case 0: // LDW.D
						set_global_register(SRC_CODE, READ_W(dreg + (extra_s & ~1)));
						break;
					case 1: // LDD.D
						set_global_register(SRC_CODE, READ_W(dreg + (extra_s & ~1)));
						set_global_register(SRC_CODE + 1, READ_W(dreg + (extra_s & ~1) + 4));
						m_icount -= m_clock_cycles_1; // extra cycle
						break;
					case 2: // LDW.IOD
						set_global_register(SRC_CODE, IO_READ_W(dreg + (extra_s & ~3)));
						break;
					case 3: // LDD.IOD
						set_global_register(SRC_CODE, IO_READ_W(dreg + (extra_s & ~3)));
						set_global_register(SRC_CODE + 1, IO_READ_W(dreg + (extra_s & ~3) + 4));
						m_icount -= m_clock_cycles_1; // extra cycle
						break;
				}
				break;
		}
	}

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_ldxx1_local_local()
{
	uint16_t next_1 = READ_OP(PC);
	PC += 2;

	const uint16_t sub_type = (next_1 & 0x3000) >> 12;

	uint32_t extra_s;
	if (next_1 & 0x8000)
	{
		const uint16_t next_2 = READ_OP(PC);
		PC += 2;
		m_instruction_length = (3<<19);

		extra_s = next_2;
		extra_s |= ((next_1 & 0xfff) << 16);

		if (next_1 & 0x4000)
			extra_s |= 0xf0000000;
	}
	else
	{
		m_instruction_length = (2<<19);
		extra_s = next_1 & 0xfff;

		if (next_1 & 0x4000)
			extra_s |= 0xfffff000;
	}

	check_delay_PC();

	const uint32_t fp = GET_FP;
	const uint32_t src_code = (SRC_CODE + fp) & 0x3f;
	const uint32_t dst_code = (DST_CODE + fp) & 0x3f;
	const uint32_t dreg = m_local_regs[dst_code];

	switch (sub_type)
	{
		case 0: // LDBS.D
			m_local_regs[src_code] = (int32_t)(int8_t)READ_B(dreg + extra_s);
			break;

		case 1: // LDBU.D
			m_local_regs[src_code] = READ_B(dreg + extra_s);
			break;

		case 2:
			if (extra_s & 1)
				m_local_regs[src_code] = (int32_t)(int16_t)READ_HW(dreg + (extra_s & ~1));
			else
				m_local_regs[src_code] = READ_HW(dreg + (extra_s & ~1));
			break;

		case 3:
			switch (extra_s & 3)
			{
				case 0: // LDW.D
					m_local_regs[src_code] = READ_W(dreg + (extra_s & ~1));
					break;
				case 1: // LDD.D
					m_local_regs[src_code] = READ_W(dreg + (extra_s & ~1));
					m_local_regs[(src_code + 1) & 0x3f] = READ_W(dreg + (extra_s & ~1) + 4);
					m_icount -= m_clock_cycles_1; // extra cycle
					break;
				case 2: // LDW.IOD
					m_local_regs[src_code] = IO_READ_W(dreg + (extra_s & ~3));
					break;
				case 3: // LDD.IOD
					m_local_regs[src_code] = IO_READ_W(dreg + (extra_s & ~3));
					m_local_regs[(src_code + 1) & 0x3f] = IO_READ_W(dreg + (extra_s & ~3) + 4);
					m_icount -= m_clock_cycles_1; // extra cycle
					break;
			}
			break;
	}

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_ldxx2_global_global()
{
	uint16_t next_1 = READ_OP(PC);
	PC += 2;

	const uint16_t sub_type = (next_1 & 0x3000) >> 12;

	uint32_t extra_s;
	if (next_1 & 0x8000)
	{
		const uint16_t next_2 = READ_OP(PC);
		PC += 2;
		m_instruction_length = (3<<19);

		extra_s = next_2;
		extra_s |= ((next_1 & 0xfff) << 16);

		if (next_1 & 0x4000)
			extra_s |= 0xf0000000;
	}
	else
	{
		m_instruction_length = (2<<19);
		extra_s = next_1 & 0xfff;

		if (next_1 & 0x4000)
			extra_s |= 0xfffff000;
	}

	check_delay_PC();

	const uint32_t dst_code = DST_CODE;

	if (dst_code < 2)
	{
		m_icount -= m_clock_cycles_1;
		DEBUG_PRINTF(("Denoted PC or SR in hyperstone_ldxx2. PC = %08X\n", PC));
		return;
	}

	const uint32_t src_code = SRC_CODE;
	const uint32_t dreg = m_global_regs[dst_code];

	switch (sub_type)
	{
		case 0: // LDBS.N
			m_global_regs[src_code] = (int32_t)(int8_t)READ_B(dreg);
			if (src_code != dst_code)
				m_global_regs[dst_code] += extra_s;
			break;

		case 1: // LDBU.N
			m_global_regs[src_code] = READ_B(dreg);
			if(src_code != dst_code)
				m_global_regs[dst_code] += extra_s;
			break;

		case 2:
			if (extra_s & 1) // LDHS.N
				m_global_regs[src_code] = (int32_t)(int16_t)READ_HW(dreg);
			else // LDHU.N
				m_global_regs[src_code] = READ_HW(dreg);

			if(src_code != dst_code)
				m_global_regs[dst_code] += (extra_s & ~1);
			break;

		case 3:
			switch (extra_s & 3)
			{
				case 0: // LDW.N
					m_global_regs[src_code] = READ_W(dreg);
					if(src_code != dst_code)
						m_global_regs[dst_code] += extra_s & ~1;
					break;
				case 1: // LDD.N
					m_global_regs[src_code] = READ_W(dreg);
					m_global_regs[src_code + 1] = READ_W(dreg + 4);

					if (src_code != dst_code && (src_code + 1) != dst_code)
						m_global_regs[dst_code] += extra_s & ~1;

					m_icount -= m_clock_cycles_1; // extra cycle
					break;
				case 2: // Reserved
					DEBUG_PRINTF(("Executed Reserved instruction in hyperstone_ldxx2. PC = %08X\n", PC));
					break;
				case 3: // LDW.S
					if (dreg < SP)
						m_global_regs[src_code] = READ_W(dreg);
					else
						m_global_regs[src_code] = m_local_regs[(dreg & 0xfc) >> 2];

					if (src_code != dst_code)
						m_global_regs[dst_code] += extra_s & ~3;

					m_icount -= m_clock_cycles_2; // extra cycles
					break;
			}
			break;
	}

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_ldxx2_global_local()
{
	uint16_t next_1 = READ_OP(PC);
	PC += 2;

	const uint16_t sub_type = (next_1 & 0x3000) >> 12;

	uint32_t extra_s;
	if (next_1 & 0x8000)
	{
		const uint16_t next_2 = READ_OP(PC);
		PC += 2;
		m_instruction_length = (3<<19);

		extra_s = next_2;
		extra_s |= ((next_1 & 0xfff) << 16);

		if (next_1 & 0x4000)
			extra_s |= 0xf0000000;
	}
	else
	{
		m_instruction_length = (2<<19);
		extra_s = next_1 & 0xfff;

		if (next_1 & 0x4000)
			extra_s |= 0xfffff000;
	}

	check_delay_PC();

	const uint32_t fp = GET_FP;
	const uint32_t src_code = (SRC_CODE + fp) & 0x3f;
	const uint32_t srcf_code = (SRC_CODE + fp + 1) & 0x3f;

	const uint32_t dst_code = DST_CODE;
	const uint32_t dreg = m_global_regs[dst_code];

	if (dst_code < 2)
	{
		DEBUG_PRINTF(("Denoted PC or SR in hyperstone_ldxx2. PC = %08X\n", PC));
		m_icount -= m_clock_cycles_1;
		return;
	}

	switch (sub_type)
	{
		case 0: // LDBS.N
			m_local_regs[src_code] = (int32_t)(int8_t)READ_B(dreg);
			set_global_register(dst_code, dreg + extra_s);
			break;

		case 1: // LDBU.N
			m_local_regs[src_code] = READ_B(dreg);
			set_global_register(dst_code, dreg + extra_s);
			break;

		case 2:
			if (extra_s & 1) // LDHS.N
				m_local_regs[src_code] = (int32_t)(int16_t)READ_HW(dreg);
			else // LDHU.N
				m_local_regs[src_code] = READ_HW(dreg);
			set_global_register(dst_code, dreg + (extra_s & ~1));
			break;

		case 3:
			switch (extra_s & 3)
			{
				case 0: // LDW.N
					m_local_regs[src_code] = READ_W(dreg);
					set_global_register(dst_code, dreg + (extra_s & ~1));
					break;
				case 1: // LDD.N
					m_local_regs[src_code] = READ_W(dreg);
					m_local_regs[srcf_code] = READ_W(dreg + 4);
					set_global_register(dst_code, dreg + (extra_s & ~1));
					m_icount -= m_clock_cycles_1; // extra cycle
					break;
				case 2: // Reserved
					DEBUG_PRINTF(("Executed Reserved instruction in hyperstone_ldxx2. PC = %08X\n", PC));
					break;
				case 3: // LDW.S
					if (dreg < SP)
						m_local_regs[src_code] = READ_W(dreg);
					else
						m_local_regs[src_code] = m_local_regs[(dreg & 0xfc) >> 2];
					set_global_register(dst_code, dreg + (extra_s & ~3));
					m_icount -= m_clock_cycles_2; // extra cycles
					break;
			}
			break;
	}

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_ldxx2_local_global()
{
	uint16_t next_1 = READ_OP(PC);
	PC += 2;

	const uint16_t sub_type = (next_1 & 0x3000) >> 12;

	uint32_t extra_s;
	if (next_1 & 0x8000)
	{
		const uint16_t next_2 = READ_OP(PC);
		PC += 2;
		m_instruction_length = (3<<19);

		extra_s = next_2;
		extra_s |= ((next_1 & 0xfff) << 16);

		if (next_1 & 0x4000)
			extra_s |= 0xf0000000;
	}
	else
	{
		m_instruction_length = (2<<19);
		extra_s = next_1 & 0xfff;

		if (next_1 & 0x4000)
			extra_s |= 0xfffff000;
	}

	check_delay_PC();

	const uint32_t src_code = SRC_CODE;
	const uint32_t dst_code = (DST_CODE + GET_FP) & 0x3f;
	const uint32_t dreg = m_local_regs[dst_code];

	switch (sub_type)
	{
		case 0: // LDBS.N
			set_global_register(src_code, (int32_t)(int8_t)READ_B(dreg));
			m_local_regs[dst_code] += extra_s;
			break;

		case 1: // LDBU.N
			set_global_register(src_code, READ_B(dreg));
			m_local_regs[dst_code] += extra_s;
			break;

		case 2:
			if (extra_s & 1) // LDHS.N
				set_global_register(src_code, (int32_t)(int16_t)READ_HW(dreg));
			else // LDHU.N
				set_global_register(src_code, READ_HW(dreg));
			m_local_regs[dst_code] += extra_s & ~1;
			break;

		case 3:
			switch (extra_s & 3)
			{
				case 0: // LDW.N
					set_global_register(src_code, READ_W(dreg));
					m_local_regs[dst_code] += extra_s & ~1;
					break;
				case 1: // LDD.N
					set_global_register(src_code, READ_W(dreg));
					set_global_register(src_code + 1, READ_W(dreg + 4));
					m_local_regs[dst_code] += extra_s & ~1;
					m_icount -= m_clock_cycles_1; // extra cycle
					break;
				case 2: // Reserved
					DEBUG_PRINTF(("Executed Reserved instruction in hyperstone_ldxx2. PC = %08X\n", PC));
					break;
				case 3: // LDW.S
					if (dreg < SP)
						set_global_register(src_code, READ_W(dreg));
					else
						set_global_register(src_code, m_local_regs[(dreg & 0xfc) >> 2]);
					m_local_regs[dst_code] += extra_s & ~3;
					m_icount -= m_clock_cycles_2; // extra cycles
					break;
			}
			break;
	}

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_ldxx2_local_local()
{
	uint16_t next_1 = READ_OP(PC);
	PC += 2;

	const uint16_t sub_type = (next_1 & 0x3000) >> 12;

	uint32_t extra_s;
	if (next_1 & 0x8000)
	{
		const uint16_t next_2 = READ_OP(PC);
		PC += 2;
		m_instruction_length = (3<<19);

		extra_s = next_2;
		extra_s |= ((next_1 & 0xfff) << 16);

		if (next_1 & 0x4000)
			extra_s |= 0xf0000000;
	}
	else
	{
		m_instruction_length = (2<<19);
		extra_s = next_1 & 0xfff;

		if (next_1 & 0x4000)
			extra_s |= 0xfffff000;
	}

	check_delay_PC();

	const uint32_t fp = GET_FP;
	const uint32_t src_code = (SRC_CODE + fp) & 0x3f;
	const uint32_t srcf_code = (SRC_CODE + fp + 1) & 0x3f;

	const uint32_t dst_code = (DST_CODE + fp) & 0x3f;
	const uint32_t dreg = m_local_regs[dst_code];

	switch (sub_type)
	{
		case 0: // LDBS.N
			m_local_regs[src_code] = (int32_t)(int8_t)READ_B(dreg);
			m_local_regs[dst_code] += extra_s;
			break;

		case 1: // LDBU.N
			m_local_regs[src_code] = READ_B(dreg);
			m_local_regs[dst_code] += extra_s;
			break;

		case 2:
			if (extra_s & 1) // LDHS.N
				m_local_regs[src_code] = (int32_t)(int16_t)READ_HW(dreg);
			else // LDHU.N
				m_local_regs[src_code] = READ_HW(dreg);
			m_local_regs[dst_code] += extra_s & ~1;
			break;

		case 3:
			switch (extra_s & 3)
			{
				case 0: // LDW.N
					m_local_regs[src_code] = READ_W(dreg);
					m_local_regs[dst_code] += extra_s & ~1;
					break;
				case 1: // LDD.N
					m_local_regs[src_code] = READ_W(dreg);
					m_local_regs[srcf_code] = READ_W(dreg + 4);
					m_local_regs[dst_code] += extra_s & ~1;
					m_icount -= m_clock_cycles_1; // extra cycle
					break;
				case 2: // Reserved
					DEBUG_PRINTF(("Executed Reserved instruction in hyperstone_ldxx2. PC = %08X\n", PC));
					break;
				case 3: // LDW.S
					if (dreg < SP)
						m_local_regs[src_code] = READ_W(dreg);
					else
						m_local_regs[src_code] = m_local_regs[(dreg & 0xfc) >> 2];
					m_local_regs[dst_code] += extra_s & ~3;
					m_icount -= m_clock_cycles_2; // extra cycles
					break;
			}
			break;
	}

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_stxx1_global_global()
{
	uint16_t next_1 = READ_OP(PC);
	PC += 2;

	const uint16_t sub_type = (next_1 & 0x3000) >> 12;

	uint32_t extra_s;
	if (next_1 & 0x8000)
	{
		const uint16_t next_2 = READ_OP(PC);
		PC += 2;
		m_instruction_length = (3<<19);

		extra_s = next_2;
		extra_s |= ((next_1 & 0xfff) << 16);

		if (next_1 & 0x4000)
			extra_s |= 0xf0000000;
	}
	else
	{
		m_instruction_length = (2<<19);
		extra_s = next_1 & 0xfff;

		if (next_1 & 0x4000)
			extra_s |= 0xfffff000;
	}

	check_delay_PC();

	const uint32_t src_code = SRC_CODE;
	const uint32_t dst_code = DST_CODE;
	const uint32_t dreg = m_global_regs[dst_code];
	const uint32_t sreg = ((src_code == SR_REGISTER) ? 0 : m_global_regs[src_code]);
	const uint32_t sregf = ((src_code == SR_REGISTER) ? 0 : m_global_regs[src_code + 1]);

	if (dst_code == SR_REGISTER)
	{
		switch (sub_type)
		{
			case 0: // STBS.A
				// TODO: missing trap on range error
				WRITE_B(extra_s, (uint8_t)sreg);
				break;

			case 1: // STBU.A
				WRITE_B(extra_s, (uint8_t)sreg);
				break;

			case 2: // STHS.A, STHU.A
				WRITE_HW(extra_s, (uint16_t)sreg);
				// TODO: missing trap on range error with STHS.A
				break;

			case 3:
				switch (extra_s & 3)
				{
					case 0: // STW.A
						WRITE_W(extra_s & ~1, sreg);
						break;
					case 1: // STD.A
						extra_s &= ~1;
						WRITE_W(extra_s, sreg);
						WRITE_W(extra_s + 4, sregf);
						m_icount -= m_clock_cycles_1; // extra cycle
						break;
					case 2: // STW.IOA
						IO_WRITE_W(extra_s & ~3, sreg);
						break;
					case 3: // STD.IOA
						extra_s &= ~3;
						IO_WRITE_W(extra_s, sreg);
						IO_WRITE_W(extra_s + 4, sregf);
						m_icount -= m_clock_cycles_1; // extra cycle
						break;
				}
				break;
		}
	}
	else
	{
		switch (sub_type)
		{
			case 0: // STBS.D
				// TODO: missing trap on range error
				WRITE_B(dreg + extra_s, (uint8_t)sreg);
				break;

			case 1: // STBU.D
				WRITE_B(dreg + extra_s, (uint8_t)sreg);
				break;

			case 2: // STHS.D, STHU.D
				WRITE_HW(dreg + (extra_s & ~1), (uint16_t)sreg);
				// TODO: missing trap on range error with STHS.D
				break;

			case 3:
				switch (extra_s & 3)
				{
					case 0: // STW.D
						WRITE_W(dreg + (extra_s & ~1), sreg);
						break;
					case 1: // STD.D
						extra_s &= ~1;
						WRITE_W(dreg + extra_s, sreg);
						WRITE_W(dreg + extra_s + 4, sregf);
						m_icount -= m_clock_cycles_1; // extra cycle
						break;
					case 2: // STW.IOD
						IO_WRITE_W(dreg + (extra_s & ~3), sreg);
						break;
					case 3: // STD.IOD
						extra_s &= ~3;
						IO_WRITE_W(dreg + extra_s, sreg);
						IO_WRITE_W(dreg + extra_s + 4, sregf);
						m_icount -= m_clock_cycles_1; // extra cycle
						break;
				}
				break;
		}
	}

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_stxx1_global_local()
{
	uint16_t next_1 = READ_OP(PC);
	PC += 2;

	const uint16_t sub_type = (next_1 & 0x3000) >> 12;

	uint32_t extra_s;
	if (next_1 & 0x8000)
	{
		const uint16_t next_2 = READ_OP(PC);
		PC += 2;
		m_instruction_length = (3<<19);

		extra_s = next_2;
		extra_s |= ((next_1 & 0xfff) << 16);

		if (next_1 & 0x4000)
			extra_s |= 0xf0000000;
	}
	else
	{
		m_instruction_length = (2<<19);
		extra_s = next_1 & 0xfff;

		if (next_1 & 0x4000)
			extra_s |= 0xfffff000;
	}

	check_delay_PC();

	const uint32_t dst_code = DST_CODE;
	const uint32_t src_code = (SRC_CODE + GET_FP) & 0x3f;
	const uint32_t sreg = m_local_regs[src_code];

	if (dst_code == SR_REGISTER)
	{
		switch (sub_type)
		{
			case 0: // STBS.A
				WRITE_B(extra_s, (uint8_t)sreg);
				// TODO: missing trap on range error
				break;

			case 1: // STBU.A
				WRITE_B(extra_s, (uint8_t)sreg);
				break;

			case 2: // STHS.A & STHU.A
				WRITE_HW(extra_s, (uint16_t)sreg);
				// TODO: missing trap on range error with STHS.A
				break;

			case 3:
				switch (extra_s & 3)
				{
					case 0: // STW.A
						WRITE_W(extra_s & ~1, sreg);
						break;
					case 1: // STD.A
						WRITE_W(extra_s & ~1, sreg);
						WRITE_W((extra_s & ~1) + 4, m_local_regs[(src_code + 1) & 0x3f]);
						m_icount -= m_clock_cycles_1; // extra cycle
						break;
					case 2: // STW.IOA
						IO_WRITE_W(extra_s & ~3, sreg);
						break;
					case 3: // STD.IOA
						IO_WRITE_W(extra_s & ~3, sreg);
						IO_WRITE_W((extra_s & ~3) + 4, m_local_regs[(src_code + 1) & 0x3f]);
						m_icount -= m_clock_cycles_1; // extra cycle
						break;
				}
				break;
		}
	}
	else
	{
		const uint32_t dreg = m_global_regs[dst_code];
		switch (sub_type)
		{
			case 0: // STBS.D
				WRITE_B(dreg + extra_s, (uint8_t)sreg);
				// TODO: missing trap on range error
				break;

			case 1: // STBU.D
				WRITE_B(dreg + extra_s, (uint8_t)sreg);
				break;

			case 2: // STHS.D & STHU.D
				WRITE_HW(dreg + (extra_s & ~1), (uint16_t)sreg);
				// TODO: Missing trap on range error for STHS.D
				break;

			case 3:
				switch (extra_s & 3)
				{
					case 0: // STW.D
						WRITE_W(dreg + (extra_s & ~1), sreg);
						break;
					case 1: // STD.D
						WRITE_W(dreg + (extra_s & ~1), sreg);
						WRITE_W(dreg + (extra_s & ~1) + 4, m_local_regs[(src_code + 1) & 0x3f]);
						m_icount -= m_clock_cycles_1; // extra cycle
						break;
					case 2: // STW.IOD
						IO_WRITE_W(dreg + (extra_s & ~3), sreg);
						break;
					case 3: // STD.IOD
						IO_WRITE_W(dreg + (extra_s & ~3), sreg);
						IO_WRITE_W(dreg + (extra_s & ~3) + 4, m_local_regs[(src_code + 1) & 0x3f]);
						m_icount -= m_clock_cycles_1; // extra cycle
						break;
				}
				break;
		}
	}

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_stxx1_local_global()
{
	uint16_t next_1 = READ_OP(PC);
	PC += 2;

	const uint16_t sub_type = (next_1 & 0x3000) >> 12;

	uint32_t extra_s;
	if (next_1 & 0x8000)
	{
		const uint16_t next_2 = READ_OP(PC);
		PC += 2;
		m_instruction_length = (3<<19);

		extra_s = next_2;
		extra_s |= ((next_1 & 0xfff) << 16);

		if (next_1 & 0x4000)
			extra_s |= 0xf0000000;
	}
	else
	{
		m_instruction_length = (2<<19);
		extra_s = next_1 & 0xfff;

		if (next_1 & 0x4000)
			extra_s |= 0xfffff000;
	}

	check_delay_PC();

	const uint32_t fp = GET_FP;
	const uint32_t src_code = SRC_CODE;
	const uint32_t sreg = ((src_code == SR_REGISTER) ? 0 : m_global_regs[src_code]);
	const uint32_t dreg = m_local_regs[(DST_CODE + fp) & 0x3f];

	switch (sub_type)
	{
		case 0: // STBS.D
			/* TODO: missing trap on range error */
			WRITE_B(dreg + extra_s, (uint8_t)sreg);
			break;

		case 1: // STBU.D
			WRITE_B(dreg + extra_s, (uint8_t)sreg);
			break;

		case 2: // STHS.D, STHU.D
			WRITE_HW(dreg + (extra_s & ~1), (uint16_t)sreg);
			// missing trap on range error with STHS.D
			break;

		case 3:
			switch (extra_s & 3)
			{
				case 0: // STW.D
					WRITE_W(dreg + (extra_s & ~1), sreg);
					break;
				case 1: // STD.D
					WRITE_W(dreg + (extra_s & ~1), sreg);
					WRITE_W(dreg + (extra_s & ~1) + 4, src_code == SR_REGISTER ? 0 : m_global_regs[src_code + 1]);
					m_icount -= m_clock_cycles_1; // extra cycle
					break;
				case 2: // STW.IOD
					IO_WRITE_W(dreg + (extra_s & ~3), sreg);
					break;
				case 3: // STD.IOD
					IO_WRITE_W(dreg + (extra_s & ~3), sreg);
					IO_WRITE_W(dreg + (extra_s & ~3) + 4, src_code == SR_REGISTER ? 0 : m_global_regs[src_code + 1]);
					m_icount -= m_clock_cycles_1; // extra cycle
					break;
			}
			break;
	}
}

void hyperstone_device::hyperstone_stxx1_local_local()
{
	uint16_t next_1 = READ_OP(PC);
	PC += 2;

	const uint16_t sub_type = (next_1 & 0x3000) >> 12;

	uint32_t extra_s;
	if (next_1 & 0x8000)
	{
		const uint16_t next_2 = READ_OP(PC);
		PC += 2;
		m_instruction_length = (3<<19);

		extra_s = next_2;
		extra_s |= ((next_1 & 0xfff) << 16);

		if (next_1 & 0x4000)
			extra_s |= 0xf0000000;
	}
	else
	{
		m_instruction_length = (2<<19);
		extra_s = next_1 & 0xfff;

		if (next_1 & 0x4000)
			extra_s |= 0xfffff000;
	}

	check_delay_PC();

	const uint32_t fp = GET_FP;
	const uint32_t sreg = m_local_regs[(SRC_CODE + fp) & 0x3f];
	const uint32_t dreg = m_local_regs[(DST_CODE + fp) & 0x3f];

	switch (sub_type)
	{
		case 0: // STBS.D
			/* TODO: missing trap on range error */
			WRITE_B(dreg + extra_s, (uint8_t)sreg);
			break;

		case 1: // STBU.D
			WRITE_B(dreg + extra_s, (uint8_t)sreg);
			break;

		case 2: // STHS.D, STHU.D
			WRITE_HW(dreg + (extra_s & ~1), (uint16_t)sreg);
			// missing trap on range error with STHS.D
			break;

		case 3:
			switch (extra_s & 3)
			{
				case 0: // STW.D
					WRITE_W(dreg + (extra_s & ~1), sreg);
					break;
				case 1: // STD.D
					WRITE_W(dreg + (extra_s & ~1), sreg);
					WRITE_W(dreg + (extra_s & ~1) + 4, m_local_regs[(SRC_CODE + fp + 1) & 0x3f]);
					m_icount -= m_clock_cycles_1; // extra cycle
					break;
				case 2: // STW.IOD
					IO_WRITE_W(dreg + (extra_s & ~3), sreg);
					break;
				case 3: // STD.IOD
					IO_WRITE_W(dreg + (extra_s & ~3), sreg);
					IO_WRITE_W(dreg + (extra_s & ~3) + 4, m_local_regs[(SRC_CODE + fp + 1) & 0x3f]);
					m_icount -= m_clock_cycles_1; // extra cycle
					break;
			}
			break;
	}
}

void hyperstone_device::hyperstone_stxx2_global_global()
{
	uint16_t next_1 = READ_OP(PC);
	PC += 2;

	const uint16_t sub_type = (next_1 & 0x3000) >> 12;

	uint32_t extra_s;
	if (next_1 & 0x8000)
	{
		const uint16_t next_2 = READ_OP(PC);
		PC += 2;
		m_instruction_length = (3<<19);

		extra_s = next_2;
		extra_s |= ((next_1 & 0xfff) << 16);

		if (next_1 & 0x4000)
			extra_s |= 0xf0000000;
	}
	else
	{
		m_instruction_length = (2<<19);
		extra_s = next_1 & 0xfff;

		if (next_1 & 0x4000)
			extra_s |= 0xfffff000;
	}

	check_delay_PC();

	const uint32_t src_code = SRC_CODE;
	uint32_t sreg = (src_code == SR_REGISTER) ? 0 : m_global_regs[src_code];
	uint32_t sregf = (src_code == SR_REGISTER) ? 0 : m_global_regs[src_code + 1];
	const uint32_t dst_code = DST_CODE;
	const uint32_t dreg = m_global_regs[dst_code];

	if (dst_code < 2)
	{
		m_icount -= m_clock_cycles_1;
		return;
	}

	switch (sub_type)
	{
		case 0: // STBS.N
			// TODO: missing trap on range error
			WRITE_B(dreg, (uint8_t)sreg);
			m_global_regs[dst_code] += extra_s;
			break;

		case 1: // STBU.N
			WRITE_B(dreg, (uint8_t)sreg);
			m_global_regs[dst_code] += extra_s;
			break;

		case 2: // STHS.N, STHU.N
			WRITE_HW(dreg, (uint16_t)sreg);
			m_global_regs[dst_code] += extra_s & ~1;
			// TODO: missing trap on range error with STHS.N
			break;

		case 3:
			switch (extra_s & 3)
			{
				case 0: // STW.N
					WRITE_W(dreg, sreg);
					m_global_regs[dst_code] += extra_s & ~1;
					break;
				case 1: // STD.N
					WRITE_W(dreg, sreg);
					m_global_regs[dst_code] += extra_s & ~1;

					if((src_code + 1) == dst_code)
						WRITE_W(dreg + 4, sregf + (extra_s & ~1));  // because DREG == SREGF and DREG has been incremented
					else
						WRITE_W(dreg + 4, sregf);
					m_icount -= m_clock_cycles_1; // extra cycle
					break;
				case 2: // Reserved
					DEBUG_PRINTF(("Executed Reserved instruction in hyperstone_stxx2. PC = %08X\n", PC));
					break;
				case 3: // STW.S
					if(dreg < SP)
						WRITE_W(dreg, sreg);
					else
						m_local_regs[(dreg & 0xfc) >> 2] = sreg;
					m_global_regs[dst_code] += (extra_s & ~3);
					m_icount -= m_clock_cycles_2; // extra cycles
					break;
			}
			break;
	}
}

void hyperstone_device::hyperstone_stxx2_global_local()
{
	uint16_t next_1 = READ_OP(PC);
	PC += 2;

	const uint16_t sub_type = (next_1 & 0x3000) >> 12;

	uint32_t extra_s;
	if (next_1 & 0x8000)
	{
		const uint16_t next_2 = READ_OP(PC);
		PC += 2;
		m_instruction_length = (3<<19);

		extra_s = next_2;
		extra_s |= ((next_1 & 0xfff) << 16);

		if (next_1 & 0x4000)
			extra_s |= 0xf0000000;
	}
	else
	{
		m_instruction_length = (2<<19);
		extra_s = next_1 & 0xfff;

		if (next_1 & 0x4000)
			extra_s |= 0xfffff000;
	}

	check_delay_PC();

	const uint32_t dst_code = DST_CODE;

	if (dst_code < 2)
	{
		DEBUG_PRINTF(("Denoted PC or SR in hyperstone_stxx2. PC = %08X\n", PC));
		m_icount -= m_clock_cycles_1;
		return;
	}

	const uint32_t src_code = (SRC_CODE + GET_FP) & 0x3f;
	const uint32_t sreg = m_local_regs[src_code];
	const uint32_t sregf = m_local_regs[(src_code + 1) & 0x3f];
	const uint32_t dreg = m_global_regs[dst_code];

	switch (sub_type)
	{
		case 0: // STBS.N
			// TODO: missing trap on range error
			WRITE_B(dreg, (uint8_t)sreg);
			set_global_register(dst_code, dreg + extra_s);
			break;

		case 1: // STBU.N

			WRITE_B(dreg, (uint8_t)sreg);
			set_global_register(dst_code, dreg + extra_s);
			break;

		case 2: // STHS.N, STHU.N
			WRITE_HW(dreg, (uint16_t)sreg);
			set_global_register(dst_code, dreg + (extra_s & ~1));
			// TODO: Missing trap on range error with STHS.N
			break;

		case 3:
			switch (extra_s & 3)
			{
				case 0: // STW.N
					WRITE_W(dreg, sreg);
					set_global_register(dst_code, dreg + extra_s);
					break;
				case 1: // STD.N
					WRITE_W(dreg, sreg);
					WRITE_W(dreg + 4, sregf);
					set_global_register(dst_code, dreg + (extra_s & ~1));
					m_icount -= m_clock_cycles_1; // extra cycle
					break;
				case 2: // Reserved
					DEBUG_PRINTF(("Executed Reserved instruction in hyperstone_stxx2. PC = %08X\n", PC));
					break;
				case 3: // STW.S
					if(dreg < SP)
						WRITE_W(dreg, sreg);
					else
						m_local_regs[(dreg & 0xfc) >> 2] = sreg;
					set_global_register(dst_code, dreg + (extra_s & ~3));
					m_icount -= m_clock_cycles_2; // extra cycles
					break;
			}
			break;
	}

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_stxx2_local_global()
{
	uint16_t next_1 = READ_OP(PC);
	PC += 2;

	const uint16_t sub_type = (next_1 & 0x3000) >> 12;

	uint32_t extra_s;
	if (next_1 & 0x8000)
	{
		const uint16_t next_2 = READ_OP(PC);
		PC += 2;
		m_instruction_length = (3<<19);

		extra_s = next_2;
		extra_s |= ((next_1 & 0xfff) << 16);

		if (next_1 & 0x4000)
			extra_s |= 0xf0000000;
	}
	else
	{
		m_instruction_length = (2<<19);
		extra_s = next_1 & 0xfff;

		if (next_1 & 0x4000)
			extra_s |= 0xfffff000;
	}

	check_delay_PC();

	const uint32_t fp = GET_FP;
	const uint32_t src_code = SRC_CODE;
	const uint32_t sreg = (src_code == SR_REGISTER) ? 0 : m_global_regs[src_code];
	const uint32_t d_code = DST_CODE;
	const uint32_t dst_code = (d_code + fp) & 0x3f;
	const uint32_t dreg = m_local_regs[dst_code];

	switch (sub_type)
	{
		case 0: // STBS.N
			// TODO: missing trap on range error
			WRITE_B(dreg, (uint8_t)sreg);
			m_local_regs[dst_code] += extra_s;
			break;

		case 1: // STBU.N
			WRITE_B(dreg, (uint8_t)sreg);
			m_local_regs[dst_code] += extra_s;
			break;

		case 2: // STHS.N, STHU.N
			WRITE_HW(dreg, (uint16_t)sreg);
			m_local_regs[dst_code] += extra_s & ~1;
			// TODO: missing trap on range error for STHS.N
			break;

		case 3:
			switch (extra_s & 3)
			{
				case 0: // STW.N
					WRITE_W(dreg, sreg);
					m_local_regs[dst_code] += extra_s & ~1;
					break;
				case 1: // STD.N
					WRITE_W(dreg, sreg);
					m_local_regs[dst_code] += extra_s & ~1;
					WRITE_W(dreg + 4, (src_code == SR_REGISTER) ? 0 : m_global_regs[src_code + 1]);
					m_icount -= m_clock_cycles_1; // extra cycle
					break;
				case 2: // Reserved
					DEBUG_PRINTF(("Executed Reserved instruction in hyperstone_stxx2. PC = %08X\n", PC));
					break;
				case 3: // STW.S
					if(dreg < SP)
						WRITE_W(dreg, sreg);
					else
						m_local_regs[(dreg & 0xfc) >> 2] = sreg;
					m_local_regs[dst_code] += extra_s & ~3;
					m_icount -= m_clock_cycles_2; // extra cycles
					break;
			}
			break;
	}

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_stxx2_local_local()
{
	uint16_t next_1 = READ_OP(PC);
	PC += 2;

	const uint16_t sub_type = (next_1 & 0x3000) >> 12;

	uint32_t extra_s;
	if (next_1 & 0x8000)
	{
		const uint16_t next_2 = READ_OP(PC);
		PC += 2;
		m_instruction_length = (3<<19);

		extra_s = next_2;
		extra_s |= ((next_1 & 0xfff) << 16);

		if (next_1 & 0x4000)
			extra_s |= 0xf0000000;
	}
	else
	{
		m_instruction_length = (2<<19);
		extra_s = next_1 & 0xfff;

		if (next_1 & 0x4000)
			extra_s |= 0xfffff000;
	}

	check_delay_PC();
	const uint32_t fp = GET_FP;
	const uint32_t sreg = m_local_regs[(SRC_CODE + fp) & 0x3f];
	const uint32_t dst_code = (DST_CODE + fp) & 0x3f;
	const uint32_t dreg = m_local_regs[dst_code];

	switch (sub_type)
	{
		case 0: // STBS.N
			/* TODO: missing trap on range error */
			WRITE_B(dreg, (uint8_t)sreg);
			m_local_regs[dst_code] += extra_s;
			break;

		case 1: // STBU.N
			WRITE_B(dreg, (uint8_t)sreg);
			m_local_regs[dst_code] += extra_s;
			break;

		case 2:
			WRITE_HW(dreg, (uint16_t)sreg);
			m_local_regs[dst_code] += extra_s & ~1;
			// Missing trap on range error with STHS.N
			break;

		case 3:
			switch (extra_s & 3)
			{
				case 0: // STW.N
					WRITE_W(dreg, sreg);
					m_local_regs[dst_code] += extra_s & ~1;
					break;
				case 1: // STD.N
					WRITE_W(dreg, sreg);
					m_local_regs[dst_code] += extra_s & ~1;
					if (((SRC_CODE + 1) & 0x3f) == DST_CODE)
						WRITE_W(dreg + 4, m_local_regs[(SRC_CODE + fp + 1) & 0x3f] + (extra_s & ~1));  // because DREG == SREGF and DREG has been incremented
					else
						WRITE_W(dreg + 4, m_local_regs[(SRC_CODE + fp + 1) & 0x3f]);
					m_icount -= m_clock_cycles_1; // extra cycle
					break;
				case 2: // Reserved
					DEBUG_PRINTF(("Executed Reserved instruction in hyperstone_stxx2. PC = %08X\n", PC));
					break;
				case 3: // STW.S
					if(dreg < SP)
						WRITE_W(dreg, sreg);
					else
						m_local_regs[(dreg & 0xfc) >> 2] = sreg;

					m_local_regs[dst_code] += extra_s & ~3;
					m_icount -= m_clock_cycles_2; // extra cycles
					break;
			}
	}

	m_icount -= m_clock_cycles_1;
}



void hyperstone_device::hyperstone_shri_global()
{
	check_delay_PC();

	const uint32_t dst_code = DST_CODE;

	uint32_t val = m_global_regs[dst_code];

	SR &= ~(C_MASK | Z_MASK | N_MASK);

	const uint32_t n = N_VALUE;
	if (n)
		SR |= (val >> (n - 1)) & 1;

	val >>= n;

	m_global_regs[dst_code] = val;
	if (val == 0)
		SR |= Z_MASK;
	SR |= SIGN_TO_N(val);

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_shri_local()
{
	check_delay_PC();

	const uint32_t dst_code = (DST_CODE + GET_FP) & 0x3f;
	uint32_t val = m_local_regs[dst_code];

	SR &= ~(C_MASK | Z_MASK | N_MASK);

	const uint32_t n = N_VALUE;
	if (n)
		SR |= (val >> (n - 1)) & 1;

	val >>= n;

	m_local_regs[dst_code] = val;

	if (val == 0)
		SR |= Z_MASK;
	SR |= SIGN_TO_N(val);

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_sari_global()
{
	check_delay_PC();

	const uint32_t dst_code = DST_CODE;
	uint32_t val = m_global_regs[dst_code];

	const uint32_t n = N_VALUE;

	SR &= ~(C_MASK | Z_MASK | N_MASK);
	if (n)
	{
		SR |= (val >> (n - 1)) & 1;

		uint32_t sign_bit = val & 0x80000000;
		val >>= n;

		if (sign_bit)
			val |= 0xffffffff << (32 - n);
	}

	set_global_register(dst_code, val);
	if (val == 0)
		SR |= Z_MASK;
	SR |= SIGN_TO_N(val);

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_sari_local()
{
	check_delay_PC();

	const uint32_t dst_code = (DST_CODE + GET_FP) & 0x3f;

	uint32_t val = m_local_regs[dst_code];
	uint32_t sign_bit = val & 0x80000000;

	const uint32_t n = N_VALUE;

	SR &= ~(C_MASK | Z_MASK | N_MASK);
	if (n)
	{
		SR |= (val >> (n - 1)) & 1;

		val >>= n;

		if (sign_bit)
			val |= 0xffffffff << (32 - n);
	}

	m_local_regs[dst_code] = val;
	if (val == 0)
		SR |= Z_MASK;
	SR |= SIGN_TO_N(val);

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_shli_global()
{
	check_delay_PC();

	const uint32_t dst_code = DST_CODE;

	uint32_t val = m_global_regs[dst_code];
	const uint32_t n = N_VALUE;
	SR &= ~(C_MASK | V_MASK | Z_MASK | N_MASK);
	SR |= n ? (((val << (n - 1)) & 0x80000000) ? 1 : 0) : 0;
	uint64_t mask = ((1U << (32 - n)) - 1) ^ 0xffffffff;
	uint32_t val2 = val << n;

	if (((val & mask) && (!(val2 & 0x80000000))) || (((val & mask) ^ mask) && (val2 & 0x80000000)))
		SR |= V_MASK;

	set_global_register(dst_code, val2);
	if (val2 == 0)
		SR |= Z_MASK;
	SR |= SIGN_TO_N(val2);

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_shli_local()
{
	check_delay_PC();

	const uint32_t dst_code = (DST_CODE + GET_FP) & 0x3f;

	uint32_t val = m_local_regs[dst_code];
	const uint32_t n = N_VALUE;
	SR &= ~(C_MASK | V_MASK | Z_MASK | N_MASK);
	SR |= n ? (((val << (n - 1)) & 0x80000000) ? 1 : 0) : 0;
	uint64_t mask = ((1U << (32 - n)) - 1) ^ 0xffffffff;
	uint32_t val2 = val << n;

	if (((val & mask) && (!(val2 & 0x80000000))) || (((val & mask) ^ mask) && (val2 & 0x80000000)))
		SR |= V_MASK;

	m_local_regs[dst_code] = val2;
	if (val2 == 0)
		SR |= Z_MASK;
	SR |= SIGN_TO_N(val2);

	m_icount -= m_clock_cycles_1;
}



void hyperstone_device::hyperstone_mulu_global_global()
{
	check_delay_PC();

	const uint32_t src_code = SRC_CODE;
	const uint32_t dst_code = DST_CODE;

	if (src_code < 2 || dst_code < 2)
	{
		DEBUG_PRINTF(("Denoted PC or SR in hyperstone_mulu instruction. PC = %08X\n", PC));
		return;
	}

	const uint32_t dreg = m_global_regs[dst_code];
	const uint32_t sreg = m_global_regs[src_code];
	const uint64_t double_word = (uint64_t)sreg *(uint64_t)dreg;

	const uint32_t high_order = (uint32_t)(double_word >> 32);

	set_global_register(dst_code, high_order);
	set_global_register(dst_code + 1, (uint32_t)double_word);

	SR &= ~(Z_MASK | N_MASK);
	if (double_word == 0)
		SR |= Z_MASK;
	SR |= SIGN_TO_N(high_order);

	if(sreg <= 0xffff && dreg <= 0xffff)
		m_icount -= m_clock_cycles_4;
	else
		m_icount -= m_clock_cycles_6;
}

void hyperstone_device::hyperstone_mulu_global_local()
{
	check_delay_PC();

	const uint32_t src_code = (SRC_CODE + GET_FP) & 0x3f;
	const uint32_t dst_code = DST_CODE;

	if (dst_code < 2)
	{
		DEBUG_PRINTF(("Denoted PC or SR in hyperstone_mulu instruction. PC = %08X\n", PC));
		return;
	}

	const uint32_t dreg = m_global_regs[dst_code];
	const uint32_t sreg = m_local_regs[src_code];
	const uint64_t double_word = (uint64_t)sreg *(uint64_t)dreg;

	const uint32_t high_order = (uint32_t)(double_word >> 32);

	set_global_register(dst_code, high_order);
	set_global_register(dst_code + 1, (uint32_t)double_word);

	SR &= ~(Z_MASK | N_MASK);
	if (double_word == 0)
		SR |= Z_MASK;
	SR |= SIGN_TO_N(high_order);

	if(sreg <= 0xffff && dreg <= 0xffff)
		m_icount -= m_clock_cycles_4;
	else
		m_icount -= m_clock_cycles_6;
}

void hyperstone_device::hyperstone_mulu_local_global()
{
	check_delay_PC();

	const uint32_t src_code = SRC_CODE;
	const uint32_t dst_code = (DST_CODE + GET_FP) & 0x3f;
	const uint32_t dstf_code = (dst_code + 1) & 0x3f;

	if (src_code < 2)
	{
		DEBUG_PRINTF(("Denoted PC or SR in hyperstone_mulu instruction. PC = %08X\n", PC));
		return;
	}

	const uint32_t dreg = m_local_regs[dst_code];
	const uint32_t sreg = m_global_regs[src_code];
	const uint64_t double_word = (uint64_t)sreg *(uint64_t)dreg;

	const uint32_t high_order = (uint32_t)(double_word >> 32);

	m_local_regs[dst_code] = high_order;
	m_local_regs[dstf_code] = (uint32_t)double_word;

	SR &= ~(Z_MASK | N_MASK);
	if (double_word == 0)
		SR |= Z_MASK;
	SR |= SIGN_TO_N(high_order);

	if(sreg <= 0xffff && dreg <= 0xffff)
		m_icount -= m_clock_cycles_4;
	else
		m_icount -= m_clock_cycles_6;
}

void hyperstone_device::hyperstone_mulu_local_local()
{
	check_delay_PC();

	const uint32_t fp = GET_FP;
	const uint32_t code = DST_CODE;
	const uint32_t dst_code = (code + fp) & 0x3f;
	const uint32_t dstf_code = (code + 1 + fp) & 0x3f;
	const uint32_t dreg = m_local_regs[dst_code];

	const uint32_t sreg = m_local_regs[(SRC_CODE + fp) & 0x3f];
	const uint64_t double_word = (uint64_t)sreg *(uint64_t)dreg;

	const uint32_t high_order = (uint32_t)(double_word >> 32);

	m_local_regs[dst_code] = high_order;
	m_local_regs[dstf_code] = (uint32_t)double_word;

	SR &= ~(Z_MASK | N_MASK);
	if (double_word == 0)
		SR |= Z_MASK;
	SR |= SIGN_TO_N(high_order);

	if(sreg <= 0xffff && dreg <= 0xffff)
		m_icount -= m_clock_cycles_4;
	else
		m_icount -= m_clock_cycles_6;
}

void hyperstone_device::hyperstone_muls_global_global()
{
	check_delay_PC();

	const uint32_t src_code = SRC_CODE;
	const uint32_t dst_code = DST_CODE;
	const uint32_t dstf_code = dst_code + 1;

	if (src_code < 2 || dst_code < 2)
	{
		DEBUG_PRINTF(("Denoted PC or SR in hyperstone_muls instruction. PC = %08X\n", PC));
		return;
	}

	const uint32_t dreg = m_global_regs[dst_code];
	const uint32_t sreg = m_global_regs[src_code];
	const int64_t double_word = (int64_t)(int32_t)sreg * (int64_t)(int32_t)dreg;

	const uint32_t high_order = (uint32_t)(double_word >> 32);

	set_global_register(dst_code, high_order);
	set_global_register(dstf_code, (uint32_t)double_word);

	SR &= ~(Z_MASK | N_MASK);
	if (double_word == 0)
		SR |= Z_MASK;
	SR |= SIGN_TO_N(high_order);

	if((sreg >= 0xffff8000 && sreg <= 0x7fff) && (dreg >= 0xffff8000 && dreg <= 0x7fff))
		m_icount -= m_clock_cycles_4;
	else
		m_icount -= m_clock_cycles_6;
}

void hyperstone_device::hyperstone_muls_global_local()
{
	check_delay_PC();

	const uint32_t src_code = (SRC_CODE + GET_FP) & 0x3f;
	const uint32_t dst_code = DST_CODE;
	const uint32_t dstf_code = dst_code + 1;

	if (dst_code < 2)
	{
		DEBUG_PRINTF(("Denoted PC or SR in hyperstone_muls instruction. PC = %08X\n", PC));
		return;
	}

	const uint32_t dreg = m_global_regs[dst_code];
	const uint32_t sreg = m_local_regs[src_code];
	const int64_t double_word = (int64_t)(int32_t)sreg * (int64_t)(int32_t)dreg;

	const uint32_t high_order = (uint32_t)(double_word >> 32);

	set_global_register(dst_code, high_order);
	set_global_register(dstf_code, (uint32_t)double_word);

	SR &= ~(Z_MASK | N_MASK);
	if (double_word == 0)
		SR |= Z_MASK;
	SR |= SIGN_TO_N(high_order);

	if((sreg >= 0xffff8000 && sreg <= 0x7fff) && (dreg >= 0xffff8000 && dreg <= 0x7fff))
		m_icount -= m_clock_cycles_4;
	else
		m_icount -= m_clock_cycles_6;
}

void hyperstone_device::hyperstone_muls_local_global()
{
	check_delay_PC();

	const uint32_t fp = GET_FP;
	const uint32_t src_code = SRC_CODE;

	const uint32_t d_code = DST_CODE;
	const uint32_t dst_code = (d_code + fp) & 0x3f;
	const uint32_t dstf_code = (d_code + 1 + fp) & 0x3f;

	if (src_code < 2)
	{
		DEBUG_PRINTF(("Denoted PC or SR in hyperstone_muls instruction. PC = %08X\n", PC));
		return;
	}

	const uint32_t dreg = m_local_regs[dst_code];
	const uint32_t sreg = m_local_regs[src_code];
	const int64_t double_word = (int64_t)(int32_t)sreg * (int64_t)(int32_t)dreg;

	const uint32_t high_order = (uint32_t)(double_word >> 32);

	m_local_regs[dst_code] = high_order;
	m_local_regs[dstf_code] = (uint32_t)double_word;

	SR &= ~(Z_MASK | N_MASK);
	if (double_word == 0)
		SR |= Z_MASK;
	SR |= SIGN_TO_N(high_order);

	if((sreg >= 0xffff8000 && sreg <= 0x7fff) && (dreg >= 0xffff8000 && dreg <= 0x7fff))
		m_icount -= m_clock_cycles_4;
	else
		m_icount -= m_clock_cycles_6;
}

void hyperstone_device::hyperstone_muls_local_local()
{
	check_delay_PC();

	const uint32_t fp = GET_FP;
	const uint32_t src_code = (SRC_CODE + fp) & 0x3f;
	const uint32_t sreg = m_local_regs[src_code];

	const uint32_t d_code = DST_CODE;
	const uint32_t dst_code = (d_code + fp) & 0x3f;
	const uint32_t dstf_code = (d_code + 1 + fp) & 0x3f;
	const uint32_t dreg = m_local_regs[dst_code];

	const int64_t double_word = (int64_t)(int32_t)sreg * (int64_t)(int32_t)dreg;

	const uint32_t high_order = (uint32_t)(double_word >> 32);

	m_local_regs[dst_code] = high_order;
	m_local_regs[dstf_code] = (uint32_t)double_word;

	SR &= ~(Z_MASK | N_MASK);
	if (double_word == 0)
		SR |= Z_MASK;
	SR |= SIGN_TO_N(high_order);

	if((sreg >= 0xffff8000 && sreg <= 0x7fff) && (dreg >= 0xffff8000 && dreg <= 0x7fff))
		m_icount -= m_clock_cycles_4;
	else
		m_icount -= m_clock_cycles_6;
}

void hyperstone_device::hyperstone_set_global()
{
	check_delay_PC();

	const uint32_t dst_code = DST_CODE;
	const uint32_t n = N_VALUE;

	if (dst_code < 2)
	{
		m_icount -= m_clock_cycles_1;
		return;
	}

	switch (n)
	{
		// SETADR
		case 0:
			m_global_regs[dst_code] = (SP & 0xfffffe00) | (GET_FP << 2) | (((SP & 0x100) && (SIGN_BIT(SR) == 0)) ? 1 : 0);
			break;

		// Reserved
		case 1:
		case 16:
		case 17:
		case 19:
			DEBUG_PRINTF(("Used reserved N value (%d) in hyperstone_set. PC = %08X\n", n, PC));
			break;

		// SETxx
		case 2:
			m_global_regs[dst_code] = 1;
			break;

		case 3:
			m_global_regs[dst_code] = 0;
			break;

		case 4:
			if (SR & (N_MASK | Z_MASK))
				m_global_regs[dst_code] = 1;
			else
				m_global_regs[dst_code] = 0;
			break;

		case 5:
			if (SR & (N_MASK | Z_MASK))
				m_global_regs[dst_code] = 0;
			else
				m_global_regs[dst_code] = 1;
			break;

		case 6:
			if (SR & N_MASK)
				m_global_regs[dst_code] = 1;
			else
				m_global_regs[dst_code] = 0;
			break;

		case 7:
			if (SR & N_MASK)
				m_global_regs[dst_code] = 0;
			else
				m_global_regs[dst_code] = 1;
			break;

		case 8:
			if (SR & (C_MASK | Z_MASK))
				m_global_regs[dst_code] = 1;
			else
				m_global_regs[dst_code] = 0;
			break;

		case 9:
			if (SR & (C_MASK | Z_MASK))
				m_global_regs[dst_code] = 0;
			else
				m_global_regs[dst_code] = 1;
			break;

		case 10:
			if (SR & C_MASK)
				m_global_regs[dst_code] = 1;
			else
				m_global_regs[dst_code] = 0;
			break;

		case 11:
			if (SR & C_MASK)
				m_global_regs[dst_code] = 0;
			else
				m_global_regs[dst_code] = 1;
			break;

		case 12:
			if (SR & Z_MASK)
				m_global_regs[dst_code] = 1;
			else
				m_global_regs[dst_code] = 0;
			break;

		case 13:
			if (SR & Z_MASK)
				m_global_regs[dst_code] = 0;
			else
				m_global_regs[dst_code] = 1;
			break;

		case 14:
			if (SR & V_MASK)
				m_global_regs[dst_code] = 1;
			else
				m_global_regs[dst_code] = 0;
			break;

		case 15:
			if (SR & V_MASK)
				m_global_regs[dst_code] = 0;
			else
				m_global_regs[dst_code] = 1;
			break;

		case 18:
			m_global_regs[dst_code] = ~0;
			break;

		case 20:
			if (SR & (N_MASK | Z_MASK))
				m_global_regs[dst_code] = ~0;
			else
				m_global_regs[dst_code] = 0;
			break;

		case 21:
			if (SR & (N_MASK | Z_MASK))
				m_global_regs[dst_code] = 0;
			else
				m_global_regs[dst_code] = ~0;
			break;

		case 22:
			if (SR & N_MASK)
				m_global_regs[dst_code] = ~0;
			else
				m_global_regs[dst_code] = 0;
			break;

		case 23:
			if (SR & N_MASK)
				m_global_regs[dst_code] = 0;
			else
				m_global_regs[dst_code] = ~0;
			break;

		case 24:
			if (SR & (C_MASK | Z_MASK))
				m_global_regs[dst_code] = ~0;
			else
				m_global_regs[dst_code] = 0;
			break;

		case 25:
			if (SR & (C_MASK | Z_MASK))
				m_global_regs[dst_code] = 0;
			else
				m_global_regs[dst_code] = ~0;
			break;

		case 26:
			if (SR & C_MASK)
				m_global_regs[dst_code] = ~0;
			else
				m_global_regs[dst_code] = 0;
			break;

		case 27:
			if (SR & C_MASK)
				m_global_regs[dst_code] = 0;
			else
				m_global_regs[dst_code] = ~0;
			break;

		case 28:
			if (SR & Z_MASK)
				m_global_regs[dst_code] = ~0;
			else
				m_global_regs[dst_code] = 0;
			break;

		case 29:
			if (SR & Z_MASK)
				m_global_regs[dst_code] = 0;
			else
				m_global_regs[dst_code] = ~0;
			break;

		case 30:
			if (SR & V_MASK)
				m_global_regs[dst_code] = ~0;
			else
				m_global_regs[dst_code] = 0;
			break;

		case 31:
			if (SR & V_MASK)
				m_global_regs[dst_code] = 0;
			else
				m_global_regs[dst_code] = ~0;
			break;
	}

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_set_local()
{
	check_delay_PC();

	const uint32_t dst_code = (DST_CODE + GET_FP) & 0x3f;
	const uint32_t n = N_VALUE;

	switch (n)
	{
		// SETADR
		case 0:
			m_local_regs[dst_code] = (SP & 0xfffffe00) | (GET_FP << 2) | (((SP & 0x100) && (SIGN_BIT(SR) == 0)) ? 1 : 0);
			break;

		// Reserved
		case 1:
		case 16:
		case 17:
		case 19:
			DEBUG_PRINTF(("Used reserved N value (%d) in hyperstone_set. PC = %08X\n", n, PC));
			break;

		// SETxx
		case 2:
			m_local_regs[dst_code] = 1;
			break;

		case 3:
			m_local_regs[dst_code] = 0;
			break;

		case 4:
			if (SR & (N_MASK | Z_MASK))
				m_local_regs[dst_code] = 1;
			else
				m_local_regs[dst_code] = 0;
			break;

		case 5:
			if (SR & (N_MASK | Z_MASK))
				m_local_regs[dst_code] = 0;
			else
				m_local_regs[dst_code] = 1;
			break;

		case 6:
			if (SR & N_MASK)
				m_local_regs[dst_code] = 1;
			else
				m_local_regs[dst_code] = 0;
			break;

		case 7:
			if (SR & N_MASK)
				m_local_regs[dst_code] = 0;
			else
				m_local_regs[dst_code] = 1;
			break;

		case 8:
			if (SR & (C_MASK | Z_MASK))
				m_local_regs[dst_code] = 1;
			else
				m_local_regs[dst_code] = 0;
			break;

		case 9:
			if (SR & (C_MASK | Z_MASK))
				m_local_regs[dst_code] = 0;
			else
				m_local_regs[dst_code] = 1;
			break;

		case 10:
			if (SR & C_MASK)
				m_local_regs[dst_code] = 1;
			else
				m_local_regs[dst_code] = 0;
			break;

		case 11:
			if (SR & C_MASK)
				m_local_regs[dst_code] = 0;
			else
				m_local_regs[dst_code] = 1;
			break;

		case 12:
			if (SR & Z_MASK)
				m_local_regs[dst_code] = 1;
			else
				m_local_regs[dst_code] = 0;
			break;

		case 13:
			if (SR & Z_MASK)
				m_local_regs[dst_code] = 0;
			else
				m_local_regs[dst_code] = 1;
			break;

		case 14:
			if (SR & V_MASK)
				m_local_regs[dst_code] = 1;
			else
				m_local_regs[dst_code] = 0;
			break;

		case 15:
			if (SR & V_MASK)
				m_local_regs[dst_code] = 0;
			else
				m_local_regs[dst_code] = 1;
			break;

		case 18:
			m_local_regs[dst_code] = ~0;
			break;

		case 20:
			if (SR & (N_MASK | Z_MASK))
				m_local_regs[dst_code] = ~0;
			else
				m_local_regs[dst_code] = 0;
			break;

		case 21:
			if (SR & (N_MASK | Z_MASK))
				m_local_regs[dst_code] = 0;
			else
				m_local_regs[dst_code] = ~0;
			break;

		case 22:
			if (SR & N_MASK)
				m_local_regs[dst_code] = ~0;
			else
				m_local_regs[dst_code] = 0;
			break;

		case 23:
			if (SR & N_MASK)
				m_local_regs[dst_code] = 0;
			else
				m_local_regs[dst_code] = ~0;
			break;

		case 24:
			if (SR & (C_MASK | Z_MASK))
				m_local_regs[dst_code] = ~0;
			else
				m_local_regs[dst_code] = 0;
			break;

		case 25:
			if (SR & (C_MASK | Z_MASK))
				m_local_regs[dst_code] = 0;
			else
				m_local_regs[dst_code] = ~0;
			break;

		case 26:
			if (SR & C_MASK)
				m_local_regs[dst_code] = ~0;
			else
				m_local_regs[dst_code] = 0;
			break;

		case 27:
			if (SR & C_MASK)
				m_local_regs[dst_code] = 0;
			else
				m_local_regs[dst_code] = ~0;
			break;

		case 28:
			if (SR & Z_MASK)
				m_local_regs[dst_code] = ~0;
			else
				m_local_regs[dst_code] = 0;
			break;

		case 29:
			if (SR & Z_MASK)
				m_local_regs[dst_code] = 0;
			else
				m_local_regs[dst_code] = ~0;
			break;

		case 30:
			if (SR & V_MASK)
				m_local_regs[dst_code] = ~0;
			else
				m_local_regs[dst_code] = 0;
			break;

		case 31:
			if (SR & V_MASK)
				m_local_regs[dst_code] = 0;
			else
				m_local_regs[dst_code] = ~0;
			break;
	}

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_mul_global_global()
{
	check_delay_PC();

	const uint32_t src_code = SRC_CODE;
	const uint32_t dst_code = DST_CODE;

	if (src_code < 2 || dst_code < 2)
	{
		DEBUG_PRINTF(("Denoted PC or SR in hyperstone_mul instruction. PC = %08X\n", PC));
		return;
	}

	const uint32_t sreg = m_global_regs[src_code];
	const uint32_t dreg = m_global_regs[dst_code];
	const uint32_t result = sreg * dreg;

	set_global_register(dst_code, result);

	SR &= ~(Z_MASK | N_MASK);
	if (result == 0)
		SR |= Z_MASK;
	SR |= SIGN_TO_N(result);

	if ((sreg >= 0xffff8000 && sreg <= 0x7fff) && (dreg >= 0xffff8000 && dreg <= 0x7fff))
		m_icount -= 3 << m_clck_scale;
	else
		m_icount -= 5 << m_clck_scale;
}

void hyperstone_device::hyperstone_mul_global_local()
{
	check_delay_PC();

	const uint32_t src_code = (SRC_CODE + GET_FP) & 0x3f;
	const uint32_t dst_code = DST_CODE;

	if (dst_code < 2)
	{
		DEBUG_PRINTF(("Denoted PC or SR in hyperstone_mul instruction. PC = %08X\n", PC));
		return;
	}

	const uint32_t sreg = m_local_regs[src_code];
	const uint32_t dreg = m_global_regs[dst_code];
	const uint32_t result = sreg * dreg;

	set_global_register(dst_code, result);

	SR &= ~(Z_MASK | N_MASK);
	if (result == 0)
		SR |= Z_MASK;
	SR |= SIGN_TO_N(result);

	if ((sreg >= 0xffff8000 && sreg <= 0x7fff) && (dreg >= 0xffff8000 && dreg <= 0x7fff))
		m_icount -= 3 << m_clck_scale;
	else
		m_icount -= 5 << m_clck_scale;
}

void hyperstone_device::hyperstone_mul_local_global()
{
	check_delay_PC();

	const uint32_t src_code = SRC_CODE;
	const uint32_t dst_code = (DST_CODE + GET_FP) & 0x3f;

	if (src_code < 2)
	{
		DEBUG_PRINTF(("Denoted PC or SR in hyperstone_mul instruction. PC = %08X\n", PC));
		return;
	}

	const uint32_t sreg = m_global_regs[src_code];
	const uint32_t dreg = m_local_regs[dst_code];
	const uint32_t result = sreg * dreg;

	m_local_regs[dst_code] = result;

	SR &= ~(Z_MASK | N_MASK);
	if (result == 0)
		SR |= Z_MASK;
	SR |= SIGN_TO_N(result);

	if ((sreg >= 0xffff8000 && sreg <= 0x7fff) && (dreg >= 0xffff8000 && dreg <= 0x7fff))
		m_icount -= 3 << m_clck_scale;
	else
		m_icount -= 5 << m_clck_scale;
}

void hyperstone_device::hyperstone_mul_local_local()
{
	check_delay_PC();

	const uint32_t fp = GET_FP;
	const uint32_t dst_code = (DST_CODE + fp) & 0x3f;
	const uint32_t sreg = m_local_regs[(SRC_CODE + fp) & 0x3f];
	const uint32_t dreg = m_local_regs[dst_code];
	const uint32_t result = sreg * dreg;

	m_local_regs[dst_code] = result;

	SR &= ~(Z_MASK | N_MASK);
	if (result == 0)
		SR |= Z_MASK;
	SR |= SIGN_TO_N(result);

	if ((sreg >= 0xffff8000 && sreg <= 0x7fff) && (dreg >= 0xffff8000 && dreg <= 0x7fff))
		m_icount -= 3 << m_clck_scale;
	else
		m_icount -= 5 << m_clck_scale;
}


void hyperstone_device::hyperstone_extend()
{
	m_instruction_length = (2<<19);
	const uint32_t func = READ_OP(PC);
	PC += 2;
	check_delay_PC();

	//TODO: add locks, overflow error and other things
	const uint32_t fp = GET_FP;
	const uint32_t vals = m_local_regs[(SRC_CODE + fp) & 0x3f];
	const uint32_t vald = m_local_regs[(DST_CODE + fp) & 0x3f];

	switch (func) // extended opcode
	{
		// signed or unsigned multiplication, single word product
		case EMUL:
		case 0x100: // used in "N" type cpu
			m_global_regs[15] = (uint32_t)(vals * vald);
			break;

		// unsigned multiplication, double word product
		case EMULU:
		{
			const uint64_t result = (uint64_t)vals * (uint64_t)vald;
			m_global_regs[14] = (uint32_t)(result >> 32);
			m_global_regs[15] = (uint32_t)result;
			break;
		}
		// signed multiplication, double word product
		case EMULS:
		{
			const int64_t result = (int64_t)(int32_t)vals * (int64_t)(int32_t)vald;
			m_global_regs[14] = (uint32_t)(result >> 32);
			m_global_regs[15] = (uint32_t)result;

			break;
		}

		// signed multiply/add, single word product sum
		case EMAC:
			m_global_regs[15] += (int32_t)vals * (int32_t)vald;
			break;

		// signed multiply/add, double word product sum
		case EMACD:
		{
			int64_t result = (int64_t)concat_64(m_global_regs[14], m_global_regs[15]) + (int64_t)((int64_t)(int32_t)vals * (int64_t)(int32_t)vald);
			m_global_regs[14] = (uint32_t)(result >> 32);
			m_global_regs[15] = (uint32_t)result;
			break;
		}
		// signed multiply/substract, single word product difference
		case EMSUB:
			m_global_regs[15] = (int32_t)m_global_regs[15] - ((int32_t)vals * (int32_t)vald);
			break;

		// signed multiply/substract, double word product difference
		case EMSUBD:
		{
			int64_t result = (int64_t)concat_64(m_global_regs[14], m_global_regs[15]) - (int64_t)((int64_t)(int32_t)vals * (int64_t)(int32_t)vald);
			m_global_regs[14] = (uint32_t)(result >> 32);
			m_global_regs[15] = (uint32_t)result;

			break;
		}

		// signed half-word multiply/add, single word product sum
		case EHMAC:
			m_global_regs[15] = (int32_t)m_global_regs[15] + ((int32_t)((vald & 0xffff0000) >> 16) * (int32_t)((vals & 0xffff0000) >> 16)) + ((int32_t)(vald & 0xffff) * (int32_t)(vals & 0xffff));
			break;

		// signed half-word multiply/add, double word product sum
		case EHMACD:
		{
			int64_t result = (int64_t)concat_64(m_global_regs[14], m_global_regs[15]) + (int64_t)((int64_t)(int32_t)((vald & 0xffff0000) >> 16) * (int64_t)(int32_t)((vals & 0xffff0000) >> 16)) + ((int64_t)(int32_t)(vald & 0xffff) * (int64_t)(int32_t)(vals & 0xffff));
			m_global_regs[14] = (uint32_t)(result >> 32);
			m_global_regs[15] = (uint32_t)result;
			break;
		}

		// half-word complex multiply
		case EHCMULD:
			m_global_regs[14] = (((vald & 0xffff0000) >> 16) * ((vals & 0xffff0000) >> 16)) - ((vald & 0xffff) * (vals & 0xffff));
			m_global_regs[15] = (((vald & 0xffff0000) >> 16) * (vals & 0xffff)) + ((vald & 0xffff) * ((vals & 0xffff0000) >> 16));
			break;

		// half-word complex multiply/add
		case EHCMACD:
			m_global_regs[14] += (((vald & 0xffff0000) >> 16) * ((vals & 0xffff0000) >> 16)) - ((vald & 0xffff) * (vals & 0xffff));
			m_global_regs[15] += (((vald & 0xffff0000) >> 16) * (vals & 0xffff)) + ((vald & 0xffff) * ((vals & 0xffff0000) >> 16));
			break;

		// half-word (complex) add/substract
		// Ls is not used and should denote the same register as Ld
		case EHCSUMD:
		{
			const uint32_t r14 = m_global_regs[14];
			const uint32_t r15 = m_global_regs[15];
			m_global_regs[14] = (((((vals & 0xffff0000) >> 16) + r14) << 16) & 0xffff0000) | (((vals & 0xffff) + r15) & 0xffff);
			m_global_regs[15] = (((((vals & 0xffff0000) >> 16) - r14) << 16) & 0xffff0000) | (((vals & 0xffff) - r15) & 0xffff);
			break;
		}

		// half-word (complex) add/substract with fixed point adjustment
		// Ls is not used and should denote the same register as Ld
		case EHCFFTD:
		{
			const uint32_t r14 = m_global_regs[14];
			const uint32_t r15 = m_global_regs[15];
			m_global_regs[14] = (((((vals & 0xffff0000) >> 16) + (r14 >> 15)) << 16) & 0xffff0000) | (((vals & 0xffff) + (r15 >> 15)) & 0xffff);
			m_global_regs[15] = (((((vals & 0xffff0000) >> 16) - (r14 >> 15)) << 16) & 0xffff0000) | (((vals & 0xffff) - (r15 >> 15)) & 0xffff);
			break;
		}

		// half-word (complex) add/substract with fixed point adjustment and shift
		// Ls is not used and should denote the same register as Ld
		case EHCFFTSD:
		{
			const uint32_t r14 = m_global_regs[14];
			const uint32_t r15 = m_global_regs[15];
			m_global_regs[14] = ((((((vals & 0xffff0000) >> 16) + (r14 >> 15)) >> 1) << 16) & 0xffff0000) | (((((vals & 0xffff) + (r15 >> 15)) >> 1) & 0xffff));
			m_global_regs[15] = ((((((vals & 0xffff0000) >> 16) - (r14 >> 15)) >> 1) << 16) & 0xffff0000) | (((((vals & 0xffff) - (r15 >> 15)) >> 1) & 0xffff));
			break;
		}

		default:
			DEBUG_PRINTF(("Executed Illegal extended opcode (%X). PC = %08X\n", func, PC));
			break;
	}

	m_icount -= m_clock_cycles_1; //TODO: with the latency it can change
}



void hyperstone_device::hyperstone_ldwr_global_local()
{
	check_delay_PC();
	set_global_register(SRC_CODE, READ_W(m_local_regs[(DST_CODE + GET_FP) & 0x3f]));
	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_ldwr_local_local() // ldwr local,local
{
	check_delay_PC();
	const uint32_t fp = GET_FP;
	m_local_regs[(SRC_CODE + fp) & 0x3f] = READ_W(m_local_regs[(DST_CODE + fp) & 0x3f]);
	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_lddr_global_local()
{
	check_delay_PC();

	const uint32_t src_code = SRC_CODE;
	const uint32_t dreg = m_local_regs[(DST_CODE + GET_FP) & 0x3f];
	set_global_register(src_code, READ_W(dreg));
	set_global_register(src_code + 1, READ_W(dreg + 4));

	m_icount -= m_clock_cycles_2;
}

void hyperstone_device::hyperstone_lddr_local_local()
{
	check_delay_PC();

	const uint32_t fp = GET_FP;
	const uint32_t src_code = SRC_CODE + fp;
	const uint32_t dreg = m_local_regs[(DST_CODE + fp) & 0x3f];

	m_local_regs[src_code & 0x3f] = READ_W(dreg);
	m_local_regs[(src_code + 1) & 0x3f] = READ_W(dreg + 4);

	m_icount -= m_clock_cycles_2;
}

void hyperstone_device::hypesrtone_ldwp_global_local()
{
	check_delay_PC();

	const uint32_t dst_code = (DST_CODE + GET_FP) & 0x3f;
	set_global_register(SRC_CODE, READ_W(m_local_regs[dst_code]));
	m_local_regs[dst_code] += 4;

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hypesrtone_ldwp_local_local()
{
	check_delay_PC();

	const uint32_t fp = GET_FP;
	const uint32_t src_code = (SRC_CODE + fp) & 0x3f;
	const uint32_t dst_code = (DST_CODE + fp) & 0x3f;

	m_local_regs[src_code] = READ_W(m_local_regs[dst_code]);

	// post increment the destination register if it's different from the source one
	// (needed by Hidden Catch)
	if (src_code != dst_code)
		m_local_regs[dst_code] += 4;

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_lddp_global_local()
{
	check_delay_PC();

	const uint32_t src_code = SRC_CODE;
	const uint32_t dst_code = (DST_CODE + GET_FP) & 0x3f;
	const uint32_t dreg = m_local_regs[dst_code];

	set_global_register(src_code, READ_W(dreg));
	set_global_register(src_code + 1, READ_W(dreg + 4));

	m_local_regs[dst_code] += 8;

	m_icount -= m_clock_cycles_2;
}

void hyperstone_device::hyperstone_lddp_local_local()
{
	check_delay_PC();

	const uint32_t fp = GET_FP;
	const uint32_t src_code = SRC_CODE + fp;
	const uint32_t dst_code = (DST_CODE + fp) & 0x3f;
	const uint32_t dreg = m_local_regs[dst_code];
	const bool same_srcf_dst = (((src_code + 1) & 0x3f) == dst_code);

	m_local_regs[src_code & 0x3f] = READ_W(dreg);
	m_local_regs[(src_code + 1) & 0x3f] = READ_W(dreg + 4);

	// post increment the destination register if it's different from the source one
	// and from the "next source" one
	if (src_code != dst_code && !same_srcf_dst)
	{
		m_local_regs[dst_code] += 8;
	}

	m_icount -= m_clock_cycles_2;
}

void hyperstone_device::hyperstone_stwr_global()
{
	check_delay_PC();

	const uint32_t src_code = SRC_CODE;
	const uint32_t sreg = ((src_code == SR_REGISTER) ? 0 : m_global_regs[src_code]);
	WRITE_W(m_local_regs[(DST_CODE + GET_FP) & 0x3f], sreg);

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_stwr_local()
{
	check_delay_PC();

	const uint32_t fp = GET_FP;
	WRITE_W(m_local_regs[(DST_CODE + fp) & 0x3f], m_local_regs[(SRC_CODE + fp) & 0x3f]);

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_stdr_global()
{
	check_delay_PC();

	const uint32_t src_code = SRC_CODE;
	const bool src_is_sr = (src_code == SR_REGISTER);
	const uint32_t sreg = src_is_sr ? 0 : m_global_regs[src_code];
	const uint32_t sregf = src_is_sr ? 0 : m_global_regs[src_code + 1];
	const uint32_t dreg = m_local_regs[(DST_CODE + GET_FP) & 0x3f];

	WRITE_W(dreg, sreg);
	WRITE_W(dreg + 4, sregf);

	m_icount -= m_clock_cycles_2;
}

void hyperstone_device::hyperstone_stdr_local()
{
	check_delay_PC();

	const uint32_t fp = GET_FP;
	const uint32_t src_code = (SRC_CODE + fp) & 0x3f;
	const uint32_t dreg = m_local_regs[(DST_CODE + fp) & 0x3f];

	WRITE_W(dreg, m_local_regs[src_code]);
	WRITE_W(dreg + 4, m_local_regs[(src_code + 1) & 0x3f]);

	m_icount -= m_clock_cycles_2;
}

void hyperstone_device::hyperstone_stwp_global_local()
{
	check_delay_PC();

	const uint32_t src_code = SRC_CODE;
	const uint32_t sreg = ((src_code == SR_REGISTER) ? 0 : m_global_regs[src_code]);

	const uint32_t dst_code = (DST_CODE + GET_FP) & 0x3f;
	const uint32_t dreg = m_local_regs[dst_code];

	WRITE_W(dreg, sreg);
	m_local_regs[dst_code] += 4;

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_stwp_local_local()
{
	check_delay_PC();

	const uint32_t fp = GET_FP;
	const uint32_t dst_code = (DST_CODE + fp) & 0x3f;
	WRITE_W(m_local_regs[dst_code], m_local_regs[(SRC_CODE + fp) & 0x3f]);
	m_local_regs[dst_code] += 4;

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_stdp_global_local()
{
	check_delay_PC();

	const uint32_t src_code = SRC_CODE;
	const uint32_t sreg = (src_code == SR_REGISTER) ? 0 : m_global_regs[src_code];
	const uint32_t sregf = (src_code == SR_REGISTER) ? 0 : m_global_regs[src_code + 1];

	const uint32_t dst_code = (DST_CODE + GET_FP) & 0x3f;
	const uint32_t dreg = m_local_regs[dst_code];

	WRITE_W(dreg, sreg);
	WRITE_W(dreg + 4, sregf);
	m_local_regs[dst_code] += 8;

	m_icount -= m_clock_cycles_2;
}

void hyperstone_device::hyperstone_stdp_local_local()
{
	check_delay_PC();
	const uint32_t fp = GET_FP;
	const uint32_t src_code = SRC_CODE + fp;
	const uint32_t srcf_code = (SRC_CODE + fp + 1) & 0x3f;
	const uint32_t dst_code = (DST_CODE + fp) & 0x3f;

	const uint32_t sreg = m_local_regs[src_code & 0x3f];
	const uint32_t sregf = m_local_regs[srcf_code];
	const uint32_t dreg = m_local_regs[dst_code]; /* registers offset by frame pointer */

	bool same_srcf_dst = (srcf_code == dst_code);

	WRITE_W(dreg, sreg);
	m_local_regs[dst_code] += 8;

	if (same_srcf_dst)
		WRITE_W(dreg + 4, sregf + 8); // because DREG == SREGF and DREG has been incremented
	else
		WRITE_W(dreg + 4, sregf);

	m_icount -= m_clock_cycles_2;
}



void hyperstone_device::hyperstone_dbv()
{
	if (SR & V_MASK)
	{
		const int32_t offset = decode_pcrel();
		check_delay_PC();
		m_delay_slot = true;
		m_delay_pc = PC + offset;
		m_intblock = 3;
	}
	else
	{
		ignore_pcrel();
		check_delay_PC();
	}

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_dbnv()
{
	if (SR & V_MASK)
	{
		ignore_pcrel();
		check_delay_PC();
	}
	else
	{
		const int32_t offset = decode_pcrel();
		check_delay_PC();
		m_delay_slot = true;
		m_delay_pc = PC + offset;
		m_intblock = 3;
	}

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_dbe()
{
	if (SR & Z_MASK)
	{
		const int32_t offset = decode_pcrel();
		check_delay_PC();
		m_delay_slot = true;
		m_delay_pc = PC + offset;
		m_intblock = 3;
	}
	else
	{
		ignore_pcrel();
		check_delay_PC();
	}

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_dbne()
{
	if (SR & Z_MASK)
	{
		ignore_pcrel();
		check_delay_PC();
	}
	else
	{
		const int32_t offset = decode_pcrel();
		check_delay_PC();
		m_delay_slot = true;
		m_delay_pc = PC + offset;
		m_intblock = 3;
	}
	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_dbc()
{
	if (SR & C_MASK)
	{
		const int32_t offset = decode_pcrel();
		check_delay_PC();
		m_delay_slot = true;
		m_delay_pc = PC + offset;
		m_intblock = 3;
	}
	else
	{
		ignore_pcrel();
		check_delay_PC();
	}

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_dbnc()
{
	if (SR & C_MASK)
	{
		ignore_pcrel();
		check_delay_PC();
	}
	else
	{
		const int32_t offset = decode_pcrel();
		check_delay_PC();
		m_delay_slot = true;
		m_delay_pc = PC + offset;
		m_intblock = 3;
	}

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_dbse()
{
	if (SR & (C_MASK | Z_MASK))
	{
		const int32_t offset = decode_pcrel();
		check_delay_PC();
		m_delay_slot = true;
		m_delay_pc = PC + offset;
		m_intblock = 3;
	}
	else
	{
		ignore_pcrel();
		check_delay_PC();
	}

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_dbht()
{
	if (SR & (C_MASK | Z_MASK))
	{
		ignore_pcrel();
		check_delay_PC();
	}
	else
	{
		const int32_t offset = decode_pcrel();
		check_delay_PC();
		m_delay_slot = true;
		m_delay_pc = PC + offset;
		m_intblock = 3;
	}

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_dbn()
{
	if (SR & N_MASK)
	{
		const int32_t offset = decode_pcrel();
		check_delay_PC();
		m_delay_slot = true;
		m_delay_pc = PC + offset;
		m_intblock = 3;
	}
	else
	{
		ignore_pcrel();
		check_delay_PC();
	}

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_dbnn()
{
	if (SR & N_MASK)
	{
		ignore_pcrel();
		check_delay_PC();
	}
	else
	{
		const int32_t offset = decode_pcrel();
		check_delay_PC();
		m_delay_slot = true;
		m_delay_pc = PC + offset;
		m_intblock = 3;
	}

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_dble()
{
	if (SR & (N_MASK | Z_MASK))
	{
		const int32_t offset = decode_pcrel();
		check_delay_PC();
		m_delay_slot = true;
		m_delay_pc = PC + offset;
		m_intblock = 3;
	}
	else
	{
		ignore_pcrel();
		check_delay_PC();
	}

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_dbgt()
{
	if (SR & (N_MASK | Z_MASK))
	{
		ignore_pcrel();
		check_delay_PC();
	}
	else
	{
		const int32_t offset = decode_pcrel();
		check_delay_PC();
		m_delay_slot = true;
		m_delay_pc = PC + offset;
		m_intblock = 3;
	}

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_dbr()
{
	const int32_t offset = decode_pcrel();
	check_delay_PC();

	m_delay_slot = true;
	m_delay_pc = PC + offset;
	m_intblock = 3;
}

void hyperstone_device::hyperstone_frame()
{
	check_delay_PC();

	uint8_t realfp = GET_FP - SRC_CODE;
	uint8_t dst_code = DST_CODE;

	SET_FP(realfp);
	SET_FL(dst_code);
	SR &= ~M_MASK;

	int8_t difference = ((SP & 0x1fc) >> 2) + (64 - 10) - (realfp + GET_FL); // really it's 7 bits

	/* convert to 8 bits */
	if(difference > 63)
		difference = (int8_t)(difference|0x80);
	else if( difference < -64 )
		difference = difference & 0x7f;

	if (difference < 0) // else it's finished
	{
		bool tmp_flag = SP >= UB;

		for (; difference < 0; difference++)
		{
			WRITE_W(SP, m_local_regs[(SP & 0xfc) >> 2]);
			SP += 4;
		}

		if (tmp_flag)
		{
			uint32_t addr = get_trap_addr(TRAPNO_FRAME_ERROR);
			execute_exception(addr);
		}
	}

	//TODO: no 1!
	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_call_global()
{
	uint16_t imm_1 = READ_OP(PC);
	PC += 2;

	int32_t extra_s = 0;

	if (E_BIT(imm_1))
	{
		uint16_t imm_2 = READ_OP(PC);

		PC += 2;
		SET_ILC(3<<19);

		extra_s = imm_2;
		extra_s |= ((imm_1 & 0x3fff) << 16);

		if (S_BIT_CONST(imm_1))
			extra_s |= 0xc0000000;
	}
	else
	{
		extra_s = imm_1 & 0x3fff;

		SET_ILC(2<<19);

		if (S_BIT_CONST(imm_1))
			extra_s |= 0xffffc000;
	}

	check_delay_PC();

	uint32_t src_code = SRC_CODE;
	uint32_t dst_code = DST_CODE;

	uint32_t sreg = m_global_regs[src_code];

	if (src_code == SR_REGISTER)
		sreg = 0;

	if (!DST_CODE)
		dst_code = 16;

	uint32_t fp = GET_FP;
	uint32_t dreg_index = dst_code + fp;
	m_local_regs[dreg_index & 0x3f] = (PC & ~1) | GET_S;
	m_local_regs[(dreg_index + 1) & 0x3f] = SR;

	SET_FP(fp + dst_code);
	SET_FL(6); //default value for call
	SR &= ~M_MASK;

	PPC = PC;
	PC = (extra_s & ~1) + sreg;

	m_intblock = 2;

	//TODO: add interrupt locks, errors, ....

	//TODO: no 1!
	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_call_local()
{
	uint16_t imm_1 = READ_OP(PC);
	PC += 2;

	int32_t extra_s = 0;

	if (E_BIT(imm_1))
	{
		uint16_t imm_2 = READ_OP(PC);

		PC += 2;
		SET_ILC(3<<19);

		extra_s = imm_2;
		extra_s |= ((imm_1 & 0x3fff) << 16);

		if (S_BIT_CONST(imm_1))
			extra_s |= 0xc0000000;
	}
	else
	{
		extra_s = imm_1 & 0x3fff;

		SET_ILC(2<<19);

		if (S_BIT_CONST(imm_1))
			extra_s |= 0xffffc000;
	}

	check_delay_PC();

	uint32_t src_code = SRC_CODE;
	uint32_t dst_code = DST_CODE;

	if (!DST_CODE)
		dst_code = 16;

	uint32_t fp = GET_FP;
	extra_s = (extra_s & ~1) + m_local_regs[(src_code + fp) & 0x3f];

	uint32_t dreg_index = dst_code + fp;
	m_local_regs[dreg_index & 0x3f] = (PC & ~1) | GET_S;
	m_local_regs[(dreg_index + 1) & 0x3f] = SR;

	SET_FP(fp + dst_code);
	SET_FL(6); //default value for call
	SR &= ~M_MASK;

	PPC = PC;
	PC = extra_s;

	m_intblock = 2;

	//TODO: add interrupt locks, errors, ....

	//TODO: no 1!
	m_icount -= m_clock_cycles_1;
}



void hyperstone_device::hyperstone_bv()
{
	if (SR & V_MASK)
	{
		execute_br();
	}
	else
	{
		ignore_pcrel();
		check_delay_PC();
		m_icount -= m_clock_cycles_1;
	}
}

void hyperstone_device::hyperstone_bnv()
{
	if (SR & V_MASK)
	{
		ignore_pcrel();
		check_delay_PC();
		m_icount -= m_clock_cycles_1;
	}
	else
	{
		execute_br();
	}
}

void hyperstone_device::hyperstone_be()
{
	if (SR & Z_MASK)
	{
		execute_br();
	}
	else
	{
		ignore_pcrel();
		check_delay_PC();
		m_icount -= m_clock_cycles_1;
	}
}

void hyperstone_device::hyperstone_bne()
{
	if (SR & Z_MASK)
	{
		ignore_pcrel();
		check_delay_PC();
		m_icount -= m_clock_cycles_1;
	}
	else
	{
		execute_br();
	}
}

void hyperstone_device::hyperstone_bc()
{
	if (SR & C_MASK)
	{
		execute_br();
	}
	else
	{
		ignore_pcrel();
		check_delay_PC();
		m_icount -= m_clock_cycles_1;
	}
}

void hyperstone_device::hyperstone_bnc()
{
	if (SR & C_MASK)
	{
		ignore_pcrel();
		check_delay_PC();
		m_icount -= m_clock_cycles_1;
	}
	else
	{
		execute_br();
	}
}

void hyperstone_device::hyperstone_bse()
{
	if (SR & (C_MASK | Z_MASK))
	{
		execute_br();
	}
	else
	{
		ignore_pcrel();
		check_delay_PC();
		m_icount -= m_clock_cycles_1;
	}
}

void hyperstone_device::hyperstone_bht()
{
	if (SR & (C_MASK | Z_MASK))
	{
		ignore_pcrel();
		check_delay_PC();
		m_icount -= m_clock_cycles_1;
	}
	else
	{
		execute_br();
	}
}

void hyperstone_device::hyperstone_bn()
{
	if (SR & N_MASK)
	{
		execute_br();
	}
	else
	{
		ignore_pcrel();
		check_delay_PC();
		m_icount -= m_clock_cycles_1;
	}
}

void hyperstone_device::hyperstone_bnn()
{
	if (SR & N_MASK)
	{
		ignore_pcrel();
		check_delay_PC();
		m_icount -= m_clock_cycles_1;
	}
	else
	{
		execute_br();
	}
}

void hyperstone_device::hyperstone_ble()
{
	if (SR & (N_MASK | Z_MASK))
	{
		execute_br();
	}
	else
	{
		ignore_pcrel();
		check_delay_PC();
		m_icount -= m_clock_cycles_1;
	}
}

void hyperstone_device::hyperstone_bgt()
{
	if (SR & (N_MASK | Z_MASK))
	{
		ignore_pcrel();
		check_delay_PC();
		m_icount -= m_clock_cycles_1;
	}
	else
	{
		execute_br();
	}
}
