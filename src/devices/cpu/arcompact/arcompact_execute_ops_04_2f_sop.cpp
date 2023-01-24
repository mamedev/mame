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
	return m_pc + (size>>0);
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

uint32_t arcompact_device::handleop32_ASL_single(uint32_t op)
{
	return arcompact_handle04_2f_helper(op, "ASL");
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

uint32_t arcompact_device::handleop32_LSR_single_f_b_c(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	uint8_t creg = common32_get_creg(op);

	uint32_t c;

	int size = check_c_limm(creg);

	c = m_regs[creg];
	/* todo: is the limm, limm syntax valid? (it's pointless.) */

	uint32_t result = c >> 1;
	m_regs[breg] = result;

	if (F)
	{
		if (result & 0x80000000) { status32_set_n(); } else { status32_clear_n(); }
		if (result == 0x00000000) { status32_set_z(); } else { status32_clear_z(); }
		if (c & 0x00000001) { status32_set_c(); } else { status32_clear_c(); }
	}

	return m_pc + size;
}


uint32_t arcompact_device::handleop32_LSR_single_f_b_u6(uint32_t op)
{
	int size = 4;

	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	uint32_t u = common32_get_u6(op);
	uint32_t c;

	c = u;

	uint32_t result = c >> 1;
	m_regs[breg] = result;

	if (F)
	{
		if (result & 0x80000000) { status32_set_n(); } else { status32_clear_n(); }
		if (result == 0x00000000) { status32_set_z(); } else { status32_clear_z(); }
		if (c & 0x00000001) { status32_set_c(); } else { status32_clear_c(); }
	}
	return m_pc + size;
}

uint32_t arcompact_device::handleop32_LSR_single(uint32_t op)
{
	int p = (op & 0x00c00000) >> 22;

	switch (p)
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



uint32_t arcompact_device::handleop32_ROR_single_f_a_b_c(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	uint8_t creg = common32_get_creg(op);
	 //uint8_t areg = common32_get_areg(op); // areg bits already used as opcode select

	uint32_t c;

	int size = check_c_limm(creg);

	c = m_regs[creg];
	/* todo: is the limm, limm syntax valid? (it's pointless.) */

	int shift = 1; uint32_t mask = (1 << (shift)) - 1; mask <<= (32-shift); uint32_t result = ((c >> shift) & ~mask) | ((c << (32-shift)) & mask);
	m_regs[breg] = result;

	if (F)
	{
		if (result & 0x80000000) { status32_set_n(); }
		else { status32_clear_n(); }
		if (result == 0x00000000) { status32_set_z(); }
		else { status32_clear_z(); }
		if (c == 0x00000001) { status32_set_c(); }
		else { status32_clear_c(); }
	}
	return m_pc + size;
}


uint32_t arcompact_device::handleop32_ROR_single_f_a_b_u6(uint32_t op)
{
	int size = 4;

	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	uint32_t u = common32_get_u6(op);
	 //uint8_t areg = common32_get_areg(op); // areg bits already used as opcode select

	uint32_t c;

	c = u;


	int shift = 1; uint32_t mask = (1 << (shift)) - 1; mask <<= (32-shift); uint32_t result = ((c >> shift) & ~mask) | ((c << (32-shift)) & mask);
	m_regs[breg] = result;

	if (F)
	{
		if (result & 0x80000000) { status32_set_n(); }
		else { status32_clear_n(); }
		if (result == 0x00000000) { status32_set_z(); }
		else { status32_clear_z(); }
		if (c == 0x00000001) { status32_set_c(); }
		else { status32_clear_c(); }
	}
	return m_pc + size;
}


uint32_t arcompact_device::handleop32_ROR_single_f_b_b_s12(uint32_t op)
{
	int size = 4;
	arcompact_fatal("illegal handleop32_ROR_single_f_b_b_s12 (ares bits already used as opcode select, can't be used as s12) (ROR)\n");
	return m_pc + size;
}


uint32_t arcompact_device::handleop32_ROR_single_cc_f_b_b_c(uint32_t op)
{
	int size = 4;
	arcompact_fatal("illegal handleop32_ROR_single_cc_f_b_b_c (ares bits already used as opcode select, can't be used as Q condition) (ROR)\n");
	return m_pc + size;
}
uint32_t arcompact_device::handleop32_ROR_single_cc_f_b_b_u6(uint32_t op)
{
	int size = 4;
	arcompact_fatal("illegal handleop32_ROR_single_cc_f_b_b_u6 (ares bits already used as opcode select, can't be used as Q condition) (ROR)\n");
	return m_pc + size;
}

uint32_t arcompact_device::handleop32_ROR_single_cc(uint32_t op)
{
	int M = (op & 0x00000020) >> 5;

	switch (M)
	{
		case 0x00: return handleop32_ROR_single_cc_f_b_b_c(op);
		case 0x01: return handleop32_ROR_single_cc_f_b_b_u6(op);
	}

	return 0;
}

uint32_t arcompact_device::handleop32_ROR_single(uint32_t op)
{
	int p = (op & 0x00c00000) >> 22;

	switch (p)
	{
		case 0x00: return handleop32_ROR_single_f_a_b_c(op);
		case 0x01: return handleop32_ROR_single_f_a_b_u6(op);
		case 0x02: return handleop32_ROR_single_f_b_b_s12(op);
		case 0x03: return handleop32_ROR_single_cc(op);
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


uint32_t arcompact_device::handleop32_EXTB_f_a_b_c(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	uint8_t creg = common32_get_creg(op);
	 //uint8_t areg = common32_get_areg(op); // areg bits already used as opcode select

	uint32_t c;

	int size = check_c_limm(creg);

	c = m_regs[creg];
	/* todo: is the limm, limm syntax valid? (it's pointless.) */

	uint32_t result = c & 0x000000ff;
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_EXTB (EXTB) (F set)\n"); // not yet supported
	}
	return m_pc + size;
}


uint32_t arcompact_device::handleop32_EXTB_f_a_b_u6(uint32_t op)
{
	int size = 4;

	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	uint32_t u = common32_get_u6(op);
	 //uint8_t areg = common32_get_areg(op); // areg bits already used as opcode select

	uint32_t c;

	c = u;


	uint32_t result = c & 0x000000ff;
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_EXTB (EXTB) (F set)\n"); // not yet supported
	}
	return m_pc + size;
}


uint32_t arcompact_device::handleop32_EXTB_f_b_b_s12(uint32_t op)
{
	int size = 4;
	arcompact_fatal("illegal handleop32_EXTB_f_b_b_s12 (ares bits already used as opcode select, can't be used as s12) (EXTB)\n");
	return m_pc + size;
}


uint32_t arcompact_device::handleop32_EXTB_cc_f_b_b_c(uint32_t op)
{
	int size = 4;
	arcompact_fatal("illegal handleop32_EXTB_cc_f_b_b_c (ares bits already used as opcode select, can't be used as Q condition) (EXTB)\n");
	return m_pc + size;
}
uint32_t arcompact_device::handleop32_EXTB_cc_f_b_b_u6(uint32_t op)
{
	int size = 4;
	arcompact_fatal("illegal handleop32_EXTB_cc_f_b_b_u6 (ares bits already used as opcode select, can't be used as Q condition) (EXTB)\n");
	return m_pc + size;
}




uint32_t arcompact_device::handleop32_EXTB_cc(uint32_t op)
{
	int M = (op & 0x00000020) >> 5;

	switch (M)
	{
		case 0x00: return handleop32_EXTB_cc_f_b_b_c(op);
		case 0x01: return handleop32_EXTB_cc_f_b_b_u6(op);
	}

	return 0;
}

uint32_t arcompact_device::handleop32_EXTB(uint32_t op)
{
	int p = (op & 0x00c00000) >> 22;

	switch (p)
	{
		case 0x00: return handleop32_EXTB_f_a_b_c(op);
		case 0x01: return handleop32_EXTB_f_a_b_u6(op);
		case 0x02: return handleop32_EXTB_f_b_b_s12(op);
		case 0x03: return handleop32_EXTB_cc(op);
	}

	return 0;
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS               ss ssss
// EXTW<.f> b,c                    0010 0bbb 0010 1111   FBBB CCCC CC00 1000
// EXTW<.f> b,u6                   0010 0bbb 0110 1111   FBBB uuuu uu00 1000
// EXTW<.f> b,limm                 0010 0bbb 0010 1111   FBBB 1111 1000 1000 (+ Limm)
//
// EXTW<.f> 0,c                    0010 0110 0010 1111   F111 CCCC CC00 1000
// EXTW<.f> 0,u6                   0010 0110 0110 1111   F111 uuuu uu00 1000
// EXTW<.f> 0,limm                 0010 0110 0010 1111   F111 1111 1000 1000 (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_EXTW_f_a_b_c(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	uint8_t creg = common32_get_creg(op);
	 //uint8_t areg = common32_get_areg(op); // areg bits already used as opcode select

	uint32_t c;

	int size = check_c_limm(creg);

	c = m_regs[creg];
	/* todo: is the limm, limm syntax valid? (it's pointless.) */

	uint32_t result = c & 0x0000ffff;
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_EXTW (EXTW) (F set)\n"); // not yet supported
	}
	return m_pc + size;
}


uint32_t arcompact_device::handleop32_EXTW_f_a_b_u6(uint32_t op)
{
	int size = 4;

	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	uint32_t u = common32_get_u6(op);
	 //uint8_t areg = common32_get_areg(op); // areg bits already used as opcode select

	uint32_t c;

	c = u;


	uint32_t result = c & 0x0000ffff;
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_EXTW (EXTW) (F set)\n"); // not yet supported
	}
	return m_pc + size;
}


