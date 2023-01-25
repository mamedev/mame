// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "arcompact.h"
#include "arcompactdasm.h"

uint32_t arcompact_device::arcompact_handle04_2f_helper(uint32_t op, const char* optext)
{
	int size;

	int p = common32_get_p(op);
	//uint8_t breg = common32_get_breg(op);

	if (p == 0)
	{
		uint8_t creg = common32_get_creg(op);
		size = check_c_limm(creg);
	}
	else if (p == 1)
	{
	}
	else if (p == 2)
	{
	}
	else if (p == 3)
	{
	}

	arcompact_log("unimplemented %s %08x (type 04_2f)", optext, op);
	return m_pc + size;
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS               ss ssss
// ASL<.f> b,c                     0010 0bbb 0010 1111   FBBB CCCC CC00 0000
// ASL<.f> b,u6                    0010 0bbb 0110 1111   FBBB uuuu uu00 0000
// ASL<.f> b,limm                  0010 0bbb 0010 1111   FBBB 1111 1000 0000 (+ Limm)
//
// ASL<.f> 0,c                     0010 0110 0010 1111   F111 CCCC CC00 0000
// ASL<.f> 0,u6                    0010 0110 0110 1111   F111 uuuu uu00 0000
// ASL<.f> 0,limm                  0010 0110 0010 1111   F111 1111 1000 0000 (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_ASL_single_do_op(uint32_t src, uint8_t set_flags)
{
	uint32_t result = src + src;
	if (set_flags)
	{
		do_flags_nz(result);
		if ((src & 0x80000000) != (result & 0x80000000)) { status32_set_v(); } else { status32_clear_v(); }
		if (src & 0x80000000) { status32_set_c(); } else { status32_clear_c(); }
	}
	return result;
}

uint32_t arcompact_device::handleop32_ASL_single_f_b_c(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	uint8_t creg = common32_get_creg(op);
	int size = check_b_c_limm(breg, creg);
	m_regs[breg] = handleop32_ASL_single_do_op(m_regs[creg], common32_get_F(op));
	return m_pc + size;
}

uint32_t arcompact_device::handleop32_ASL_single_f_b_u6(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	int size = check_b_limm(breg);
	m_regs[breg] = handleop32_ASL_single_do_op(common32_get_u6(op), common32_get_F(op));
	return m_pc + size;
}

uint32_t arcompact_device::handleop32_ASL_single(uint32_t op)
{
	switch ((op & 0x00c00000) >> 22)
	{
		case 0x00: return handleop32_ASL_single_f_b_c(op);
		case 0x01: return handleop32_ASL_single_f_b_u6(op);
		case 0x02:
		case 0x03:
			arcompact_fatal("illegal handleop32_ASL_single_f_b_b_s12 (ares bits already used as opcode select, can't be used as s12) (ASL1)\n");
			return 0;
	}
	return 0;
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS               ss ssss
// ASR<.f> b,c                     0010 0bbb 0010 1111   FBBB CCCC CC00 0001
// ASR<.f> b,u6                    0010 0bbb 0110 1111   FBBB uuuu uu00 0001
// ASR<.f> b,limm                  0010 0bbb 0010 1111   FBBB 1111 1000 0001 (+ Limm)
//
// ASR<.f> 0,c                     0010 0110 0010 1111   F111 CCCC CC00 0001
// ASR<.f> 0,u6                    0010 0110 0110 1111   F111 uuuu uu00 0001
// ASR<.f> 0,limm                  0010 0110 0010 1111   F111 1111 1000 0001 (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_ASR_single(uint32_t op)  { return arcompact_handle04_2f_helper(op, "ASR"); } // ASR

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS               ss ssss
// LSR<.f> b,c                     0010 0bbb 0010 1111   FBBB CCCC CC00 0010
// LSR<.f> b,u6                    0010 0bbb 0110 1111   FBBB uuuu uu00 0010
// LSR<.f> b,limm                  0010 0bbb 0010 1111   FBBB 1111 1000 0010 (+ Limm)
//
// LSR<.f> 0,c                     0010 0110 0010 1111   F111 CCCC CC00 0010
// LSR<.f> 0,u6                    0010 0110 0110 1111   F111 uuuu uu00 0010
// LSR<.f> 0,limm                  0010 0110 0010 1111   F111 1111 1000 0010 (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_LSR_single_do_op(uint32_t src, uint8_t set_flags)
{
	uint32_t result = src >> 1;
	if (set_flags)
	{
		if (result & 0x80000000) { status32_set_n(); } else { status32_clear_n(); }
		if (result == 0x00000000) { status32_set_z(); } else { status32_clear_z(); }
		if (src & 0x00000001) { status32_set_c(); } else { status32_clear_c(); }
	}
	return result;
}

uint32_t arcompact_device::handleop32_LSR_single_f_b_c(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	uint8_t creg = common32_get_creg(op);
	int size = check_b_c_limm(breg, creg);
	m_regs[breg] = handleop32_LSR_single_do_op(m_regs[creg], common32_get_F(op));
	return m_pc + size;
}

uint32_t arcompact_device::handleop32_LSR_single_f_b_u6(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	int size = check_b_limm(breg);
	m_regs[breg] = handleop32_LSR_single_do_op(common32_get_u6(op), common32_get_F(op));
	return m_pc + size;
}

uint32_t arcompact_device::handleop32_LSR_single(uint32_t op)
{
	switch ((op & 0x00c00000) >> 22)
	{
		case 0x00: return handleop32_LSR_single_f_b_c(op);
		case 0x01: return handleop32_LSR_single_f_b_u6(op);
		case 0x02:
		case 0x03:
			arcompact_fatal("illegal handleop32_LSR_single_f_b_b_s12 (ares bits already used as opcode select, can't be used as s12) (LSR1)\n");
			return 0;
	}
	return 0;
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS               ss ssss
// ROR<.f> b,c                     0010 0bbb 0010 1111   FBBB CCCC CC00 0011
// ROR<.f> b,u6                    0010 0bbb 0110 1111   FBBB uuuu uu00 0011
// ROR<.f> b,limm                  0010 0bbb 0010 1111   FBBB 1111 1000 0011 (+ Limm)
//
// ROR<.f> 0,c                     0010 0110 0010 1111   F111 CCCC CC00 0011
// ROR<.f> 0,u6                    0010 0110 0110 1111   F111 uuuu uu00 0011
// ROR<.f> 0,limm                  0010 0110 0010 1111   F111 1111 1000 0011 (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_ROR_do_op(uint32_t src, uint8_t set_flags)
{
	uint32_t result = src >> 1;
	if (src & 1)
		result |= 0x80000000;

	if (set_flags)
	{
		do_flags_nz(result);
		if (src & 0x00000001) { status32_set_c(); } else { status32_clear_c(); }
	}

	return result;
}

uint32_t arcompact_device::handleop32_ROR_f_b_c(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	uint8_t creg = common32_get_creg(op);
	int size = check_b_c_limm(breg, creg);
	m_regs[breg] = handleop32_ROR_do_op(m_regs[creg], common32_get_F(op));
	return m_pc + size;
}

uint32_t arcompact_device::handleop32_ROR_f_b_u6(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	int size = check_b_limm(breg);
	m_regs[breg] = handleop32_ROR_do_op(common32_get_u6(op), common32_get_F(op));
	return m_pc + size;
}

uint32_t arcompact_device::handleop32_ROR(uint32_t op)
{
	switch ((op & 0x00c00000) >> 22)
	{
		case 0x00: return handleop32_ROR_f_b_c(op);
		case 0x01: return handleop32_ROR_f_b_u6(op);
		case 0x02:
		case 0x03:
			arcompact_fatal("illegal handleop32_ROR_f_b_b_s12 (ares bits already used as opcode select, can't be used as s12) (LSR1)\n");
			return 0;
	}
	return 0;
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS               ss ssss
// RRC<.f> b,c                     0010 0bbb 0010 1111   FBBB CCCC CC00 0100
// RRC<.f> b,u6                    0010 0bbb 0110 1111   FBBB uuuu uu00 0100
// RRC<.f> b,limm                  0010 0bbb 0010 1111   FBBB 1111 1000 0100 (+ Limm)
//
// RRC<.f> 0,c                     0010 0110 0010 1111   F111 CCCC CC00 0100
// RRC<.f> 0,u6                    0010 0110 0110 1111   F111 uuuu uu00 0100
// RRC<.f> 0,limm                  0010 0110 0010 1111   F111 1111 1000 0100 (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_RRC(uint32_t op)  { return arcompact_handle04_2f_helper(op, "RCC"); } // RCC

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS               ss ssss
// SEXB<.f> b,c                    0010 0bbb 0010 1111   FBBB CCCC CC00 0101
// SEXB<.f> b,u6                   0010 0bbb 0110 1111   FBBB uuuu uu00 0101
// SEXB<.f> b,limm                 0010 0bbb 0010 1111   FBBB 1111 1000 0101 (+ Limm)
//
// SEXB<.f> 0,c                    0010 0110 0010 1111   F111 CCCC CC00 0101
// SEXB<.f> 0,u6                   0010 0110 0110 1111   F111 uuuu uu00 0101
// SEXB<.f> 0,limm                 0010 0110 0010 1111   F111 1111 1000 0101 (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_SEXB(uint32_t op)  { return arcompact_handle04_2f_helper(op, "SEXB"); } // SEXB

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS               ss ssss
// SEXW<.f> b,c                    0010 0bbb 0010 1111   FBBB CCCC CC00 0110
// SEXW<.f> b,u6                   0010 0bbb 0110 1111   FBBB uuuu uu00 0110
// SEXW<.f> b,limm                 0010 0bbb 0010 1111   FBBB 1111 1000 0110 (+ Limm)
//
// SEXW<.f> 0,c                    0010 0110 0010 1111   F111 CCCC CC00 0110
// SEXW<.f> 0,u6                   0010 0110 0110 1111   F111 uuuu uu00 0110
// SEXW<.f> 0,limm                 0010 0110 0010 1111   F111 1111 1000 0110 (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_SEXW(uint32_t op)  { return arcompact_handle04_2f_helper(op, "SEXW"); } // SEXW

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS               ss ssss
// EXTB<.f> b,c                    0010 0bbb 0010 1111   FBBB CCCC CC00 0111
// EXTB<.f> b,u6                   0010 0bbb 0110 1111   FBBB uuuu uu00 0111
// EXTB<.f> b,limm                 0010 0bbb 0010 1111   FBBB 1111 1000 0111 (+ Limm)
//
// EXTB<.f> 0,c                    0010 0110 0010 1111   F111 CCCC CC00 0111
// EXTB<.f> 0,u6                   0010 0110 0110 1111   F111 uuuu uu00 0111
// EXTB<.f> 0,limm                 0010 0110 0010 1111   F111 1111 1000 0111 (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_EXTB_do_op(uint32_t src, uint8_t set_flags)
{
	uint32_t result = src & 0xff;
	if (set_flags)
	{
		do_flags_nz(result);
	}
	return result;
}

uint32_t arcompact_device::handleop32_EXTB_f_b_c(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	uint8_t creg = common32_get_creg(op);
	int size = check_b_c_limm(breg, creg);
	m_regs[breg] = handleop32_EXTB_do_op(m_regs[creg], common32_get_F(op));
	return m_pc + size;
}

uint32_t arcompact_device::handleop32_EXTB_f_b_u6(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	int size = check_b_limm(breg);
	m_regs[breg] = handleop32_EXTB_do_op(common32_get_u6(op), common32_get_F(op));
	return m_pc + size;
}

uint32_t arcompact_device::handleop32_EXTB(uint32_t op)
{
	switch ((op & 0x00c00000) >> 22)
	{
		case 0x00: return handleop32_EXTB_f_b_c(op);
		case 0x01: return handleop32_EXTB_f_b_u6(op);
		case 0x02:
		case 0x03:
			arcompact_fatal("illegal handleop32_EXTB_f_b_b_s12 (ares bits already used as opcode select, can't be used as s12) (LSR1)\n");
			return 0;
	}
	return 0;
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Zero Extend Word 
//                                 IIII I      SS SSSS               ss ssss
// EXTW<.f> b,c                    0010 0bbb 0010 1111   FBBB CCCC CC00 1000
// EXTW<.f> b,u6                   0010 0bbb 0110 1111   FBBB uuuu uu00 1000
// EXTW<.f> b,limm                 0010 0bbb 0010 1111   FBBB 1111 1000 1000 (+ Limm)
//
// EXTW<.f> 0,c                    0010 0110 0010 1111   F111 CCCC CC00 1000
// EXTW<.f> 0,u6                   0010 0110 0110 1111   F111 uuuu uu00 1000
// EXTW<.f> 0,limm                 0010 0110 0010 1111   F111 1111 1000 1000 (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_EXTW_do_op(uint32_t src, uint8_t set_flags)
{
	uint32_t result = src & 0xffff;
	if (set_flags)
	{
		do_flags_nz(result);
	}
	return result;
}

uint32_t arcompact_device::handleop32_EXTW_f_b_c(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	uint8_t creg = common32_get_creg(op);
	int size = check_b_c_limm(breg, creg);
	m_regs[breg] = handleop32_EXTW_do_op(m_regs[creg], common32_get_F(op));
	return m_pc + size;
}

uint32_t arcompact_device::handleop32_EXTW_f_b_u6(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	int size = check_b_limm(breg);
	m_regs[breg] = handleop32_EXTW_do_op(common32_get_u6(op), common32_get_F(op));
	return m_pc + size;
}

uint32_t arcompact_device::handleop32_EXTW(uint32_t op)
{
	switch ((op & 0x00c00000) >> 22)
	{
		case 0x00: return handleop32_EXTW_f_b_c(op);
		case 0x01: return handleop32_EXTW_f_b_u6(op);
		case 0x02:
		case 0x03:
			arcompact_fatal("illegal handleop32_EXTW_f_b_b_s12 (ares bits already used as opcode select, can't be used as s12) (LSR1)\n");
			return 0;
	}
	return 0;
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS               ss ssss
// ABS<.f> b,c                     0010 0bbb 0010 1111   FBBB CCCC CC00 1001
// ABS<.f> b,u6                    0010 0bbb 0110 1111   FBBB uuuu uu00 1001
// ABS<.f> b,limm                  0010 0bbb 0010 1111   FBBB 1111 1000 1001 (+ Limm)
//
// ABS<.f> 0,c                     0010 0110 0010 1111   F111 CCCC CC00 1001
// ABS<.f> 0,u6                    0010 0110 0110 1111   F111 uuuu uu00 1001
// ABS<.f> 0,limm                  0010 0110 0010 1111   F111 1111 1000 1001 (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_ABS(uint32_t op)  { return arcompact_handle04_2f_helper(op, "ABS"); } // ABS

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS               ss ssss
// NOT<.f> b,c                     0010 0bbb 0010 1111   FBBB CCCC CC00 1010
// NOT<.f> b,u6                    0010 0bbb 0110 1111   FBBB uuuu uu00 1010
// NOT<.f> b,limm                  0010 0bbb 0010 1111   FBBB 1111 1000 1010 (+ Limm)
//
// NOT<.f> 0,c                     0010 0110 0010 1111   F111 CCCC CC00 1010
// NOT<.f> 0,u6                    0010 0110 0110 1111   F111 uuuu uu00 1010
// NOT<.f> 0,limm                  0010 0110 0010 1111   F111 1111 1000 1010 (+ Limm)
//
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_NOT(uint32_t op)  { return arcompact_handle04_2f_helper(op, "NOT"); } // NOT

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Rotate Left Through Carry
//                                 IIII I      SS SSSS               ss ssss
// RLC<.f> b,c                     0010 0bbb 0010 1111   FBBB CCCC CC00 1011
// RLC<.f> b,u6                    0010 0bbb 0110 1111   FBBB uuuu uu00 1011
// RLC<.f> b,limm                  0010 0bbb 0010 1111   FBBB 1111 1000 1011 (+ Limm)
//
// RLC<.f> 0,c                     0010 0110 0010 1111   F111 CCCC CC00 1011
// RLC<.f> 0,u6                    0010 0110 0110 1111   F111 uuuu uu00 1011
// RLC<.f> 0,limm                  0010 0110 0010 1111   F111 1111 1000 1011 (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_RLC_do_op(uint32_t src, uint8_t set_flags)
{
	uint32_t result = src << 1;
	if (status32_check_c())
		result |= 1;

	if (set_flags)
	{
		do_flags_nz(result);
		if (src & 0x80000000)
			status32_set_c();
		else
			status32_clear_c();
	}
	return result;
}

uint32_t arcompact_device::handleop32_RLC_f_b_c(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	uint8_t creg = common32_get_creg(op);
	int size = check_b_c_limm(breg, creg);
	m_regs[breg] = handleop32_RLC_do_op(m_regs[creg], common32_get_F(op));
	return m_pc + size;
}

uint32_t arcompact_device::handleop32_RLC_f_b_u6(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	int size = check_b_limm(breg);
	m_regs[breg] = handleop32_RLC_do_op(common32_get_u6(op), common32_get_F(op));
	return m_pc + size;
}

uint32_t arcompact_device::handleop32_RLC(uint32_t op)
{
	switch ((op & 0x00c00000) >> 22)
	{
		case 0x00: return handleop32_RLC_f_b_c(op);
		case 0x01: return handleop32_RLC_f_b_u6(op);
		case 0x02:
		case 0x03:
			arcompact_fatal("illegal handleop32_RLC_f_b_b_s12 (ares bits already used as opcode select, can't be used as s12) (LSR1)\n");
			return 0;
	}
	return 0;
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Atomic Exchange 
//                                 IIII I      SS SSSS               ss ssss
// EX<.di> b,[c]                   0010 0bbb 0010 1111   DBBB CCCC CC00 1100
// EX<.di> b,[u6]                  0010 0bbb 0110 1111   DBBB uuuu uu00 1100
// EX<.di> b,[limm]                0010 0bbb 0010 1111   DBBB 1111 1000 1100 (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_EX(uint32_t op)  { return arcompact_handle04_2f_helper(op, "EX"); } // EX
