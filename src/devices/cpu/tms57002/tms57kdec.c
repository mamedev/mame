// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    tms57kdec.c

    TMS57002 "DASP" emulator.

***************************************************************************/

#include "emu.h"
#include "debugger.h"
#include "tms57002.h"

inline int tms57002_device::xmode(UINT32 opcode, char type, cstate *cs)
{
	if(((opcode & 0x400) && (type == 'c')) || (!(opcode & 0x400) && (type == 'd'))) {
		if(opcode & 0x100)
			return 0;
		else if(opcode & 0x80)
			cs->inc |= type == 'c' ? INC_CA : INC_ID;

		return 1;
	}
	else if(opcode & 0x200)
		cs->inc |= type == 'c' ? INC_CA : INC_ID;

	return 1;
}

inline int tms57002_device::sfao(UINT32 st1)
{
	return st1 & ST1_SFAO ? 1 : 0;
}

inline int tms57002_device::dbp(UINT32 st1)
{
	return st1 & ST1_DBP ? 1 : 0;
}

inline int tms57002_device::crm(UINT32 st1)
{
	int crm = (st1 & ST1_CRM) >> ST1_CRM_SHIFT;
	return crm <= 2 ? crm : 0;
}

inline int tms57002_device::sfai(UINT32 st1)
{
	return st1 & ST1_SFAI ? 1 : 0;
}

inline int tms57002_device::sfmo(UINT32 st1)
{
	return (st1 & ST1_SFMO) >> ST1_SFMO_SHIFT;
}

inline int tms57002_device::rnd(UINT32 st1)
{
	int rnd = (st1 & ST1_RND) >> ST1_RND_SHIFT;
	return rnd <= 4 ? rnd : 0;
}

inline int tms57002_device::movm(UINT32 st1)
{
	return st1 & ST1_MOVM ? 1 : 0;
}

inline int tms57002_device::sfma(UINT32 st1)
{
	return (st1 & ST1_SFMA) >> ST1_SFMA_SHIFT;
}

void tms57002_device::decode_error(UINT32 opcode)
{
	char buf[256];
	UINT8 opr[3];
	if(unsupported_inst_warning)
		return;

	unsupported_inst_warning = 1;
	opr[0] = opcode;
	opr[1] = opcode >> 8;
	opr[2] = opcode >> 16;

	disasm_disassemble(buf, pc, opr, opr, 0);
	popmessage("tms57002: %s - Contact Mamedev", buf);
}

void tms57002_device::decode_cat1(UINT32 opcode, unsigned short *op, cstate *cs)
{
	switch(opcode >> 18) {
	case 0x00: // nop
		break;

#define CDEC1
#include "cpu/tms57002/tms57002.inc"
#undef CDEC1

	default:
		decode_error(opcode);
		break;
	}
}

void tms57002_device::decode_cat2_pre(UINT32 opcode, unsigned short *op, cstate *cs)
{
	switch((opcode >> 11) & 0x7f) {
	case 0x00: // nop
		break;

#define CDEC2A
#include "cpu/tms57002/tms57002.inc"
#undef CDEC2A

	default:
		decode_error(opcode);
		break;
	}
}

void tms57002_device::decode_cat2_post(UINT32 opcode, unsigned short *op, cstate *cs)
{
	switch((opcode >> 11) & 0x7f) {
	case 0x00: // nop
		break;

#define CDEC2B
#include "cpu/tms57002/tms57002.inc"
#undef CDEC2B

	default:
		decode_error(opcode);
		break;
	}
}

void tms57002_device::decode_cat3(UINT32 opcode, unsigned short *op, cstate *cs)
{
	switch((opcode >> 11) & 0x7f) {
	case 0x00: // nop
		break;

#define CDEC3
#include "cpu/tms57002/tms57002.inc"
#undef CDEC3

	default:
		decode_error(opcode);
		break;
	}
}
