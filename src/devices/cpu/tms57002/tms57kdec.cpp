// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    tms57kdec.cpp

    TMS57002 "DASP" emulator.

***************************************************************************/

#include "emu.h"
#include "tms57002.h"
#include <algorithm>

inline int tms57002_device::xmode(u32 opcode, char type, cstate *cs)
{
	if (((opcode & 0x400) && (type == 'c')) || (!(opcode & 0x400) && (type == 'd')))
	{
		if (opcode & 0x100)
			return 0;
		else if (opcode & 0x80)
			cs->inc |= type == 'c' ? INC_CA : INC_ID;

		return 1;
	}
	else if (opcode & 0x200)
		cs->inc |= type == 'c' ? INC_CA : INC_ID;

	return 1;
}

inline int tms57002_device::sfao(u32 st1)
{
	return st1 & ST1_SFAO ? 1 : 0;
}

inline int tms57002_device::dbp(u32 st1)
{
	return st1 & ST1_DBP ? 1 : 0;
}

inline int tms57002_device::crm(u32 st1)
{
	// value overridden during cvar update
	if (update_counter_head != update_counter_tail)
		return 0;

	int crm = (st1 & ST1_CRM) >> ST1_CRM_SHIFT;
	return crm <= 2 ? crm : 0;
}

inline int tms57002_device::sfai(u32 st1)
{
	return st1 & ST1_SFAI ? 1 : 0;
}

inline int tms57002_device::sfmo(u32 st1)
{
	return (st1 & ST1_SFMO) >> ST1_SFMO_SHIFT;
}

inline int tms57002_device::rnd(u32 st1)
{
	int rnd = (st1 & ST1_RND) >> ST1_RND_SHIFT;
	return rnd <= 4 ? rnd : 0;
}

inline int tms57002_device::movm(u32 st1)
{
	return st1 & ST1_MOVM ? 1 : 0;
}

inline int tms57002_device::sfma(u32 st1)
{
	return (st1 & ST1_SFMA) >> ST1_SFMA_SHIFT;
}

void tms57002_device::decode_error(u32 opcode)
{
	if (unsupported_inst_warning)
		return;

	unsupported_inst_warning = 1;
	popmessage("tms57002: %06x - Contact Mamedev", opcode);
}

void tms57002_device::decode_cat1(u32 opcode, unsigned short *op, cstate *cs)
{
	switch (opcode >> 18)
	{
	case 0x00: // nop
		break;

#define CDEC1
#include "cpu/tms57002/tms57002.hxx"
#undef CDEC1

	default:
		logerror("Unhandled cat1 opcode %02x\n",opcode >> 18);
		decode_error(opcode);
		break;
	}
}

void tms57002_device::decode_cat2_pre(u32 opcode, unsigned short *op, cstate *cs)
{
	switch ((opcode >> 11) & 0x7f)
	{
	case 0x00: // nop
		break;

#define CDEC2A
#include "cpu/tms57002/tms57002.hxx"
#undef CDEC2A

	default:
		logerror("Unhandled cat2_pre opcode %02x \n",(opcode >> 11) & 0x7f);
		decode_error(opcode);
		break;
	}
}

void tms57002_device::decode_cat2_post(u32 opcode, unsigned short *op, cstate *cs)
{
	switch ((opcode >> 11) & 0x7f)
	{
	case 0x00: // nop
		break;

#define CDEC2B
#include "cpu/tms57002/tms57002.hxx"
#undef CDEC2B

	default:
		logerror("Unhandled cat2_post opcode %02x\n",(opcode >> 11) & 0x7f);
		decode_error(opcode);
		break;
	}
}

void tms57002_device::decode_cat3(u32 opcode, unsigned short *op, cstate *cs)
{
	switch ((opcode >> 11) & 0x7f)
	{
	case 0x00: // nop
		break;

#define CDEC3
#include "cpu/tms57002/tms57002.hxx"
#undef CDEC3

	default:
		logerror("Unhandled cat3  opcode %02x\n",(opcode >> 11) & 0x7f);
		decode_error(opcode);
		break;
	}
}
