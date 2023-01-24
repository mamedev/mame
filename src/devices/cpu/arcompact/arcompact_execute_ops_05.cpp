// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "arcompact.h"
#include "arcompactdasm.h"

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS
// ASL<.f> a,b,c                   0010 1bbb 0000 0000   FBBB CCCC CCAA AAAA
// ASL<.f> a,b,u6                  0010 1bbb 0100 0000   FBBB uuuu uuAA AAAA
// ASL<.f> b,b,s12                 0010 1bbb 1000 0000   FBBB ssss ssSS SSSS
// ASL<.cc><.f> b,b,c              0010 1bbb 1100 0000   FBBB CCCC CC0Q QQQQ
// ASL<.cc><.f> b,b,u6             0010 1bbb 1100 0000   FBBB uuuu uu1Q QQQQ
// ASL<.f> a,limm,c                0010 1110 0000 0000   F111 CCCC CCAA AAAA (+ Limm)
// ASL<.f> a,b,limm                0010 1bbb 0000 0000   FBBB 1111 10AA AAAA (+ Limm)
// ASL<.cc><.f> b,b,limm           0010 1bbb 1100 0000   FBBB 1111 100Q QQQQ (+ Limm)
//
// ASL<.f> 0,b,c                   0010 1bbb 0000 0000   FBBB CCCC CC11 1110
// ASL<.f> 0,b,u6                  0010 1bbb 0100 0000   FBBB uuuu uu11 1110
// ASL<.cc><.f> 0,limm,c           0010 1110 1100 0000   F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_ASL_multiple_do_op(uint32_t src1, uint32_t src2, uint8_t set_flags)
{
	uint32_t result = src1 << (src2&0x1f);
	if (set_flags)
		arcompact_fatal("handleop32_ASL_multiple (ASL) (F set)\n"); // not yet supported
	return result;
}

uint32_t arcompact_device::handleop32_ASL_multiple_f_a_b_c(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	uint8_t creg = common32_get_creg(op);
	int size = check_b_c_limm(breg, creg);
	m_regs[common32_get_areg(op)] = handleop32_ASL_multiple_do_op(m_regs[breg], m_regs[creg], common32_get_F(op));
	return m_pc + size;
}
uint32_t arcompact_device::handleop32_ASL_multiple_f_a_b_u6(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	uint32_t u = common32_get_u6(op);
	int size = check_b_limm(breg);
	m_regs[common32_get_areg(op)] = handleop32_ASL_multiple_do_op(m_regs[breg], u, common32_get_F(op));
	return m_pc + size;
}

uint32_t arcompact_device::handleop32_ASL_multiple_f_b_b_s12(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	uint32_t S = common32_get_s12(op);
	int size = check_b_limm(breg);
	m_regs[breg] = handleop32_ASL_multiple_do_op(m_regs[breg], S, common32_get_F(op));
	return m_pc + size;
}

uint32_t arcompact_device::handleop32_ASL_multiple_cc_f_b_b_c(uint32_t op)
{
	int size = 4;
	arcompact_fatal("handleop32_ASL_multiple_cc_f_b_b_c (ASL)\n");
	return m_pc + size;
}

uint32_t arcompact_device::handleop32_ASL_multiple_cc_f_b_b_u6(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	uint32_t u = common32_get_u6(op);
	int size = check_b_limm(breg);
	if (!check_condition(common32_get_condition(op)))
		return m_pc + size;
	m_regs[breg] = handleop32_ASL_multiple_do_op(m_regs[breg], u, common32_get_F(op));
	return m_pc + size;
}

uint32_t arcompact_device::handleop32_ASL_multiple_cc(uint32_t op)
{
	int M = (op & 0x00000020) >> 5;

	switch (M)
	{
		case 0x00: return handleop32_ASL_multiple_cc_f_b_b_c(op);
		case 0x01: return handleop32_ASL_multiple_cc_f_b_b_u6(op);
	}

	return 0;
}

