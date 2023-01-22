// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "arcompact.h"
#include "arcompactdasm.h"


uint32_t arcompact_device::handleop32_FLAG(uint32_t op)
{
	// leapster bios uses formats for FLAG that are not defined, but I guess work anyway (P modes 0 / 1)
	return arcompact_handle04_helper(op, arcompact_disassembler::opcodes_04[0x29], /*"FLAG"*/ 1,1);
}

void arcompact_device::do_flags(uint32_t result, uint32_t b, uint32_t c)
{
	do_flags_nz(result);

	if ((b & 0x80000000) == (c & 0x80000000))
	{
		if ((result & 0x80000000) != (b & 0x80000000))
		{
			STATUS32_SET_V;
		}
		else
		{
			STATUS32_CLEAR_V;
		}
	}
	if (b < c)
	{
		STATUS32_SET_C;
	}
	else
	{
		STATUS32_CLEAR_C;
	}
}

void arcompact_device::do_flags_nz(uint32_t result)
{
	if (result & 0x80000000) { STATUS32_SET_N; }
	else { STATUS32_CLEAR_N; }
	if (result == 0x00000000) { STATUS32_SET_Z; }
	else { STATUS32_CLEAR_Z; }
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

uint32_t arcompact_device::handleop32_ADD_p00(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	uint8_t creg = common32_get_creg(op);
	uint8_t areg = common32_get_areg(op);
	int size = check_b_c_limm(breg, creg);
	uint32_t b = m_regs[breg];
	uint32_t c = m_regs[creg];

	/* todo: is the limm, limm syntax valid? (it's pointless.) */

	uint32_t result = b + c;
	m_regs[areg] = result;

	if (F)
		do_flags(result, b, c);

	return m_pc + (size >> 0);
}

uint32_t arcompact_device::handleop32_ADD_p01(uint32_t op)
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

uint32_t arcompact_device::handleop32_ADD_p10(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	int32_t S = common32_get_s12(op);

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


uint32_t arcompact_device::handleop32_ADD_p11_m0(uint32_t op)
{
	int size = 4;
	arcompact_fatal("handleop32_ADD_p11_m0 (ADD)\n");
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_ADD_p11_m1(uint32_t op)
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

uint32_t arcompact_device::handleop32_ADD_p11(uint32_t op)
{
	int M = (op & 0x00000020) >> 5;

	switch (M)
	{
		case 0x00: return handleop32_ADD_p11_m0(op);
		case 0x01: return handleop32_ADD_p11_m1(op);
	}

	return 0;
}

uint32_t arcompact_device::handleop32_ADD(uint32_t op)
{
	int p = (op & 0x00c00000) >> 22;

	switch (p)
	{
		case 0x00: return handleop32_ADD_p00(op);
		case 0x01: return handleop32_ADD_p01(op);
		case 0x02: return handleop32_ADD_p10(op);
		case 0x03: return handleop32_ADD_p11(op);
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

uint32_t arcompact_device::handleop32_ADC(uint32_t op)
{
	return arcompact_handle04_helper(op, arcompact_disassembler::opcodes_04[0x01], /*"ADC"*/ 0,0);
}

uint32_t arcompact_device::handleop32_SUB_p00(uint32_t op)
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
	{
		arcompact_fatal("handleop32_SUB (SUB) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_SUB_p01(uint32_t op)
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
	{
		arcompact_fatal("handleop32_SUB (SUB) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_SUB_p10(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	int32_t S = common32_get_s12(op);

	/* breg is also the destination, so this might not fetch LIMM */
	int size = check_b_limm(breg);

	uint32_t b = m_regs[breg];
	uint32_t c = (uint32_t)S;


	uint32_t result = b - c;
	m_regs[breg] = result;

	if (F)
	{
		arcompact_fatal("handleop32_SUB (SUB) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_SUB_p11_m0(uint32_t op)
{
	int size = 4;
	arcompact_fatal("handleop32_SUB_p11_m0 (SUB)\n");
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_SUB_p11_m1(uint32_t op)
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
	{
		arcompact_fatal("handleop32_SUB (SUB) (F set)\n"); // not yet supported
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_AND_p00(uint32_t op)
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
	{
		if (result & 0x80000000) { STATUS32_SET_N; }
		else { STATUS32_CLEAR_N; }
		if (result == 0x00000000) { STATUS32_SET_Z; }
		else { STATUS32_CLEAR_Z; }
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_AND_p01(uint32_t op)
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


uint32_t arcompact_device::handleop32_AND_p10(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	int32_t S = common32_get_s12(op);

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


uint32_t arcompact_device::handleop32_AND_p11_m0(uint32_t op)
{
	int size = 4;
	arcompact_fatal("handleop32_AND_p11_m0 (AND)\n");
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_AND_p11_m1(uint32_t op)
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


uint32_t arcompact_device::handleop32_OR_p00(uint32_t op)
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


uint32_t arcompact_device::handleop32_OR_p01(uint32_t op)
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


uint32_t arcompact_device::handleop32_OR_p10(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	int32_t S = common32_get_s12(op);

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


uint32_t arcompact_device::handleop32_OR_p11_m0(uint32_t op)
{
	int size = 4;
	arcompact_fatal("handleop32_OR_p11_m0 (OR)\n");
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_OR_p11_m1(uint32_t op)
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


uint32_t arcompact_device::handleop32_BIC_p00(uint32_t op)
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


uint32_t arcompact_device::handleop32_BIC_p01(uint32_t op)
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


uint32_t arcompact_device::handleop32_BIC_p10(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	int32_t S = common32_get_s12(op);

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


uint32_t arcompact_device::handleop32_BIC_p11_m0(uint32_t op)
{
	int size = 4;
	arcompact_fatal("handleop32_BIC_p11_m0 (BIC)\n");
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_BIC_p11_m1(uint32_t op)
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


uint32_t arcompact_device::handleop32_XOR_p00(uint32_t op)
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


uint32_t arcompact_device::handleop32_XOR_p01(uint32_t op)
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


uint32_t arcompact_device::handleop32_XOR_p10(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	int32_t S = common32_get_s12(op);

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


uint32_t arcompact_device::handleop32_XOR_p11_m0(uint32_t op)
{
	int size = 4;
	arcompact_fatal("handleop32_XOR_p11_m0 (XOR)\n");
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_XOR_p11_m1(uint32_t op)
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


uint32_t arcompact_device::handleop32_MOV_p00(uint32_t op)
{
	int size = 4;

	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	uint8_t creg = common32_get_creg(op);
	 //uint8_t areg = common32_get_areg(op); // areg is reserved / not used

	if (creg == LIMM_REG)
	{
		GET_LIMM_32;
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


uint32_t arcompact_device::handleop32_MOV_p01(uint32_t op)
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


uint32_t arcompact_device::handleop32_MOV_p10(uint32_t op)
{
	int size = 4;

	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	int32_t S = common32_get_s12(op);

	uint32_t c = (uint32_t)S;


	uint32_t result = c;
	m_regs[breg] = result;

	if (F)
		do_flags_nz(result);

	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_MOV_p11_m0(uint32_t op)
{
	int size = 4;
	arcompact_fatal("handleop32_MOV_p11_m0 (MOV)\n");
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_MOV_p11_m1(uint32_t op)
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


uint32_t arcompact_device::handleop32_RSUB_p00(uint32_t op)
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


uint32_t arcompact_device::handleop32_RSUB_p01(uint32_t op)
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


uint32_t arcompact_device::handleop32_RSUB_p10(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	int32_t S = common32_get_s12(op);

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


uint32_t arcompact_device::handleop32_RSUB_p11_m0(uint32_t op)
{
	int size = 4;
	arcompact_fatal("handleop32_RSUB_p11_m0 (RSUB)\n");
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_RSUB_p11_m1(uint32_t op)
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


uint32_t arcompact_device::handleop32_BSET_p00(uint32_t op)
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


uint32_t arcompact_device::handleop32_BSET_p01(uint32_t op)
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


uint32_t arcompact_device::handleop32_BSET_p10(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	int32_t S = common32_get_s12(op);

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


uint32_t arcompact_device::handleop32_BSET_p11_m0(uint32_t op)
{
	int size = 4;
	arcompact_fatal("handleop32_BSET_p11_m0 (BSET)\n");
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_BSET_p11_m1(uint32_t op)
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


uint32_t arcompact_device::handleop32_BMSK_p00(uint32_t op)
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


uint32_t arcompact_device::handleop32_BMSK_p01(uint32_t op)
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


uint32_t arcompact_device::handleop32_BMSK_p10(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	int32_t S = common32_get_s12(op);

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


uint32_t arcompact_device::handleop32_BMSK_p11_m0(uint32_t op)
{
	int size = 4;
	arcompact_fatal("handleop32_BMSK_p11_m0 (BMSK)\n");
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_BMSK_p11_m1(uint32_t op)
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

uint32_t arcompact_device::handleop32_SUB_p11(uint32_t op)
{
	int M = (op & 0x00000020) >> 5;

	switch (M)
	{
		case 0x00: return handleop32_SUB_p11_m0(op);
		case 0x01: return handleop32_SUB_p11_m1(op);
	}

	return 0;
}

uint32_t arcompact_device::handleop32_SUB(uint32_t op)
{
	int p = (op & 0x00c00000) >> 22;

	switch (p)
	{
		case 0x00: return handleop32_SUB_p00(op);
		case 0x01: return handleop32_SUB_p01(op);
		case 0x02: return handleop32_SUB_p10(op);
		case 0x03: return handleop32_SUB_p11(op);
	}

	return 0;
}

uint32_t arcompact_device::handleop32_AND_p11(uint32_t op)
{
	int M = (op & 0x00000020) >> 5;

	switch (M)
	{
		case 0x00: return handleop32_AND_p11_m0(op);
		case 0x01: return handleop32_AND_p11_m1(op);
	}

	return 0;
}

uint32_t arcompact_device::handleop32_AND(uint32_t op)
{
	int p = (op & 0x00c00000) >> 22;

	switch (p)
	{
		case 0x00: return handleop32_AND_p00(op);
		case 0x01: return handleop32_AND_p01(op);
		case 0x02: return handleop32_AND_p10(op);
		case 0x03: return handleop32_AND_p11(op);
	}

	return 0;
}

uint32_t arcompact_device::handleop32_OR_p11(uint32_t op)
{
	int M = (op & 0x00000020) >> 5;

	switch (M)
	{
		case 0x00: return handleop32_OR_p11_m0(op);
		case 0x01: return handleop32_OR_p11_m1(op);
	}

	return 0;
}

uint32_t arcompact_device::handleop32_OR(uint32_t op)
{
	int p = (op & 0x00c00000) >> 22;

	switch (p)
	{
		case 0x00: return handleop32_OR_p00(op);
		case 0x01: return handleop32_OR_p01(op);
		case 0x02: return handleop32_OR_p10(op);
		case 0x03: return handleop32_OR_p11(op);
	}

	return 0;
}

uint32_t arcompact_device::handleop32_BIC_p11(uint32_t op)
{
	int M = (op & 0x00000020) >> 5;

	switch (M)
	{
		case 0x00: return handleop32_BIC_p11_m0(op);
		case 0x01: return handleop32_BIC_p11_m1(op);
	}

	return 0;
}

uint32_t arcompact_device::handleop32_BIC(uint32_t op)
{
	int p = (op & 0x00c00000) >> 22;

	switch (p)
	{
		case 0x00: return handleop32_BIC_p00(op);
		case 0x01: return handleop32_BIC_p01(op);
		case 0x02: return handleop32_BIC_p10(op);
		case 0x03: return handleop32_BIC_p11(op);
	}

	return 0;
}

uint32_t arcompact_device::handleop32_XOR_p11(uint32_t op)
{
	int M = (op & 0x00000020) >> 5;

	switch (M)
	{
		case 0x00: return handleop32_XOR_p11_m0(op);
		case 0x01: return handleop32_XOR_p11_m1(op);
	}

	return 0;
}

uint32_t arcompact_device::handleop32_XOR(uint32_t op)
{
	int p = (op & 0x00c00000) >> 22;

	switch (p)
	{
		case 0x00: return handleop32_XOR_p00(op);
		case 0x01: return handleop32_XOR_p01(op);
		case 0x02: return handleop32_XOR_p10(op);
		case 0x03: return handleop32_XOR_p11(op);
	}

	return 0;
}

uint32_t arcompact_device::handleop32_MOV_p11(uint32_t op)
{
	int M = (op & 0x00000020) >> 5;

	switch (M)
	{
		case 0x00: return handleop32_MOV_p11_m0(op);
		case 0x01: return handleop32_MOV_p11_m1(op);
	}

	return 0;
}

uint32_t arcompact_device::handleop32_MOV(uint32_t op)
{
	int p = (op & 0x00c00000) >> 22;

	switch (p)
	{
		case 0x00: return handleop32_MOV_p00(op);
		case 0x01: return handleop32_MOV_p01(op);
		case 0x02: return handleop32_MOV_p10(op);
		case 0x03: return handleop32_MOV_p11(op);
	}

	return 0;
}

uint32_t arcompact_device::handleop32_RSUB_p11(uint32_t op)
{
	int M = (op & 0x00000020) >> 5;

	switch (M)
	{
		case 0x00: return handleop32_RSUB_p11_m0(op);
		case 0x01: return handleop32_RSUB_p11_m1(op);
	}

	return 0;
}

uint32_t arcompact_device::handleop32_RSUB(uint32_t op)
{
	int p = (op & 0x00c00000) >> 22;

	switch (p)
	{
		case 0x00: return handleop32_RSUB_p00(op);
		case 0x01: return handleop32_RSUB_p01(op);
		case 0x02: return handleop32_RSUB_p10(op);
		case 0x03: return handleop32_RSUB_p11(op);
	}

	return 0;
}

uint32_t arcompact_device::handleop32_BSET_p11(uint32_t op)
{
	int M = (op & 0x00000020) >> 5;

	switch (M)
	{
		case 0x00: return handleop32_BSET_p11_m0(op);
		case 0x01: return handleop32_BSET_p11_m1(op);
	}

	return 0;
}

uint32_t arcompact_device::handleop32_BSET(uint32_t op)
{
	int p = (op & 0x00c00000) >> 22;

	switch (p)
	{
		case 0x00: return handleop32_BSET_p00(op);
		case 0x01: return handleop32_BSET_p01(op);
		case 0x02: return handleop32_BSET_p10(op);
		case 0x03: return handleop32_BSET_p11(op);
	}

	return 0;
}

uint32_t arcompact_device::handleop32_BMSK_p11(uint32_t op)
{
	int M = (op & 0x00000020) >> 5;

	switch (M)
	{
		case 0x00: return handleop32_BMSK_p11_m0(op);
		case 0x01: return handleop32_BMSK_p11_m1(op);
	}

	return 0;
}

uint32_t arcompact_device::handleop32_BMSK(uint32_t op)
{
	int p = (op & 0x00c00000) >> 22;

	switch (p)
	{
		case 0x00: return handleop32_BMSK_p00(op);
		case 0x01: return handleop32_BMSK_p01(op);
		case 0x02: return handleop32_BMSK_p10(op);
		case 0x03: return handleop32_BMSK_p11(op);
	}

	return 0;
}

uint32_t arcompact_device::handleop32_ADD1_p11(uint32_t op)
{
	int M = (op & 0x00000020) >> 5;

	switch (M)
	{
		case 0x00: return handleop32_ADD1_p11_m0(op);
		case 0x01: return handleop32_ADD1_p11_m1(op);
	}

	return 0;
}

uint32_t arcompact_device::handleop32_ADD1(uint32_t op)
{
	int p = (op & 0x00c00000) >> 22;

	switch (p)
	{
		case 0x00: return handleop32_ADD1_p00(op);
		case 0x01: return handleop32_ADD1_p01(op);
		case 0x02: return handleop32_ADD1_p10(op);
		case 0x03: return handleop32_ADD1_p11(op);
	}

	return 0;
}

uint32_t arcompact_device::handleop32_ADD2_p11(uint32_t op)
{
	int M = (op & 0x00000020) >> 5;

	switch (M)
	{
		case 0x00: return handleop32_ADD2_p11_m0(op);
		case 0x01: return handleop32_ADD2_p11_m1(op);
	}

	return 0;
}

uint32_t arcompact_device::handleop32_ADD2(uint32_t op)
{
	int p = (op & 0x00c00000) >> 22;

	switch (p)
	{
		case 0x00: return handleop32_ADD2_p00(op);
		case 0x01: return handleop32_ADD2_p01(op);
		case 0x02: return handleop32_ADD2_p10(op);
		case 0x03: return handleop32_ADD2_p11(op);
	}

	return 0;
}

uint32_t arcompact_device::handleop32_ADD3_p11(uint32_t op)
{
	int M = (op & 0x00000020) >> 5;

	switch (M)
	{
		case 0x00: return handleop32_ADD3_p11_m0(op);
		case 0x01: return handleop32_ADD3_p11_m1(op);
	}

	return 0;
}

uint32_t arcompact_device::handleop32_ADD3(uint32_t op)
{
	int p = (op & 0x00c00000) >> 22;

	switch (p)
	{
		case 0x00: return handleop32_ADD3_p00(op);
		case 0x01: return handleop32_ADD3_p01(op);
		case 0x02: return handleop32_ADD3_p10(op);
		case 0x03: return handleop32_ADD3_p11(op);
	}

	return 0;
}

uint32_t arcompact_device::handleop32_SUB1_p11(uint32_t op)
{
	int M = (op & 0x00000020) >> 5;

	switch (M)
	{
		case 0x00: return handleop32_SUB1_p11_m0(op);
		case 0x01: return handleop32_SUB1_p11_m1(op);
	}

	return 0;
}

uint32_t arcompact_device::handleop32_SUB1(uint32_t op)
{
	int p = (op & 0x00c00000) >> 22;

	switch (p)
	{
		case 0x00: return handleop32_SUB1_p00(op);
		case 0x01: return handleop32_SUB1_p01(op);
		case 0x02: return handleop32_SUB1_p10(op);
		case 0x03: return handleop32_SUB1_p11(op);
	}

	return 0;
}

uint32_t arcompact_device::handleop32_SUB2_p11(uint32_t op)
{
	int M = (op & 0x00000020) >> 5;

	switch (M)
	{
		case 0x00: return handleop32_SUB2_p11_m0(op);
		case 0x01: return handleop32_SUB2_p11_m1(op);
	}

	return 0;
}

uint32_t arcompact_device::handleop32_SUB2(uint32_t op)
{
	int p = (op & 0x00c00000) >> 22;

	switch (p)
	{
		case 0x00: return handleop32_SUB2_p00(op);
		case 0x01: return handleop32_SUB2_p01(op);
		case 0x02: return handleop32_SUB2_p10(op);
		case 0x03: return handleop32_SUB2_p11(op);
	}

	return 0;
}


uint32_t arcompact_device::handleop32_ADD1_p00(uint32_t op)
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


uint32_t arcompact_device::handleop32_ADD1_p01(uint32_t op)
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


uint32_t arcompact_device::handleop32_ADD1_p10(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	int32_t S = common32_get_s12(op);

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


uint32_t arcompact_device::handleop32_ADD1_p11_m0(uint32_t op)
{
	int size = 4;
	arcompact_fatal("handleop32_ADD1_p11_m0 (ADD1)\n");
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_ADD1_p11_m1(uint32_t op)
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


uint32_t arcompact_device::handleop32_ADD2_p00(uint32_t op)
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


uint32_t arcompact_device::handleop32_ADD2_p01(uint32_t op)
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


uint32_t arcompact_device::handleop32_ADD2_p10(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	int32_t S = common32_get_s12(op);

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


uint32_t arcompact_device::handleop32_ADD2_p11_m0(uint32_t op)
{
	int size = 4;
	arcompact_fatal("handleop32_ADD2_p11_m0 (ADD2)\n");
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_ADD2_p11_m1(uint32_t op)
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


uint32_t arcompact_device::handleop32_ADD3_p00(uint32_t op)
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


uint32_t arcompact_device::handleop32_ADD3_p01(uint32_t op)
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


uint32_t arcompact_device::handleop32_ADD3_p10(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	int32_t S = common32_get_s12(op);

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


uint32_t arcompact_device::handleop32_ADD3_p11_m0(uint32_t op)
{
	int size = 4;
	arcompact_fatal("handleop32_ADD3_p11_m0 (ADD3)\n");
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_ADD3_p11_m1(uint32_t op)
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


uint32_t arcompact_device::handleop32_SUB1_p00(uint32_t op)
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


uint32_t arcompact_device::handleop32_SUB1_p01(uint32_t op)
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


uint32_t arcompact_device::handleop32_SUB1_p10(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	int32_t S = common32_get_s12(op);

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


uint32_t arcompact_device::handleop32_SUB1_p11_m0(uint32_t op)
{
	int size = 4;
	arcompact_fatal("handleop32_SUB1_p11_m0 (SUB1)\n");
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_SUB1_p11_m1(uint32_t op)
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


uint32_t arcompact_device::handleop32_SUB2_p00(uint32_t op)
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


uint32_t arcompact_device::handleop32_SUB2_p01(uint32_t op)
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


uint32_t arcompact_device::handleop32_SUB2_p10(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	int32_t S = common32_get_s12(op);

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


uint32_t arcompact_device::handleop32_SUB2_p11_m0(uint32_t op)
{
	int size = 4;
	arcompact_fatal("handleop32_SUB2_p11_m0 (SUB2)\n");
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_SUB2_p11_m1(uint32_t op)
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


uint32_t arcompact_device::handleop32_SUB3_p00(uint32_t op)
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


uint32_t arcompact_device::handleop32_SUB3_p01(uint32_t op)
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


uint32_t arcompact_device::handleop32_SUB3_p10(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	int32_t S = common32_get_s12(op);

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


uint32_t arcompact_device::handleop32_SUB3_p11_m0(uint32_t op)
{
	int size = 4;
	arcompact_fatal("handleop32_SUB3_p11_m0 (SUB3)\n");
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_SUB3_p11_m1(uint32_t op)
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

uint32_t arcompact_device::handleop32_SUB3_p11(uint32_t op)
{
	int M = (op & 0x00000020) >> 5;

	switch (M)
	{
		case 0x00: return handleop32_SUB3_p11_m0(op);
		case 0x01: return handleop32_SUB3_p11_m1(op);
	}

	return 0;
}

uint32_t arcompact_device::handleop32_SUB3(uint32_t op)
{
	int p = (op & 0x00c00000) >> 22;

	switch (p)
	{
		case 0x00: return handleop32_SUB3_p00(op);
		case 0x01: return handleop32_SUB3_p01(op);
		case 0x02: return handleop32_SUB3_p10(op);
		case 0x03: return handleop32_SUB3_p11(op);
	}

	return 0;
}

uint32_t arcompact_device::handleop32_Jcc_p11(uint32_t op)
{
	int M = (op & 0x00000020) >> 5;

	switch (M)
	{
		case 0x00: return handleop32_Jcc_p11_m0(op);
		case 0x01: return handleop32_Jcc_p11_m1(op);
	}

	return 0;
}

uint32_t arcompact_device::handleop32_Jcc(uint32_t op)
{
	int p = (op & 0x00c00000) >> 22;

	switch (p)
	{
		case 0x00: return handleop32_Jcc_p00(op);
		case 0x01: return handleop32_Jcc_p01(op);
		case 0x02: return handleop32_Jcc_p10(op);
		case 0x03: return handleop32_Jcc_p11(op);
	}

	return 0;
}

uint32_t arcompact_device::handleop32_Jcc_D_p11(uint32_t op)
{
	int M = (op & 0x00000020) >> 5;

	switch (M)
	{
		case 0x00: return handleop32_Jcc_D_p11_m0(op);
		case 0x01: return handleop32_Jcc_D_p11_m1(op);
	}

	return 0;
}

uint32_t arcompact_device::handleop32_Jcc_D(uint32_t op)
{
	int p = (op & 0x00c00000) >> 22;

	switch (p)
	{
		case 0x00: return handleop32_Jcc_D_p00(op);
		case 0x01: return handleop32_Jcc_D_p01(op);
		case 0x02: return handleop32_Jcc_D_p10(op);
		case 0x03: return handleop32_Jcc_D_p11(op);
	}

	return 0;
}



uint32_t arcompact_device::handleop32_LR_p00(uint32_t op)
{
	int size = 4;

	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	uint8_t creg = common32_get_creg(op);
	 //uint8_t areg = common32_get_areg(op); // areg is reserved / not used

	if (creg == LIMM_REG)
	{
		GET_LIMM_32;
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


uint32_t arcompact_device::handleop32_LR_p01(uint32_t op)
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


uint32_t arcompact_device::handleop32_LR_p10(uint32_t op)
{
	int size = 4;

	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	int32_t S = common32_get_s12(op);

	uint32_t c = (uint32_t)S;


	m_regs[breg] = READAUX(c);


	if (F)
	{
		// no flag changes
	}
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_LR_p11_m0(uint32_t op)
{
	int size = 4;
	arcompact_fatal("handleop32_LR_p11_m0 (LR)\n");
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_LR_p11_m1(uint32_t op)
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

uint32_t arcompact_device::handleop32_LR_p11(uint32_t op)
{
	int M = (op & 0x00000020) >> 5;

	switch (M)
	{
		case 0x00: return handleop32_LR_p11_m0(op);
		case 0x01: return handleop32_LR_p11_m1(op);
	}

	return 0;
}

uint32_t arcompact_device::handleop32_LR(uint32_t op)
{
	int p = (op & 0x00c00000) >> 22;

	switch (p)
	{
		case 0x00: return handleop32_LR_p00(op);
		case 0x01: return handleop32_LR_p01(op);
		case 0x02: return handleop32_LR_p10(op);
		case 0x03: return handleop32_LR_p11(op);
	}

	return 0;
}


uint32_t arcompact_device::handleop32_SR_p00(uint32_t op)
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


uint32_t arcompact_device::handleop32_SR_p01(uint32_t op)
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


uint32_t arcompact_device::handleop32_SR_p10(uint32_t op)
{
	uint8_t breg = common32_get_breg(op);
	uint8_t F = common32_get_F(op);
	int32_t S = common32_get_s12(op);

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


uint32_t arcompact_device::handleop32_SR_p11_m0(uint32_t op)
{
	int size = 4;
	arcompact_fatal("handleop32_SR_p11_m0 (SR)\n");
	return m_pc + (size >> 0);
}


uint32_t arcompact_device::handleop32_SR_p11_m1(uint32_t op)
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

uint32_t arcompact_device::handleop32_SR_p11(uint32_t op)
{
	int M = (op & 0x00000020) >> 5;

	switch (M)
	{
		case 0x00: return handleop32_SR_p11_m0(op);
		case 0x01: return handleop32_SR_p11_m1(op);
	}

	return 0;
}

uint32_t arcompact_device::handleop32_SR(uint32_t op)
{
	int p = (op & 0x00c00000) >> 22;

	switch (p)
	{
		case 0x00: return handleop32_SR_p00(op);
		case 0x01: return handleop32_SR_p01(op);
		case 0x02: return handleop32_SR_p10(op);
		case 0x03: return handleop32_SR_p11(op);
	}

	return 0;
}



uint32_t arcompact_device::handleop32_SBC(uint32_t op)
{
	return arcompact_handle04_helper(op, arcompact_disassembler::opcodes_04[0x03], /*"SBC"*/ 0,0);
}

uint32_t arcompact_device::handleop32_MAX(uint32_t op)
{
	return arcompact_handle04_helper(op, arcompact_disassembler::opcodes_04[0x08], /*"MAX"*/ 0,0);
}

uint32_t arcompact_device::handleop32_MIN(uint32_t op)
{
	return arcompact_handle04_helper(op, arcompact_disassembler::opcodes_04[0x09], /*"MIN"*/ 0,0);
}

uint32_t arcompact_device::handleop32_TST(uint32_t op)
{
	return arcompact_handle04_helper(op, arcompact_disassembler::opcodes_04[0x0b], /*"TST"*/ 1,0);
}

uint32_t arcompact_device::handleop32_CMP(uint32_t op)
{
	return arcompact_handle04_helper(op, arcompact_disassembler::opcodes_04[0x0c], /*"CMP"*/ 1,0);
}

uint32_t arcompact_device::handleop32_RCMP(uint32_t op)
{
	return arcompact_handle04_helper(op, arcompact_disassembler::opcodes_04[0x0d], /*"RCMP"*/ 1,0);
}

uint32_t arcompact_device::handleop32_BCLR(uint32_t op)
{
	return arcompact_handle04_helper(op, arcompact_disassembler::opcodes_04[0x10], /*"BCLR"*/ 0,0);
}

uint32_t arcompact_device::handleop32_BTST(uint32_t op)
{
	return arcompact_handle04_helper(op, arcompact_disassembler::opcodes_04[0x11], /*"BTST"*/ 0,0);
}

uint32_t arcompact_device::handleop32_BXOR(uint32_t op)
{
	return arcompact_handle04_helper(op, arcompact_disassembler::opcodes_04[0x12], /*"BXOR"*/ 0,0);
}

uint32_t arcompact_device::handleop32_MPY(uint32_t op)
{
	return arcompact_handle04_helper(op, arcompact_disassembler::opcodes_04[0x1a], /*"MPY"*/ 0,0);
} // *

uint32_t arcompact_device::handleop32_MPYH(uint32_t op)
{
	return arcompact_handle04_helper(op, arcompact_disassembler::opcodes_04[0x1b], /*"MPYH"*/ 0,0);
} // *

uint32_t arcompact_device::handleop32_MPYHU(uint32_t op)
{
	return arcompact_handle04_helper(op, arcompact_disassembler::opcodes_04[0x1c], /*"MPYHU"*/ 0,0);
} // *

uint32_t arcompact_device::handleop32_MPYU(uint32_t op)
{
	return arcompact_handle04_helper(op, arcompact_disassembler::opcodes_04[0x1d], /*"MPYU"*/ 0,0);
} // *

uint32_t arcompact_device::handleop32_Jcc_p00(uint32_t op)
{
	int size;

	uint8_t creg = common32_get_creg(op);
	uint8_t F = common32_get_F(op);

	if (creg == LIMM_REG)
	{
		// opcode          iiii i--- ppII IIII F--- CCCC CC-- ----
		// J limm          0010 0RRR 0010 0000 0RRR 1111 10RR RRRR  [LIMM]  (creg = LIMM)

		GET_LIMM_32;
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

uint32_t arcompact_device::handleop32_Jcc_p01(uint32_t op)
{
	// opcode          iiii i--- ppII IIII F--- uuuu uu-- ----
	// J u6            0010 0RRR 0110 0000 0RRR uuuu uuRR RRRR
	int size = 4;
	arcompact_log("2 unimplemented J %08x", op);
	return m_pc + (size>>0);
}

uint32_t arcompact_device::handleop32_Jcc_p10(uint32_t op)
{
	// opcode          iiii i--- ppII IIII F--- ssss ssSS SSSS
	// J s12           0010 0RRR 1010 0000 0RRR ssss ssSS SSSS
	int size = 4;
	arcompact_log("3 unimplemented J %08x", op);
	return m_pc + (size>>0);
}


uint32_t arcompact_device::handleop32_Jcc_p11_m0(uint32_t op) // Jcc   (no link, no delay)
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
		GET_LIMM_32;
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
			arcompact_fatal ("fatal handleop32_Jcc_p11_m0 J %08x (F not set but ILINK1 or ILINK2 used as dst)", op);
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
			arcompact_log("unimplemented handleop32_Jcc_p11_m0 J %08x (F set)", op);
		}
		else
		{
			arcompact_fatal ("fatal handleop32_Jcc_p11_m0 J %08x (F set but not ILINK1 or ILINK2 used as dst)", op);

		}
	}


	return m_pc + (size>>0);
}

uint32_t arcompact_device::handleop32_Jcc_p11_m1(uint32_t op)
{
	// opcode          iiii i--- ppII IIII F--- uuuu uumq qqqq
	// Jcc u6          0010 0RRR 1110 0000 0RRR uuuu uu1Q QQQQ
	int size = 4;
	arcompact_log("unimplemented handleop32_Jcc_p11_m1 J %08x (u6)", op);
	return m_pc + (size>>0);
}


uint32_t arcompact_device::handleop32_Jcc_D_p00(uint32_t op)
{
	int size = 4;

	uint8_t creg = common32_get_creg(op);
	uint8_t F = common32_get_F(op);

	if (creg == LIMM_REG)
	{
		GET_LIMM_32;
		size = 8;

		handle_jump_to_addr(1,0,m_regs[LIMM_REG], m_pc + (size >> 0));
	}
	else
	{
		return handle_jump_to_register(1,0,creg, m_pc + (size>>0), F); // delay, no link
	}

	return m_pc + (size>>0);
}

uint32_t arcompact_device::handleop32_Jcc_D_p01(uint32_t op)
{
	int size = 4;
	arcompact_log("unimplemented J.D (u6 type) %08x", op);
	return m_pc + (size>>0);
}

uint32_t arcompact_device::handleop32_Jcc_D_p10(uint32_t op)
{
	int size = 4;
	arcompact_log("unimplemented J.D (s12 type) %08x", op);
	return m_pc + (size>>0);
}


uint32_t arcompact_device::handleop32_Jcc_D_p11_m0(uint32_t op) // Jcc.D   (no link, delay)
{
	int size = 4;

	uint8_t creg = common32_get_creg(op);
	uint8_t condition = common32_get_condition(op);
	uint8_t F = common32_get_F(op);

	//uint32_t c = 0;

	if (creg == LIMM_REG)
	{
		GET_LIMM_32;
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

uint32_t arcompact_device::handleop32_Jcc_D_p11_m1(uint32_t op)
{
	int size = 4;
	arcompact_log("unimplemented handleop32_Jcc_D_p11_m1 J.D %08x (u6)", op);
	return m_pc + (size>>0);
}

uint32_t arcompact_device::handleop32_JLcc(uint32_t op)
{
	return arcompact_handle04_helper(op, arcompact_disassembler::opcodes_04[0x22], /*"JL"*/ 1,1);
}

uint32_t arcompact_device::handleop32_JLcc_D(uint32_t op)
{
	return arcompact_handle04_helper(op, arcompact_disassembler::opcodes_04[0x23], /*"JL.D"*/ 1,1);
}

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
		int32_t S = common32_get_s12(op);
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
			uint32_t realoffset = PC_ALIGNED32 + (u * 2);
			return realoffset;
		}
		else
		{
			// otherwise set up the loop positions
			m_LP_START = m_pc + (size >> 0);
			m_LP_END = PC_ALIGNED32 + (u * 2);
			return m_pc + (size>>0);
		}

	}

	return m_pc + (size>>0);
}
