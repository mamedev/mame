// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "arcompact_helper.ipp"

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

uint32_t arcompact_device::handleop32_ASL_single_do_op(arcompact_device &o, uint32_t src, bool set_flags)
{
	uint32_t result = src + src;
	if (set_flags)
	{
		o.do_flags_nz(result);
		if ((src & 0x80000000) != (result & 0x80000000)) { o.status32_set_v(); }
		else { o.status32_clear_v(); }
		if (src & 0x80000000) { o.status32_set_c(); }
		else { o.status32_clear_c(); }
	}
	return result;
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

uint32_t arcompact_device::handleop32_ASR_single_do_op(arcompact_device &o, uint32_t src, bool set_flags)
{
	uint32_t result = src >> 1;
	if (src & 0x80000000)
		result |= 0x80000000;

	if (set_flags)
	{
		o.do_flags_nz(result);
		if (src & 0x00000001) { o.status32_set_c(); }
		else { o.status32_clear_c(); }
	}
	return result;
}

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

uint32_t arcompact_device::handleop32_LSR_single_do_op(arcompact_device &o, uint32_t src, bool set_flags)
{
	uint32_t result = src >> 1;
	if (set_flags)
	{
		o.do_flags_nz(result);
		if (src & 0x00000001) { o.status32_set_c(); }
		else { o.status32_clear_c(); }
	}
	return result;
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

uint32_t arcompact_device::handleop32_ROR_do_op(arcompact_device &o, uint32_t src, bool set_flags)
{
	uint32_t result = src >> 1;
	if (src & 1)
		result |= 0x80000000;

	if (set_flags)
	{
		o.do_flags_nz(result);
		if (src & 0x00000001) { o.status32_set_c(); }
		else { o.status32_clear_c(); }
	}

	return result;
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

uint32_t arcompact_device::handleop32_RRC_do_op(arcompact_device &o, uint32_t src, bool set_flags)
{
	uint32_t result = src >> 1;
	if (o.status32_check_c())
		result |= 0x80000000;

	if (set_flags)
	{
		o.do_flags_nz(result);
		if (src & 0x00000001)
			o.status32_set_c();
		else
			o.status32_clear_c();
	}
	return result;
}

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

uint32_t arcompact_device::handleop32_SEXB_do_op(arcompact_device& o, uint32_t src, bool set_flags)
{
	uint32_t result = util::sext(src, 8);
	if (set_flags)
		o.do_flags_nz(result);
	return result;
}

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

uint32_t arcompact_device::handleop32_SEXW_do_op(arcompact_device& o, uint32_t src, bool set_flags)
{
	uint32_t result = util::sext(src, 16);
	if (set_flags)
		o.do_flags_nz(result);
	return result;
}

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

uint32_t arcompact_device::handleop32_EXTB_do_op(arcompact_device &o, uint32_t src, bool set_flags)
{
	uint32_t result = src & 0xff;
	if (set_flags)
	{
		o.do_flags_nz(result);
	}
	return result;
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

uint32_t arcompact_device::handleop32_EXTW_do_op(arcompact_device &o, uint32_t src, bool set_flags)
{
	uint32_t result = src & 0xffff;
	if (set_flags)
		o.do_flags_nz(result);
	return result;
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

uint32_t arcompact_device::handleop32_ABS_do_op(arcompact_device &o, uint32_t src, bool set_flags)
{
	uint32_t result;
	if (src & 0x80000000)
		result = 0x80000000 - (src & 0x7fffffff);
	else
		result = src;

	if (set_flags)
	{
		if (result == 0x00000000) { o.status32_set_z(); }
		else { o.status32_clear_z(); }
		if (src == 0x80000000) { o.status32_set_n(); }
		else { o.status32_clear_n(); }
		if (src & 0x80000000) { o.status32_set_c(); }
		else { o.status32_clear_c(); }
		if (src == 0x80000000) { o.status32_set_v(); }
		else { o.status32_clear_v(); }
	}

	return result;
}

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

uint32_t arcompact_device::handleop32_NOT_do_op(arcompact_device &o, uint32_t src, bool set_flags)
{
	uint32_t result = src ^ 0xffffffff;
	if (set_flags)
		o.do_flags_nz(result);
	return result;
}

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

uint32_t arcompact_device::handleop32_RLC_do_op(arcompact_device &o, uint32_t src, bool set_flags)
{
	uint32_t result = src << 1;
	if (o.status32_check_c())
		result |= 1;

	if (set_flags)
	{
		o.do_flags_nz(result);
		if (src & 0x80000000)
			o.status32_set_c();
		else
			o.status32_clear_c();
	}
	return result;
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Atomic Exchange
//                                 IIII I      SS SSSS               ss ssss
// EX<.di> b,[c]                   0010 0bbb 0010 1111   DBBB CCCC CC00 1100
// EX<.di> b,[u6]                  0010 0bbb 0110 1111   DBBB uuuu uu00 1100
// EX<.di> b,[limm]                0010 0bbb 0010 1111   DBBB 1111 1000 1100 (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_EX(uint32_t op)
{
	int size = 4;

	int p = common32_get_p(op);
	uint8_t breg = common32_get_breg(op);
	uint32_t b = m_regs[breg];

	if (p == 0)
	{
		uint8_t creg = common32_get_creg(op);
		size = check_limm(creg);
		uint32_t temp = READ32(m_regs[creg]);
		WRITE32(m_regs[creg], b);
		m_regs[breg] = temp;
	}
	else if (p == 1)
	{
		uint8_t u = common32_get_u6(op);
		uint32_t temp = READ32(u);
		WRITE32(u, b);
		m_regs[breg] = temp;
	}
	else
	{
		fatalerror("EX used with p == 2 or p == 3");
	}

	return m_pc + size;
}