uint32_t arcompact_device::handleop32_EXTW_f_b_b_s12(uint32_t op)
{
	int size = 4;
	arcompact_fatal("illegal handleop32_EXTW_f_b_b_s12 (ares bits already used as opcode select, can't be used as s12) (EXTW)\n");
	return m_pc + size;
}


uint32_t arcompact_device::handleop32_EXTW_cc_f_b_b_c(uint32_t op)
{
	int size = 4;
	arcompact_fatal("illegal handleop32_EXTW_cc_f_b_b_c (ares bits already used as opcode select, can't be used as Q condition) (EXTW)\n");
	return m_pc + size;
}
uint32_t arcompact_device::handleop32_EXTW_cc_f_b_b_u6(uint32_t op)
{
	int size = 4;
	arcompact_fatal("illegal handleop32_EXTW_cc_f_b_b_u6 (ares bits already used as opcode select, can't be used as Q condition) (EXTW)\n");
	return m_pc + size;
}


uint32_t arcompact_device::handleop32_EXTW_cc(uint32_t op)
{
	int M = (op & 0x00000020) >> 5;

	switch (M)
	{
		case 0x00: return handleop32_EXTW_cc_f_b_b_c(op);
		case 0x01: return handleop32_EXTW_cc_f_b_b_u6(op);
	}

	return 0;
}

