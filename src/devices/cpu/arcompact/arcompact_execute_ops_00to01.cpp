// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "arcompact_helper.ipp"

// helpers for below this group of opcodes

inline uint32_t arcompact_device::get_01_01_01_address_offset(uint32_t op)
{
	uint32_t address = (op & 0x00fe0000) >> 17;
	address |= ((op & 0x00008000) >> 15) << 7;
	address = util::sext(address, 8);
	return address;
}

inline uint32_t arcompact_device::BRxx_takejump(uint32_t address, uint8_t n, int size, bool link)
{
	uint32_t realaddress = (m_pc & 0xfffffffc) + (address * 2);
	if (n)
	{
		m_delayactive = true;
		m_delayjump = realaddress;
		m_delaylinks = link;
		return m_pc + size; // jump is delayed, so return next instruction
	}
	else
	{
		if (link)
			m_regs[REG_BLINK] = m_pc + size;
		return realaddress;
	}
}

inline bool arcompact_device::BRxx_condition(uint8_t condition, uint32_t b, uint32_t c)
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

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BREQ<.d> b,c,s9                 0000 1bbb ssss sss1   SBBB CCCC CCN0 0000
// BREQ b,limm,s9                  0000 1bbb ssss sss1   SBBB 1111 1000 0000 (+ Limm)
// BREQ limm,c,s9                  0000 1110 ssss sss1   S111 CCCC CC00 0000 (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BRNE<.d> b,c,s9                 0000 1bbb ssss sss1   SBBB CCCC CCN0 0001
// BRNE b,limm,s9                  0000 1bbb ssss sss1   SBBB 1111 1000 0001 (+ Limm)
// BRNE limm,c,s9                  0000 1110 ssss sss1   S111 CCCC CC00 0001 (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BRLT<.d> b,c,s9                 0000 1bbb ssss sss1   SBBB CCCC CCN0 0010
// BRLT b,limm,s9                  0000 1bbb ssss sss1   SBBB 1111 1000 0010 (+ Limm)
// BRLT limm,c,s9                  0000 1110 ssss sss1   S111 CCCC CC00 0010 (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BRGE<.d> b,c,s9                 0000 1bbb ssss sss1   SBBB CCCC CCN0 0011
// BRGE b,limm,s9                  0000 1bbb ssss sss1   SBBB 1111 1000 0011 (+ Limm)
// BRGE limm,c,s9                  0000 1110 ssss sss1   S111 CCCC CC00 0011 (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BRLO<.d> b,c,s9                 0000 1bbb ssss sss1   SBBB CCCC CCN0 0100
// BRLO b,limm,s9                  0000 1bbb ssss sss1   SBBB 1111 1000 0100 (+ Limm)
// BRLO limm,c,s9                  0000 1110 ssss sss1   S111 CCCC CC00 0100 (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BRHS b,limm,s9                  0000 1bbb ssss sss1   SBBB 1111 1000 0101 (+ Limm)
// BRHS limm,c,s9                  0000 1110 ssss sss1   S111 CCCC CC00 0101 (+ Limm)
// BRHS<.d> b,c,s9                 0000 1bbb ssss sss1   SBBB CCCC CCN0 0101
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BBIT0<.d> b,c,s9                0000 1bbb ssss sss1   SBBB CCCC CCN0 1110
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BBIT1<.d> b,c,s9                0000 1bbb ssss sss1   SBBB CCCC CCN0 1111
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// Branch on Compare / Bit Test - Register-Register
uint32_t arcompact_device::handleop32_BRxx_reg_reg(uint32_t op, uint8_t condition)
{
	uint8_t creg = common32_get_creg(op);
	uint8_t breg = common32_get_breg(op);
	int size = check_limm(breg, creg);
	uint32_t b = m_regs[breg];
	uint32_t c = m_regs[creg];
	if (BRxx_condition(condition, b, c))
	{
		uint32_t address = get_01_01_01_address_offset(op);
		return BRxx_takejump(address, (op & 0x00000020) >> 5, size, false);
	}
	return m_pc + size;
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BREQ<.d> b,u6,s9                0000 1bbb ssss sss1   SBBB UUUU UUN1 0000
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BRNE<.d> b,u6,s9                0000 1bbb ssss sss1   SBBB UUUU UUN1 0001
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BRLT<.d> b,u6,s9                0000 1bbb ssss sss1   SBBB UUUU UUN1 0010
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BRGE<.d> b,u6,s9                0000 1bbb ssss sss1   SBBB UUUU UUN1 0011
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BRLO<.d> b,u6,s9                0000 1bbb ssss sss1   SBBB UUUU UUN1 0100
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BRHS<.d> b,u6,s9                0000 1bbb ssss sss1   SBBB UUUU UUN1 0101
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BBIT0<.d> b,u6,s9               0000 1bbb ssss sss1   SBBB uuuu uuN1 1110
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BBIT1<.d> b,u6,s9               0000 1bbb ssss sss1   SBBB uuuu uuN1 1111
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// Branch on Compare / Bit Test - Register-Immediate
uint32_t arcompact_device::handleop32_BRxx_reg_imm(uint32_t op, uint8_t condition)
{
	uint32_t u = common32_get_u6(op);
	uint8_t breg = common32_get_breg(op);
	int size = check_limm(breg);
	uint32_t b = m_regs[breg];
	if (BRxx_condition(condition, b, u))
	{
		uint32_t address = get_01_01_01_address_offset(op);
		return BRxx_takejump(address, (op & 0x00000020) >> 5, size, false);
	}
	return m_pc + size;
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Branch Conditionally
// B<cc><.d> s21                   0000 0sss ssss sss0   SSSS SSSS SSNQ QQQQ
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_B_cc_D_s21(uint32_t op)
{
	if (!check_condition(common32_get_condition(op)))
		return m_pc + 4;
	uint32_t address = (op & 0x07fe0000) >> 17;
	address |= ((op & 0x0000ffc0) >> 6) << 10;
	address = util::sext(address, 20);
	return BRxx_takejump(address, (op & 0x00000020) >> 5, 4, false);
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Branch Unconditionally Far
// B<.d> s25                       0000 0sss ssss sss1   SSSS SSSS SSNR tttt
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_B_D_s25(uint32_t op)
{
	uint32_t address = (op & 0x07fe0000) >> 17;
	address |= ((op & 0x0000ffc0) >> 6) << 10;
	address |= (op & 0x0000000f) << 20;
	address = util::sext(address, 24);
	return BRxx_takejump(address, (op & 0x00000020) >> 5, 4, false);
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Branch and Link Conditionally
// BL<.cc><.d> s21                 0000 1sss ssss ss00   SSSS SSSS SSNQ QQQQ
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_BL_cc_d_s21(uint32_t op)
{
	if (!check_condition(common32_get_condition(op)))
		return m_pc + 4;
	uint32_t address = (op & 0x07fc0000) >> 17; // bit 0 is always 0
	address |= ((op & 0x0000ffc0) >> 6) << 10;
	address = util::sext(address, 20);
	return BRxx_takejump(address, (op & 0x00000020) >> 5, 4, true);
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Branch and Link Unconditionally Far
// BL<.d> s25                      0000 1sss ssss ss10   SSSS SSSS SSNR tttt
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_BL_d_s25(uint32_t op)
{
	uint32_t address = (op & 0x07fc0000) >> 17; // bit 0 is always 0
	address |= ((op & 0x0000ffc0) >> 6) << 10;
	address |= (op & 0x0000000f) << 20;
	address = util::sext(address, 24);
	return BRxx_takejump(address, (op & 0x00000020) >> 5, 4, true);
}
