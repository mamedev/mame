/***************************************************************************

    i8xc196.h

    MCS96, c196 branch, the enhanced 16 bits bus version

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

    THIS SOFTWARE IS PROVIDED BY OLIVIER GALIBERT ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL OLIVIER GALIBERT BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

***************************************************************************/

#include "emu.h"
#include "i8xc196.h"

i8xc196_device::i8xc196_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock) :
	mcs96_device(mconfig, type, name, tag, owner, clock, 16)
{
}

offs_t i8xc196_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	return disasm_generic(buffer, pc, oprom, opram, options, disasm_entries);
}

void i8xc196_device::io_w8(UINT8 adr, UINT8 data)
{
	switch(adr) {
	case 0:
		break;
	case 1:
		break;
	default:
		logerror("%s: io_w8 %02x, %02x (%04x)\n", tag(), adr, data, PPC);
	}
	return;
}

void i8xc196_device::io_w16(UINT8 adr, UINT16 data)
{
	switch(adr) {
	case 0:
		break;
	default:
		io_w8(adr, data);
		io_w8(adr+1, data>>8);
		break;
	}
	return;
}

UINT8 i8xc196_device::io_r8(UINT8 adr)
{
	switch(adr) {
	case 0x00:
		return 0x00;
	case 0x01:
		return 0x00;
	}
	UINT8 data = 0x00;
	logerror("%s: io_r8 %02x, %02x (%04x)\n", tag(), adr, data, PPC);
	return data;
}

UINT16 i8xc196_device::io_r16(UINT8 adr)
{
	if(adr < 2)
		return 0x0000;
	UINT16 data = 0x0000;
	logerror("%s: io_r16 %02x, %04x (%04x)\n", tag(), adr, data, PPC);
	return data;
}

#include "cpu/mcs96/i8xc196.inc"
