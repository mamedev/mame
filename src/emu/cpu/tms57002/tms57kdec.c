/***************************************************************************

    tms57kdec.c

    TMS57002 "DASP" emulator.

****************************************************************************

    Copyright Olivier Galibert
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

***************************************************************************/

#include "emu.h"
#include "debugger.h"
#include "tms57002.h"

inline int tms57002_device::xmode(UINT32 opcode, char type)
{
	if(((opcode & 0x400) && (type == 'c')) || (!(opcode & 0x400) && (type == 'd'))) {
		if(opcode & 0x100)
			return 0;
		else if(opcode & 0x80)
			return 2;
		else
			return 1;
	} else if(opcode & 0x200)
		return 2;

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
	return (st1 & ST1_CRM) >> ST1_CRM_SHIFT;
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
	return (st1 & ST1_RND) >> ST1_RND_SHIFT;
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

