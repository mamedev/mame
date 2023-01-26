// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "arcompact.h"

// helpers for below this group of opcodes

int32_t arcompact_device::get_01_01_01_address_offset(uint32_t op)
{
	int32_t address = (op & 0x00fe0000) >> 17;
	address |= ((op & 0x00008000) >> 15) << 7;
	if (address & 0x80) address = -0x80 + (address & 0x7f);
	return address;
}

uint32_t arcompact_device::BRxx_takejump(uint32_t address, uint8_t n, int size)
{
	uint32_t realaddress = (m_pc & 0xfffffffc) + (address * 2);
	if (n)
	{
		m_delayactive = 1;
		m_delayjump = realaddress;
		m_delaylinks = 0;
		return m_pc + size; // jump is delayed, so return next instruction
	}
	else
	{
		return realaddress;
	}
}

bool arcompact_device::BRxx_condition(uint8_t condition, uint32_t b, uint32_t c)
{
	switch (condition)
	{
	case 0x00: return (b == c) ? true : false; // BREQ
	case 0x01: return (b != c) ? true : false; // BRNE
	case 0x02: return ((int32_t)b < (int32_t)c) ? true : false; // BRLT
	case 0x03: return ((int32_t)b >= (int32_t)c) ? true : false; // BRGE
	case 0x04: return (b < c) ? true : false; // BRLO
	case 0x05: return (b >= c) ? true : false; // BRHS
	case 0x06: return (!(b & (1 << (c & 0x1f)))) ? true : false; // BBIT0
	case 0x07: return (b & (1 << (c & 0x1f))) ? true : false; // BBIT1
	}
	return false;
}

// Branch on Compare / Bit Test - Register-Register
uint32_t arcompact_device::handleop32_BRxx_reg_reg(uint32_t op, uint8_t condition)
{
	int32_t address = get_01_01_01_address_offset(op);
	uint8_t creg = common32_get_creg(op);
	uint8_t breg = common32_get_breg(op);
	int n = (op & 0x00000020) >> 5;
	int size = check_b_c_limm(breg, creg);
	uint32_t b = m_regs[breg];
	uint32_t c = m_regs[creg];
	if (BRxx_condition(condition, b, c))
	{
		return BRxx_takejump(address, n, size);
	}
	return m_pc + size;
}