uint32_t arcompact_device::handleop32_ASL_multiple(uint32_t op)
{
	int p = (op & 0x00c00000) >> 22;

	switch (p)
	{
		case 0x00: return handleop32_ASL_multiple_f_a_b_c(op);
		case 0x01: return handleop32_ASL_multiple_f_a_b_u6(op);
		case 0x02: return handleop32_ASL_multiple_f_b_b_s12(op);
		case 0x03: return handleop32_ASL_multiple_cc(op);
	}

	return 0;
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// LSR<.f> a,b,c                   0010 1bbb 0000 0001   FBBB CCCC CCAA AAAA
// LSR<.f> a,b,u6                  0010 1bbb 0100 0001   FBBB uuuu uuAA AAAA
// LSR<.f> b,b,s12                 0010 1bbb 1000 0001   FBBB ssss ssSS SSSS
// LSR<.cc><.f> b,b,c              0010 1bbb 1100 0001   FBBB CCCC CC0Q QQQQ
// LSR<.cc><.f> b,b,u6             0010 1bbb 1100 0001   FBBB uuuu uu1Q QQQQ
// LSR<.f> a,limm,c                0010 1110 0000 0001   F111 CCCC CCAA AAAA (+ Limm)
// LSR<.f> a,b,limm                0010 1bbb 0000 0001   FBBB 1111 10AA AAAA (+ Limm)
// LSR<.cc><.f> b,b,limm           0010 1bbb 1100 0001   FBBB 1111 100Q QQQQ (+ Limm)
//
// LSR<.f> 0,b,c                   0010 1bbb 0000 0001   FBBB CCCC CC11 1110
// LSR<.f> 0,b,u6                  0010 1bbb 0100 0001   FBBB uuuu uu11 1110
// LSR<.cc><.f> 0,limm,c           0010 1110 1100 0001   F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_LSR_multiple_f_a_b_c(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	uint8_t creg = common32_get_creg(op);
	uint8_t areg = common32_get_areg(op);

	int size = check_b_c_limm(breg, creg);

	uint32_t b = m_regs[breg];
	uint32_t c = m_regs[creg];

	/* todo: is the limm, limm syntax valid? (it's pointless.) */

	uint32_t result = b >> (c&0x1f);
	m_regs[areg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_LSR_multiple (LSR) (F set)\n"); // not yet supported
	}
	return m_pc + size;
}


uint32_t arcompact_device::handleop32_LSR_multiple_f_a_b_u6(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	uint32_t u = common32_get_u6(op);
	uint8_t areg = common32_get_areg(op);

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	int size = check_b_limm(breg);

	uint32_t b = m_regs[breg];
	uint32_t c = u;


	uint32_t result = b >> (c&0x1f);
	m_regs[areg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_LSR_multiple (LSR) (F set)\n"); // not yet supported
	}
	return m_pc + size;
}


uint32_t arcompact_device::handleop32_LSR_multiple_f_b_b_s12(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	uint32_t S = common32_get_s12(op);

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	int size = check_b_limm(breg);


	uint32_t b = m_regs[breg];
	uint32_t c = (uint32_t)S;


	uint32_t result = b >> (c&0x1f);
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_LSR_multiple (LSR) (F set)\n"); // not yet supported
	}
	return m_pc + size;
}


uint32_t arcompact_device::handleop32_LSR_multiple_cc_f_b_b_c(uint32_t op)
{
	int size = 4;
	arcompact_fatal("handleop32_LSR_multiple_cc_f_b_b_c (LSR)\n");
	return m_pc + size;
}


uint32_t arcompact_device::handleop32_LSR_multiple_cc_f_b_b_u6(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	uint32_t u = common32_get_u6(op);


	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	int size = check_b_limm(breg);

	uint32_t b = m_regs[breg];
	uint32_t c = u;


	uint8_t condition = common32_get_condition(op);
	if (!check_condition(condition))
		return m_pc + size;

	uint32_t result = b >> (c&0x1f);
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_LSR_multiple (LSR) (F set)\n"); // not yet supported
	}
	return m_pc + size;
}


