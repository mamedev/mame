// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    57002dsm.c

    TMS57002 "DASP" emulator.

***************************************************************************/

#include "emu.h"
#include "debugger.h"
#include "tms57002.h"

static const char *get_memadr(UINT32 opcode, char type)
{
	static char buff[2][10];
	static int index = 0;
	char *buf;

	index = 1-index;
	buf = buff[index];

	if(((opcode & 0x400) && (type == 'c')) || (!(opcode & 0x400) && (type == 'd'))) {
		if(opcode & 0x100)
			sprintf(buf, "%c(%02x)", type, opcode & 0xff);
		else if(opcode & 0x80)
			sprintf(buf, "%c*+", type);
		else
			sprintf(buf, "%c*", type);
	} else if(opcode & 0x200)
		sprintf(buf, "%c*+", type);
	else
		sprintf(buf, "%c*", type);
	return buf;
}


CPU_DISASSEMBLE(tms57002)
{
	UINT32 opcode = opram[0] | (opram[1] << 8) | (opram[2] << 16);
	UINT8 fa = opcode >> 18;
	char *buf = buffer;
	if(fa == 0x3f) {
		switch((opcode >> 11) & 0x7f) { // category 3

#define DASM3
#include "cpu/tms57002/tms57002.inc"
#undef  DASM3

		default:
			sprintf(buf, "unk c3 %02x", (opcode >> 11) & 0x7f);
			break;
		}
	} else {
		switch(fa) { // category 1
		case 0x00:
			buf[0] = 0;
			break;

#define DASM1
#include "cpu/tms57002/tms57002.inc"
#undef  DASM1

		default:
			sprintf(buf, "unk c1 %02x", fa);
			break;
		}

		buf += strlen(buf);
		if(buf != buffer) {
			strcpy(buf, " ; ");
			buf += 3;
		}

		switch((opcode >> 11) & 0x7f) { // category 2
		case 0x00:
			if(buf != buffer)
				buf[-3] = 0;
			else
				sprintf(buf, "nop");
			break;

#define DASM2
#include "cpu/tms57002/tms57002.inc"
#undef  DASM2

		default:
			sprintf(buf, "unk c2 %02x", (opcode >> 11) & 0x7f);
			break;
		}
	}

	return 1;
}
