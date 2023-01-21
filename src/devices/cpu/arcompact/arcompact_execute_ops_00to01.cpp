// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "arcompact.h"
#include "arcompactdasm.h"


uint32_t arcompact_device::handleop32_B_cc_D_s21(uint32_t op)
{
	int size = 4;

	COMMON32_GET_CONDITION

	if (!check_condition(condition))
		return m_pc + (size>>0);

	// Branch Conditionally
	// 0000 0sss ssss sss0 SSSS SSSS SSNQ QQQQ
	int32_t address = (op & 0x07fe0000) >> 17;
	address |= ((op & 0x0000ffc0) >> 6) << 10;
	if (address & 0x80000) address = -0x80000 + (address & 0x7ffff);
	int n = (op & 0x00000020) >> 5; op &= ~0x00000020;

	uint32_t realaddress = PC_ALIGNED32 + (address * 2);

	if (n)
	{
		m_delayactive = 1;
		m_delayjump = realaddress;
		m_delaylinks = 0; // don't link
	}
	else
	{
	//  m_regs[REG_BLINK] = m_pc + (size >> 0);  // don't link
		return realaddress;
	}


	return m_pc + (size>>0);
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
//  int res =  (op & 0x00000010) >> 4; op &= ~0x00000010; // should be set to 0

	uint32_t realaddress = PC_ALIGNED32 + (address * 2);

	if (n)
	{
		m_delayactive = 1;
		m_delayjump = realaddress;
		m_delaylinks = 0; // don't link
	}
	else
	{
	//  m_regs[REG_BLINK] = m_pc + (size >> 0);  // don't link
		return realaddress;
	}


	return m_pc + (size>>0);

}

uint32_t arcompact_device::handleop32_BL_cc_d_s21(uint32_t op)
{
	int size = 4;

	// Branch and Link Conditionally
	arcompact_log("unimplemented BLcc %08x", op);
	return m_pc + (size>>0);
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
//  int res =  (op & 0x00000010) >> 4; op &= ~0x00000010;

	uint32_t realaddress = PC_ALIGNED32 + (address * 2);

	if (n)
	{
		m_delayactive = 1;
		m_delayjump = realaddress;
		m_delaylinks = 1;
	}
	else
	{
		m_regs[REG_BLINK] = m_pc + (size >> 0);
		return realaddress;
	}


	return m_pc + (size>>0);
}



uint32_t arcompact_device::arcompact_01_01_00_helper(uint32_t op, const char* optext)
{
	int size;

	// Branch on Compare / Bit Test - Register-Register

	uint8_t creg = common32_get_creg(op);
	uint8_t breg = common32_get_breg(op);
	//int n = (op & 0x00000020) >> 5;


	if ((breg != LIMM_REG) && (creg != LIMM_REG))
	{
	}
	else
	{
		//uint32_t limm;
		//GET_LIMM_32;
		size = 8;
	}

	arcompact_log("unimplemented %s %08x (reg-reg)", optext, op);
	return m_pc + (size>>0);
}


// register - register cases

#define BR_REGREG_SETUP \
	/* Branch on Compare / Bit Test - Register-Register */ \
	GET_01_01_01_BRANCH_ADDR; \
	uint8_t creg = common32_get_creg(op); \
	uint8_t breg = common32_get_breg(op); \
	int n = (op & 0x00000020) >> 5; \
	uint32_t b,c; \
	int size = check_b_c_limm(breg, creg); \
	b = m_regs[breg]; \
	c = m_regs[creg];
	
#define BR_TAKEJUMP \
	/* take jump */ \
	uint32_t realaddress = PC_ALIGNED32 + (address * 2); \
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

	// BREQ
	if (b == c)
	{
		BR_TAKEJUMP
	}

	return m_pc + (size>>0);
}

uint32_t arcompact_device::handleop32_BRNE_reg_reg(uint32_t op) // register - register BRNE
{
	BR_REGREG_SETUP

	// BRNE
	if (b != c)
	{
		BR_TAKEJUMP
	}

	return m_pc + (size>>0);
}

