// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "arcompact.h"
#include "arcompactdasm.h"

uint32_t arcompact_device::arcompact_handle05_2f_0x_helper(uint32_t op, const char* optext)
{
	int size;

	int p = common32_get_p(op);
	//uint8_t breg = common32_get_breg(op);

	if (p == 0)
	{
		uint8_t creg = common32_get_creg(op);

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

	arcompact_log("unimplemented %s %08x", optext, op);
	return m_pc + (size>>0);
}

uint32_t arcompact_device::handleop32_SWAP(uint32_t op)  { return arcompact_handle05_2f_0x_helper(op, "SWAP");  }
uint32_t arcompact_device::handleop32_NORM(uint32_t op)  { return arcompact_handle05_2f_0x_helper(op, "NORM");  }
uint32_t arcompact_device::handleop32_SAT16(uint32_t op)  { return arcompact_handle05_2f_0x_helper(op, "SAT16"); }
uint32_t arcompact_device::handleop32_RND16(uint32_t op)  { return arcompact_handle05_2f_0x_helper(op, "RND16"); }
uint32_t arcompact_device::handleop32_ABSSW(uint32_t op)  { return arcompact_handle05_2f_0x_helper(op, "ABSSW"); }
uint32_t arcompact_device::handleop32_ABSS(uint32_t op)  { return arcompact_handle05_2f_0x_helper(op, "ABSS");  }
uint32_t arcompact_device::handleop32_NEGSW(uint32_t op)  { return arcompact_handle05_2f_0x_helper(op, "NEGSW"); }
uint32_t arcompact_device::handleop32_NEGS(uint32_t op)  { return arcompact_handle05_2f_0x_helper(op, "NEGS");  }
uint32_t arcompact_device::handleop32_NORMW(uint32_t op)  { return arcompact_handle05_2f_0x_helper(op, "NORMW"); }
