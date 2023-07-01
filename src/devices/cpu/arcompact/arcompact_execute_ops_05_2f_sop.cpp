// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "arcompact_helper.ipp"

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// SWAP - Swap words (optional extension on ARCtangent-A5 / ARC600, built in on ARC700)
//
// SWAP<.f> b,c                    0010 1bbb 0010 1111   FBBB CCCC CC00 0000
// SWAP<.f> b,u6                   0010 1bbb 0110 1111   FBBB uuuu uu00 0000
// SWAP<.f> b,limm                 0010 1bbb 0010 1111   FBBB 1111 1000 0000 (+ Limm)
//
// SWAP<.f> 0,c                    0010 1110 0010 1111   F111 CCCC CC00 0000
// SWAP<.f> 0,u6                   0010 1110 0110 1111   F111 uuuu uu00 0000
// SWAP<.f> 0,limm                 0010 1110 0010 1111   F111 1111 1000 0000 (+ Limm)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

uint32_t arcompact_device::handleop32_SWAP_do_op(arcompact_device &o, uint32_t src, bool set_flags)
{
	uint32_t result = (((src & 0xffff0000) >> 16) | ((src & 0x0000ffff) << 16));

	if (set_flags)
		o.do_flags_nz(result);

	return result;
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

uint32_t arcompact_device::handleop32_NORM_do_op(arcompact_device &o, uint32_t src, bool set_flags)
{
	uint32_t result;

	if ((src == 0xffffffff) || (src == 0x00000000))
		result = 0x1f;
	else if (src & 0x80000000)
		result = count_leading_ones_32(src);
	else
		result = count_leading_zeros_32(src);

	if (set_flags)
		o.do_flags_nz(src);

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

uint32_t arcompact_device::handleop32_SAT16_do_op(arcompact_device& o, uint32_t src, bool set_flags)
{
	fatalerror("Unhandled SAT16");
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

uint32_t arcompact_device::handleop32_RND16_do_op(arcompact_device& o, uint32_t src, bool set_flags)
{
	fatalerror("Unhandled RND16");
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

uint32_t arcompact_device::handleop32_ABSSW_do_op(arcompact_device& o, uint32_t src, bool set_flags)
{
	fatalerror("Unhandled ABSSW");
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

uint32_t arcompact_device::handleop32_ABSS_do_op(arcompact_device& o, uint32_t src, bool set_flags)
{
	fatalerror("Unhandled ABSS");
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

uint32_t arcompact_device::handleop32_NEGSW_do_op(arcompact_device& o, uint32_t src, bool set_flags)
{
	fatalerror("Unhandled NEGSW");
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

uint32_t arcompact_device::handleop32_NEGS_do_op(arcompact_device& o, uint32_t src, bool set_flags)
{
	fatalerror("Unhandled NEGS");
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

uint32_t arcompact_device::handleop32_NORMW_do_op(arcompact_device &o, uint32_t src, bool set_flags)
{
	uint32_t result;

	uint32_t source = src;
	if (source & 0x00008000)
		source |= 0xffff0000;
	else
		source &= 0x0000ffff;

	if ((source == 0x0000ffff) || (source == 0x00000000))
		result = 0x0f;
	else if (source & 0x00008000)
		result = count_leading_ones_32(source) - 0x10;
	else
		result = count_leading_zeros_32(source) - 0x10;

	if (set_flags)
		fatalerror("handleop32_NORMW (F set)\n"); // not yet supported

	return result;
}