uint32_t arcompact_device::handleop32_EXTW(uint32_t op)
{
	int p = (op & 0x00c00000) >> 22;

	switch (p)
	{
		case 0x00: return handleop32_EXTW_f_a_b_c(op);
		case 0x01: return handleop32_EXTW_f_a_b_u6(op);
		case 0x02: return handleop32_EXTW_f_b_b_s12(op);
		case 0x03: return handleop32_EXTW_cc(op);
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
//                                 IIII I      SS SSSS               ss ssss
// RLC<.f> b,c                     0010 0bbb 0010 1111   FBBB CCCC CC00 1011
// RLC<.f> b,u6                    0010 0bbb 0110 1111   FBBB uuuu uu00 1011
// RLC<.f> b,limm                  0010 0bbb 0010 1111   FBBB 1111 1000 1011 (+ Limm)
//
// RLC<.f> 0,c                     0010 0110 0010 1111   F111 CCCC CC00 1011
// RLC<.f> 0,u6                    0010 0110 0110 1111   F111 uuuu uu00 1011
// RLC<.f> 0,limm                  0010 0110 0010 1111   F111 1111 1000 1011 (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_RLC(uint32_t op)  { return arcompact_handle04_2f_helper(op, "RLC"); } // RLC

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS               ss ssss
// EX<.di> b,[c]                   0010 0bbb 0010 1111   DBBB CCCC CC00 1100
// EX<.di> b,[u6]                  0010 0bbb 0110 1111   DBBB uuuu uu00 1100
// EX<.di> b,[limm]                0010 0bbb 0010 1111   DBBB 1111 1000 1100 (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_EX(uint32_t op)  { return arcompact_handle04_2f_helper(op, "EX"); } // EX
