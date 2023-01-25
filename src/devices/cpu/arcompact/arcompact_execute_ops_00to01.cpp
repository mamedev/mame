// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "arcompact.h"

#define GET_01_01_01_BRANCH_ADDR \
	int32_t address = (op & 0x00fe0000) >> 17; \
	address |= ((op & 0x00008000) >> 15) << 7; \
	if (address & 0x80) address = -0x80 + (address & 0x7f);

uint32_t arcompact_device::handleop32_B_cc_D_s21(uint32_t op)
{
	int size = 4;
	uint8_t condition = common32_get_condition(op);

	if (!check_condition(condition))
		return m_pc + size;

	// Branch Conditionally
	// 0000 0sss ssss sss0 SSSS SSSS SSNQ QQQQ
	int32_t address = (op & 0x07fe0000) >> 17;
	address |= ((op & 0x0000ffc0) >> 6) << 10;
	if (address & 0x80000) address = -0x80000 + (address & 0x7ffff);
	int n = (op & 0x00000020) >> 5; op &= ~0x00000020;

	uint32_t realaddress = (m_pc&0xfffffffc) + (address * 2);

	if (n)
	{
		m_delayactive = 1;
		m_delayjump = realaddress;
		m_delaylinks = 0; // don't link
	}
	else
	{
		return realaddress;
	}
	return m_pc + size;
}