uint32_t arcompact_device::handleop32_BRLT_reg_reg(uint32_t op) // regiter - register BRLT
{
	BR_REGREG_SETUP

	// BRLT  (signed operation)
	if ((int32_t)b < (int32_t)c)
	{
		BR_TAKEJUMP
	}

	return m_pc + (size>>0);

}
uint32_t arcompact_device::handleop32_BRGE_reg_reg(uint32_t op) // register - register BRGE
{
	BR_REGREG_SETUP

	// BRGE  (signed operation)
	if ((int32_t)b >= (int32_t)c)
	{
		BR_TAKEJUMP
	}

	return m_pc + (size>>0);
}

uint32_t arcompact_device::handleop32_BRLO_reg_reg(uint32_t op) // register - register BRLO
{
	BR_REGREG_SETUP

	// BRLO (unsigned operation)
	if (b < c)
	{
		BR_TAKEJUMP
	}

	return m_pc + (size>>0);
}


uint32_t arcompact_device::handleop32_BRHS_reg_reg(uint32_t op) // register - register BRHS
{
	BR_REGREG_SETUP

	// BRHS (unsigned operation)
	if (b >= c)
	{
		BR_TAKEJUMP
	}

	return m_pc + (size>>0);
}


uint32_t arcompact_device::handleop32_BBIT0_reg_reg(uint32_t op)  { return arcompact_01_01_00_helper( op, "BBIT0");}
uint32_t arcompact_device::handleop32_BBIT1_reg_reg(uint32_t op)  { return arcompact_01_01_00_helper( op, "BBIT1");}

uint32_t arcompact_device::arcompact_01_01_01_helper(uint32_t op, const char* optext)
{
	int size = 4;
	arcompact_log("unimplemented %s %08x (reg-imm)", optext, op);
	return m_pc + (size>>0);
}

#define BR_REGIMM_SETUP \
	GET_01_01_01_BRANCH_ADDR \
	COMMON32_GET_u6; \
	uint8_t breg = common32_get_breg(op); \
	int n = (op & 0x00000020) >> 5; \
	uint32_t b,c; \
	/* comparing a LIMM  to an immediate is pointless, is it a valid encoding? */ \
	int size = check_b_limm(breg); \
	b = m_regs[breg]; \
	c = u;


// register -immediate cases
uint32_t arcompact_device::handleop32_BREQ_reg_imm(uint32_t op) // BREQ reg-imm
{
	BR_REGIMM_SETUP

	// BREQ
	if (b == c)
	{
		BR_TAKEJUMP
	}

	return m_pc + (size>>0);
}

uint32_t arcompact_device::handleop32_BRNE_reg_imm(uint32_t op) // BRNE reg-imm
{
	BR_REGIMM_SETUP

	// BRNE
	if (b != c)
	{
		BR_TAKEJUMP
	}

	return m_pc + (size>>0);
}



uint32_t arcompact_device::handleop32_BRLT_reg_imm(uint32_t op) // BRLT reg-imm
{
	BR_REGIMM_SETUP

	// BRLT  (signed operation)
	if ((int32_t)b < (int32_t)c)
	{
		BR_TAKEJUMP
	}

	return m_pc + (size>>0);

}
uint32_t arcompact_device::handleop32_BRGE_reg_imm(uint32_t op)
{
	BR_REGIMM_SETUP

	// BRGE  (signed operation)
	if ((int32_t)b >= (int32_t)c)
	{
		BR_TAKEJUMP
	}

	return m_pc + (size>>0);
}

uint32_t arcompact_device::handleop32_BRLO_reg_imm(uint32_t op) //  register - immediate BRLO
{
	BR_REGIMM_SETUP

	// BRLO (unsigned operation)
	if (b < c)
	{
		BR_TAKEJUMP
	}

	return m_pc + (size>>0);

}

uint32_t arcompact_device::handleop32_BRHS_reg_imm(uint32_t op) // register - immediate BRHS
{
	BR_REGIMM_SETUP

	// BRHS (unsigned operation)
	if (b >= c)
	{
		BR_TAKEJUMP
	}

	return m_pc + (size>>0);
}

uint32_t arcompact_device::handleop32_BBIT0_reg_imm(uint32_t op)  { return arcompact_01_01_01_helper(op, "BBIT0"); }
uint32_t arcompact_device::handleop32_BBIT1_reg_imm(uint32_t op)  { return arcompact_01_01_01_helper(op, "BBIT1"); }
