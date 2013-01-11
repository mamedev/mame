/***************************************************************************

    m65ce02.c

    6502 with Z register and some more stuff

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
#include "m65ce02.h"

const device_type M65CE02 = &device_creator<m65ce02_device>;

m65ce02_device::m65ce02_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	m65c02_device(mconfig, M65CE02, "M65CE02", tag, owner, clock)
{
}

m65ce02_device::m65ce02_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock) :
	m65c02_device(mconfig, type, name, tag, owner, clock)
{
}

offs_t m65ce02_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	return disassemble_generic(buffer, pc, oprom, opram, options, disasm_entries);
}

void m65ce02_device::init()
{
	m65c02_device::init();
	state_add(M65CE02_Z, "Z", Z);
	state_add(M65CE02_B, "B", B).callimport().formatstr("%2s");
	save_item(NAME(B));
	save_item(NAME(Z));
	save_item(NAME(TMP3));
	Z = 0x00;
	B = 0x0000;
	TMP3 = 0x0000;
}

void m65ce02_device::device_start()
{
	if(direct_disabled)
		mintf = new mi_default_nd;
	else
		mintf = new mi_default_normal;

	init();
}

void m65ce02_device::device_reset()
{
	m65c02_device::device_reset();
	Z = 0x00;
	B = 0x0000;
}

void m65ce02_device::state_import(const device_state_entry &entry)
{
	switch(entry.index()) {
	case STATE_GENFLAGS:
	case M6502_P:
		P = P | F_B;
		break;
	case M65CE02_B:
		B <<= 8;
		break;
	}
}

void m65ce02_device::state_export(const device_state_entry &entry)
{
}

void m65ce02_device::state_string_export(const device_state_entry &entry, astring &string)
{
	switch(entry.index()) {
	case STATE_GENFLAGS:
	case M6502_P:
		string.printf("%c%c%c%c%c%c%c",
						P & F_N ? 'N' : '.',
						P & F_V ? 'V' : '.',
						P & F_E ? 'E' : '.',
						P & F_D ? 'D' : '.',
						P & F_I ? 'I' : '.',
						P & F_Z ? 'Z' : '.',
						P & F_C ? 'C' : '.');
		break;
	case M65CE02_B:
		string.printf("%02x", B >> 8);
		break;
	}
}

#include "cpu/m6502/m65ce02.inc"
