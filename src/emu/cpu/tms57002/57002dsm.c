/***************************************************************************

    57002dsm.c

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

const char *tms57002_device::get_memadr(UINT32 opcode, char type)
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

UINT32 tms57002_device::disasm_min_opcode_bytes() const
{
	return 4;
}

UINT32 tms57002_device::disasm_max_opcode_bytes() const
{
	return 4;
}

offs_t tms57002_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
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
