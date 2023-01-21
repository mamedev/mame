// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "arcompact.h"
#include "arcompactdasm.h"


uint32_t arcompact_device::arcompact_handle04_3x_helper(uint32_t op, int dsize, int extend)
{
	int size;

	COMMON32_GET_breg;
	COMMON32_GET_creg

	if ((breg == LIMM_REG) || (creg == LIMM_REG))
	{
		GET_LIMM_32;
		size = 8;
	}

	arcompact_log("unimplemented LD %08x (type 04_3x)", op);
	return m_pc + (size>>0);
}

uint32_t arcompact_device::handleop32_LD_0(uint32_t op)  { return arcompact_handle04_3x_helper(op,0,0); }
// ZZ value of 0x0 with X of 1 is illegal
uint32_t arcompact_device::handleop32_LD_1(uint32_t op)  { return arcompact_handle04_3x_helper(op,0,1); }
uint32_t arcompact_device::handleop32_LD_2(uint32_t op)  { return arcompact_handle04_3x_helper(op,1,0); }
uint32_t arcompact_device::handleop32_LD_3(uint32_t op)  { return arcompact_handle04_3x_helper(op,1,1); }
uint32_t arcompact_device::handleop32_LD_4(uint32_t op)  { return arcompact_handle04_3x_helper(op,2,0); }
uint32_t arcompact_device::handleop32_LD_5(uint32_t op)  { return arcompact_handle04_3x_helper(op,2,1); }
// ZZ value of 0x3 is illegal
uint32_t arcompact_device::handleop32_LD_6(uint32_t op)  { return arcompact_handle04_3x_helper(op,3,0); }
uint32_t arcompact_device::handleop32_LD_7(uint32_t op)  { return arcompact_handle04_3x_helper(op,3,1); }
