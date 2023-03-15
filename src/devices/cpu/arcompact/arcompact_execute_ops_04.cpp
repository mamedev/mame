// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "arcompact_helper.ipp"

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ADD<.f> a,b,c                   0010 0bbb 0000 0000   FBBB CCCC CCAA AAAA
// ADD<.f> a,limm,c                0010 0110 0000 0000   F111 CCCC CCAA AAAA (+ Limm)
// ADD<.f> a,b,limm                0010 0bbb 0000 0000   FBBB 1111 10AA AAAA (+ Limm)
// ADD<.f> 0,b,c                   0010 0bbb 0000 0000   FBBB CCCC CC11 1110
// ADD<.f> 0,b,limm                0010 0bbb 0000 0000   FBBB 1111 1011 1110 (+ Limm)
//
// ADD<.f> a,b,u6                  0010 0bbb 0100 0000   FBBB uuuu uuAA AAAA
// ADD<.f> 0,b,u6                  0010 0bbb 0100 0000   FBBB uuuu uu11 1110
//
// ADD<.f> b,b,s12                 0010 0bbb 1000 0000   FBBB ssss ssSS SSSS
//
// ADD<.cc><.f> b,b,c              0010 0bbb 1100 0000   FBBB CCCC CC0Q QQQQ
// ADD<.cc><.f> b,b,limm           0010 0bbb 1100 0000   FBBB 1111 100Q QQQQ (+ Limm)
// ADD<.cc><.f> 0,limm,c           0010 0110 1100 0000   F111 CCCC CC0Q QQQQ (+ Limm)
//
// ADD<.cc><.f> b,b,u6             0010 0bbb 1100 0000   FBBB uuuu uu1Q QQQQ
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_ADD_do_op(arcompact_device &o, uint32_t src1, uint32_t src2, bool set_flags)
{
	uint32_t result = src1 + src2;
	if (set_flags)
		o.do_flags_add(result, src1, src2);
	return result;
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ADC<.f> a,b,c                   0010 0bbb 0000 0001   FBBB CCCC CCAA AAAA
// ADC<.f> 0,b,c                   0010 0bbb 0000 0001   FBBB CCCC CC11 1110
// ADC<.f> a,limm,c                0010 0110 0000 0001   F111 CCCC CCAA AAAA (+ Limm)
// ADC<.f> a,b,limm                0010 0bbb 0000 0001   FBBB 1111 10AA AAAA (+ Limm)
// ADC<.f> 0,b,limm                0010 0bbb 0000 0001   FBBB 1111 1011 1110 (+ Limm)

// ADC<.f> a,b,u6                  0010 0bbb 0100 0001   FBBB uuuu uuAA AAAA
// ADC<.f> 0,b,u6                  0010 0bbb 0100 0001   FBBB uuuu uu11 1110

// ADC<.f> b,b,s12                 0010 0bbb 1000 0001   FBBB ssss ssSS SSSS
//
// ADC<.cc><.f> b,b,c              0010 0bbb 1100 0001   FBBB CCCC CC0Q QQQQ
// ADC<.cc><.f> b,b,limm           0010 0bbb 1100 0001   FBBB 1111 100Q QQQQ (+ Limm)
// ADC<.cc><.f> 0,limm,c           0010 0110 1100 0001   F111 CCCC CC0Q QQQQ (+ Limm)
//
// ADC<.cc><.f> b,b,u6             0010 0bbb 1100 0001   FBBB uuuu uu1Q QQQQ
//
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_ADC_do_op(arcompact_device &o, uint32_t src1, uint32_t src2, bool set_flags)
{
	uint8_t c = o.status32_check_c() ? 1 : 0;
	uint32_t result = src1 + (src2 + c);
	if (set_flags) // TODO: verify
		o.do_flags_add(result, src1, (src2 + c));
	return result;
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// SUB<.f> a,b,c                   0010 0bbb 0000 0010   FBBB CCCC CCAA AAAA
// SUB<.f> a,b,u6                  0010 0bbb 0100 0010   FBBB uuuu uuAA AAAA
// SUB<.f> b,b,s12                 0010 0bbb 1000 0010   FBBB ssss ssSS SSSS
// SUB<.cc><.f> b,b, c             0010 0bbb 1100 0010   FBBB CCCC CC0Q QQQQ
// SUB<.cc><.f> b,b,u6             0010 0bbb 1100 0010   FBBB uuuu uu1Q QQQQ
// SUB<.f> a,limm,c                0010 0110 0000 0010   F111 CCCC CCAA AAAA (+ Limm)
// SUB<.f> a,b,limm                0010 0bbb 0000 0010   FBBB 1111 10AA AAAA (+ Limm)
// SUB<.cc><.f> b,b,limm           0010 0bbb 1100 0010   FBBB 1111 100Q QQQQ (+ Limm)
//
// SUB <.f> 0,b,c                  0010 0bbb 0000 0010   FBBB CCCC CC11 1110
// SUB <.f> 0,b,u6                 0010 0bbb 0100 0010   FBBB uuuu uu11 1110
// SUB <.f> 0,b,limm               0010 0bbb 0000 0010   FBBB 1111 1011 1110 (+ Limm)
// SUB <.cc><.f> 0,limm,c          0010 0110 1100 0010   F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_SUB_do_op(arcompact_device &o, uint32_t src1, uint32_t src2, bool set_flags)
{
	uint32_t result = src1 - src2;
	if (set_flags)
		o.do_flags_sub(result, src1, src2);
	return result;
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// SBC<.f> a,b,c                   0010 0bbb 0000 0011   FBBB CCCC CCAA AAAA
// SBC<.f> a,b,u6                  0010 0bbb 0100 0011   FBBB uuuu uuAA AAAA
// SBC<.f> b,b,s12                 0010 0bbb 1000 0011   FBBB ssss ssSS SSSS
// SBC<.cc><.f> b,b,c              0010 0bbb 1100 0011   FBBB CCCC CC0Q QQQQ
// SBC<.cc><.f> b,b,u6             0010 0bbb 1100 0011   FBBB uuuu uu1Q QQQQ
// SBC<.f> a,limm,c                0010 0110 0000 0011   F111 CCCC CCAA AAAA (+ Limm)
// SBC<.f> a,b,limm                0010 0bbb 0000 0011   FBBB 1111 10AA AAAA (+ Limm)
// SBC<.cc><.f> b,b,limm           0010 0bbb 1100 0011   FBBB 1111 100Q QQQQ (+ Limm)
//
// SBC<.f> 0,b,c                   0010 0bbb 0000 0011   FBBB CCCC CC11 1110
// SBC<.f> 0,b,u6                  0010 0bbb 0100 0011   FBBB uuuu uu11 1110
// SBC<.f> 0,b,limm                0010 0bbb 0000 0011   FBBB 1111 1011 1110 (+ Limm)
// SBC<.cc><.f> 0,limm,c           0010 0110 1100 0011   F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_SBC_do_op(arcompact_device &o, uint32_t src1, uint32_t src2, bool set_flags)
{
	uint8_t c = o.status32_check_c() ? 1 : 0;
	uint32_t result = src1 - (src2 + c);
	if (set_flags) // TODO: verify
		o.do_flags_sub(result, src1, (src2 + c));
	return result;
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// AND<.f> a,b,c                   0010 0bbb 0000 0100   FBBB CCCC CCAA AAAA
// AND<.f> a,b,u6                  0010 0bbb 0100 0100   FBBB uuuu uuAA AAAA
// AND<.f> b,b,s12                 0010 0bbb 1000 0100   FBBB ssss ssSS SSSS
// AND<.cc><.f> b,b,c              0010 0bbb 1100 0100   FBBB CCCC CC0Q QQQQ
// AND<.cc><.f> b,b,u6             0010 0bbb 1100 0100   FBBB uuuu uu1Q QQQQ
// AND<.f> a,limm,c                0010 0110 0000 0100   F111 CCCC CCAA AAAA (+ Limm)
// AND<.f> a,b,limm                0010 0bbb 0000 0100   FBBB 1111 10AA AAAA (+ Limm)
// AND<.cc><.f> b,b,limm           0010 0bbb 1100 0100   FBBB 1111 100Q QQQQ (+ Limm)
//
// AND<.f> 0,b,c                   0010 0bbb 0000 0100   FBBB CCCC CC11 1110
// AND<.f> 0,b,u6                  0010 0bbb 0100 0100   FBBB uuuu uu11 1110
// AND<.f> 0,b,limm                0010 0bbb 0000 0100   FBBB 1111 1011 1110 (+ Limm)
// AND<.cc><.f> 0,limm,c           0010 0110 1100 0100   F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_AND_do_op(arcompact_device &o, uint32_t src1, uint32_t src2, bool set_flags)
{
	uint32_t result = src1 & src2;
	if (set_flags)
		o.do_flags_nz(result);
	return result;
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// OR<.f> a,b,c                    0010 0bbb 0000 0101   FBBB CCCC CCAA AAAA
// OR<.f> a,b,u6                   0010 0bbb 0100 0101   FBBB uuuu uuAA AAAA
// OR<.f> b,b,s12                  0010 0bbb 1000 0101   FBBB ssss ssSS SSSS
// OR<.cc><.f> b,b,c               0010 0bbb 1100 0101   FBBB CCCC CC0Q QQQQ
// OR<.cc><.f> b,b,u6              0010 0bbb 1100 0101   FBBB uuuu uu1Q QQQQ
// OR<.f> a,limm,c                 0010 0110 0000 0101   F111 CCCC CCAA AAAA (+ Limm)
// OR<.f> a,b,limm                 0010 0bbb 0000 0101   FBBB 1111 10AA AAAA (+ Limm)
// OR<.cc><.f> b,b,limm            0010 0bbb 1100 0101   FBBB 1111 100Q QQQQ (+ Limm)
//
// OR<.f> 0,b,c                    0010 0bbb 0000 0101   FBBB CCCC CC11 1110
// OR<.f> 0,b,u6                   0010 0bbb 0100 0101   FBBB uuuu uu11 1110
// OR<.f> 0,b,limm                 0010 0bbb 0000 0101   FBBB 1111 1011 1110 (+ Limm)
// OR<.cc><.f> 0,limm,c            0010 0110 1100 010  1 F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_OR_do_op(arcompact_device &o, uint32_t src1, uint32_t src2, bool set_flags)
{
	uint32_t result = src1 | src2;
	if (set_flags)
		o.do_flags_nz(result);
	return result;
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Bitwise AND Operation with Inverted Source
//                                 IIII I      SS SSSS
// BIC<.f> a,b,c                   0010 0bbb 0000 0110   FBBB CCCC CCAA AAAA
// BIC<.f> a,b,u6                  0010 0bbb 0100 0110   FBBB uuuu uuAA AAAA
// BIC<.f> b,b,s12                 0010 0bbb 1000 0110   FBBB ssss ssSS SSSS
// BIC<.cc><.f> b,b,c              0010 0bbb 1100 0110   FBBB CCCC CC0Q QQQQ
// BIC<.cc><.f> b,b,u6             0010 0bbb 1100 0110   FBBB uuuu uu1Q QQQQ
// BIC<.f> a,limm,c                0010 0110 0000 0110   F111 CCCC CCAA AAAA (+ Limm)
// BIC<.f> a,b,limm                0010 0bbb 0000 0110   FBBB 1111 10AA AAAA (+ Limm)
// BIC<.cc><.f> b,b,limm           0010 0bbb 1100 0110   FBBB 1111 100Q QQQQ (+ Limm)
//
// BIC<.f> 0,b,c                   0010 0bbb 0000 0110   FBBB CCCC CC11 1110
// BIC<.f> 0,b,u6                  0010 0bbb 0100 0110   FBBB uuuu uu11 1110
// BIC<.f> 0,b,limm                0010 0bbb 0000 0110   FBBB 1111 1011 1110 (+ Limm)
// BIC<.cc><.f> 0,limm,c           0010 0110 1100 0110   F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_BIC_do_op(arcompact_device &o, uint32_t src1, uint32_t src2, bool set_flags)
{
	uint32_t result = src1 & (~src2);
	if (set_flags)
		o.do_flags_nz(result);
	return result;
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS
// XOR<.f> a,b,c                   0010 0bbb 0000 0111   FBBB CCCC CCAA AAAA
// XOR<.f> a,b,u6                  0010 0bbb 0100 0111   FBBB uuuu uuAA AAAA
// XOR<.f> b,b,s12                 0010 0bbb 1000 0111   FBBB ssss ssSS SSSS
// XOR<.cc><.f> b,b,c              0010 0bbb 1100 0111   FBBB CCCC CC0Q QQQQ
// XOR<.cc><.f> b,b,u6             0010 0bbb 1100 0111   FBBB uuuu uu1Q QQQQ
// XOR<.f> a,limm,c                0010 0110 0000 0111   F111 CCCC CCAA AAAA (+ Limm)
// XOR<.f> a,b,limm                0010 0bbb 0000 0111   FBBB 1111 10AA AAAA (+ Limm)
// XOR<.cc><.f> b,b,limm           0010 0bbb 1100 0111   FBBB 1111 100Q QQQQ (+ Limm)
//
// XOR<.f> 0,b,c                   0010 0bbb 0000 0111   FBBB CCCC CC11 1110
// XOR<.f> 0,b,u6                  0010 0bbb 0100 0111   FBBB uuuu uu11 1110
// XOR<.f> 0,b,limm                0010 0bbb 0000 0111   FBBB 1111 1011 1110 (+ Limm)
// XOR<.cc><.f> 0,limm,c           0010 0110 1100 0111   F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_XOR_do_op(arcompact_device &o, uint32_t src1, uint32_t src2, bool set_flags)
{
	uint32_t result = src1 ^ src2;
	if (set_flags)
		o.do_flags_nz(result);
	return result;
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS
// MAX<.f> a,b,c                   0010 0bbb 0000 1000   FBBB CCCC CCAA AAAA
// MAX<.f> a,b,u6                  0010 0bbb 0100 1000   FBBB uuuu uuAA AAAA
// MAX<.f> b,b,s12                 0010 0bbb 1000 1000   FBBB ssss ssSS SSSS
// MAX<.cc><.f> b,b,c              0010 0bbb 1100 1000   FBBB CCCC CC0Q QQQQ
// MAX<.cc><.f> b,b,u6             0010 0bbb 1100 1000   FBBB uuuu uu1Q QQQQ
// MAX<.f> a,limm,c                0010 0110 0000 1000   F111 CCCC CCAA AAAA (+ Limm)
// MAX<.f> a,b,limm                0010 0bbb 0000 1000   FBBB 1111 10AA AAAA (+ Limm)
// MAX<.cc><.f> b,b,limm           0010 0bbb 1100 1000   FBBB 1111 100Q QQQQ (+ Limm)
//
// MAX<.f> 0,b,c                   0010 0bbb 0000 1000   FBBB CCCC CC11 1110
// MAX<.f> 0,b,u6                  0010 0bbb 0100 1000   FBBB uuuu uu11 1110
// MAX<.f> 0,b,limm                0010 0bbb 0000 1000   FBBB 1111 1011 1110 (+ Limm)
// MAX<.cc><.f> 0,limm,c           0010 0110 1100 1000   F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_MAX_do_op(arcompact_device &o, uint32_t src1, uint32_t src2, bool set_flags)
{
	uint32_t alu = src1 - src2;
	uint32_t result;

	if ((int32_t)src2 >= (int32_t)src1)
		result = src2;
	else
		result = src1;

	if (set_flags) // TODO: verify
	{
		o.do_flags_nz(alu);
		o.do_flags_overflow(alu, src1, src2);
		if ((int32_t)src2 >= (int32_t)src1)
			o.status32_set_c();
		else
			o.status32_clear_c();
	}
	return result;
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS
// MIN<.f> a,b,c                   0010 0bbb 0000 1001   FBBB CCCC CCAA AAAA
// MIN<.f> a,b,u6                  0010 0bbb 0100 1001   FBBB uuuu uuAA AAAA
// MIN<.f> b,b,s12                 0010 0bbb 1000 1001   FBBB ssss ssSS SSSS
// MIN<.cc><.f> b,b,c              0010 0bbb 1100 1001   FBBB CCCC CC0Q QQQQ
// MIN<.cc><.f> b,b,u6             0010 0bbb 1100 1001   FBBB uuuu uu1Q QQQQ
// MIN<.f> a,limm,c                0010 0110 0000 1001   F111 CCCC CCAA AAAA (+ Limm)
// MIN<.f> a,b,limm                0010 0bbb 0000 1001   FBBB 1111 10AA AAAA (+ Limm)
// MIN<.cc><.f> b,b,limm           0010 0bbb 1100 1001   FBBB 1111 100Q QQQQ (+ Limm)
//
// MIN<.f> 0,b,c                   0010 0bbb 0000 1001   FBBB CCCC CC11 1110
// MIN<.f> 0,b,u6                  0010 0bbb 0100 1001   FBBB uuuu uu11 1110
// MIN<.f> 0,b,limm                0010 0bbb 0000 1001   FBBB 1111 1011 1110 (+ Limm)
// MIN<.cc><.f> 0,limm,c           0010 0110 1100 1001   F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_MIN_do_op(arcompact_device &o, uint32_t src1, uint32_t src2, bool set_flags)
{
	uint32_t alu = src1 - src2;
	uint32_t result;

	if ((int32_t)src2 <= (int32_t)src1)
		result = src2;
	else
		result = src1;

	if (set_flags) // TODO: verify
	{
		o.do_flags_nz(alu);
		o.do_flags_overflow(alu, src1, src2);
		if ((int32_t)src2 <= (int32_t)src1)
			o.status32_set_c();
		else
			o.status32_clear_c();
	}
	return result;
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Unusual format, a is not used, dest is always b
//
//                                 IIII I      SS SSSS
// MOV<.f> b,s12                   0010 0bbb 1000 1010   FBBB ssss ssSS SSSS
// MOV<.f> 0,s12                   0010 0110 1000 1010   F111 ssss ssSS SSSS (is b is 'Limm' there's no destination)

// MOV<.cc><.f> b,c                0010 0bbb 1100 1010   FBBB CCCC CC0Q QQQQ
// MOV<.cc><.f> b,u6               0010 0bbb 1100 1010   FBBB uuuu uu1Q QQQQ
// MOV<.cc><.f> b,limm             0010 0bbb 1100 1010   FBBB 1111 100Q QQQQ (+ Limm)
//
// MOV<.cc><.f> 0,c                0010 0110 1100 1010   F111 CCCC CC0Q QQQQ
// MOV<.cc><.f> 0,u6               0010 0110 1100 1010   F111 uuuu uu1Q QQQQ
// MOV<.cc><.f> 0,limm             0010 0110 1100 1010   F111 1111 100Q QQQQ (+ Limm)
//
// NOP                             0010 0110 0100 1010   0111 0000 0000 0000 (NOP is a custom encoded MOV where b is 'LIMM' and u6 is 0)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void arcompact_device::handleop32_MOV_do_op(uint32_t breg, uint32_t src2, bool set_flags)
{
	m_regs[breg] = src2;
	if (set_flags)
		do_flags_nz(m_regs[breg]);
}

uint32_t arcompact_device::handleop32_MOV(uint32_t op)
{
	switch ((op & 0x00c00000) >> 22)
	{
	case 0x00:
	{
		uint8_t breg = common32_get_breg(op);
		uint8_t creg = common32_get_creg(op);
		int size = check_limm(creg);
		handleop32_MOV_do_op(breg, m_regs[creg], common32_get_F(op));
		return m_pc + size;
	}
	case 0x01:
	{
		uint8_t breg = common32_get_breg(op);
		handleop32_MOV_do_op(breg, common32_get_u6(op), common32_get_F(op));
		return m_pc + 4;
	}
	case 0x02:
	{
		uint8_t breg = common32_get_breg(op);
		handleop32_MOV_do_op(breg, common32_get_s12(op), common32_get_F(op));
		return m_pc + 4;
	}
	case 0x03:
	{
		switch ((op & 0x00000020) >> 5)
		{
		case 0x00:
		{
			uint8_t breg = common32_get_breg(op);
			uint8_t creg = common32_get_creg(op);
			int size = check_limm(creg);
			if (check_condition(common32_get_condition(op)))
				handleop32_MOV_do_op(breg, m_regs[creg], common32_get_F(op));
			return m_pc + size;
		}
		case 0x01:
		{
			uint8_t breg = common32_get_breg(op);
			if (check_condition(common32_get_condition(op)))
				handleop32_MOV_do_op(breg, common32_get_u6(op), common32_get_F(op));
			return m_pc + 4;
		}
		}
		return 0;
	}
	}
	return 0;
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS
// TST b,s12                       0010 0bbb 1000 1011   1BBB ssss ssSS SSSS
// TST<.cc> b,c                    0010 0bbb 1100 1011   1BBB CCCC CC0Q QQQQ
// TST<.cc> b,u6                   0010 0bbb 1100 1011   1BBB uuuu uu1Q QQQQ
// TST<.cc> b,limm                 0010 0bbb 1100 1011   1BBB 1111 100Q QQQQ (+ Limm)
// TST<.cc> limm,c                 0010 0110 1100 1011   1111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void arcompact_device::handleop32_TST_do_op(arcompact_device &o, uint32_t src1, uint32_t src2)
{
	uint32_t result = src1 & src2;
	o.do_flags_nz(result);
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS
// CMP b,s12                       0010 0bbb 1000 1100   1BBB ssss ssSS SSSS
// CMP<.cc> b,c                    0010 0bbb 1100 1100   1BBB CCCC CC0Q QQQQ
// CMP<.cc> b,u6                   0010 0bbb 1100 1100   1BBB uuuu uu1Q QQQQ
// CMP<.cc> b,limm                 0010 0bbb 1100 1100   1BBB 1111 100Q QQQQ (+ Limm)
// CMP<.cc> limm,c                 0010 0110 1100 1100   1111 CCCC CC0Q QQQQ (+ Limm)
//
// Note F is always encoded as 1, a is ignored (encodings with a would be redundant)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void arcompact_device::handleop32_CMP_do_op(arcompact_device &o, uint32_t src1, uint32_t src2)
{
	uint32_t result = src1 - src2;
	o.do_flags_sub(result, src1, src2);
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// RCMP b,s12                      0010 0bbb 1000 1101   1BBB ssss ssSS SSSS
// RCMP<.cc> b,c                   0010 0bbb 1100 1101   1BBB CCCC CC0Q QQQQ
// RCMP<.cc> b,u6                  0010 0bbb 1100 1101   1BBB uuuu uu1Q QQQQ
// RCMP<.cc> b,limm                0010 0bbb 1100 1101   1BBB 1111 100Q QQQQ (+ Limm)
// RCMP<.cc> limm,c                0010 0110 1100 1101   1111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void arcompact_device::handleop32_RCMP_do_op(arcompact_device &o, uint32_t src1, uint32_t src2)
{
	uint32_t result = src2 - src1;
	o.do_flags_sub(result, src2, src1);
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// RSUB<.f> a,b,c                  0010 0bbb 0000 1110   FBBB CCCC CCAA AAAA
// RSUB<.f> a,b,u6                 0010 0bbb 0100 1110   FBBB uuuu uuAA AAAA
// NEG<.f> a,b                     0010 0bbb 0100 1110   FBBB 0000 00AA AAAA (NEG is an alias)
//
// RSUB<.f> b,b,s12                0010 0bbb 1000 1110   FBBB ssss ssSS SSSS
// RSUB<.cc><.f> b,b,c             0010 0bbb 1100 1110   FBBB CCCC CC0Q QQQQ
// RSUB<.cc><.f> b,b,u6            0010 0bbb 1100 1110   FBBB uuuu uu1Q QQQQ
// NEG<.cc><.f> b,b                0010 0bbb 1100 1110   FBBB 0000 001Q QQQQ (NEG is an alias)
//
// RSUB<.f> a,limm,c               0010 0110 0000 1110   F111 CCCC CCAA AAAA (+ Limm)
// RSUB<.f> a,b,limm               0010 0bbb 0000 1110   FBBB 1111 10AA AAAA (+ Limm)
// RSUB<.cc><.f> b,b,limm          0010 0bbb 1100 1110   FBBB 1111 100Q QQQQ (+ Limm)
//
// RSUB<.f> 0,b,c                  0010 0bbb 0000 1110   FBBB CCCC CC11 1110
// RSUB<.f> 0,b,u6                 0010 0bbb 0100 1110   FBBB uuuu uu11 1110
// RSUB<.f> 0,b,limm               0010 0bbb 0000 1110   FBBB 1111 1011 1110 (+ Limm)
// RSUB<.cc><.f> 0,limm,c          0010 0110 1100 1110   F111 CCCC CC0Q QQQQ (+ Limm)
//
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_RSUB_do_op(arcompact_device &o, uint32_t src1, uint32_t src2, bool set_flags)
{
	uint32_t result = src2 - src1;
	if (set_flags)
		o.do_flags_sub(result, src2, src1);
	return result;
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BSET<.f> a,b,c                  0010 0bbb 0000 1111   FBBB CCCC CCAA AAAA
// BSET<.f> a,b,u6                 0010 0bbb 0100 1111   FBBB uuuu uuAA AAAA
// BSET<.cc><.f> b,b,c             0010 0bbb 1100 1111   FBBB CCCC CC0Q QQQQ
// BSET<.cc><.f> b,b,u6            0010 0bbb 1100 1111   FBBB uuuu uu1Q QQQQ
// BSET<.f> a,limm,c               0010 0110 0000 1111   F111 CCCC CCAA AAAA (+ Limm)
//
// BSET<.f> 0,b,c                  0010 0bbb 0000 1111   FBBB CCCC CC11 1110
// BSET<.f> 0,b,u6                 0010 0bbb 0100 1111   FBBB uuuu uu11 1110
// BSET<.cc><.f> 0,limm,c          0010 0110 1100 1111   F110 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_BSET_do_op(arcompact_device &o, uint32_t src1, uint32_t src2, bool set_flags)
{
	uint32_t result = src1 | (1 << (src2 & 0x1f));
	if (set_flags)
		o.do_flags_nz(result);
	return result;
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BCLR<.f> a,b,c                  0010 0bbb 0001 0000   FBBB CCCC CCAA AAAA
// BCLR<.f> a,b,u6                 0010 0bbb 0101 0000   FBBB uuuu uuAA AAAA
// BCLR<.cc><.f> b,b,c             0010 0bbb 1101 0000   FBBB CCCC CC0Q QQQQ
// BCLR<.cc><.f> b,b,u6            0010 0bbb 1101 0000   FBBB uuuu uu1Q QQQQ
// BCLR<.f> a,limm,c               0010 0110 0001 0000   F111 CCCC CCAA AAAA (+ Limm)
//
// BCLR<.f> 0,b,c                  0010 0bbb 0001 0000   FBBB CCCC CC11 1110
// BCLR<.f> 0,b,u6                 0010 0bbb 0101 0000   FBBB uuuu uu11 1110
// BCLR<.cc><.f> 0,limm,c          0010 0110 1101 0000   F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_BCLR_do_op(arcompact_device &o, uint32_t src1, uint32_t src2, bool set_flags)
{
	uint32_t result = src1 & ~(1 << (src2 & 0x1f));
	if (set_flags)
		o.do_flags_nz(result);
	return result;
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BTST<.cc> b,c                   0010 0bbb 1101 0001   1BBB CCCC CC0Q QQQQ
// BTST<.cc> b,u6                  0010 0bbb 1101 0001   1BBB uuuu uu1Q QQQQ
// BTST<.cc> limm,c                0010 0110 1101 0001   1111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void arcompact_device::handleop32_BTST_do_op(arcompact_device &o, uint32_t src1, uint32_t src2)
{
	uint32_t result = src1 & (1 << (src2 & 0x1f));
	o.do_flags_nz(result);
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BXOR<.f> a,b,c                  0010 0bbb 0001 0010   FBBB CCCC CCAA AAAA
// BXOR<.f> a,b,u6                 0010 0bbb 0101 0010   FBBB uuuu uuAA AAAA
// BXOR<.cc><.f> b,b,c             0010 0bbb 1101 0010   FBBB CCCC CC0Q QQQQ
// BXOR<.cc><.f> b,b,u6            0010 0bbb 1101 0010   FBBB uuuu uu1Q QQQQ
// BXOR<.f> a,limm,c               0010 0110 0001 0010   F111 CCCC CCAA AAAA (+ Limm)
//
// BXOR<.f> 0,b,c                  0010 0bbb 0001 0010   FBBB CCCC CC11 1110
// BXOR<.f> 0,b,u6                 0010 0bbb 0101 0010   FBBB uuuu uu11 1110
// BXOR<.cc><.f> 0,limm,c          0010 0110 1101 0010   F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_BXOR_do_op(arcompact_device &o, uint32_t src1, uint32_t src2, bool set_flags)
{
	uint32_t result = src1 ^ (1 << (src2 & 0x1f));
	if (set_flags)
		o.do_flags_nz(result);
	return result;
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BMSK<.f> a,b,c                  0010 0bbb 0001 0011   FBBB CCCC CCAA AAAA
// BMSK<.f> a,b,u6                 0010 0bbb 0101 0011   FBBB uuuu uuAA AAAA
// BMSK<.cc><.f> b,b,c             0010 0bbb 1101 0011   FBBB CCCC CC0Q QQQQ
// BMSK<.cc><.f> b,b,u6            0010 0bbb 1101 0011   FBBB uuuu uu1Q QQQQ
// BMSK<.f> a,limm,c               0010 0110 0001 0011   F111 CCCC CCAA AAAA (+ Limm)
//
// BMSK<.f> 0,b,c                  0010 0bbb 0001 0011   FBBB CCCC CC11 1110
// BMSK<.f> 0,b,u6                 0010 0bbb 0101 0011   FBBB uuuu uu11 1110
// BMSK<.cc><.f> 0,limm,c          0010 0110 1101 0011   F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_BMSK_do_op(arcompact_device &o, uint32_t src1, uint32_t src2, bool set_flags)
{
	uint32_t result = src1 & ((1 << (src2 + 1)) - 1);
	if (set_flags)
		o.do_flags_nz(result);
	return result;
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS
// ADD1<.f> a,b,c                  0010 0bbb 0001 0100   FBBB CCCC CCAA AAAA
// ADD1<.f> a,b,u6                 0010 0bbb 0101 0100   FBBB uuuu uuAA AAAA
// ADD1<.f> b,b,s12                0010 0bbb 1001 0100   FBBB ssss ssSS SSSS
// ADD1<.cc><.f> b,b,c             0010 0bbb 1101 0100   FBBB CCCC CC0Q QQQQ
// ADD1<.cc><.f> b,b,u6            0010 0bbb 1101 0100   FBBB uuuu uu1Q QQQQ
// ADD1<.f> a,limm,c               0010 0110 0001 0100   F111 CCCC CCAA AAAA (+ Limm)
// ADD1<.f> a,b,limm               0010 0bbb 0001 0100   FBBB 1111 10AA AAAA (+ Limm)
// ADD1<.cc><.f> b,b,limm          0010 0bbb 1101 0100   FBBB 1111 100Q QQQQ (+ Limm)
//
// ADD1<.f> 0,b,c                  0010 0bbb 0001 0100   FBBB CCCC CC11 1110
// ADD1<.f> 0,b,u6                 0010 0bbb 0101 0100   FBBB uuuu uu11 1110
// ADD1<.f> 0,b,limm               0010 0bbb 0001 0100   FBBB 1111 1011 1110 (+ Limm)
// ADD1<.cc><.f> 0,limm,c          0010 0110 1101 0100   F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_ADD1_do_op(arcompact_device &o, uint32_t src1, uint32_t src2, bool set_flags)
{
	uint32_t result = src1 + (src2 << 1);
	if (set_flags)
		o.do_flags_add(result, src1, src2 << 1);
	return result;
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS
// ADD2<.f> a,b,c                  0010 0bbb 0001 0101   FBBB CCCC CCAA AAAA
// ADD2<.f> a,b,u6                 0010 0bbb 0101 0101   FBBB uuuu uuAA AAAA
// ADD2<.f> b,b,s12                0010 0bbb 1001 0101   FBBB ssss ssSS SSSS
// ADD2<.cc><.f> b,b,c             0010 0bbb 1101 0101   FBBB CCCC CC0Q QQQQ
// ADD2<.cc><.f> b,b,u6            0010 0bbb 1101 0101   FBBB uuuu uu1Q QQQQ
// ADD2<.f> a,limm,c               0010 0110 0001 0101   F111 CCCC CCAA AAAA (+ Limm)
// ADD2<.f> a,b,limm               0010 0bbb 0001 0101   FBBB 1111 10AA AAAA (+ Limm)
// ADD2<.cc><.f> b,b,limm          0010 0bbb 1101 0101   FBBB 1111 100Q QQQQ (+ Limm)
//
// ADD2<.f> 0,b,c                  0010 0bbb 0001 0101   FBBB CCCC CC11 1110
// ADD2<.f> 0,b,u6                 0010 0bbb 0101 0101   FBBB uuuu uu11 1110
// ADD2<.f> 0,b,limm               0010 0bbb 0001 0101   FBBB 1111 1011 1110 (+ Limm)
// ADD2<.cc><.f> 0,limm,c          0010 0110 1101 0101   F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_ADD2_do_op(arcompact_device &o, uint32_t src1, uint32_t src2, bool set_flags)
{
	uint32_t result = src1 + (src2 << 2);
	if (set_flags)
		o.do_flags_add(result, src1, src2 << 2);
	return result;
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS
// ADD3<.f> a,b,c                  0010 0bbb 0001 0110   FBBB CCCC CCAA AAAA
// ADD3<.f> a,b,u6                 0010 0bbb 0101 0110   FBBB uuuu uuAA AAAA
// ADD3<.f> b,b,s12                0010 0bbb 1001 0110   FBBB ssss ssSS SSSS
// ADD3<.cc><.f> b,b,c             0010 0bbb 1101 0110   FBBB CCCC CC0Q QQQQ
// ADD3<.cc><.f> b,b,u6            0010 0bbb 1101 0110   FBBB uuuu uu1Q QQQQ
// ADD3<.f> a,limm,c               0010 0110 0001 0110   F111 CCCC CCAA AAAA (+ Limm)
// ADD3<.f> a,b,limm               0010 0bbb 0001 0110   FBBB 1111 10AA AAAA (+ Limm)
// ADD3<.cc><.f> b,b,limm          0010 0bbb 1101 0110   FBBB 1111 100Q QQQQ (+ Limm)
//
// ADD3<.f> 0,b,c                  0010 0bbb 0001 0110   FBBB CCCC CC11 1110
// ADD3<.f> 0,b,u6                 0010 0bbb 0101 0110   FBBB uuuu uu11 1110
// ADD3<.f> 0,b,limm               0010 0bbb 0001 0110   FBBB 1111 1011 1110 (+ Limm)
// ADD3<.cc><.f> 0,limm,c          0010 0110 1101 0110   F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_ADD3_do_op(arcompact_device &o, uint32_t src1, uint32_t src2, bool set_flags)
{
	uint32_t result = src1 + (src2 << 3);
	if (set_flags)
		o.do_flags_add(result, src1, src2 << 3);
	return result;
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS
// SUB1<.f> a,b,c                  0010 0bbb 0001 0111   FBBB CCCC CCAA AAAA
// SUB1<.f> a,b,u6                 0010 0bbb 0101 0111   FBBB uuuu uuAA AAAA
// SUB1<.f> b,b,s12                0010 0bbb 1001 0111   FBBB ssss ssSS SSSS
// SUB1<.cc><.f> b,b,c             0010 0bbb 1101 0111   FBBB CCCC CC0Q QQQQ
// SUB1<.cc><.f> b,b,u6            0010 0bbb 1101 0111   FBBB uuuu uu1Q QQQQ
// SUB1<.f> a,limm,c               0010 0110 0001 0111   F111 CCCC CCAA AAAA (+ Limm)
// SUB1<.f> a,b,limm               0010 0bbb 0001 0111   FBBB 1111 10AA AAAA (+ Limm)
// SUB1<.cc><.f> b,b,limm          0010 0bbb 1101 0111   FBBB 1111 100Q QQQQ (+ Limm)
//
// SUB1<.f> 0,b,c                  0010 0bbb 0001 0111   FBBB CCCC CC11 1110
// SUB1<.f> 0,b,u6                 0010 0bbb 0101 0111   FBBB uuuu uu11 1110
// SUB1<.f> 0,b,limm               0010 0bbb 0001 0111   FBBB 1111 1011 1110 (+ Limm)
// SUB1<.cc><.f> 0,limm,c          0010 0110 1101 0111   F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_SUB1_do_op(arcompact_device &o, uint32_t src1, uint32_t src2, bool set_flags)
{
	uint32_t result = src1 - (src2 << 1);
	if (set_flags)
		o.do_flags_sub(result, src1, (src2 << 1));
	return result;
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS
// SUB2<.f> a,b,c                  0010 0bbb 0001 1000   FBBB CCCC CCAA AAAA
// SUB2<.f> a,b,u6                 0010 0bbb 0101 1000   FBBB uuuu uuAA AAAA
// SUB2<.f> b,b,s12                0010 0bbb 1001 1000   FBBB ssss ssSS SSSS
// SUB2<.cc><.f> b,b,c             0010 0bbb 1101 1000   FBBB CCCC CC0Q QQQQ
// SUB2<.cc><.f> b,b,u6            0010 0bbb 1101 1000   FBBB uuuu uu1Q QQQQ
// SUB2<.f> a,limm,c               0010 0110 0001 1000   F111 CCCC CCAA AAAA (+ Limm)
// SUB2<.f> a,b,limm               0010 0bbb 0001 1000   FBBB 1111 10AA AAAA (+ Limm)
// SUB2<.cc><.f> b,b,limm          0010 0bbb 1101 1000   FBBB 1111 100Q QQQQ (+ Limm)
//
// SUB2<.f> 0,b,c                  0010 0bbb 0001 1000   FBBB CCCC CC11 1110
// SUB2<.f> 0,b,u6                 0010 0bbb 0101 1000   FBBB uuuu uu11 1110
// SUB2<.f> 0,b,limm               0010 0bbb 0001 1000   FBBB 1111 1011 1110 (+ Limm)
// SUB2<.cc><.f> 0,limm,c          0010 0110 1101 1000   F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_SUB2_do_op(arcompact_device &o, uint32_t src1, uint32_t src2, bool set_flags)
{
	uint32_t result = src1 - (src2 << 2);
	if (set_flags)
		o.do_flags_sub(result, src1, (src2 << 2));
	return result;
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS
// SUB3<.f> a,b,c                  0010 0bbb 0001 1001   FBBB CCCC CCAA AAAA
// SUB3<.f> a,b,u6                 0010 0bbb 0101 1001   FBBB uuuu uuAA AAAA
// SUB3<.f> b,b,s12                0010 0bbb 1001 1001   FBBB ssss ssSS SSSS
// SUB3<.cc><.f> b,b,c             0010 0bbb 1101 1001   FBBB CCCC CC0Q QQQQ
// SUB3<.cc><.f> b,b,u6            0010 0bbb 1101 1001   FBBB uuuu uu1Q QQQQ
// SUB3<.f> a,limm,c               0010 0110 0001 1001   F111 CCCC CCAA AAAA (+ Limm)
// SUB3<.f> a,b,limm               0010 0bbb 0001 1001   FBBB 1111 10AA AAAA (+ Limm)
// SUB3<.cc><.f> b,b,limm          0010 0bbb 1101 1001   FBBB 1111 100Q QQQQ (+ Limm)
//
// SUB3<.f> 0,b,c                  0010 0bbb 0001 1001   FBBB CCCC CC11 1110
// SUB3<.f> 0,b,u6                 0010 0bbb 0101 1001   FBBB uuuu uu11 1110
// SUB3<.f> 0,limm,c               0010 0110 0001 1001   F111 CCCC CC11 1110 (+ Limm)
// SUB3<.cc><.f> 0,limm,c          0010 0110 1101 1001   F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_SUB3_do_op(arcompact_device &o, uint32_t src1, uint32_t src2, bool set_flags)
{
	uint32_t result = src1 - (src2 << 3);
	if (set_flags)
		o.do_flags_sub(result, src1, (src2 << 3));
	return result;
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Optional on ARC700 only, not available on ARCtangent-A5 or ARC600
//
//                                 IIII I      SS SSSS
// MPY<.f> a,b,c                   0010 0bbb 0001 1010   FBBB CCCC CCAA AAAA
// MPY<.f> a,b,u6                  0010 0bbb 0101 1010   FBBB uuuu uuAA AAAA
// MPY<.f> b,b,s12                 0010 0bbb 1001 1010   FBBB ssss ssSS SSSS
// MPY<.cc><.f> b,b,c              0010 0bbb 1101 1010   FBBB CCCC CC0Q QQQQ
// MPY<.cc><.f> b,b,u6             0010 0bbb 1101 1010   FBBB uuuu uu1Q QQQQ
// MPY<.f> a,limm,c                0010 0110 0001 1010   F111 CCCC CCAA AAAA (+ Limm)
// MPY<.f> a,b,limm                0010 0bbb 0001 1010   FBBB 1111 10AA AAAA (+ Limm)
// MPY<.cc><.f> b,b,limm           0010 0bbb 1101 1010   FBBB 1111 100Q QQQQ (+ Limm)
//
// MPY<.f> 0,b,c                   0010 0bbb 0001 1010   FBBB CCCC CC11 1110
// MPY<.f> 0,b,u6                  0010 0bbb 0101 1010   FBBB uuuu uu11 1110
// MPY<.cc><.f> 0,limm,c           0010 0110 1101 1010   F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_MPY_do_op(arcompact_device &o, uint32_t src1, uint32_t src2, bool set_flags)
{
	uint32_t result = 0;
	fatalerror("MPY not supported");
	if (set_flags)
		fatalerror("MPY flags not supported");
	return result;
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Optional on ARC700 only, not available on ARCtangent-A5 or ARC600
//
//                                 IIII I      SS SSSS
// MPYH<.f> a,b,c                  0010 0bbb 0001 1011   FBBB CCCC CCAA AAAA
// MPYH<.f> a,b,u6                 0010 0bbb 0101 1011   FBBB uuuu uuAA AAAA
// MPYH<.f> b,b,s12                0010 0bbb 1001 1011   FBBB ssss ssSS SSSS
// MPYH<.cc><.f> b,b,c             0010 0bbb 1101 1011   FBBB CCCC CC0Q QQQQ
// MPYH<.cc><.f> b,b,u6            0010 0bbb 1101 1011   FBBB uuuu uu1Q QQQQ
// MPYH<.f> a,limm,c               0010 0110 0001 1011   F111 CCCC CCAA AAAA (+ Limm)
// MPYH<.f> a,b,limm               0010 0bbb 0001 1010   FBBB 1111 10AA AAAA (+ Limm)
// MPYH<.cc><.f> b,b,limm          0010 0bbb 1101 1011   FBBB 1111 100Q QQQQ (+ Limm)
//
// MPYH<.f> 0,b,c                  0010 0bbb 0001 1011   FBBB CCCC CC11 1110
// MPYH<.f> 0,b,u6                 0010 0bbb 0101 1011   FBBB uuuu uu11 1110
// MPYH<.cc><.f> 0,limm,c          0010 0110 1101 1011   F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_MPYH_do_op(arcompact_device &o, uint32_t src1, uint32_t src2, bool set_flags)
{
	uint32_t result = 0;
	fatalerror("MPYH not supported");
	if (set_flags)
		fatalerror("MPYH flags not supported");
	return result;
}
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Optional on ARC700 only, not available on ARCtangent-A5 or ARC600
//
//                                 IIII I      SS SSSS
// MPYHU<.f> a,b,c                 0010 0bbb 0001 1100   FBBB CCCC CCAA AAAA
// MPYHU<.f> a,b,u6                0010 0bbb 0101 1100   FBBB uuuu uuAA AAAA
// MPYHU<.f> b,b,s12               0010 0bbb 1001 1100   FBBB ssss ssSS SSSS
// MPYHU<.cc><.f> b,b,c            0010 0bbb 1101 1100   FBBB CCCC CC0Q QQQQ
// MPYHU<.cc><.f> b,b,u6           0010 0bbb 1101 1100   FBBB uuuu uu1Q QQQQ
// MPYHU<.f> a,limm,c              0010 0110 0001 1100   F111 CCCC CCAA AAAA (+ Limm)
// MPYHU<.f> a,b,limm              0010 0bbb 0001 1100   FBBB 1111 10AA AAAA (+ Limm)
// MPYHU<.cc><.f> b,b,limm         0010 0bbb 1101 1100   FBBB 1111 100Q QQQQ (+ Limm)
//
// MPYHU<.f> 0,b,c                 0010 0bbb 0001 1100   FBBB CCCC CC11 1110
// MPYHU<.f> 0,b,u6                0010 0bbb 0101 1100   FBBB uuuu uu11 1110
// MPYHU<.cc><.f> 0,limm,c         0010 0110 1101 1100   F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_MPYHU_do_op(arcompact_device &o, uint32_t src1, uint32_t src2, bool set_flags)
{
	uint32_t result = 0;
	fatalerror("MPYHU not supported");
	if (set_flags)
		fatalerror("MPYHU flags not supported");
	return result;
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Optional on ARC700 only, not available on ARCtangent-A5 or ARC600
//
// MPYU<.f> a,b,c                  0010 0bbb 0001 1101   FBBB CCCC CCAA AAAA
// MPYU<.f> a,b,u6                 0010 0bbb 0101 1101   FBBB uuuu uuAA AAAA
// MPYU<.f> b,b,s12                0010 0bbb 1001 1101   FBBB ssss ssSS SSSS
// MPYU<.cc><.f> b,b,c             0010 0bbb 1101 1101   FBBB CCCC CC0Q QQQQ
// MPYU<.cc><.f> b,b,u6            0010 0bbb 1101 1101   FBBB uuuu uu1Q QQQQ
// MPYU<.f> a,limm,c               0010 0110 0001 1101   F111 CCCC CCAA AAAA (+ Limm)
// MPYU<.f> a,b,limm               0010 0bbb 0001 1101   FBBB 1111 10AA AAAA (+ Limm)
// MPYU<.cc><.f> b,b,limm          0010 0bbb 1101 1101   FBBB 1111 100Q QQQQ (+ Limm)
//
// MPYU<.f> 0,b,c                  0010 0bbb 0001 1101   FBBB CCCC CC11 1110
// MPYU<.f> 0,b,u6                 0010 0bbb 0101 1101   FBBB uuuu uu11 1110
// MPYU<.cc><.f> 0,limm,c          0010 0110 1101 1101   F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_MPYU_do_op(arcompact_device &o, uint32_t src1, uint32_t src2, bool set_flags)
{
	uint32_t result = 0;
	fatalerror("MPYU not supported");
	if (set_flags)
		fatalerror("MPYU flags not supported");
	return result;
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Neither a nor b are used, would putting limm in them cause the limm to be read?
//
//                                 IIII I      SS SSSS
//                                           PP
// General Operations Reg-Reg      0010 0bbb 00ii iiii   FBBB CCCC CCAA AAAA
// FLAG c                          0010 0000 0010 1001   0000 0000 0100 0000 (Leapster BIOS uses this redundant encoding where A is unused?)
//
//                                           PP
// Gen Op Reg+6-bit unsigned Imm   0010 0bbb 01ii iiii   FBBB UUUU UUAA AAAA
// no listed FLAG encodings
//
//                                           PP
// Gen Op Reg+12-bit signed Imm    0010 0bbb 10ii iiii   FBBB ssss ssSS SSSS
// FLAG s12                        0010 0rrr 1010 1001   0RRR ssss ssSS SSSS
//
//                                           PP                      M
// Gen Op Conditional Register     0010 0bbb 11ii iiii   FBBB CCCC CC0Q QQQQ
// FLAG<.cc> c                     0010 0rrr 1110 1001   0RRR CCCC CC0Q QQQQ
// FLAG<.cc> limm                  0010 0rrr 1110 1001   0RRR 1111 100Q QQQQ (+ Limm)
//
//                                           PP                      M
// Gen Op ConReg 6-bit unsign Imm  0010 0bbb 11ii iiii   FBBB UUUU UU1Q QQQQ
// FLAG<.cc> u6                    0010 0rrr 1110 1001   0RRR uuuu uu1Q QQQQ
//
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void arcompact_device::handleop32_FLAG_do_op(uint32_t source)
{
	if (!(source & 0x0001)) // H means ignore all others (and halts CPU?)
	{
		// privileged  mode only
		if (source & 0x0002) { status32_set_e1(); }
		else { status32_clear_e1(); }
		if (source & 0x0004) { status32_set_e2(); }
		else { status32_clear_e2(); }
		//(source & 0x0008)
		//(source & 0x0010)
		//(source & 0x0020)
		//(source & 0x0040)
		//(source & 0x0080)
		if (source & 0x0100) { status32_set_v(); }
		else { status32_clear_v(); }
		if (source & 0x0200) { status32_set_c(); }
		else { status32_clear_c(); }
		if (source & 0x0400) { status32_set_n(); }
		else { status32_clear_n(); }
		if (source & 0x0800) { status32_set_z(); }
		else { status32_clear_z(); }
	}
	else
	{
		fatalerror("FLAG operation with H set (halt CPU?)");
	}
}

uint32_t arcompact_device::handleop32_FLAG(uint32_t op)
{
	switch ((op & 0x00c00000) >> 22)
	{
	case 0x00:
	{
		uint8_t creg = common32_get_creg(op);
		int size = check_limm(creg);
		handleop32_FLAG_do_op(m_regs[creg]);
		return m_pc + size;
	}
	case 0x01:
	{
		handleop32_FLAG_do_op(common32_get_u6(op));
		return m_pc + 4;
	}
	case 0x02:
	{
		handleop32_FLAG_do_op(common32_get_s12(op));
		return m_pc + 4;
	}
	case 0x03:
	{
		switch ((op & 0x00000020) >> 5)
		{
		case 0x00:
		{
			uint8_t creg = common32_get_creg(op);
			int size = check_limm(creg);
			if (check_condition(common32_get_condition(op)))
				handleop32_FLAG_do_op(m_regs[creg]);
			return m_pc + size;
		}
		case 0x01:
		{
			if (check_condition(common32_get_condition(op)))
				handleop32_FLAG_do_op(common32_get_u6(op));
			return m_pc + 4;
		}
		}
		return 0;
	}
	}
	return 0;
}
