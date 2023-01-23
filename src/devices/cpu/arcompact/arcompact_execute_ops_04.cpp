// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "arcompact.h"
#include "arcompactdasm.h"




void arcompact_device::do_flags(uint32_t result, uint32_t b, uint32_t c)
{
	do_flags_nz(result);

	if ((b & 0x80000000) == (c & 0x80000000))
	{
		if ((result & 0x80000000) != (b & 0x80000000))
		{
			status32_set_v();
		}
		else
		{
			status32_clear_v();
		}
	}
	if (result < b)
	{
		status32_set_c();
	}
	else
	{
		status32_clear_c();
	}
}

void arcompact_device::do_flags_nz(uint32_t result)
{
	if (result & 0x80000000) { status32_set_n(); }
	else { status32_clear_n(); }
	if (result == 0x00000000) { status32_set_z(); }
	else { status32_clear_z(); }
}


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

uint32_t arcompact_device::handleop32_ADD_f_a_b_c(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	uint8_t creg = common32_get_creg(op);
	int size = check_b_c_limm(breg, creg);
	uint8_t F = common32_get_F(op);
	uint8_t areg = common32_get_areg(op);
	uint32_t b = m_regs[breg];
	uint32_t c = m_regs[creg];

	/* todo: is the limm, limm syntax valid? (it's pointless.) */

	uint32_t result = b + c;
	m_regs[areg] = result;

	if (F)
		do_flags(result, b, c);

	return m_pc + (size >> 0);
}

uint32_t arcompact_device::handleop32_ADD_f_a_b_u6(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	uint32_t u = common32_get_u6(op);
	uint8_t areg = common32_get_areg(op);

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	int size = check_b_limm(breg);

	uint32_t b = m_regs[breg];
	uint32_t c = u;


	uint32_t result = b + c;
	m_regs[areg] = result;

	if (F)
		do_flags(result, b, c);

	return m_pc + (size >> 0);
}