uint32_t arcompact_device::handleop32_B_D_s25(uint32_t op)
{
	int size = 4;
	// Branch Unconditionally Far
	int32_t address = (op & 0x07fe0000) >> 17;
	address |= ((op & 0x0000ffc0) >> 6) << 10;
	address |= ((op & 0x0000000f) >> 0) << 20;
	if (address & 0x800000) address = -0x800000 + (address & 0x7fffff);
	int n = (op & 0x00000020) >> 5; op &= ~0x00000020;

	uint32_t realaddress = (m_pc&0xfffffffc) + (address * 2);
	if (n)
	{
		m_delayactive = 1;
		m_delayjump = realaddress;
		m_delaylinks = 0; // don't link
	}
	else
	{
		return realaddress;
	}
	return m_pc + size;
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BL<.cc><.d> s21                 0000 1sss ssss ss00   SSSS SSSS SSNQ QQQQ
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_BL_cc_d_s21(uint32_t op)
{
	int size = 4;
	// Branch and Link Conditionally
	uint8_t condition = common32_get_condition(op);

	if (!check_condition(condition))
		return m_pc + size;

	int32_t address =   (op & 0x07fc0000) >> 17;
	address |=        ((op & 0x0000ffc0) >> 6) << 10;

	if (address & 0x40000) address = -0x40000 + (address&0x3ffff);
	int n = (op & 0x00000020) >> 5; op &= ~0x00000020;

	uint32_t realaddress = (m_pc&0xfffffffc) + (address * 2);

	if (n)
	{
		m_delayactive = 1;
		m_delayjump = realaddress;
		m_delaylinks = 1;
	}
	else
	{
		m_regs[REG_BLINK] = m_pc + size;
		return realaddress;
	}
	return m_pc + size;
}

uint32_t arcompact_device::handleop32_BL_d_s25(uint32_t op)
{
	int size = 4;
	// Branch and Link Unconditionally Far
	// 00001 sssssssss 10  SSSSSSSSSS N R TTTT
	int32_t address =   (op & 0x07fc0000) >> 17;
	address |=        ((op & 0x0000ffc0) >> 6) << 10;
	address |=        ((op & 0x0000000f) >> 0) << 20;
	if (address & 0x800000) address = -0x800000 + (address&0x7fffff);
	int n = (op & 0x00000020) >> 5; op &= ~0x00000020;

	uint32_t realaddress = (m_pc&0xfffffffc) + (address * 2);

	if (n)
	{
		m_delayactive = 1;
		m_delayjump = realaddress;
		m_delaylinks = 1;
	}
	else
	{
		m_regs[REG_BLINK] = m_pc + size;
		return realaddress;
	}
	return m_pc + size;
}

// register - register cases

#define BR_REGREG_SETUP \
	/* Branch on Compare / Bit Test - Register-Register */ \
	GET_01_01_01_BRANCH_ADDR; \
	uint8_t creg = common32_get_creg(op); \
	uint8_t breg = common32_get_breg(op); \
	int n = (op & 0x00000020) >> 5; \
	int size = check_b_c_limm(breg, creg); \
	uint32_t b = m_regs[breg]; \
	uint32_t c = m_regs[creg];

#define BR_TAKEJUMP \
	/* take jump */ \
	uint32_t realaddress = (m_pc&0xfffffffc) + (address * 2); \
		\
	if (n) \
	{ \
		m_delayactive = 1; \
		m_delayjump = realaddress; \
		m_delaylinks = 0; \
	} \
	else \
	{ \
		return realaddress; \
	}

uint32_t arcompact_device::handleop32_BREQ_reg_reg(uint32_t op)  // register - register BREQ
{
	BR_REGREG_SETUP	
	if (b == c) // BREQ
	{
		BR_TAKEJUMP
	}
	return m_pc + size;
}

uint32_t arcompact_device::handleop32_BRNE_reg_reg(uint32_t op) // register - register BRNE
{
	BR_REGREG_SETUP // BRNE	
	if (b != c)
	{
		BR_TAKEJUMP
	}
	return m_pc + size;
}

uint32_t arcompact_device::handleop32_BRLT_reg_reg(uint32_t op) // regiter - register BRLT
{
	BR_REGREG_SETUP
	if ((int32_t)b < (int32_t)c) // BRLT (signed operation)
	{
		BR_TAKEJUMP
	}
	return m_pc + size;
}

uint32_t arcompact_device::handleop32_BRGE_reg_reg(uint32_t op) // register - register BRGE
{
	BR_REGREG_SETUP	
	if ((int32_t)b >= (int32_t)c) // BRGE  (signed operation)
	{
		BR_TAKEJUMP
	}
	return m_pc + size;
}

uint32_t arcompact_device::handleop32_BRLO_reg_reg(uint32_t op) // register - register BRLO
{
	BR_REGREG_SETUP
	if (b < c) 	// BRLO (unsigned operation)
	{
		BR_TAKEJUMP
	}

	return m_pc + size;
}

uint32_t arcompact_device::handleop32_BRHS_reg_reg(uint32_t op) // register - register BRHS
{
	BR_REGREG_SETUP
	if (b >= c) // BRHS (unsigned operation)
	{
		BR_TAKEJUMP
	}
	return m_pc + size;
}

uint32_t arcompact_device::handleop32_BBIT0_reg_reg(uint32_t op)
{
	BR_REGREG_SETUP
	if (!(b & (1 << (c & 0x1f)))) // Branch if bit is 0
	{
		BR_TAKEJUMP
	}
	return m_pc + size;
}

uint32_t arcompact_device::handleop32_BBIT1_reg_reg(uint32_t op)
{
	BR_REGREG_SETUP
	if (b & (1 << (c & 0x1f))) // Branch if bit is 1
	{
		BR_TAKEJUMP
	}
	return m_pc + size;
}

#define BR_REGIMM_SETUP \
	GET_01_01_01_BRANCH_ADDR \
	uint32_t u = common32_get_u6(op); \
	uint8_t breg = common32_get_breg(op); \
	int n = (op & 0x00000020) >> 5; \
	int size = check_b_limm(breg); \
	uint32_t b = m_regs[breg]; \
	uint32_t c = u;

// register -immediate cases
uint32_t arcompact_device::handleop32_BREQ_reg_imm(uint32_t op) // BREQ reg-imm
{
	BR_REGIMM_SETUP	
	if (b == c) // BREQ
	{
		BR_TAKEJUMP
	}
	return m_pc + size;
}

uint32_t arcompact_device::handleop32_BRNE_reg_imm(uint32_t op) // BRNE reg-imm
{
	BR_REGIMM_SETUP	
	if (b != c) // BRNE
	{
		BR_TAKEJUMP
	}
	return m_pc + size;
}

uint32_t arcompact_device::handleop32_BRLT_reg_imm(uint32_t op) // BRLT reg-imm
{
	BR_REGIMM_SETUP
	if ((int32_t)b < (int32_t)c) // BRLT  (signed operation)
	{
		BR_TAKEJUMP
	}
	return m_pc + size;

}
uint32_t arcompact_device::handleop32_BRGE_reg_imm(uint32_t op)
{
	BR_REGIMM_SETUP
	if ((int32_t)b >= (int32_t)c) // BRGE  (signed operation)
	{
		BR_TAKEJUMP
	}
	return m_pc + size;
}

uint32_t arcompact_device::handleop32_BRLO_reg_imm(uint32_t op) //  register - immediate BRLO
{
	BR_REGIMM_SETUP // BRLO (unsigned operation)	
	if (b < c)
	{
		BR_TAKEJUMP
	}
	return m_pc + size;

}

uint32_t arcompact_device::handleop32_BRHS_reg_imm(uint32_t op) // register - immediate BRHS
{
	BR_REGIMM_SETUP
	if (b >= c) // BRHS (unsigned operation)
	{
		BR_TAKEJUMP
	}
	return m_pc + size;
}

uint32_t arcompact_device::handleop32_BBIT0_reg_imm(uint32_t op)
{
	BR_REGIMM_SETUP
	if (!(b & (1 << (c & 0x1f)))) // Branch if bit is 0
	{
		BR_TAKEJUMP
	}
	return m_pc + size;
}

uint32_t arcompact_device::handleop32_BBIT1_reg_imm(uint32_t op)
{
	BR_REGIMM_SETUP
	if (b & (1 << (c & 0x1f))) // Branch if bit is 1
	{
		BR_TAKEJUMP
	}
	return m_pc + size;
}
