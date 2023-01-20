// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "arcompact.h"
#include "arcompactdasm.h"

uint32_t arcompact_device::arcompact_handle04_2f_helper(uint32_t op, const char* optext)
{
	int size;

	COMMON32_GET_p;
	//COMMON32_GET_breg;

	if (p == 0)
	{
		COMMON32_GET_creg

		if (creg == LIMM_REG)
		{
			//uint32_t limm;
			//GET_LIMM_32;
			size = 8;
		}
		else
		{
		}
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

uint32_t arcompact_device::handleop32_ASL_single(uint32_t op)  { return arcompact_handle04_2f_helper(op, "ASL"); } // ASL
uint32_t arcompact_device::handleop32_ASR_single(uint32_t op)  { return arcompact_handle04_2f_helper(op, "ASR"); } // ASR

uint32_t arcompact_device::handleop32_RRC(uint32_t op)  { return arcompact_handle04_2f_helper(op, "RCC"); } // RCC
uint32_t arcompact_device::handleop32_SEXB(uint32_t op)  { return arcompact_handle04_2f_helper(op, "SEXB"); } // SEXB
uint32_t arcompact_device::handleop32_SEXW(uint32_t op)  { return arcompact_handle04_2f_helper(op, "SEXW"); } // SEXW


uint32_t arcompact_device::handleop32_ABS(uint32_t op)  { return arcompact_handle04_2f_helper(op, "ABS"); } // ABS
uint32_t arcompact_device::handleop32_NOT(uint32_t op)  { return arcompact_handle04_2f_helper(op, "NOT"); } // NOT
uint32_t arcompact_device::handleop32_RLC(uint32_t op)  { return arcompact_handle04_2f_helper(op, "RCL"); } // RLC
uint32_t arcompact_device::handleop32_EX(uint32_t op)  { return arcompact_handle04_2f_helper(op, "EX"); } // EX