uint32_t arcompact_device::handleop32_ADD_f_b_b_s12(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	uint32_t S = common32_get_s12(op);

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	int size = check_b_limm(breg);

	uint32_t b = m_regs[breg];
	uint32_t c = (uint32_t)S;


	uint32_t result = b + c;
	m_regs[breg] = result;

	if (F)
		do_flags(result, b, c);

	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_ADD_cc_f_b_b_c(uint32_t op)
{
	int size = 4;
	arcompact_fatal("handleop32_ADD_cc_f_b_b_c (ADD)\n");
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_ADD_cc_f_b_b_u6(uint32_t op)
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
		return m_pc + (size>>0);

	uint32_t result = b + c;
	m_regs[breg] = result;

	if (F)
		do_flags(result, b, c);

	return m_pc + (size >> 0);
}

uint32_t arcompact_device::handleop32_ADD_cc(uint32_t op)
{
	int M = (op & 0x00000020) >> 5;

	switch (M)
	{
		case 0x00: return handleop32_ADD_cc_f_b_b_c(op);
		case 0x01: return handleop32_ADD_cc_f_b_b_u6(op);
	}

	return 0;
}

uint32_t arcompact_device::handleop32_ADD(uint32_t op)
{
	int p = (op & 0x00c00000) >> 22;

	switch (p)
	{
		case 0x00: return handleop32_ADD_f_a_b_c(op);
		case 0x01: return handleop32_ADD_f_a_b_u6(op);
		case 0x02: return handleop32_ADD_f_b_b_s12(op);
		case 0x03: return handleop32_ADD_cc(op);
	}

	return 0;
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


uint32_t arcompact_device::handleop32_ADC_f_a_b_c(uint32_t op)
{
	arcompact_fatal("ADC with P00 not supported");
	return m_pc + (0 >> 0);
}

uint32_t arcompact_device::handleop32_ADC_f_a_b_u6(uint32_t op)
{
	arcompact_fatal("ADC with P01 not supported");
	return m_pc + (0 >> 0);
}

uint32_t arcompact_device::handleop32_ADC_f_b_b_s12(uint32_t op)
{
	arcompact_fatal("ADC with P10 not supported");
	return m_pc + (0 >> 0);
}

uint32_t arcompact_device::handleop32_ADC_cc_f_b_b_c(uint32_t op)
{
	arcompact_fatal("ADC with P11 M0 not supported");
	return m_pc + (0 >> 0);
}

uint32_t arcompact_device::handleop32_ADC_cc_f_b_b_u6(uint32_t op)
{
	arcompact_fatal("ADC with P11 M0 not supported");
	return m_pc + (0 >> 0);
}

uint32_t arcompact_device::handleop32_ADC_cc(uint32_t op)
{
	int M = (op & 0x00000020) >> 5;

	switch (M)
	{
		case 0x00: return handleop32_ADC_cc_f_b_b_c(op);
		case 0x01: return handleop32_ADC_cc_f_b_b_u6(op);
	}

	return 0;
}

uint32_t arcompact_device::handleop32_ADC(uint32_t op)
{
	int p = (op & 0x00c00000) >> 22;

	switch (p)
	{
		case 0x00: return handleop32_ADC_f_a_b_c(op);
		case 0x01: return handleop32_ADC_f_a_b_u6(op);
		case 0x02: return handleop32_ADC_f_b_b_s12(op);
		case 0x03: return handleop32_ADC_cc(op);
	}

	return 0;
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

uint32_t arcompact_device::handleop32_SUB_f_a_b_c(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	uint8_t creg = common32_get_creg(op);
	uint8_t areg = common32_get_areg(op);

	int size = check_b_c_limm(breg, creg);

	uint32_t b = m_regs[breg];
	uint32_t c = m_regs[creg];

	/* todo: is the limm, limm syntax valid? (it's pointless.) */

	uint32_t result = b - c;
	m_regs[areg] = result;

	if (F)
		do_flags(result, b, c);

	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_SUB_f_a_b_u6(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	uint32_t u = common32_get_u6(op);
	uint8_t areg = common32_get_areg(op);

	int size = check_b_limm(breg);

	uint32_t b = m_regs[breg];
	uint32_t c = u;


	uint32_t result = b - c;
	m_regs[areg] = result;

	if (F)
		do_flags(result, b, c);

	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_SUB_f_b_b_s12(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	int size = check_b_limm(breg);
	uint8_t F = common32_get_F(op);
	uint32_t S = common32_get_s12(op);
	uint32_t b = m_regs[breg];
	uint32_t c = S;
	uint32_t result = b - c;
	m_regs[breg] = result;

	if (F)
		do_flags(result, b, c);

	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_SUB_cc_f_b_b_c(uint32_t op)
{
	int size = 4;
	arcompact_fatal("handleop32_SUB_cc_f_b_b_c (SUB)\n");
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_SUB_cc_f_b_b_u6(uint32_t op)
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
		return m_pc + (size>>0);

	uint32_t result = b - c;
	m_regs[breg] = result;

	if (F)
		do_flags(result, b, c);

	return m_pc + (size >> 0);
}

uint32_t arcompact_device::handleop32_SUB_cc(uint32_t op)
{
	int M = (op & 0x00000020) >> 5;

	switch (M)
	{
		case 0x00: return handleop32_SUB_cc_f_b_b_c(op);
		case 0x01: return handleop32_SUB_cc_f_b_b_u6(op);
	}

	return 0;
}

uint32_t arcompact_device::handleop32_SUB(uint32_t op)
{
	int p = (op & 0x00c00000) >> 22;

	switch (p)
	{
		case 0x00: return handleop32_SUB_f_a_b_c(op);
		case 0x01: return handleop32_SUB_f_a_b_u6(op);
		case 0x02: return handleop32_SUB_f_b_b_s12(op);
		case 0x03: return handleop32_SUB_cc(op);
	}

	return 0;
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


uint32_t arcompact_device::handleop32_SBC_f_a_b_c(uint32_t op)
{
	arcompact_fatal("SBC with P00 not supported");
	return m_pc + (0 >> 0);
}

uint32_t arcompact_device::handleop32_SBC_f_a_b_u6(uint32_t op)
{
	arcompact_fatal("SBC with P01 not supported");
	return m_pc + (0 >> 0);
}

uint32_t arcompact_device::handleop32_SBC_f_b_b_s12(uint32_t op)
{
	arcompact_fatal("SBC with P10 not supported");
	return m_pc + (0 >> 0);
}

uint32_t arcompact_device::handleop32_SBC_cc_f_b_b_c(uint32_t op)
{
	arcompact_fatal("SBC with P11 M0 not supported");
	return m_pc + (0 >> 0);
}

uint32_t arcompact_device::handleop32_SBC_cc_f_b_b_u6(uint32_t op)
{
	arcompact_fatal("SBC with P11 M0 not supported");
	return m_pc + (0 >> 0);
}

uint32_t arcompact_device::handleop32_SBC_cc(uint32_t op)
{
	int M = (op & 0x00000020) >> 5;

	switch (M)
	{
		case 0x00: return handleop32_SBC_cc_f_b_b_c(op);
		case 0x01: return handleop32_SBC_cc_f_b_b_u6(op);
	}

	return 0;
}

uint32_t arcompact_device::handleop32_SBC(uint32_t op)
{
	int p = (op & 0x00c00000) >> 22;

	switch (p)
	{
		case 0x00: return handleop32_SBC_f_a_b_c(op);
		case 0x01: return handleop32_SBC_f_a_b_u6(op);
		case 0x02: return handleop32_SBC_f_b_b_s12(op);
		case 0x03: return handleop32_SBC_cc(op);
	}

	return 0;
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

uint32_t arcompact_device::handleop32_AND_f_a_b_c(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	uint8_t creg = common32_get_creg(op);
	uint8_t areg = common32_get_areg(op);

	int size = check_b_c_limm(breg, creg);

	uint32_t b = m_regs[breg];
	uint32_t c = m_regs[creg];

	/* todo: is the limm, limm syntax valid? (it's pointless.) */

	uint32_t result = b & c;
	if (areg != LIMM_REG) { m_regs[areg] = result; }

	if (F)
		do_flags_nz(result);

	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_AND_f_a_b_u6(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	uint32_t u = common32_get_u6(op);
	uint8_t areg = common32_get_areg(op);

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	int size = check_b_limm(breg);

	uint32_t b = m_regs[breg];
	uint32_t c = u;

	uint32_t result = b & c;
	if (areg != LIMM_REG) { m_regs[areg] = result; }

	if (F)
		do_flags_nz(result);

	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_AND_f_b_b_s12(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	uint32_t S = common32_get_s12(op);

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	int size = check_b_limm(breg);

	uint32_t b = m_regs[breg];
	uint32_t c = (uint32_t)S;


	uint32_t result = b & c;
	if (breg != LIMM_REG) { m_regs[breg] = result; }

	if (F)
		do_flags_nz(result);

	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_AND_cc_f_b_b_c(uint32_t op)
{
	int size = 4;
	arcompact_fatal("handleop32_AND_cc_f_b_b_c (AND)\n");
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_AND_cc_f_b_b_u6(uint32_t op)
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
		return m_pc + (size>>0);

	uint32_t result = b & c;
	if (breg != LIMM_REG) { m_regs[breg] = result; }

	if (F)
		do_flags_nz(result);

	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_AND_cc(uint32_t op)
{
	int M = (op & 0x00000020) >> 5;

	switch (M)
	{
		case 0x00: return handleop32_AND_cc_f_b_b_c(op);
		case 0x01: return handleop32_AND_cc_f_b_b_u6(op);
	}

	return 0;
}

uint32_t arcompact_device::handleop32_AND(uint32_t op)
{
	int p = (op & 0x00c00000) >> 22;

	switch (p)
	{
		case 0x00: return handleop32_AND_f_a_b_c(op);
		case 0x01: return handleop32_AND_f_a_b_u6(op);
		case 0x02: return handleop32_AND_f_b_b_s12(op);
		case 0x03: return handleop32_AND_cc(op);
	}

	return 0;
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

uint32_t arcompact_device::handleop32_OR_f_a_b_c(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	uint8_t creg = common32_get_creg(op);
	uint8_t areg = common32_get_areg(op);

	int size = check_b_c_limm(breg, creg);

	uint32_t b = m_regs[breg];
	uint32_t c = m_regs[creg];

	/* todo: is the limm, limm syntax valid? (it's pointless.) */

	uint32_t result = b | c;
	m_regs[areg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_OR (OR) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_OR_f_a_b_u6(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	uint32_t u = common32_get_u6(op);
	uint8_t areg = common32_get_areg(op);

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	int size = check_b_limm(breg);

	uint32_t b = m_regs[breg];
	uint32_t c = u;


	uint32_t result = b | c;
	m_regs[areg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_OR (OR) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_OR_f_b_b_s12(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	uint32_t S = common32_get_s12(op);

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	int size = check_b_limm(breg);

	uint32_t b = m_regs[breg];
	uint32_t c = (uint32_t)S;


	uint32_t result = b | c;
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_OR (OR) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_OR_cc_f_b_b_c(uint32_t op)
{
	int size = 4;
	arcompact_fatal("handleop32_OR_cc_f_b_b_c (OR)\n");
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_OR_cc_f_b_b_u6(uint32_t op)
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
		return m_pc + (size>>0);

	uint32_t result = b | c;
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_OR (OR) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_OR_cc(uint32_t op)
{
	int M = (op & 0x00000020) >> 5;

	switch (M)
	{
		case 0x00: return handleop32_OR_cc_f_b_b_c(op);
		case 0x01: return handleop32_OR_cc_f_b_b_u6(op);
	}

	return 0;
}

uint32_t arcompact_device::handleop32_OR(uint32_t op)
{
	int p = (op & 0x00c00000) >> 22;

	switch (p)
	{
		case 0x00: return handleop32_OR_f_a_b_c(op);
		case 0x01: return handleop32_OR_f_a_b_u6(op);
		case 0x02: return handleop32_OR_f_b_b_s12(op);
		case 0x03: return handleop32_OR_cc(op);
	}

	return 0;
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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

uint32_t arcompact_device::handleop32_BIC_f_a_b_c(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	uint8_t creg = common32_get_creg(op);
	uint8_t areg = common32_get_areg(op);

	int size = check_b_c_limm(breg, creg);

	uint32_t b = m_regs[breg];
	uint32_t c = m_regs[creg];

	/* todo: is the limm, limm syntax valid? (it's pointless.) */

	uint32_t result = b & (~c);
	m_regs[areg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_BIC (BIC) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_BIC_f_a_b_u6(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	uint32_t u = common32_get_u6(op);
	uint8_t areg = common32_get_areg(op);

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	int size = check_b_limm(breg);

	uint32_t b = m_regs[breg];
	uint32_t c = u;


	uint32_t result = b & (~c);
	m_regs[areg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_BIC (BIC) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_BIC_f_b_b_s12(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	uint32_t S = common32_get_s12(op);

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	int size = check_b_limm(breg);

	uint32_t b = m_regs[breg];
	uint32_t c = (uint32_t)S;


	uint32_t result = b & (~c);
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_BIC (BIC) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_BIC_cc_f_b_b_c(uint32_t op)
{
	int size = 4;
	arcompact_fatal("handleop32_BIC_cc_f_b_b_c (BIC)\n");
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_BIC_cc_f_b_b_u6(uint32_t op)
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
		return m_pc + (size>>0);

	uint32_t result = b & (~c);
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_BIC (BIC) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}

uint32_t arcompact_device::handleop32_BIC_cc(uint32_t op)
{
	int M = (op & 0x00000020) >> 5;

	switch (M)
	{
		case 0x00: return handleop32_BIC_cc_f_b_b_c(op);
		case 0x01: return handleop32_BIC_cc_f_b_b_u6(op);
	}

	return 0;
}

uint32_t arcompact_device::handleop32_BIC(uint32_t op)
{
	int p = (op & 0x00c00000) >> 22;

	switch (p)
	{
		case 0x00: return handleop32_BIC_f_a_b_c(op);
		case 0x01: return handleop32_BIC_f_a_b_u6(op);
		case 0x02: return handleop32_BIC_f_b_b_s12(op);
		case 0x03: return handleop32_BIC_cc(op);
	}

	return 0;
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

uint32_t arcompact_device::handleop32_XOR_f_a_b_c(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	uint8_t creg = common32_get_creg(op);
	uint8_t areg = common32_get_areg(op);

	int size = check_b_c_limm(breg, creg);

	uint32_t b = m_regs[breg];
	uint32_t c = m_regs[creg];

	/* todo: is the limm, limm syntax valid? (it's pointless.) */

	uint32_t result = b ^ c;
	m_regs[areg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_XOR (XOR) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_XOR_f_a_b_u6(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	uint32_t u = common32_get_u6(op);
	uint8_t areg = common32_get_areg(op);

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	int size = check_b_limm(breg);

	uint32_t b = m_regs[breg];
	uint32_t c = u;


	uint32_t result = b ^ c;
	m_regs[areg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_XOR (XOR) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_XOR_f_b_b_s12(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	uint32_t S = common32_get_s12(op);

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	int size = check_b_limm(breg);

	uint32_t b = m_regs[breg];
	uint32_t c = (uint32_t)S;


	uint32_t result = b ^ c;
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_XOR (XOR) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_XOR_cc_f_b_b_c(uint32_t op)
{
	int size = 4;
	arcompact_fatal("handleop32_XOR_cc_f_b_b_c (XOR)\n");
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_XOR_cc_f_b_b_u6(uint32_t op)
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
		return m_pc + (size>>0);

	uint32_t result = b ^ c;
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_XOR (XOR) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_XOR_cc(uint32_t op)
{
	int M = (op & 0x00000020) >> 5;

	switch (M)
	{
		case 0x00: return handleop32_XOR_cc_f_b_b_c(op);
		case 0x01: return handleop32_XOR_cc_f_b_b_u6(op);
	}

	return 0;
}

uint32_t arcompact_device::handleop32_XOR(uint32_t op)
{
	int p = (op & 0x00c00000) >> 22;

	switch (p)
	{
		case 0x00: return handleop32_XOR_f_a_b_c(op);
		case 0x01: return handleop32_XOR_f_a_b_u6(op);
		case 0x02: return handleop32_XOR_f_b_b_s12(op);
		case 0x03: return handleop32_XOR_cc(op);
	}

	return 0;
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


uint32_t arcompact_device::handleop32_MAX_f_a_b_c(uint32_t op)
{
	arcompact_fatal("MAX with P00 not supported");
	return m_pc + (0 >> 0);
}

uint32_t arcompact_device::handleop32_MAX_f_a_b_u6(uint32_t op)
{
	arcompact_fatal("MAX with P01 not supported");
	return m_pc + (0 >> 0);
}

uint32_t arcompact_device::handleop32_MAX_f_b_b_s12(uint32_t op)
{
	arcompact_fatal("MAX with P10 not supported");
	return m_pc + (0 >> 0);
}

uint32_t arcompact_device::handleop32_MAX_cc_f_b_b_c(uint32_t op)
{
	arcompact_fatal("MAX with P11 M0 not supported");
	return m_pc + (0 >> 0);
}

uint32_t arcompact_device::handleop32_MAX_cc_f_b_b_u6(uint32_t op)
{
	arcompact_fatal("MAX with P11 M0 not supported");
	return m_pc + (0 >> 0);
}

uint32_t arcompact_device::handleop32_MAX_cc(uint32_t op)
{
	int M = (op & 0x00000020) >> 5;

	switch (M)
	{
		case 0x00: return handleop32_MAX_cc_f_b_b_c(op);
		case 0x01: return handleop32_MAX_cc_f_b_b_u6(op);
	}

	return 0;
}

uint32_t arcompact_device::handleop32_MAX(uint32_t op)
{
	int p = (op & 0x00c00000) >> 22;

	switch (p)
	{
		case 0x00: return handleop32_MAX_f_a_b_c(op);
		case 0x01: return handleop32_MAX_f_a_b_u6(op);
		case 0x02: return handleop32_MAX_f_b_b_s12(op);
		case 0x03: return handleop32_MAX_cc(op);
	}

	return 0;
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


uint32_t arcompact_device::handleop32_MIN_f_a_b_c(uint32_t op)
{
	arcompact_fatal("MIN with P00 not supported");
	return m_pc + (0 >> 0);
}

uint32_t arcompact_device::handleop32_MIN_f_a_b_u6(uint32_t op)
{
	arcompact_fatal("MIN with P01 not supported");
	return m_pc + (0 >> 0);
}

uint32_t arcompact_device::handleop32_MIN_f_b_b_s12(uint32_t op)
{
	arcompact_fatal("MIN with P10 not supported");
	return m_pc + (0 >> 0);
}

uint32_t arcompact_device::handleop32_MIN_cc_f_b_b_c(uint32_t op)
{
	arcompact_fatal("MIN with P11 M0 not supported");
	return m_pc + (0 >> 0);
}

uint32_t arcompact_device::handleop32_MIN_cc_f_b_b_u6(uint32_t op)
{
	arcompact_fatal("MIN with P11 M0 not supported");
	return m_pc + (0 >> 0);
}

uint32_t arcompact_device::handleop32_MIN_cc(uint32_t op)
{
	int M = (op & 0x00000020) >> 5;

	switch (M)
	{
		case 0x00: return handleop32_MIN_cc_f_b_b_c(op);
		case 0x01: return handleop32_MIN_cc_f_b_b_u6(op);
	}

	return 0;
}

uint32_t arcompact_device::handleop32_MIN(uint32_t op)
{
	int p = (op & 0x00c00000) >> 22;

	switch (p)
	{
		case 0x00: return handleop32_MIN_f_a_b_c(op);
		case 0x01: return handleop32_MIN_f_a_b_u6(op);
		case 0x02: return handleop32_MIN_f_b_b_s12(op);
		case 0x03: return handleop32_MIN_cc(op);
	}

	return 0;
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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
// NOP                             0010 0110 0100 1010   0111 0000 0000 0000 (NOP is a custom encoded MOV where b is 'LIMM')
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_MOV_f_a_b_c(uint32_t op)
{
	int size = 4;

	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	uint8_t creg = common32_get_creg(op);
	 //uint8_t areg = common32_get_areg(op); // areg is reserved / not used

	if (creg == LIMM_REG)
	{
		get_limm_32bit_opcode();
		size = 8;
	}

	uint32_t c = m_regs[creg];
	/* todo: is the limm, limm syntax valid? (it's pointless.) */

	uint32_t result = c;
	m_regs[breg] = result;

	if (F)
		do_flags_nz(result);

	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_MOV_f_a_b_u6(uint32_t op)
{
	int size = 4;

	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	uint32_t u = common32_get_u6(op);
	 //uint8_t areg = common32_get_areg(op); // areg is reserved / not used

	uint32_t c = u;


	uint32_t result = c;
	m_regs[breg] = result;

	if (F)
		do_flags_nz(result);

	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_MOV_f_b_b_s12(uint32_t op)
{
	int size = 4;

	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	uint32_t S = common32_get_s12(op);

	uint32_t c = (uint32_t)S;


	uint32_t result = c;
	m_regs[breg] = result;

	if (F)
		do_flags_nz(result);

	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_MOV_cc_f_b_b_c(uint32_t op)
{
	int size = 4;
	arcompact_fatal("handleop32_MOV_cc_f_b_b_c (MOV)\n");
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_MOV_cc_f_b_b_u6(uint32_t op)
{
	int size = 4;

	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	uint32_t u = common32_get_u6(op);


	uint32_t c = u;


	uint8_t condition = common32_get_condition(op);
	if (!check_condition(condition))
		return m_pc + (size>>0);

	uint32_t result = c;
	m_regs[breg] = result;

	if (F)
		do_flags_nz(result);

	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_MOV_cc(uint32_t op)
{
	int M = (op & 0x00000020) >> 5;

	switch (M)
	{
		case 0x00: return handleop32_MOV_cc_f_b_b_c(op);
		case 0x01: return handleop32_MOV_cc_f_b_b_u6(op);
	}

	return 0;
}

uint32_t arcompact_device::handleop32_MOV(uint32_t op)
{
	int p = (op & 0x00c00000) >> 22;

	switch (p)
	{
		case 0x00: return handleop32_MOV_f_a_b_c(op);
		case 0x01: return handleop32_MOV_f_a_b_u6(op);
		case 0x02: return handleop32_MOV_f_b_b_s12(op);
		case 0x03: return handleop32_MOV_cc(op);
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


uint32_t arcompact_device::handleop32_TST_f_a_b_c(uint32_t op)
{
	arcompact_fatal("TST with P00 not supported");
	return m_pc + (0 >> 0);
}

uint32_t arcompact_device::handleop32_TST_f_a_b_u6(uint32_t op)
{
	arcompact_fatal("TST with P01 not supported");
	return m_pc + (0 >> 0);
}

uint32_t arcompact_device::handleop32_TST_f_b_b_s12(uint32_t op)
{
	arcompact_fatal("TST with P10 not supported");
	return m_pc + (0 >> 0);
}

uint32_t arcompact_device::handleop32_TST_cc_f_b_b_c(uint32_t op)
{
	arcompact_fatal("TST with P11 M0 not supported");
	return m_pc + (0 >> 0);
}

uint32_t arcompact_device::handleop32_TST_cc_f_b_b_u6(uint32_t op)
{
	arcompact_fatal("TST with P11 M0 not supported");
	return m_pc + (0 >> 0);
}

uint32_t arcompact_device::handleop32_TST_cc(uint32_t op)
{
	int M = (op & 0x00000020) >> 5;

	switch (M)
	{
		case 0x00: return handleop32_TST_cc_f_b_b_c(op);
		case 0x01: return handleop32_TST_cc_f_b_b_u6(op);
	}

	return 0;
}

uint32_t arcompact_device::handleop32_TST(uint32_t op)
{
	int p = (op & 0x00c00000) >> 22;

	switch (p)
	{
		case 0x00: return handleop32_TST_f_a_b_c(op);
		case 0x01: return handleop32_TST_f_a_b_u6(op);
		case 0x02: return handleop32_TST_f_b_b_s12(op);
		case 0x03: return handleop32_TST_cc(op);
	}

	return 0;
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

uint32_t arcompact_device::handleop32_CMP_f_a_b_c(uint32_t op)
{
	arcompact_fatal("CMP with P00 not supported");
	return m_pc + (0 >> 0);
}

uint32_t arcompact_device::handleop32_CMP_f_a_b_u6(uint32_t op)
{
	arcompact_fatal("CMP with P01 not supported");
	return m_pc + (0 >> 0);
}

uint32_t arcompact_device::handleop32_CMP_f_b_b_s12(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	int size = check_b_limm(breg);
	uint32_t S = common32_get_s12(op);
	uint32_t b = m_regs[breg];
	uint32_t c = S;
	uint32_t result = b - c;
	do_flags(result, b, c);
	return m_pc + (size >> 0);
}

uint32_t arcompact_device::handleop32_CMP_cc_f_b_b_c(uint32_t op)
{
	arcompact_fatal("CMP with P11 M0 not supported");
	return m_pc + (0 >> 0);
}

uint32_t arcompact_device::handleop32_CMP_cc_f_b_b_u6(uint32_t op)
{
	arcompact_fatal("CMP with P11 M0 not supported");
	return m_pc + (0 >> 0);
}

uint32_t arcompact_device::handleop32_CMP_cc(uint32_t op)
{
	int M = (op & 0x00000020) >> 5;

	switch (M)
	{
		case 0x00: return handleop32_CMP_cc_f_b_b_c(op);
		case 0x01: return handleop32_CMP_cc_f_b_b_u6(op);
	}

	return 0;
}

uint32_t arcompact_device::handleop32_CMP(uint32_t op)
{
	int p = (op & 0x00c00000) >> 22;

	switch (p)
	{
		case 0x00: return handleop32_CMP_f_a_b_c(op);
		case 0x01: return handleop32_CMP_f_a_b_u6(op);
		case 0x02: return handleop32_CMP_f_b_b_s12(op);
		case 0x03: return handleop32_CMP_cc(op);
	}

	return 0;
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// RCMP b,s12                      0010 0bbb 1000 1101   1BBB ssss ssSS SSSS
// RCMP<.cc> b,c                   0010 0bbb 1100 1101   1BBB CCCC CC0Q QQQQ
// RCMP<.cc> b,u6                  0010 0bbb 1100 1101   1BBB uuuu uu1Q QQQQ
// RCMP<.cc> b,limm                0010 0bbb 1100 1101   1BBB 1111 100Q QQQQ (+ Limm)
// RCMP<.cc> limm,c                0010 0110 1100 1101   1111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_RCMP_f_a_b_c(uint32_t op)
{
	arcompact_fatal("RCMP with P00 not supported");
	return m_pc + (0 >> 0);
}

uint32_t arcompact_device::handleop32_RCMP_f_a_b_u6(uint32_t op)
{
	arcompact_fatal("RCMP with P01 not supported");
	return m_pc + (0 >> 0);
}

uint32_t arcompact_device::handleop32_RCMP_f_b_b_s12(uint32_t op)
{
	arcompact_fatal("RCMP with P10 not supported");
	return m_pc + (0 >> 0);
}

uint32_t arcompact_device::handleop32_RCMP_cc_f_b_b_c(uint32_t op)
{
	arcompact_fatal("RCMP with P11 M0 not supported");
	return m_pc + (0 >> 0);
}

uint32_t arcompact_device::handleop32_RCMP_cc_f_b_b_u6(uint32_t op)
{
	arcompact_fatal("RCMP with P11 M0 not supported");
	return m_pc + (0 >> 0);
}

uint32_t arcompact_device::handleop32_RCMP_cc(uint32_t op)
{
	int M = (op & 0x00000020) >> 5;

	switch (M)
	{
		case 0x00: return handleop32_RCMP_cc_f_b_b_c(op);
		case 0x01: return handleop32_RCMP_cc_f_b_b_u6(op);
	}

	return 0;
}

uint32_t arcompact_device::handleop32_RCMP(uint32_t op)
{
	int p = (op & 0x00c00000) >> 22;

	switch (p)
	{
		case 0x00: return handleop32_RCMP_f_a_b_c(op);
		case 0x01: return handleop32_RCMP_f_a_b_u6(op);
		case 0x02: return handleop32_RCMP_f_b_b_s12(op);
		case 0x03: return handleop32_RCMP_cc(op);
	}

	return 0;
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

uint32_t arcompact_device::handleop32_RSUB_f_a_b_c(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	uint8_t creg = common32_get_creg(op);
	uint8_t areg = common32_get_areg(op);

	int size = check_b_c_limm(breg, creg);

	uint32_t b = m_regs[breg];
	uint32_t c = m_regs[creg];

	/* todo: is the limm, limm syntax valid? (it's pointless.) */

	uint32_t result = c - b;
	m_regs[areg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_RSUB (RSUB) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_RSUB_f_a_b_u6(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	uint32_t u = common32_get_u6(op);
	uint8_t areg = common32_get_areg(op);

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	int size = check_b_limm(breg);

	uint32_t b = m_regs[breg];
	uint32_t c = u;


	uint32_t result = c - b;
	m_regs[areg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_RSUB (RSUB) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_RSUB_f_b_b_s12(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	uint32_t S = common32_get_s12(op);

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	int size = check_b_limm(breg);

	uint32_t b = m_regs[breg];
	uint32_t c = (uint32_t)S;


	uint32_t result = c - b;
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_RSUB (RSUB) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_RSUB_cc_f_b_b_c(uint32_t op)
{
	int size = 4;
	arcompact_fatal("handleop32_RSUB_cc_f_b_b_c (RSUB)\n");
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_RSUB_cc_f_b_b_u6(uint32_t op)
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
		return m_pc + (size>>0);

	uint32_t result = c - b;
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_RSUB (RSUB) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_RSUB_cc(uint32_t op)
{
	int M = (op & 0x00000020) >> 5;

	switch (M)
	{
		case 0x00: return handleop32_RSUB_cc_f_b_b_c(op);
		case 0x01: return handleop32_RSUB_cc_f_b_b_u6(op);
	}

	return 0;
}

uint32_t arcompact_device::handleop32_RSUB(uint32_t op)
{
	int p = (op & 0x00c00000) >> 22;

	switch (p)
	{
		case 0x00: return handleop32_RSUB_f_a_b_c(op);
		case 0x01: return handleop32_RSUB_f_a_b_u6(op);
		case 0x02: return handleop32_RSUB_f_b_b_s12(op);
		case 0x03: return handleop32_RSUB_cc(op);
	}

	return 0;
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

uint32_t arcompact_device::handleop32_BSET_f_a_b_c(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	uint8_t creg = common32_get_creg(op);
	uint8_t areg = common32_get_areg(op);

	int size = check_b_c_limm(breg, creg);

	uint32_t b = m_regs[breg];
	uint32_t c = m_regs[creg];

	/* todo: is the limm, limm syntax valid? (it's pointless.) */

	uint32_t result = b | (1 << (c & 0x1f));
	m_regs[areg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_BSET (BSET) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_BSET_f_a_b_u6(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	uint32_t u = common32_get_u6(op);
	uint8_t areg = common32_get_areg(op);

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	int size = check_b_limm(breg);

	uint32_t b = m_regs[breg];
	uint32_t c = u;


	uint32_t result = b | (1 << (c & 0x1f));
	m_regs[areg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_BSET (BSET) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_BSET_f_b_b_s12(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	uint32_t S = common32_get_s12(op);

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	int size = check_b_limm(breg);

	uint32_t b = m_regs[breg];
	uint32_t c = (uint32_t)S;


	uint32_t result = b | (1 << (c & 0x1f));
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_BSET (BSET) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_BSET_cc_f_b_b_c(uint32_t op)
{
	int size = 4;
	arcompact_fatal("handleop32_BSET_cc_f_b_b_c (BSET)\n");
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_BSET_cc_f_b_b_u6(uint32_t op)
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
		return m_pc + (size>>0);

	uint32_t result = b | (1 << (c & 0x1f));
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_BSET (BSET) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_BSET_cc(uint32_t op)
{
	int M = (op & 0x00000020) >> 5;

	switch (M)
	{
		case 0x00: return handleop32_BSET_cc_f_b_b_c(op);
		case 0x01: return handleop32_BSET_cc_f_b_b_u6(op);
	}

	return 0;
}

uint32_t arcompact_device::handleop32_BSET(uint32_t op)
{
	int p = (op & 0x00c00000) >> 22;

	switch (p)
	{
		case 0x00: return handleop32_BSET_f_a_b_c(op);
		case 0x01: return handleop32_BSET_f_a_b_u6(op);
		case 0x02: return handleop32_BSET_f_b_b_s12(op);
		case 0x03: return handleop32_BSET_cc(op);
	}

	return 0;
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

uint32_t arcompact_device::handleop32_BCLR_f_a_b_c(uint32_t op)
{
	arcompact_fatal("BCLR with P00 not supported");
	return m_pc + (0 >> 0);
}

uint32_t arcompact_device::handleop32_BCLR_f_a_b_u6(uint32_t op)
{
	arcompact_fatal("BCLR with P01 not supported");
	return m_pc + (0 >> 0);
}

uint32_t arcompact_device::handleop32_BCLR_f_b_b_s12(uint32_t op)
{
	arcompact_fatal("BCLR with P10 not supported");
	return m_pc + (0 >> 0);
}

uint32_t arcompact_device::handleop32_BCLR_cc_f_b_b_c(uint32_t op)
{
	arcompact_fatal("BCLR with P11 M0 not supported");
	return m_pc + (0 >> 0);
}

uint32_t arcompact_device::handleop32_BCLR_cc_f_b_b_u6(uint32_t op)
{
	arcompact_fatal("BCLR with P11 M0 not supported");
	return m_pc + (0 >> 0);
}

uint32_t arcompact_device::handleop32_BCLR_cc(uint32_t op)
{
	int M = (op & 0x00000020) >> 5;

	switch (M)
	{
		case 0x00: return handleop32_BCLR_cc_f_b_b_c(op);
		case 0x01: return handleop32_BCLR_cc_f_b_b_u6(op);
	}

	return 0;
}

uint32_t arcompact_device::handleop32_BCLR(uint32_t op)
{
	int p = (op & 0x00c00000) >> 22;

	switch (p)
	{
		case 0x00: return handleop32_BCLR_f_a_b_c(op);
		case 0x01: return handleop32_BCLR_f_a_b_u6(op);
		case 0x02: return handleop32_BCLR_f_b_b_s12(op);
		case 0x03: return handleop32_BCLR_cc(op);
	}

	return 0;
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BTST<.cc> b,c                   0010 0bbb 1101 0001   1BBB CCCC CC0Q QQQQ
// BTST<.cc> b,u6                  0010 0bbb 1101 0001   1BBB uuuu uu1Q QQQQ
// BTST<.cc> limm,c                0010 0110 1101 0001   1111 CCCC CC0Q QQQQ (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_BTST_f_a_b_c(uint32_t op)
{
	arcompact_fatal("BTST with P00 not supported");
	return m_pc + (0 >> 0);
}

uint32_t arcompact_device::handleop32_BTST_f_a_b_u6(uint32_t op)
{
	arcompact_fatal("BTST with P01 not supported");
	return m_pc + (0 >> 0);
}

uint32_t arcompact_device::handleop32_BTST_f_b_b_s12(uint32_t op)
{
	arcompact_fatal("BTST with P10 not supported");
	return m_pc + (0 >> 0);
}

uint32_t arcompact_device::handleop32_BTST_cc_f_b_b_c(uint32_t op)
{
	arcompact_fatal("BTST with P11 M0 not supported");
	return m_pc + (0 >> 0);
}

uint32_t arcompact_device::handleop32_BTST_cc_f_b_b_u6(uint32_t op)
{
	arcompact_fatal("BTST with P11 M0 not supported");
	return m_pc + (0 >> 0);
}

uint32_t arcompact_device::handleop32_BTST_cc(uint32_t op)
{
	int M = (op & 0x00000020) >> 5;

	switch (M)
	{
		case 0x00: return handleop32_BTST_cc_f_b_b_c(op);
		case 0x01: return handleop32_BTST_cc_f_b_b_u6(op);
	}

	return 0;
}

uint32_t arcompact_device::handleop32_BTST(uint32_t op)
{
	int p = (op & 0x00c00000) >> 22;

	switch (p)
	{
		case 0x00: return handleop32_BTST_f_a_b_c(op);
		case 0x01: return handleop32_BTST_f_a_b_u6(op);
		case 0x02: return handleop32_BTST_f_b_b_s12(op);
		case 0x03: return handleop32_BTST_cc(op);
	}

	return 0;
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

uint32_t arcompact_device::handleop32_BXOR_f_a_b_c(uint32_t op)
{
	arcompact_fatal("BXOR with P00 not supported");
	return m_pc + (0 >> 0);
}

uint32_t arcompact_device::handleop32_BXOR_f_a_b_u6(uint32_t op)
{
	arcompact_fatal("BXOR with P01 not supported");
	return m_pc + (0 >> 0);
}

uint32_t arcompact_device::handleop32_BXOR_f_b_b_s12(uint32_t op)
{
	arcompact_fatal("BXOR with P10 not supported");
	return m_pc + (0 >> 0);
}

uint32_t arcompact_device::handleop32_BXOR_cc_f_b_b_c(uint32_t op)
{
	arcompact_fatal("BXOR with P11 M0 not supported");
	return m_pc + (0 >> 0);
}

uint32_t arcompact_device::handleop32_BXOR_cc_f_b_b_u6(uint32_t op)
{
	arcompact_fatal("BXOR with P11 M0 not supported");
	return m_pc + (0 >> 0);
}

uint32_t arcompact_device::handleop32_BXOR_cc(uint32_t op)
{
	int M = (op & 0x00000020) >> 5;

	switch (M)
	{
		case 0x00: return handleop32_BXOR_cc_f_b_b_c(op);
		case 0x01: return handleop32_BXOR_cc_f_b_b_u6(op);
	}

	return 0;
}

uint32_t arcompact_device::handleop32_BXOR(uint32_t op)
{
	int p = (op & 0x00c00000) >> 22;

	switch (p)
	{
		case 0x00: return handleop32_BXOR_f_a_b_c(op);
		case 0x01: return handleop32_BXOR_f_a_b_u6(op);
		case 0x02: return handleop32_BXOR_f_b_b_s12(op);
		case 0x03: return handleop32_BXOR_cc(op);
	}

	return 0;
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

uint32_t arcompact_device::handleop32_BMSK_f_a_b_c(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	uint8_t creg = common32_get_creg(op);
	uint8_t areg = common32_get_areg(op);

	int size = check_b_c_limm(breg, creg);

	uint32_t b = m_regs[breg];
	uint32_t c = m_regs[creg];

	/* todo: is the limm, limm syntax valid? (it's pointless.) */

	uint32_t result = b & ((1<<(c+1))-1);
	m_regs[areg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_BMSK (BMSK) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_BMSK_f_a_b_u6(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	uint32_t u = common32_get_u6(op);
	uint8_t areg = common32_get_areg(op);

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	int size = check_b_limm(breg);

	uint32_t b = m_regs[breg];
	uint32_t c = u;


	uint32_t result = b & ((1<<(c+1))-1);
	m_regs[areg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_BMSK (BMSK) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_BMSK_f_b_b_s12(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	uint32_t S = common32_get_s12(op);

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	int size = check_b_limm(breg);

	uint32_t b = m_regs[breg];
	uint32_t c = (uint32_t)S;


	uint32_t result = b & ((1<<(c+1))-1);
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_BMSK (BMSK) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_BMSK_cc_f_b_b_c(uint32_t op)
{
	int size = 4;
	arcompact_fatal("handleop32_BMSK_cc_f_b_b_c (BMSK)\n");
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_BMSK_cc_f_b_b_u6(uint32_t op)
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
		return m_pc + (size>>0);

	uint32_t result = b & ((1<<(c+1))-1);
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_BMSK (BMSK) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}

uint32_t arcompact_device::handleop32_BMSK_cc(uint32_t op)
{
	int M = (op & 0x00000020) >> 5;

	switch (M)
	{
		case 0x00: return handleop32_BMSK_cc_f_b_b_c(op);
		case 0x01: return handleop32_BMSK_cc_f_b_b_u6(op);
	}

	return 0;
}

uint32_t arcompact_device::handleop32_BMSK(uint32_t op)
{
	int p = (op & 0x00c00000) >> 22;

	switch (p)
	{
		case 0x00: return handleop32_BMSK_f_a_b_c(op);
		case 0x01: return handleop32_BMSK_f_a_b_u6(op);
		case 0x02: return handleop32_BMSK_f_b_b_s12(op);
		case 0x03: return handleop32_BMSK_cc(op);
	}

	return 0;
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

uint32_t arcompact_device::handleop32_ADD1_f_a_b_c(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	uint8_t creg = common32_get_creg(op);
	uint8_t areg = common32_get_areg(op);

	int size = check_b_c_limm(breg, creg);

	uint32_t b = m_regs[breg];
	uint32_t c = m_regs[creg];

	/* todo: is the limm, limm syntax valid? (it's pointless.) */

	uint32_t result = b + (c << 1);
	m_regs[areg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_ADD1 (ADD1) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_ADD1_f_a_b_u6(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	uint32_t u = common32_get_u6(op);
	uint8_t areg = common32_get_areg(op);

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	int size = check_b_limm(breg);

	uint32_t b = m_regs[breg];
	uint32_t c = u;


	uint32_t result = b + (c << 1);
	m_regs[areg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_ADD1 (ADD1) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_ADD1_f_b_b_s12(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	uint32_t S = common32_get_s12(op);

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	int size = check_b_limm(breg);

	uint32_t b = m_regs[breg];
	uint32_t c = (uint32_t)S;


	uint32_t result = b + (c << 1);
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_ADD1 (ADD1) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_ADD1_cc_f_b_b_c(uint32_t op)
{
	int size = 4;
	arcompact_fatal("handleop32_ADD1_cc_f_b_b_c (ADD1)\n");
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_ADD1_cc_f_b_b_u6(uint32_t op)
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
		return m_pc + (size>>0);

	uint32_t result = b + (c << 1);
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_ADD1 (ADD1) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}

uint32_t arcompact_device::handleop32_ADD1_cc(uint32_t op)
{
	int M = (op & 0x00000020) >> 5;

	switch (M)
	{
		case 0x00: return handleop32_ADD1_cc_f_b_b_c(op);
		case 0x01: return handleop32_ADD1_cc_f_b_b_u6(op);
	}

	return 0;
}

uint32_t arcompact_device::handleop32_ADD1(uint32_t op)
{
	int p = (op & 0x00c00000) >> 22;

	switch (p)
	{
		case 0x00: return handleop32_ADD1_f_a_b_c(op);
		case 0x01: return handleop32_ADD1_f_a_b_u6(op);
		case 0x02: return handleop32_ADD1_f_b_b_s12(op);
		case 0x03: return handleop32_ADD1_cc(op);
	}

	return 0;
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

uint32_t arcompact_device::handleop32_ADD2_f_a_b_c(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	uint8_t creg = common32_get_creg(op);
	uint8_t areg = common32_get_areg(op);

	int size = check_b_c_limm(breg, creg);

	uint32_t b = m_regs[breg];
	uint32_t c = m_regs[creg];

	/* todo: is the limm, limm syntax valid? (it's pointless.) */

	uint32_t result = b + (c << 2);
	m_regs[areg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_ADD2 (ADD2) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_ADD2_f_a_b_u6(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	uint32_t u = common32_get_u6(op);
	uint8_t areg = common32_get_areg(op);

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	int size = check_b_limm(breg);

	uint32_t b = m_regs[breg];
	uint32_t c = u;


	uint32_t result = b + (c << 2);
	m_regs[areg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_ADD2 (ADD2) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_ADD2_f_b_b_s12(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	uint32_t S = common32_get_s12(op);

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	int size = check_b_limm(breg);

	uint32_t b = m_regs[breg];
	uint32_t c = (uint32_t)S;


	uint32_t result = b + (c << 2);
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_ADD2 (ADD2) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_ADD2_cc_f_b_b_c(uint32_t op)
{
	int size = 4;
	arcompact_fatal("handleop32_ADD2_cc_f_b_b_c (ADD2)\n");
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_ADD2_cc_f_b_b_u6(uint32_t op)
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
		return m_pc + (size>>0);

	uint32_t result = b + (c << 2);
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_ADD2 (ADD2) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}

uint32_t arcompact_device::handleop32_ADD2_cc(uint32_t op)
{
	int M = (op & 0x00000020) >> 5;

	switch (M)
	{
		case 0x00: return handleop32_ADD2_cc_f_b_b_c(op);
		case 0x01: return handleop32_ADD2_cc_f_b_b_u6(op);
	}

	return 0;
}

uint32_t arcompact_device::handleop32_ADD2(uint32_t op)
{
	int p = (op & 0x00c00000) >> 22;

	switch (p)
	{
		case 0x00: return handleop32_ADD2_f_a_b_c(op);
		case 0x01: return handleop32_ADD2_f_a_b_u6(op);
		case 0x02: return handleop32_ADD2_f_b_b_s12(op);
		case 0x03: return handleop32_ADD2_cc(op);
	}

	return 0;
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

uint32_t arcompact_device::handleop32_ADD3_f_a_b_c(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	uint8_t creg = common32_get_creg(op);
	uint8_t areg = common32_get_areg(op);

	int size = check_b_c_limm(breg, creg);

	uint32_t b = m_regs[breg];
	uint32_t c = m_regs[creg];

	/* todo: is the limm, limm syntax valid? (it's pointless.) */

	uint32_t result = b + (c << 3);
	m_regs[areg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_ADD3 (ADD3) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}

uint32_t arcompact_device::handleop32_ADD3_f_a_b_u6(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	uint32_t u = common32_get_u6(op);
	uint8_t areg = common32_get_areg(op);

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	int size = check_b_limm(breg);

	uint32_t b = m_regs[breg];
	uint32_t c = u;


	uint32_t result = b + (c << 3);
	m_regs[areg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_ADD3 (ADD3) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}

uint32_t arcompact_device::handleop32_ADD3_f_b_b_s12(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	uint32_t S = common32_get_s12(op);

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	int size = check_b_limm(breg);

	uint32_t b = m_regs[breg];
	uint32_t c = (uint32_t)S;


	uint32_t result = b + (c << 3);
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_ADD3 (ADD3) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_ADD3_cc_f_b_b_c(uint32_t op)
{
	int size = 4;
	arcompact_fatal("handleop32_ADD3_cc_f_b_b_c (ADD3)\n");
	return m_pc + (size >> 0);
}

uint32_t arcompact_device::handleop32_ADD3_cc_f_b_b_u6(uint32_t op)
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
		return m_pc + (size>>0);

	uint32_t result = b + (c << 3);
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_ADD3 (ADD3) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}

uint32_t arcompact_device::handleop32_ADD3_cc(uint32_t op)
{
	int M = (op & 0x00000020) >> 5;

	switch (M)
	{
		case 0x00: return handleop32_ADD3_cc_f_b_b_c(op);
		case 0x01: return handleop32_ADD3_cc_f_b_b_u6(op);
	}

	return 0;
}

uint32_t arcompact_device::handleop32_ADD3(uint32_t op)
{
	int p = (op & 0x00c00000) >> 22;

	switch (p)
	{
		case 0x00: return handleop32_ADD3_f_a_b_c(op);
		case 0x01: return handleop32_ADD3_f_a_b_u6(op);
		case 0x02: return handleop32_ADD3_f_b_b_s12(op);
		case 0x03: return handleop32_ADD3_cc(op);
	}

	return 0;
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


uint32_t arcompact_device::handleop32_SUB1_f_a_b_c(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	uint8_t creg = common32_get_creg(op);
	uint8_t areg = common32_get_areg(op);

	int size = check_b_c_limm(breg, creg);

	uint32_t b = m_regs[breg];
	uint32_t c = m_regs[creg];

	/* todo: is the limm, limm syntax valid? (it's pointless.) */

	uint32_t result = b - (c << 1);
	m_regs[areg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_SUB1 (SUB1) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_SUB1_f_a_b_u6(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	uint32_t u = common32_get_u6(op);
	uint8_t areg = common32_get_areg(op);

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	int size = check_b_limm(breg);

	uint32_t b = m_regs[breg];
	uint32_t c = u;


	uint32_t result = b - (c << 1);
	m_regs[areg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_SUB1 (SUB1) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_SUB1_f_b_b_s12(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	uint32_t S = common32_get_s12(op);

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	int size = check_b_limm(breg);

	uint32_t b = m_regs[breg];
	uint32_t c = (uint32_t)S;


	uint32_t result = b - (c << 1);
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_SUB1 (SUB1) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_SUB1_cc_f_b_b_c(uint32_t op)
{
	int size = 4;
	arcompact_fatal("handleop32_SUB1_cc_f_b_b_c (SUB1)\n");
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_SUB1_cc_f_b_b_u6(uint32_t op)
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
		return m_pc + (size>>0);

	uint32_t result = b - (c << 1);
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_SUB1 (SUB1) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}

uint32_t arcompact_device::handleop32_SUB1_cc(uint32_t op)
{
	int M = (op & 0x00000020) >> 5;

	switch (M)
	{
		case 0x00: return handleop32_SUB1_cc_f_b_b_c(op);
		case 0x01: return handleop32_SUB1_cc_f_b_b_u6(op);
	}

	return 0;
}

uint32_t arcompact_device::handleop32_SUB1(uint32_t op)
{
	int p = (op & 0x00c00000) >> 22;

	switch (p)
	{
		case 0x00: return handleop32_SUB1_f_a_b_c(op);
		case 0x01: return handleop32_SUB1_f_a_b_u6(op);
		case 0x02: return handleop32_SUB1_f_b_b_s12(op);
		case 0x03: return handleop32_SUB1_cc(op);
	}

	return 0;
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


uint32_t arcompact_device::handleop32_SUB2_f_a_b_c(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	uint8_t creg = common32_get_creg(op);
	uint8_t areg = common32_get_areg(op);

	int size = check_b_c_limm(breg, creg);

	uint32_t b = m_regs[breg];
	uint32_t c = m_regs[creg];

	/* todo: is the limm, limm syntax valid? (it's pointless.) */

	uint32_t result = b - (c << 2);
	m_regs[areg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_SUB2 (SUB2) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_SUB2_f_a_b_u6(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	uint32_t u = common32_get_u6(op);
	uint8_t areg = common32_get_areg(op);

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	int size = check_b_limm(breg);

	uint32_t b = m_regs[breg];
	uint32_t c = u;


	uint32_t result = b - (c << 2);
	m_regs[areg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_SUB2 (SUB2) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_SUB2_f_b_b_s12(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	uint32_t S = common32_get_s12(op);

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	int size = check_b_limm(breg);

	uint32_t b = m_regs[breg];
	uint32_t c = (uint32_t)S;


	uint32_t result = b - (c << 2);
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_SUB2 (SUB2) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_SUB2_cc_f_b_b_c(uint32_t op)
{
	int size = 4;
	arcompact_fatal("handleop32_SUB2_cc_f_b_b_c (SUB2)\n");
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_SUB2_cc_f_b_b_u6(uint32_t op)
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
		return m_pc + (size>>0);

	uint32_t result = b - (c << 2);
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_SUB2 (SUB2) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}

uint32_t arcompact_device::handleop32_SUB2_cc(uint32_t op)
{
	int M = (op & 0x00000020) >> 5;

	switch (M)
	{
		case 0x00: return handleop32_SUB2_cc_f_b_b_c(op);
		case 0x01: return handleop32_SUB2_cc_f_b_b_u6(op);
	}

	return 0;
}

uint32_t arcompact_device::handleop32_SUB2(uint32_t op)
{
	int p = (op & 0x00c00000) >> 22;

	switch (p)
	{
		case 0x00: return handleop32_SUB2_f_a_b_c(op);
		case 0x01: return handleop32_SUB2_f_a_b_u6(op);
		case 0x02: return handleop32_SUB2_f_b_b_s12(op);
		case 0x03: return handleop32_SUB2_cc(op);
	}

	return 0;
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

uint32_t arcompact_device::handleop32_SUB3_f_a_b_c(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	uint8_t creg = common32_get_creg(op);
	uint8_t areg = common32_get_areg(op);

	int size = check_b_c_limm(breg, creg);

	uint32_t b = m_regs[breg];
	uint32_t c = m_regs[creg];

	/* todo: is the limm, limm syntax valid? (it's pointless.) */

	uint32_t result = b - (c << 3);
	m_regs[areg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_SUB3 (SUB3) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_SUB3_f_a_b_u6(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	uint32_t u = common32_get_u6(op);
	uint8_t areg = common32_get_areg(op);

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	int size = check_b_limm(breg);

	uint32_t b = m_regs[breg];
	uint32_t c = u;


	uint32_t result = b - (c << 3);
	m_regs[areg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_SUB3 (SUB3) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_SUB3_f_b_b_s12(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	uint32_t S = common32_get_s12(op);

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	int size = check_b_limm(breg);

	uint32_t b = m_regs[breg];
	uint32_t c = (uint32_t)S;


	uint32_t result = b - (c << 3);
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_SUB3 (SUB3) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_SUB3_cc_f_b_b_c(uint32_t op)
{
	int size = 4;
	arcompact_fatal("handleop32_SUB3_cc_f_b_b_c (SUB3)\n");
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_SUB3_cc_f_b_b_u6(uint32_t op)
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
		return m_pc + (size>>0);

	uint32_t result = b - (c << 3);
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_SUB3 (SUB3) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}

uint32_t arcompact_device::handleop32_SUB3_cc(uint32_t op)
{
	int M = (op & 0x00000020) >> 5;

	switch (M)
	{
		case 0x00: return handleop32_SUB3_cc_f_b_b_c(op);
		case 0x01: return handleop32_SUB3_cc_f_b_b_u6(op);
	}

	return 0;
}

uint32_t arcompact_device::handleop32_SUB3(uint32_t op)
{
	int p = (op & 0x00c00000) >> 22;

	switch (p)
	{
		case 0x00: return handleop32_SUB3_f_a_b_c(op);
		case 0x01: return handleop32_SUB3_f_a_b_u6(op);
		case 0x02: return handleop32_SUB3_f_b_b_s12(op);
		case 0x03: return handleop32_SUB3_cc(op);
	}

	return 0;
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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


uint32_t arcompact_device::handleop32_MPY_f_a_b_c(uint32_t op)
{
	arcompact_fatal("MPY with P00 not supported");
	return m_pc + (0 >> 0);
}

uint32_t arcompact_device::handleop32_MPY_f_a_b_u6(uint32_t op)
{
	arcompact_fatal("MPY with P01 not supported");
	return m_pc + (0 >> 0);
}

uint32_t arcompact_device::handleop32_MPY_f_b_b_s12(uint32_t op)
{
	arcompact_fatal("MPY with P10 not supported");
	return m_pc + (0 >> 0);
}

uint32_t arcompact_device::handleop32_MPY_cc_f_b_b_c(uint32_t op)
{
	arcompact_fatal("MPY with P11 M0 not supported");
	return m_pc + (0 >> 0);
}

uint32_t arcompact_device::handleop32_MPY_cc_f_b_b_u6(uint32_t op)
{
	arcompact_fatal("MPY with P11 M0 not supported");
	return m_pc + (0 >> 0);
}

uint32_t arcompact_device::handleop32_MPY_cc(uint32_t op)
{
	int M = (op & 0x00000020) >> 5;

	switch (M)
	{
		case 0x00: return handleop32_MPY_cc_f_b_b_c(op);
		case 0x01: return handleop32_MPY_cc_f_b_b_u6(op);
	}

	return 0;
}

uint32_t arcompact_device::handleop32_MPY(uint32_t op)
{
	int p = (op & 0x00c00000) >> 22;

	switch (p)
	{
		case 0x00: return handleop32_MPY_f_a_b_c(op);
		case 0x01: return handleop32_MPY_f_a_b_u6(op);
		case 0x02: return handleop32_MPY_f_b_b_s12(op);
		case 0x03: return handleop32_MPY_cc(op);
	}

	return 0;
}


// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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


uint32_t arcompact_device::handleop32_MPYH_f_a_b_c(uint32_t op)
{
	arcompact_fatal("MPYH with P00 not supported");
	return m_pc + (0 >> 0);
}

uint32_t arcompact_device::handleop32_MPYH_f_a_b_u6(uint32_t op)
{
	arcompact_fatal("MPYH with P01 not supported");
	return m_pc + (0 >> 0);
}

uint32_t arcompact_device::handleop32_MPYH_f_b_b_s12(uint32_t op)
{
	arcompact_fatal("MPYH with P10 not supported");
	return m_pc + (0 >> 0);
}

uint32_t arcompact_device::handleop32_MPYH_cc_f_b_b_c(uint32_t op)
{
	arcompact_fatal("MPYH with P11 M0 not supported");
	return m_pc + (0 >> 0);
}

uint32_t arcompact_device::handleop32_MPYH_cc_f_b_b_u6(uint32_t op)
{
	arcompact_fatal("MPYH with P11 M0 not supported");
	return m_pc + (0 >> 0);
}

uint32_t arcompact_device::handleop32_MPYH_cc(uint32_t op)
{
	int M = (op & 0x00000020) >> 5;

	switch (M)
	{
		case 0x00: return handleop32_MPYH_cc_f_b_b_c(op);
		case 0x01: return handleop32_MPYH_cc_f_b_b_u6(op);
	}

	return 0;
}

uint32_t arcompact_device::handleop32_MPYH(uint32_t op)
{
	int p = (op & 0x00c00000) >> 22;

	switch (p)
	{
		case 0x00: return handleop32_MPYH_f_a_b_c(op);
		case 0x01: return handleop32_MPYH_f_a_b_u6(op);
		case 0x02: return handleop32_MPYH_f_b_b_s12(op);
		case 0x03: return handleop32_MPYH_cc(op);
	}

	return 0;
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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


uint32_t arcompact_device::handleop32_MPYHU_f_a_b_c(uint32_t op)
{
	arcompact_fatal("MPYHU with P00 not supported");
	return m_pc + (0 >> 0);
}

uint32_t arcompact_device::handleop32_MPYHU_f_a_b_u6(uint32_t op)
{
	arcompact_fatal("MPYHU with P01 not supported");
	return m_pc + (0 >> 0);
}

uint32_t arcompact_device::handleop32_MPYHU_f_b_b_s12(uint32_t op)
{
	arcompact_fatal("MPYHU with P10 not supported");
	return m_pc + (0 >> 0);
}

uint32_t arcompact_device::handleop32_MPYHU_cc_f_b_b_c(uint32_t op)
{
	arcompact_fatal("MPYHU with P11 M0 not supported");
	return m_pc + (0 >> 0);
}

uint32_t arcompact_device::handleop32_MPYHU_cc_f_b_b_u6(uint32_t op)
{
	arcompact_fatal("MPYHU with P11 M0 not supported");
	return m_pc + (0 >> 0);
}

uint32_t arcompact_device::handleop32_MPYHU_cc(uint32_t op)
{
	int M = (op & 0x00000020) >> 5;

	switch (M)
	{
		case 0x00: return handleop32_MPYHU_cc_f_b_b_c(op);
		case 0x01: return handleop32_MPYHU_cc_f_b_b_u6(op);
	}

	return 0;
}

uint32_t arcompact_device::handleop32_MPYHU(uint32_t op)
{
	int p = (op & 0x00c00000) >> 22;

	switch (p)
	{
		case 0x00: return handleop32_MPYHU_f_a_b_c(op);
		case 0x01: return handleop32_MPYHU_f_a_b_u6(op);
		case 0x02: return handleop32_MPYHU_f_b_b_s12(op);
		case 0x03: return handleop32_MPYHU_cc(op);
	}

	return 0;
}


// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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


uint32_t arcompact_device::handleop32_MPYU_f_a_b_c(uint32_t op)
{
	arcompact_fatal("MPYU with P00 not supported");
	return m_pc + (0 >> 0);
}

uint32_t arcompact_device::handleop32_MPYU_f_a_b_u6(uint32_t op)
{
	arcompact_fatal("MPYU with P01 not supported");
	return m_pc + (0 >> 0);
}

uint32_t arcompact_device::handleop32_MPYU_f_b_b_s12(uint32_t op)
{
	arcompact_fatal("MPYU with P10 not supported");
	return m_pc + (0 >> 0);
}

uint32_t arcompact_device::handleop32_MPYU_cc_f_b_b_c(uint32_t op)
{
	arcompact_fatal("MPYU with P11 M0 not supported");
	return m_pc + (0 >> 0);
}

uint32_t arcompact_device::handleop32_MPYU_cc_f_b_b_u6(uint32_t op)
{
	arcompact_fatal("MPYU with P11 M0 not supported");
	return m_pc + (0 >> 0);
}

uint32_t arcompact_device::handleop32_MPYU_cc(uint32_t op)
{
	int M = (op & 0x00000020) >> 5;

	switch (M)
	{
		case 0x00: return handleop32_MPYU_cc_f_b_b_c(op);
		case 0x01: return handleop32_MPYU_cc_f_b_b_u6(op);
	}

	return 0;
}

uint32_t arcompact_device::handleop32_MPYU(uint32_t op)
{
	int p = (op & 0x00c00000) >> 22;

	switch (p)
	{
		case 0x00: return handleop32_MPYU_f_a_b_c(op);
		case 0x01: return handleop32_MPYU_f_a_b_u6(op);
		case 0x02: return handleop32_MPYU_f_b_b_s12(op);
		case 0x03: return handleop32_MPYU_cc(op);
	}

	return 0;
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS
// Jcc [c]                         0010 0RRR 1110 0000   0RRR CCCC CC0Q QQQQ
// Jcc limm                        0010 0RRR 1110 0000   0RRR 1111 100Q QQQQ (+ Limm)
// Jcc u6                          0010 0RRR 1110 0000   0RRR uuuu uu1Q QQQQ
// Jcc.F [ilink1]                  0010 0RRR 1110 0000   1RRR 0111 010Q QQQQ
// Jcc.F [ilink2]                  0010 0RRR 1110 0000   1RRR 0111 100Q QQQQ
//                                 IIII I      SS SSSS
// J [c]                           0010 0RRR 0010 0000   0RRR CCCC CCRR RRRR
// J.F [ilink1]                    0010 0RRR 0010 0000   1RRR 0111 01RR RRRR
// J.F [ilink2]                    0010 0RRR 0010 0000   1RRR 0111 10RR RRRR
// J limm                          0010 0RRR 0010 0000   0RRR 1111 10RR RRRR (+ Limm)
// J u6                            0010 0RRR 0110 0000   0RRR uuuu uuRR RRRR
// J s12                           0010 0RRR 1010 0000   0RRR ssss ssSS SSSS
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_Jcc_f_a_b_c(uint32_t op)
{
	int size;

	uint8_t creg = common32_get_creg(op);
	uint8_t F = common32_get_F(op);

	if (creg == LIMM_REG)
	{
		// opcode          iiii i--- ppII IIII F--- CCCC CC-- ----
		// J limm          0010 0RRR 0010 0000 0RRR 1111 10RR RRRR  [LIMM]  (creg = LIMM)

		get_limm_32bit_opcode();
		size = 8;

		return m_regs[LIMM_REG];
	}
	else
	{
		// opcode          iiii i--- ppII IIII F--- CCCC CC-- ----
		// J [c]           0010 0RRR 0010 0000 0RRR CCCC CCRR RRRR
		// J.F [ilink1]    0010 0RRR 0010 0000 1RRR 0111 01RR RRRR  (creg = ILINK1, FLAG must be set)
		// J.F [ilink2]    0010 0RRR 0010 0000 1RRR 0111 10RR RRRR  (creg = ILINE2, FLAG must be set)

		if (F)
		{
			if ((creg == REG_ILINK1) || (creg == REG_ILINK2))
			{
				arcompact_log("1 unimplemented J.F %08x", op);
			}
			else
			{
				// should not use .F unless jumping to ILINK1/2
				arcompact_fatal ("illegal 1 unimplemented J.F (F should not be set) %08x", op);
			}

		}
		else
		{
			if ((creg == REG_ILINK1) || (creg == REG_ILINK2))
			{
				// should only jumping to ILINK1/2 if .F is set
				arcompact_fatal("illegal 1 unimplemented J (F not set) %08x", op);
			}
			else
			{
				return m_regs[creg];
			}
		}
	}

	return m_pc + (size>>0);
}

uint32_t arcompact_device::handleop32_Jcc_f_a_b_u6(uint32_t op)
{
	// opcode          iiii i--- ppII IIII F--- uuuu uu-- ----
	// J u6            0010 0RRR 0110 0000 0RRR uuuu uuRR RRRR
	int size = 4;
	arcompact_log("2 unimplemented J %08x", op);
	return m_pc + (size>>0);
}

uint32_t arcompact_device::handleop32_Jcc_f_b_b_s12(uint32_t op)
{
	// opcode          iiii i--- ppII IIII F--- ssss ssSS SSSS
	// J s12           0010 0RRR 1010 0000 0RRR ssss ssSS SSSS
	int size = 4;
	arcompact_log("3 unimplemented J %08x", op);
	return m_pc + (size>>0);
}


uint32_t arcompact_device::handleop32_Jcc_cc_f_b_b_c(uint32_t op) // Jcc   (no link, no delay)
{
	int size = 4;

	uint8_t creg = common32_get_creg(op);
	uint8_t condition = common32_get_condition(op);
	uint8_t F = common32_get_F(op);

	uint32_t c;

	if (creg == LIMM_REG)
	{
		// opcode          iiii i--- ppII IIII F--- cccc ccmq qqqq
		// Jcc limm        0010 0RRR 1110 0000 0RRR 1111 100Q QQQQ  [LIUMM]
		get_limm_32bit_opcode();
		size = 8;

		c = m_regs[LIMM_REG];

	}
	else
	{
		// opcode          iiii i--- ppII IIII F--- cccc ccmq qqqq
		// Jcc [c]         0010 0RRR 1110 0000 0RRR CCCC CC0Q QQQQ
		// no conditional links to ILINK1, ILINK2?

		c = m_regs[creg];
	}

	if (!check_condition(condition))
		return m_pc + (size>>0);

	if (!F)
	{
		// if F isn't set then the destination can't be ILINK1 or ILINK2

		if ((creg == REG_ILINK1) || (creg == REG_ILINK2))
		{
			arcompact_fatal ("fatal handleop32_Jcc_cc_f_b_b_c J %08x (F not set but ILINK1 or ILINK2 used as dst)", op);
		}
		else
		{
			uint32_t realaddress = c;
			return realaddress;
		}
	}

	if (F)
	{
		// if F is set then the destination MUST be ILINK1 or ILINK2

		if ((creg == REG_ILINK1) || (creg == REG_ILINK2))
		{
			arcompact_log("unimplemented handleop32_Jcc_cc_f_b_b_c J %08x (F set)", op);
		}
		else
		{
			arcompact_fatal ("fatal handleop32_Jcc_cc_f_b_b_c J %08x (F set but not ILINK1 or ILINK2 used as dst)", op);

		}
	}


	return m_pc + (size>>0);
}

uint32_t arcompact_device::handleop32_Jcc_cc_f_b_b_u6(uint32_t op)
{
	// opcode          iiii i--- ppII IIII F--- uuuu uumq qqqq
	// Jcc u6          0010 0RRR 1110 0000 0RRR uuuu uu1Q QQQQ
	int size = 4;
	arcompact_log("unimplemented handleop32_Jcc_cc_f_b_b_u6 J %08x (u6)", op);
	return m_pc + (size>>0);
}

uint32_t arcompact_device::handleop32_Jcc_cc(uint32_t op)
{
	int M = (op & 0x00000020) >> 5;

	switch (M)
	{
		case 0x00: return handleop32_Jcc_cc_f_b_b_c(op);
		case 0x01: return handleop32_Jcc_cc_f_b_b_u6(op);
	}

	return 0;
}

uint32_t arcompact_device::handleop32_Jcc(uint32_t op)
{
	int p = (op & 0x00c00000) >> 22;

	switch (p)
	{
		case 0x00: return handleop32_Jcc_f_a_b_c(op);
		case 0x01: return handleop32_Jcc_f_a_b_u6(op);
		case 0x02: return handleop32_Jcc_f_b_b_s12(op);
		case 0x03: return handleop32_Jcc_cc(op);
	}

	return 0;
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS
// Jcc.D u6                        0010 0RRR 1110 0001   0RRR uuuu uu1Q QQQQ
// Jcc.D [c]                       0010 0RRR 1110 0001   0RRR CCCC CC0Q QQQQ
//
// J.D [c]                         0010 0RRR 0010 0001   0RRR CCCC CCRR RRRR
// J.D u6                          0010 0RRR 0110 0001   0RRR uuuu uuRR RRRR
// J.D s12                         0010 0RRR 1010 0001   0RRR ssss ssSS SSSS
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


uint32_t arcompact_device::handleop32_Jcc_D_f_a_b_c(uint32_t op)
{
	int size = 4;

	uint8_t creg = common32_get_creg(op);
	uint8_t F = common32_get_F(op);

	if (creg == LIMM_REG)
	{
		get_limm_32bit_opcode();
		size = 8;

		handle_jump_to_addr(1,0,m_regs[LIMM_REG], m_pc + (size >> 0));
	}
	else
	{
		return handle_jump_to_register(1,0,creg, m_pc + (size>>0), F); // delay, no link
	}

	return m_pc + (size>>0);
}

uint32_t arcompact_device::handleop32_Jcc_D_f_a_b_u6(uint32_t op)
{
	int size = 4;
	arcompact_log("unimplemented J.D (u6 type) %08x", op);
	return m_pc + (size>>0);
}

uint32_t arcompact_device::handleop32_Jcc_D_f_b_b_s12(uint32_t op)
{
	int size = 4;
	arcompact_log("unimplemented J.D (s12 type) %08x", op);
	return m_pc + (size>>0);
}


uint32_t arcompact_device::handleop32_Jcc_D_cc_f_b_b_c(uint32_t op) // Jcc.D   (no link, delay)
{
	int size = 4;

	uint8_t creg = common32_get_creg(op);
	uint8_t condition = common32_get_condition(op);
	uint8_t F = common32_get_F(op);

	//uint32_t c = 0;

	if (creg == LIMM_REG)
	{
		get_limm_32bit_opcode();
		size = 8;
	}
	else
	{
		// opcode          iiii i--- ppII IIII F--- cccc ccmq qqqq
		// Jcc [c]         0010 0RRR 1110 0000 0RRR CCCC CC0Q QQQQ
		// no conditional links to ILINK1, ILINK2?

	//  c = m_regs[creg];
	}

	if (!check_condition(condition))
		return m_pc + (size>>0);

	if (!F)
	{
		// if F isn't set then the destination can't be ILINK1 or ILINK2

		if ((creg == REG_ILINK1) || (creg == REG_ILINK2))
		{
			arcompact_log("unimplemented Jcc.D (p11_m0 type, illegal) %08x", op);
		}
		else
		{
			arcompact_log("unimplemented Jcc.D (p11_m0 type, unimplemented) %08x", op);
		}
	}

	if (F)
	{
		// if F is set then the destination MUST be ILINK1 or ILINK2

		if ((creg == REG_ILINK1) || (creg == REG_ILINK2))
		{
			arcompact_log("unimplemented Jcc.D.F (p11_m0 type, unimplemented) %08x", op);
		}
		else
		{
			arcompact_log("unimplemented Jcc.D.F (p11_m0 type, illegal) %08x", op);
		}
	}


	return m_pc + (size>>0);
}

uint32_t arcompact_device::handleop32_Jcc_D_cc_f_b_b_u6(uint32_t op)
{
	int size = 4;
	arcompact_log("unimplemented handleop32_Jcc_D_cc_f_b_b_u6 J.D %08x (u6)", op);
	return m_pc + (size>>0);
}

uint32_t arcompact_device::handleop32_Jcc_D_cc(uint32_t op)
{
	int M = (op & 0x00000020) >> 5;

	switch (M)
	{
		case 0x00: return handleop32_Jcc_D_cc_f_b_b_c(op);
		case 0x01: return handleop32_Jcc_D_cc_f_b_b_u6(op);
	}

	return 0;
}

uint32_t arcompact_device::handleop32_Jcc_D(uint32_t op)
{
	int p = (op & 0x00c00000) >> 22;

	switch (p)
	{
		case 0x00: return handleop32_Jcc_D_f_a_b_c(op);
		case 0x01: return handleop32_Jcc_D_f_a_b_u6(op);
		case 0x02: return handleop32_Jcc_D_f_b_b_s12(op);
		case 0x03: return handleop32_Jcc_D_cc(op);
	}

	return 0;
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS
// JLcc [c]                        0010 0RRR 1110 0010   0RRR CCCC CC0Q QQQQ
// JLcc limm                       0010 0RRR 1110 0010   0RRR 1111 100Q QQQQ (+ Limm)
// JLcc u6                         0010 0RRR 1110 0010   0RRR uuuu uu1Q QQQQ
// JL [c]                          0010 0RRR 0010 0010   0RRR CCCC CCRR RRRR
// JL limm                         0010 0RRR 0010 0010   0RRR 1111 10RR RRRR (+ Limm)
// JL u6                           0010 0RRR 0110 0010   0RRR uuuu uuRR RRRR
// JL s12                          0010 0RRR 1010 0010   0RRR ssss ssSS SSSS
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


uint32_t arcompact_device::handleop32_JLcc_f_a_b_c(uint32_t op)
{
	arcompact_fatal("JLcc with P00 not supported");
	return m_pc + (0 >> 0);
}

uint32_t arcompact_device::handleop32_JLcc_f_a_b_u6(uint32_t op)
{
	arcompact_fatal("JLcc with P01 not supported");
	return m_pc + (0 >> 0);
}

uint32_t arcompact_device::handleop32_JLcc_f_b_b_s12(uint32_t op)
{
	arcompact_fatal("JLcc with P10 not supported");
	return m_pc + (0 >> 0);
}

uint32_t arcompact_device::handleop32_JLcc_cc_f_b_b_c(uint32_t op)
{
	arcompact_fatal("JLcc with P11 M0 not supported");
	return m_pc + (0 >> 0);
}

uint32_t arcompact_device::handleop32_JLcc_cc_f_b_b_u6(uint32_t op)
{
	arcompact_fatal("JLcc with P11 M0 not supported");
	return m_pc + (0 >> 0);
}

uint32_t arcompact_device::handleop32_JLcc_cc(uint32_t op)
{
	int M = (op & 0x00000020) >> 5;

	switch (M)
	{
		case 0x00: return handleop32_JLcc_cc_f_b_b_c(op);
		case 0x01: return handleop32_JLcc_cc_f_b_b_u6(op);
	}

	return 0;
}

uint32_t arcompact_device::handleop32_JLcc(uint32_t op)
{
	int p = (op & 0x00c00000) >> 22;

	switch (p)
	{
		case 0x00: return handleop32_JLcc_f_a_b_c(op);
		case 0x01: return handleop32_JLcc_f_a_b_u6(op);
		case 0x02: return handleop32_JLcc_f_b_b_s12(op);
		case 0x03: return handleop32_JLcc_cc(op);
	}

	return 0;
}


// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS
// JLcc.D u6                       0010 0RRR 1110 0011   0RRR uuuu uu1Q QQQQ
// JLcc.D [c]                      0010 0RRR 1110 0011   0RRR CCCC CC0Q QQQQ
// JL.D [c]                        0010 0RRR 0010 0011   0RRR CCCC CCRR RRRR
// JL.D u6                         0010 0RRR 0110 0011   0RRR uuuu uuRR RRRR
// JL.D s12                        0010 0RRR 1010 0011   0RRR ssss ssSS SSSS
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


uint32_t arcompact_device::handleop32_JLcc_D_f_a_b_c(uint32_t op)
{
	arcompact_fatal("JLcc_D with P00 not supported");
	return m_pc + (0 >> 0);
}

uint32_t arcompact_device::handleop32_JLcc_D_f_a_b_u6(uint32_t op)
{
	arcompact_fatal("JLcc_D with P01 not supported");
	return m_pc + (0 >> 0);
}

uint32_t arcompact_device::handleop32_JLcc_D_f_b_b_s12(uint32_t op)
{
	arcompact_fatal("JLcc_D with P10 not supported");
	return m_pc + (0 >> 0);
}

uint32_t arcompact_device::handleop32_JLcc_D_cc_f_b_b_c(uint32_t op)
{
	arcompact_fatal("JLcc_D with P11 M0 not supported");
	return m_pc + (0 >> 0);
}

uint32_t arcompact_device::handleop32_JLcc_D_cc_f_b_b_u6(uint32_t op)
{
	arcompact_fatal("JLcc_D with P11 M0 not supported");
	return m_pc + (0 >> 0);
}

uint32_t arcompact_device::handleop32_JLcc_D_cc(uint32_t op)
{
	int M = (op & 0x00000020) >> 5;

	switch (M)
	{
		case 0x00: return handleop32_JLcc_D_cc_f_b_b_c(op);
		case 0x01: return handleop32_JLcc_D_cc_f_b_b_u6(op);
	}

	return 0;
}

uint32_t arcompact_device::handleop32_JLcc_D(uint32_t op)
{
	int p = (op & 0x00c00000) >> 22;

	switch (p)
	{
		case 0x00: return handleop32_JLcc_D_f_a_b_c(op);
		case 0x01: return handleop32_JLcc_D_f_a_b_u6(op);
		case 0x02: return handleop32_JLcc_D_f_b_b_s12(op);
		case 0x03: return handleop32_JLcc_D_cc(op);
	}

	return 0;
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS
// LP<cc> u7                       0010 0RRR 1110 1000   0RRR uuuu uu1Q QQQQ
// LP s13                          0010 0RRR 1010 1000   0RRR ssss ssSS SSSS
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


uint32_t arcompact_device::handleop32_LP(uint32_t op) // LPcc (loop setup)
{
	int size = 4;
//  uint8_t breg = common32_get_breg(op); // breg is reserved
	int p = common32_get_p(op);

	if (p == 0x00)
	{
		arcompact_fatal("<illegal LPcc, p = 0x00)");
	}
	else if (p == 0x01)
	{
		arcompact_fatal("<illegal LPcc, p = 0x01)");
	}
	else if (p == 0x02) // Loop unconditional
	{ // 0010 0RRR 1010 1000 0RRR ssss ssSS SSSS
		uint32_t S = common32_get_s12(op);
		if (S & 0x800) S = -0x800 + (S&0x7ff);

		arcompact_fatal("Lp unconditional not supported %d", S);
	}
	else if (p == 0x03) // Loop conditional
	{ // 0010 0RRR 1110 1000 0RRR uuuu uu1Q QQQQ
		uint32_t u = common32_get_u6(op);
		uint8_t condition = common32_get_condition(op);
		//arcompact_fatal("Lp conditional %s not supported %d", arcompact_disassembler::conditions[condition], u);

		// if the loop condition fails then just jump to after the end of the loop, don't set any registers
		if (!check_condition(condition))
		{
			uint32_t realoffset = (m_pc&0xfffffffc) + (u * 2);
			return realoffset;
		}
		else
		{
			// otherwise set up the loop positions
			m_LP_START = m_pc + (size >> 0);
			m_LP_END = (m_pc&0xfffffffc) + (u * 2);
			return m_pc + (size>>0);
		}

	}

	return m_pc + (size>>0);
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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


uint32_t arcompact_device::handleop32_FLAG_f_a_b_c(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	uint8_t creg = common32_get_creg(op);
	int size = check_b_c_limm(breg, creg);
	//uint8_t areg = common32_get_areg(op);
	//uint32_t b = m_regs[breg];
	uint32_t c = m_regs[creg];

	/* todo: is the limm, limm syntax valid? (it's pointless.) */
	uint32_t source = c;
	if (!(source & 0x0001)) // H means ignore all others
	{
		// privileged  mode only
		//(source & 0x0002) ? SET_E1 : CLEAR_E1
		//(source & 0x0004) ? SET_E2 : CLEAR_E2
		//(source & 0x0008)
		//(source & 0x0010)
		//(source & 0x0020)
		//(source & 0x0040)
		//(source & 0x0080)
		if (source & 0x0100) { status32_set_v(); } else { status32_clear_v(); }
		if (source & 0x0200) { status32_set_c(); } else { status32_clear_c(); }
		if (source & 0x0400) { status32_set_n(); } else { status32_clear_n(); }
		if (source & 0x0800) { status32_set_z(); } else { status32_clear_z(); }
	}

	return m_pc + (size >> 0);
}

uint32_t arcompact_device::handleop32_FLAG_f_a_b_u6(uint32_t op)
{
	arcompact_fatal("FLAG with P01 not supported");
	return m_pc + (0 >> 0);
}

uint32_t arcompact_device::handleop32_FLAG_f_b_b_s12(uint32_t op)
{
	arcompact_fatal("FLAG with P10 not supported");
	return m_pc + (0 >> 0);
}

uint32_t arcompact_device::handleop32_FLAG_cc_f_b_b_c(uint32_t op)
{
	arcompact_fatal("FLAG with P11 M0 not supported");
	return m_pc + (0 >> 0);
}

uint32_t arcompact_device::handleop32_FLAG_cc_f_b_b_u6(uint32_t op)
{
	arcompact_fatal("FLAG with P11 M0 not supported");
	return m_pc + (0 >> 0);
}

uint32_t arcompact_device::handleop32_FLAG_cc(uint32_t op)
{
	int M = (op & 0x00000020) >> 5;

	switch (M)
	{
		case 0x00: return handleop32_FLAG_cc_f_b_b_c(op);
		case 0x01: return handleop32_FLAG_cc_f_b_b_u6(op);
	}

	return 0;
}

uint32_t arcompact_device::handleop32_FLAG(uint32_t op)
{
	int p = (op & 0x00c00000) >> 22;

	switch (p)
	{
		case 0x00: return handleop32_FLAG_f_a_b_c(op);
		case 0x01: return handleop32_FLAG_f_a_b_u6(op);
		case 0x02: return handleop32_FLAG_f_b_b_s12(op);
		case 0x03: return handleop32_FLAG_cc(op);
	}

	return 0;
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                 IIII I      SS SSSS
// LR b,[c]                        0010 0bbb 0010 1010   0BBB CCCC CCRR RRRR
// LR b,[limm]                     0010 0bbb 0010 1010   0BBB 1111 10RR RRRR (+ Limm)
// LR b,[u6]                       0010 0bbb 0110 1010   0BBB uuuu uu00 0000
// LR b,[s12]                      0010 0bbb 1010 1010   0BBB ssss ssSS SSSS
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_LR_f_a_b_c(uint32_t op)
{
	int size = 4;

	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	uint8_t creg = common32_get_creg(op);
	 //uint8_t areg = common32_get_areg(op); // areg is reserved / not used

	if (creg == LIMM_REG)
	{
		get_limm_32bit_opcode();
		size = 8;
	}

	uint32_t c = m_regs[creg];
	/* todo: is the limm, limm syntax valid? (it's pointless.) */

	m_regs[breg] = READAUX(c);


	if (F)
	{
		// no flag changes
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_LR_f_a_b_u6(uint32_t op)
{
	int size = 4;

	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	uint32_t u = common32_get_u6(op);
	 //uint8_t areg = common32_get_areg(op); // areg is reserved / not used

	uint32_t c = u;


	m_regs[breg] = READAUX(c);


	if (F)
	{
		// no flag changes
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_LR_f_b_b_s12(uint32_t op)
{
	int size = 4;

	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	uint32_t S = common32_get_s12(op);

	uint32_t c = (uint32_t)S;


	m_regs[breg] = READAUX(c);


	if (F)
	{
		// no flag changes
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_LR_cc_f_b_b_c(uint32_t op)
{
	int size = 4;
	arcompact_fatal("handleop32_LR_cc_f_b_b_c (LR)\n");
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_LR_cc_f_b_b_u6(uint32_t op)
{
	int size = 4;

	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	uint32_t u = common32_get_u6(op);


	uint32_t c = u;


	uint8_t condition = common32_get_condition(op);
	if (!check_condition(condition))
		return m_pc + (size>>0);

	m_regs[breg] = READAUX(c);


	if (F)
	{
		// no flag changes
	}
	return m_pc + (size >> 0);
}

uint32_t arcompact_device::handleop32_LR_cc(uint32_t op)
{
	int M = (op & 0x00000020) >> 5;

	switch (M)
	{
		case 0x00: return handleop32_LR_cc_f_b_b_c(op);
		case 0x01: return handleop32_LR_cc_f_b_b_u6(op);
	}

	return 0;
}

uint32_t arcompact_device::handleop32_LR(uint32_t op)
{
	int p = (op & 0x00c00000) >> 22;

	switch (p)
	{
		case 0x00: return handleop32_LR_f_a_b_c(op);
		case 0x01: return handleop32_LR_f_a_b_u6(op);
		case 0x02: return handleop32_LR_f_b_b_s12(op);
		case 0x03: return handleop32_LR_cc(op);
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

uint32_t arcompact_device::handleop32_SR_f_a_b_c(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	uint8_t creg = common32_get_creg(op);
	 //uint8_t areg = common32_get_areg(op); // areg is reserved / not used

	int size = check_b_c_limm(breg, creg);

	uint32_t b = m_regs[breg];
	uint32_t c = m_regs[creg];

	/* todo: is the limm, limm syntax valid? (it's pointless.) */

	WRITEAUX(c,b);


	if (F)
	{
		// no flag changes
	}
	return m_pc + (size >> 0);
}

uint32_t arcompact_device::handleop32_SR_f_a_b_u6(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	uint32_t u = common32_get_u6(op);
	 //uint8_t areg = common32_get_areg(op); // areg is reserved / not used

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	int size = check_b_limm(breg);

	uint32_t b = m_regs[breg];
	uint32_t c = u;


	WRITEAUX(c,b);


	if (F)
	{
		// no flag changes
	}
	return m_pc + (size >> 0);
}

uint32_t arcompact_device::handleop32_SR_f_b_b_s12(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	uint32_t S = common32_get_s12(op);

	/* is having b as LIMM valid here? LIMM vs. fixed u6 value makes no sense */
	int size = check_b_limm(breg);

	uint32_t b = m_regs[breg];
	uint32_t c = (uint32_t)S;


	WRITEAUX(c,b);


	if (F)
	{
		// no flag changes
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_SR_cc_f_b_b_c(uint32_t op)
{
	int size = 4;
	arcompact_fatal("handleop32_SR_cc_f_b_b_c (SR)\n");
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_SR_cc_f_b_b_u6(uint32_t op)
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
		return m_pc + (size>>0);

	WRITEAUX(c,b);


	if (F)
	{
		// no flag changes
	}
	return m_pc + (size >> 0);
}

uint32_t arcompact_device::handleop32_SR_cc(uint32_t op)
{
	int M = (op & 0x00000020) >> 5;

	switch (M)
	{
		case 0x00: return handleop32_SR_cc_f_b_b_c(op);
		case 0x01: return handleop32_SR_cc_f_b_b_u6(op);
	}

	return 0;
}

uint32_t arcompact_device::handleop32_SR(uint32_t op)
{
	int p = (op & 0x00c00000) >> 22;

	switch (p)
	{
		case 0x00: return handleop32_SR_f_a_b_c(op);
		case 0x01: return handleop32_SR_f_a_b_u6(op);
		case 0x02: return handleop32_SR_f_b_b_s12(op);
		case 0x03: return handleop32_SR_cc(op);
	}

	return 0;
}