uint32_t arcompact_device::handleop32_LSR_multiple(uint32_t op)
{
	int p = (op & 0x00c00000) >> 22;

	switch (p)
	{
		case 0x00: return handleop32_LSR_multiple_f_a_b_c(op);
		case 0x01: return handleop32_LSR_multiple_f_a_b_u6(op);
		case 0x02: return handleop32_LSR_multiple_f_b_b_s12(op);
		case 0x03: return handleop32_LSR_multiple_cc(op);
	}

	return 0;
}

uint32_t arcompact_device::handleop32_LSR_multiple_cc(uint32_t op)
{
	int M = (op & 0x00000020) >> 5;

	switch (M)
	{
		case 0x00: return handleop32_LSR_multiple_cc_f_b_b_c(op);
		case 0x01: return handleop32_LSR_multiple_cc_f_b_b_u6(op);
	}

	return 0;
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ASR<.f> a,b,c                   0010 1bbb 0000 0010   FBBB CCCC CCAA AAAA
// ASR<.f> a,b,u6                  0010 1bbb 0100 0010   FBBB uuuu uuAA AAAA
// ASR<.f> b,b,s12                 0010 1bbb 1000 0010   FBBB ssss ssSS SSSS
// ASR<.cc><.f> b,b,c              0010 1bbb 1100 0010   FBBB CCCC CC0Q QQQQ
// ASR<.cc><.f> b,b,u6             0010 1bbb 1100 0010   FBBB uuuu uu1Q QQQQ
// ASR<.f> a,limm,c                0010 1110 0000 0010   F111 CCCC CCAA AAAA (+ Limm)
// ASR<.f> a,b,limm                0010 1bbb 0000 0010   FBBB 1111 10AA AAAA (+ Limm)
// ASR<.cc><.f> b,b,limm           0010 1bbb 1100 0010   FBBB 1111 100Q QQQQ (+ Limm)
//
// ASR<.f> 0,b,c                   0010 1bbb 0000 0010   FBBB CCCC CC11 1110
// ASR<.f> 0,b,u6                  0010 1bbb 0100 0010   FBBB uuuu uu11 1110
// ASR<.cc><.f> 0,limm,c           0010 1110 1100 0010   F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_ASR_multiple(uint32_t op)  { return arcompact_handle04_helper(op, "ASR", 0,0); }

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ROR<.f> a,b,c                   0010 1bbb 0000 0011   FBBB CCCC CCAA AAAA
// ROR<.f> a,b,u6                  0010 1bbb 0100 0011   FBBB uuuu uuAA AAAA
// ROR<.f> b,b,s12                 0010 1bbb 1000 0011   FBBB ssss ssSS SSSS
// ROR<.cc><.f> b,b,c              0010 1bbb 1100 0011   FBBB CCCC CC0Q QQQQ
// ROR<.cc><.f> b,b,u6             0010 1bbb 1100 0011   FBBB uuuu uu1Q QQQQ
// ROR<.f> a,limm,c                0010 1110 0000 0011   F111 CCCC CCAA AAAA (+ Limm)
// ROR<.f> a,b,limm                0010 1bbb 0000 0011   FBBB 1111 10AA AAAA (+ Limm)
// ROR<.cc><.f> b,b,limm           0010 1bbb 1100 0011   FBBB 1111 100Q QQQQ (+ Limm)
//
// ROR<.f> 0,b,c                   0010 1bbb 0000 0011   FBBB CCCC CC11 1110
// ROR<.f> 0,b,u6                  0010 1bbb 0100 0011   FBBB uuuu uu11 1110
// ROR<.cc><.f> 0,limm,c           0010 1110 1100 0011   F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_ROR_multiple(uint32_t op)  { return arcompact_handle04_helper(op, "ROR", 0,0); }

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// MUL64 <0,>b,c                   0010 1bbb 0000 0100   0BBB CCCC CC11 1110
// MUL64 <0,>b,u6                  0010 1bbb 0100 0100   0BBB uuuu uu11 1110
// MUL64 <0,>b,s12                 0010 1bbb 1000 0100   0BBB ssss ssSS SSSS
// MUL64 <0,>limm,c                0010 1110 0000 0100   0111 CCCC CC11 1110 (+ Limm)
//
// MUL64<.cc> <0,>b,c              0010 1bbb 1100 0100   0BBB CCCC CC0Q QQQQ
// MUL64<.cc> <0,>b,u6             0010 1bbb 1100 0100   0BBB uuuu uu1Q QQQQ
// MUL64<.cc> <0,>limm,c           0010 1110 1100 0100   0111 CCCC CC0Q QQQQ (+ Limm)
// MUL64<.cc> <0,>b,limm           0010 1bbb 1100 0100   0BBB 1111 100Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_MUL64(uint32_t op)  { return arcompact_handle04_helper(op, "MUL64", 2,0); } // special

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// MULU64 <0,>b,c                  0010 1bbb 0000 0101   0BBB CCCC CC11 1110
// MULU64 <0,>b,u6                 0010 1bbb 0100 0101   0BBB uuuu uu11 1110
// MULU64 <0,>b,s12                0010 1bbb 1000 0101   0BBB ssss ssSS SSSS
// MULU64 <0,>limm,c               0010 1110 0000 0101   0111 CCCC CC11 1110 (+ Limm)
//
// MULU64<.cc> <0,>b,c             0010 1bbb 1100 0101   0BBB CCCC CC0Q QQQQ
// MULU64<.cc> <0,>b,u6            0010 1bbb 1100 0101   0BBB uuuu uu1Q QQQQ
// MULU64<.cc> <0,>limm,c          0010 1110 1100 0101   0111 CCCC CC0Q QQQQ (+ Limm)
// MULU64<.cc> <0,>b,limm          0010 1bbb 1100 0101   0BBB 1111 100Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_MULU64(uint32_t op)  { return arcompact_handle04_helper(op, "MULU64", 2,0);} // special

																												   // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ADDS<.f> a,b,c                  0010 1bbb 0000 0110   FBBB CCCC CCAA AAAA
// ADDS<.f> a,b,u6                 0010 1bbb 0100 0110   FBBB uuuu uuAA AAAA
// ADDS<.f> b,b,s12                0010 1bbb 1000 0110   FBBB ssss ssSS SSSS
// ADDS<.cc><.f> b,b,c             0010 1bbb 1100 0110   FBBB CCCC CC0Q QQQQ
// ADDS<.cc><.f> b,b,u6            0010 1bbb 1100 0110   FBBB uuuu uu1Q QQQQ
// ADDS<.f> a,limm,c               0010 1110 0000 0110   F111 CCCC CCAA AAAA (+ Limm)
// ADDS<.f> a,b,limm               0010 1bbb 0000 0110   FBBB 1111 10AA AAAA (+ Limm)
// ADDS<.cc><.f> b,b,limm          0010 1bbb 1100 0110   FBBB 1111 10QQ QQQQ (+ Limm)
//
// ADDS<.f> 0,b,c                  0010 1bbb 0000 0110   FBBB CCCC CC11 1110
// ADDS<.f> 0,b,u6                 0010 1bbb 0100 0110   FBBB uuuu uu11 1110
// ADDS<.f> 0,b,limm               0010 1bbb 0000 0110   FBBB 1111 1011 1110 (+ Limm)
// ADDS<.cc><.f> 0,limm,c          0010 1110 1100 0110   F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_ADDS(uint32_t op)  { return arcompact_handle04_helper(op, "ADDS", 0,0); }


// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// SUBS<.f> a,b,c                  0010 1bbb 0000 0111   FBBB CCCC CCAA AAAA
// SUBS<.f> a,b,u6                 0010 1bbb 0100 0111   FBBB uuuu uuAA AAAA
// SUBS<.f> b,b,s12                0010 1bbb 1000 0111   FBBB ssss ssSS SSSS
// SUBS<.cc><.f> b,b,c             0010 1bbb 1100 0111   FBBB CCCC CC0Q QQQQ
// SUBS<.cc><.f> b,b,u6            0010 1bbb 1100 0111   FBBB uuuu uu1Q QQQQ
// SUBS<.f> a,limm,c               0010 1110 0000 0111   F111 CCCC CCAA AAAA (+ Limm)
// SUBS<.f> a,b,limm               0010 1bbb 0000 0111   FBBB 1111 10AA AAAA (+ Limm)
// SUBS<.cc><.f> b,b,limm          0010 1bbb 1100 0111   FBBB 1111 10QQ QQQQ (+ Limm)
//
// SUBS<.f> 0,b,c                  0010 1bbb 0000 0111   FBBB CCCC CC11 1110
// SUBS<.f> 0,b,u6                 0010 1bbb 0100 0111   FBBB uuuu uu11 1110
// SUBS<.f> 0,b,limm               0010 1bbb 0000 0111   FBBB 1111 1011 1110 (+ Limm)
// SUBS<.cc><.f> 0,limm,c          0010 1110 1100 0111   F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_SUBS(uint32_t op)  { return arcompact_handle04_helper(op, "SUBS", 0,0); }

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// DIVAW a,b,c                     0010 1bbb 0000 1000   0BBB CCCC CCAA AAAA
// DIVAW a,b,u6                    0010 1bbb 0100 1000   0BBB uuuu uuAA AAAA
// DIVAW b,b,s12                   0010 1bbb 1000 1000   0BBB ssss ssSS SSSS
// DIVAW<.cc> b,b,c                0010 1bbb 1100 1000   0BBB CCCC CC0Q QQQQ
// DIVAW<.cc> b,b,u6               0010 1bbb 1100 1000   0BBB uuuu uu1Q QQQQ
// DIVAW a,limm,c                  0010 1110 0000 1000   0111 CCCC CCAA AAAA (+ Limm)
// DIVAW a,b,limm                  0010 1bbb 0000 1000   0BBB 1111 10AA AAAA (+ Limm)
// DIVAW<.cc> b,b,limm             0010 1bbb 1100 1000   0BBB 1111 10QQ QQQQ (+ Limm)
//
// DIVAW 0,b,c                     0010 1bbb 0000 1000   0BBB CCCC CC11 1110
// DIVAW 0,b,u6                    0010 1bbb 0100 1000   0BBB uuuu uu11 1110
// DIVAW<.cc> 0,limm,c             0010 1110 1100 1000   0111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


uint32_t arcompact_device::handleop32_DIVAW(uint32_t op)  { return arcompact_handle04_helper(op, "DIVAW", 0,0); }

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS
// ASLS<.f> a,b,c                  0010 1bbb 0000 1010   FBBB CCCC CCAA AAAA
// ASLS<.f> a,b,u6                 0010 1bbb 0100 1010   FBBB uuuu uuAA AAAA
// ASLS<.f> b,b,s12                0010 1bbb 1000 1010   FBBB ssss ssSS SSSS
// ASLS<.cc><.f> b,b,c             0010 1bbb 1100 1010   FBBB CCCC CC0Q QQQQ
// ASLS<.cc><.f> b,b,u6            0010 1bbb 1100 1010   FBBB uuuu uu1Q QQQQ
// ASLS<.f> a,limm,c               0010 1110 0000 1010   F111 CCCC CCAA AAAA (+ Limm)
// ASLS<.f> a,b,limm               0010 1bbb 0000 1010   FBBB 1111 10AA AAAA (+ Limm)
// ASLS<.cc><.f> b,b,limm          0010 1bbb 1100 1010   FBBB 1111 10QQ QQQQ (+ Limm)
// ASLS<.f> 0,b,c                  0010 1bbb 0000 1010   FBBB CCCC CC11 1110
// ASLS<.f> 0,b,u6                 0010 1bbb 0100 1010   FBBB uuuu uu11 1110
// ASLS<.cc><.f> 0,limm,c          0010 1110 1100 1010   F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_ASLS(uint32_t op)  { return arcompact_handle04_helper(op, "ASLS", 0,0); }

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ASRS<.f> a,b,c                  0010 1bbb 0000 1011   FBBB CCCC CCAA AAAA
// ASRS<.f> a,b,u6                 0010 1bbb 0100 1011   FBBB uuuu uuAA AAAA
// ASRS<.f> b,b,s12                0010 1bbb 1000 1011   FBBB ssss ssSS SSSS
// ASRS<.cc><.f> b,b,c             0010 1bbb 1100 1011   FBBB CCCC CC0Q QQQQ
// ASRS<.cc><.f> b,b,u6            0010 1bbb 1100 1011   FBBB uuuu uu1Q QQQQ
// ASRS<.f> a,limm,c               0010 1110 0000 1011   F111 CCCC CCAA AAAA (+ Limm)
// ASRS<.f> a,b,limm               0010 1bbb 0000 1011   FBBB 1111 10AA AAAA (+ Limm)
// ASRS<.cc><.f> b,b,limm          0010 1bbb 1100 1011   FBBB 1111 10QQ QQQQ (+ Limm)
//
// ASRS<.f> 0,b,c                  0010 1bbb 0000 1011   FBBB CCCC CC11 1110
// ASRS<.f> 0,b,u6                 0010 1bbb 0100 1011   FBBB uuuu uu11 1110
// ASRS<.cc><.f> 0,limm,c          0010 1110 1100 1011   F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_ASRS(uint32_t op)  { return arcompact_handle04_helper(op, "ASRS", 0,0); }

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ADDSDW<.f> a,b,c                0010 1bbb 0010 1000   FBBB CCCC CCAA AAAA
// ADDSDW<.f> a,b,u6               0010 1bbb 0110 1000   FBBB uuuu uuAA AAAA
// ADDSDW<.f> b,b,s12              0010 1bbb 1010 1000   FBBB ssss ssSS SSSS
// ADDSDW<.cc><.f> b,b,c           0010 1bbb 1110 1000   FBBB CCCC CC0Q QQQQ
// ADDSDW<.cc><.f> b,b,u6          0010 1bbb 1110 1000   FBBB uuuu uu1Q QQQQ
// ADDSDW<.f> a,limm,c             0010 1110 0010 1000   F111 CCCC CCAA AAAA (+ Limm)
// ADDSDW<.f> a,b,limm             0010 1bbb 0010 1000   FBBB 1111 10AA AAAA (+ Limm)
// ADDSDW<.cc><.f> b,b,limm        0010 1bbb 1110 1000   FBBB 1111 10QQ QQQQ (+ Limm)
//
// ADDSDW<.f> 0,b,c                0010 1bbb 0010 1000   FBBB CCCC CC11 1110
// ADDSDW<.f> 0,b,u6               0010 1bbb 0110 1000   FBBB uuuu uu11 1110
// ADDSDW<.cc><.f> 0,limm,c        0010 1110 1110 1000   F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_ADDSDW(uint32_t op)  { return arcompact_handle04_helper(op, "ADDSDW", 0,0); }

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// SUBSDW<.f> a,b,c                0010 1bbb 0010 1001   FBBB CCCC CCAA AAAA
// SUBSDW<.f> a,b,u6               0010 1bbb 0110 1001   FBBB uuuu uuAA AAAA
// SUBSDW<.f> b,b,s12              0010 1bbb 1010 1001   FBBB ssss ssSS SSSS
// SUBSDW<.cc><.f> b,b,c           0010 1bbb 1110 1001   FBBB CCCC CC0Q QQQQ
// SUBSDW<.cc><.f> b,b,u6          0010 1bbb 1110 1001   FBBB uuuu uu1Q QQQQ
// SUBSDW<.f> a,limm,c             0010 1110 0010 1001   F111 CCCC CCAA AAAA (+ Limm)
// SUBSDW<.f> a,b,limm             0010 1bbb 0010 1001   FBBB 1111 10AA AAAA (+ Limm)
// SUBSDW<.cc><.f> b,b,limm        0010 1bbb 1110 1001   FBBB 1111 10QQ QQQQ (+ Limm)
//
// SUBSDW<.f> 0,b,c                0010 1bbb 0010 1001   FBBB CCCC CC11 1110
// SUBSDW<.f> 0,b,u6               0010 1bbb 0110 1001   FBBB uuuu uu11 1110
// SUBSDW<.cc><.f> 0,limm,c        0010 1110 1110 1001   F111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_SUBSDW(uint32_t op)  { return arcompact_handle04_helper(op, "SUBSDW", 0,0); }