// Branch on Compare / Bit Test - Register-Immediate
uint32_t arcompact_device::handleop32_BRxx_reg_imm(uint32_t op, uint8_t condition)
{
	int32_t address = get_01_01_01_address_offset(op);
	uint32_t u = common32_get_u6(op);
	uint8_t breg = common32_get_breg(op);
	int n = (op & 0x00000020) >> 5;
	int size = check_b_limm(breg);
	uint32_t b = m_regs[breg];
	uint32_t c = u;
	if (BRxx_condition(condition, b, c))
	{
		return BRxx_takejump(address, n, size);
	}
	return m_pc + size;
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// B<cc><.d> s21                   0000 0sss ssss sss0   SSSS SSSS SSNQ QQQQ
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_B_cc_D_s21(uint32_t op)
{
	int size = 4;
	uint8_t condition = common32_get_condition(op);

	if (!check_condition(condition))
		return m_pc + size;

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

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Branch Unconditionally Far
// B<.d> s25                       0000 0sss ssss sss1   SSSS SSSS SSNR tttt
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_B_D_s25(uint32_t op)
{
	int size = 4;
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
// Branch and Link Conditionally
// BL<.cc><.d> s21                 0000 1sss ssss ss00   SSSS SSSS SSNQ QQQQ
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_BL_cc_d_s21(uint32_t op)
{
	int size = 4;
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

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Branch and Link Unconditionally Far
// BL<.d> s25                      0000 1sss ssss ss10   SSSS SSSS SSNR tttt
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_BL_d_s25(uint32_t op)
{
	int size = 4;
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

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BREQ<.d> b,c,s9                 0000 1bbb ssss sss1   SBBB CCCC CCN0 0000
// BREQ b,limm,s9                  0000 1bbb ssss sss1   SBBB 1111 1000 0000 (+ Limm)
// BREQ limm,c,s9                  0000 1110 ssss sss1   S111 CCCC CC00 0000 (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_BREQ_reg_reg(uint32_t op)  // register - register BREQ
{
	return handleop32_BRxx_reg_reg(op, 0);
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BRNE<.d> b,c,s9                 0000 1bbb ssss sss1   SBBB CCCC CCN0 0001
// BRNE b,limm,s9                  0000 1bbb ssss sss1   SBBB 1111 1000 0001 (+ Limm)
// BRNE limm,c,s9                  0000 1110 ssss sss1   S111 CCCC CC00 0001 (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_BRNE_reg_reg(uint32_t op) // register - register BRNE
{
	return handleop32_BRxx_reg_reg(op, 1);
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BRLT<.d> b,c,s9                 0000 1bbb ssss sss1   SBBB CCCC CCN0 0010
// BRLT b,limm,s9                  0000 1bbb ssss sss1   SBBB 1111 1000 0010 (+ Limm)
// BRLT limm,c,s9                  0000 1110 ssss sss1   S111 CCCC CC00 0010 (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_BRLT_reg_reg(uint32_t op) // regiter - register BRLT
{
	return handleop32_BRxx_reg_reg(op, 2);
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BRGE<.d> b,c,s9                 0000 1bbb ssss sss1   SBBB CCCC CCN0 0011
// BRGE b,limm,s9                  0000 1bbb ssss sss1   SBBB 1111 1000 0011 (+ Limm)
// BRGE limm,c,s9                  0000 1110 ssss sss1   S111 CCCC CC00 0011 (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_BRGE_reg_reg(uint32_t op) // register - register BRGE
{
	return handleop32_BRxx_reg_reg(op, 3);
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BRLO<.d> b,c,s9                 0000 1bbb ssss sss1   SBBB CCCC CCN0 0100
// BRLO b,limm,s9                  0000 1bbb ssss sss1   SBBB 1111 1000 0100 (+ Limm)
// BRLO limm,c,s9                  0000 1110 ssss sss1   S111 CCCC CC00 0100 (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_BRLO_reg_reg(uint32_t op) // register - register BRLO
{
	return handleop32_BRxx_reg_reg(op, 4);
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BRHS b,limm,s9                  0000 1bbb ssss sss1   SBBB 1111 1000 0101 (+ Limm)
// BRHS limm,c,s9                  0000 1110 ssss sss1   S111 CCCC CC00 0101 (+ Limm)
// BRHS<.d> b,c,s9                 0000 1bbb ssss sss1   SBBB CCCC CCN0 0101
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_BRHS_reg_reg(uint32_t op) // register - register BRHS
{
	return handleop32_BRxx_reg_reg(op, 5);
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BBIT0<.d> b,c,s9                0000 1bbb ssss sss1   SBBB CCCC CCN0 1110
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_BBIT0_reg_reg(uint32_t op)
{
	return handleop32_BRxx_reg_reg(op, 6);
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BBIT1<.d> b,c,s9                0000 1bbb ssss sss1   SBBB CCCC CCN0 1111
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_BBIT1_reg_reg(uint32_t op)
{
	return handleop32_BRxx_reg_reg(op, 7);
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BREQ<.d> b,u6,s9                0000 1bbb ssss sss1   SBBB UUUU UUN1 0000
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_BREQ_reg_imm(uint32_t op) // BREQ reg-imm
{
	return handleop32_BRxx_reg_imm(op, 0);
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BRNE<.d> b,u6,s9                0000 1bbb ssss sss1   SBBB UUUU UUN1 0001
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_BRNE_reg_imm(uint32_t op) // BRNE reg-imm
{
	return handleop32_BRxx_reg_imm(op, 1);
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BRLT<.d> b,u6,s9                0000 1bbb ssss sss1   SBBB UUUU UUN1 0010
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_BRLT_reg_imm(uint32_t op) // BRLT reg-imm
{
	return handleop32_BRxx_reg_imm(op, 2);
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BRGE<.d> b,u6,s9                0000 1bbb ssss sss1   SBBB UUUU UUN1 0011
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_BRGE_reg_imm(uint32_t op)
{
	return handleop32_BRxx_reg_imm(op, 3);
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BRLO<.d> b,u6,s9                0000 1bbb ssss sss1   SBBB UUUU UUN1 0100
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_BRLO_reg_imm(uint32_t op) //  register - immediate BRLO
{
	return handleop32_BRxx_reg_imm(op, 4);
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BRHS<.d> b,u6,s9                0000 1bbb ssss sss1   SBBB UUUU UUN1 0101
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_BRHS_reg_imm(uint32_t op) // register - immediate BRHS
{
	return handleop32_BRxx_reg_imm(op, 5);
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BBIT0<.d> b,u6,s9               0000 1bbb ssss sss1   SBBB uuuu uuN1 1110
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_BBIT0_reg_imm(uint32_t op)
{
	return handleop32_BRxx_reg_imm(op, 6);
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BBIT1<.d> b,u6,s9               0000 1bbb ssss sss1   SBBB uuuu uuN1 1111
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_BBIT1_reg_imm(uint32_t op)
{
	return handleop32_BRxx_reg_imm(op, 7);
}
