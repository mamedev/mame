// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "arcompact_helper.ipp"

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS
// LR b,[c]                        0010 0bbb 0010 1010   0BBB CCCC CCRR RRRR
// LR b,[limm]                     0010 0bbb 0010 1010   0BBB 1111 10RR RRRR (+ Limm)
// LR b,[u6]                       0010 0bbb 0110 1010   0BBB uuuu uu00 0000
// LR b,[s12]                      0010 0bbb 1010 1010   0BBB ssss ssSS SSSS
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_LR(uint32_t op)
{
	switch ((op & 0x00c00000) >> 22)
	{
	case 0x00:
	{
		int size = 4;
		uint8_t breg = common32_get_breg(op);
		uint8_t creg = common32_get_creg(op);
		if (creg == REG_LIMM)
		{
			get_limm_32bit_opcode();
			size = 8;
		}
		uint32_t c = m_regs[creg];
		m_regs[breg] = READAUX(c);
		return m_pc + size;
	}
	case 0x01:
	{
		int size = 4;
		uint8_t breg = common32_get_breg(op);
		uint32_t u = common32_get_u6(op);
		uint32_t c = u;
		m_regs[breg] = READAUX(c);
		return m_pc + size;
	}
	case 0x02:
	{
		int size = 4;
		uint8_t breg = common32_get_breg(op);
		uint32_t S = common32_get_s12(op);
		uint32_t c = (uint32_t)S;
		m_regs[breg] = READAUX(c);
		return m_pc + size;
	}
	case 0x03:
	{
		switch ((op & 0x00000020) >> 5)
		{
		case 0x00:
		{
			int size = 4;
			fatalerror("handleop32_LR_cc_f_b_b_c (LR)\n");
			return m_pc + size;
		}
		case 0x01:
		{
			int size = 4;
			uint8_t breg = common32_get_breg(op);
			uint32_t u = common32_get_u6(op);
			uint32_t c = u;
			uint8_t condition = common32_get_condition(op);
			if (!check_condition(condition))
				return m_pc + size;
			m_regs[breg] = READAUX(c);
			return m_pc + size;
		}
		}
		return 0;
	}
	}
	return 0;
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS
// SR b,[c]                        0010 0bbb 0010 1011   0BBB CCCC CCRR RRRR
// SR b,[limm]                     0010 0bbb 0010 1011   0BBB 1111 10RR RRRR (+ Limm)
// SR b,[u6]                       0010 0bbb 0110 1011   0BBB uuuu uu00 0000
// SR b,[s12]                      0010 0bbb 1010 1011   0BBB ssss ssSS SSSS
// SR limm,[c]                     0010 0110 0010 1011   0111 CCCC CCRR RRRR (+ Limm)
// SR limm,[u6]                    0010 0110 0110 1011   0111 uuuu uu00 0000
// SR limm,[s12]                   0010 0110 1010 1011   0111 ssss ssSS SSSS (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_SR(uint32_t op)
{
	switch ((op & 0x00c00000) >> 22)
	{
	case 0x00:
	{
		uint8_t breg = common32_get_breg(op);
		uint8_t creg = common32_get_creg(op);
		int size = check_limm(breg, creg);
		uint32_t b = m_regs[breg];
		uint32_t c = m_regs[creg];
		WRITEAUX(c, b);
		return m_pc + size;
	}
	case 0x01:
	{
		uint8_t breg = common32_get_breg(op);
		uint32_t u = common32_get_u6(op);
		int size = check_limm(breg);
		uint32_t b = m_regs[breg];
		uint32_t c = u;
		WRITEAUX(c, b);
		return m_pc + size;
	}
	case 0x02:
	{
		uint8_t breg = common32_get_breg(op);
		uint32_t S = common32_get_s12(op);
		int size = check_limm(breg);
		uint32_t b = m_regs[breg];
		uint32_t c = (uint32_t)S;
		WRITEAUX(c, b);
		return m_pc + size;
	}
	case 0x03:
	{
		switch ((op & 0x00000020) >> 5)
		{
		case 0x00:
		{
			int size = 4;
			fatalerror("handleop32_SR_cc_f_b_b_c (SR)\n");
			return m_pc + size;
		}
		case 0x01:
		{
			uint8_t breg = common32_get_breg(op);
			uint32_t u = common32_get_u6(op);
			int size = check_limm(breg);
			uint32_t b = m_regs[breg];
			uint32_t c = u;
			uint8_t condition = common32_get_condition(op);
			if (!check_condition(condition))
				return m_pc + size;
			WRITEAUX(c, b);
			return m_pc + size;
		}
		}
		return 0;
	}
	}
	return 0;
}
