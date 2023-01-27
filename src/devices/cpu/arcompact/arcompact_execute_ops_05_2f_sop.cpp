// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "arcompact.h"

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
			//get_limm_32bit_opcode();
			size = 8;
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
	return m_pc + size;
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// SWAP<.f> b,c                    0010 1bbb 0010 1111   FBBB CCCC CC00 0000
// SWAP<.f> b,u6                   0010 1bbb 0110 1111   FBBB uuuu uu00 0000
// SWAP<.f> b,limm                 0010 1bbb 0010 1111   FBBB 1111 1000 0000 (+ Limm)
//
// SWAP<.f> 0,c                    0010 1110 0010 1111   F111 CCCC CC00 0000
// SWAP<.f> 0,u6                   0010 1110 0110 1111   F111 uuuu uu00 0000
// SWAP<.f> 0,limm                 0010 1110 0010 1111   F111 1111 1000 0000 (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_SWAP(uint32_t op)
{
	return arcompact_handle05_2f_0x_helper(op, "SWAP");
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// NORM<.f> b,c                    0010 1bbb 0010 1111   FBBB CCCC CC00 0001
// NORM<.f> b,u6                   0010 1bbb 0110 1111   FBBB uuuu uu00 0001
// NORM<.f> b,limm                 0010 1bbb 0010 1111   FBBB 1111 1000 0001 (+ Limm)
//
// NORM<.f> 0,c                    0010 1110 0010 1111   F111 CCCC CC00 0001
// NORM<.f> 0,u6                   0010 1110 0110 1111   F111 uuuu uu00 0001
// NORM<.f> 0,limm                 0010 1110 0010 1111   F111 1111 1000 0001 (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_NORM_do_op(uint32_t src, uint8_t set_flags)
{
	uint32_t result;

	if ((src == 0xffffffff) || (src = 0x00000000))
		result = 0x1f;
	else if (src & 0x80000000)
		result = count_leading_ones_32(src);
	else
		result = count_leading_zeros_32(src);

	if (set_flags)
		do_flags_nz(src);

	return result;
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// SAT16<.f> b,c                   0010 1bbb 0010 1111   FBBB CCCC CC00 0010
// SAT16<.f> b,u6                  0010 1bbb 0110 1111   FBBB uuuu uu00 0010
// SAT16<.f> b,limm                0010 1bbb 0010 1111   FBBB 1111 1000 0010 (+ Limm)
//
// SAT16<.f> 0,c                   0010 1110 0010 1111   F111 CCCC CC00 0010
// SAT16<.f> 0,u6                  0010 1110 0110 1111   F111 uuuu uu00 0010
// SAT16<.f> 0,limm                0010 1110 0010 1111   F111 1111 1000 0010 (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_SAT16(uint32_t op)
{
	return arcompact_handle05_2f_0x_helper(op, "SAT16");
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// RND16<.f> b,c                   0010 1bbb 0010 1111   FBBB CCCC CC00 0011
// RND16<.f> b,u6                  0010 1bbb 0110 1111   FBBB uuuu uu00 0011
// RND16<.f> b,limm                0010 1bbb 0010 1111   FBBB 1111 1000 0011 (+ Limm)
//
// RND16<.f> 0,c                   0010 1110 0010 1111   F111 CCCC CC00 0011
// RND16<.f> 0,u6                  0010 1110 0110 1111   F111 uuuu uu00 0011
// RND16<.f> 0,limm                0010 1110 0010 1111   F111 1111 1000 0011 (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_RND16(uint32_t op)
{
	return arcompact_handle05_2f_0x_helper(op, "RND16");
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ABSSW<.f> b,c                   0010 1bbb 0010 1111   FBBB CCCC CC00 0100
// ABSSW<.f> b,u6                  0010 1bbb 0110 1111   FBBB uuuu uu00 0100
// ABSSW<.f> b,limm                0010 1bbb 0010 1111   FBBB 1111 1000 0100 (+ Limm)
//
// ABSSW<.f> 0,c                   0010 1110 0010 1111   F111 CCCC CC00 0100
// ABSSW<.f> 0,u6                  0010 1110 0110 1111   F111 uuuu uu00 0100
// ABSSW<.f> 0,limm                0010 1110 0010 1111   F111 1111 1000 0100 (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_ABSSW(uint32_t op)
{
	return arcompact_handle05_2f_0x_helper(op, "ABSSW");
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ABSS<.f> b,c                    0010 1bbb 0010 1111   FBBB CCCC CC00 0101
// ABSS<.f> b,u6                   0010 1bbb 0110 1111   FBBB uuuu uu00 0101
// ABSS<.f> b,limm                 0010 1bbb 0010 1111   FBBB 1111 1000 0101 (+ Limm)
//
// ABSS<.f> 0,c                    0010 1110 0010 1111   F111 CCCC CC00 0101
// ABSS<.f> 0,u6                   0010 1110 0110 1111   F111 uuuu uu00 0101
// ABSS<.f> 0,limm                 0010 1110 0010 1111   F111 1111 1000 0101 (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_ABSS(uint32_t op)
{
	return arcompact_handle05_2f_0x_helper(op, "ABSS");
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// NEGSW<.f> b,c                   0010 1bbb 0010 1111   FBBB CCCC CC00 0110
// NEGSW<.f> b,u6                  0010 1bbb 0110 1111   FBBB uuuu uu00 0110
// NEGSW<.f> b,limm                0010 1bbb 0010 1111   FBBB 1111 1000 0110 (+ Limm)
//
// NEGSW<.f> 0,c                   0010 1110 0010 1111   F111 CCCC CC00 0110
// NEGSW<.f> 0,u6                  0010 1110 0110 1111   F111 uuuu uu00 0110
// NEGSW<.f> 0,limm                0010 1110 0010 1111   F111 1111 1000 0110 (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_NEGSW(uint32_t op)
{
	return arcompact_handle05_2f_0x_helper(op, "NEGSW");
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// NEGS<.f> b,c                    0010 1bbb 0010 1111   FBBB CCCC CC00 0111
// NEGS<.f> b,u6                   0010 1bbb 0110 1111   FBBB uuuu uu00 0111
// NEGS<.f> b,limm                 0010 1bbb 0010 1111   FBBB 1111 1000 0111 (+ Limm)
//
// NEGS<.f> 0,c                    0010 1110 0010 1111   F111 CCCC CC00 0111
// NEGS<.f> 0,u6                   0010 1110 0110 1111   F111 uuuu uu00 0111
// NEGS<.f> 0,limm                 0010 1110 0010 1111   F111 1111 1000 0111 (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_NEGS(uint32_t op)
{
	return arcompact_handle05_2f_0x_helper(op, "NEGS");
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// NORMW<.f> b,c                   0010 1bbb 0010 1111   FBBB CCCC CC00 1000
// NORMW<.f> b,u6                  0010 1bbb 0110 1111   FBBB uuuu uu00 1000
// NORMW<.f> b,limm                0010 1bbb 0010 1111   FBBB 1111 1000 1000 (+ Limm)
//
// NORMW<.f> 0,c                   0010 1110 0010 1111   F111 CCCC CC00 1000
// NORMW<.f> 0,u6                  0010 1110 0110 1111   F111 uuuu uu00 1000
// NORMW<.f> 0,limm                0010 1110 0010 1111   F111 1111 1000 1000 (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_NORMW(uint32_t op)
{
	return arcompact_handle05_2f_0x_helper(op, "NORMW");
}
