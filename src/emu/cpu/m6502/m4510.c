/***************************************************************************

    m4510.c

    65ce02 with a mmu and a cia integrated

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
#include "m4510.h"

const device_type M4510 = &device_creator<m4510_device>;

m4510_device::m4510_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	m65ce02_device(mconfig, M4510, "M4510", tag, owner, clock)
{
	program_config.m_addrbus_width = 20;
	program_config.m_logaddr_width = 16;
	program_config.m_page_shift = 13;
}

offs_t m4510_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	return disassemble_generic(buffer, pc, oprom, opram, options, disasm_entries);
}

void m4510_device::device_start()
{
 	if(direct_disabled)
		mintf = new mi_4510_nd(this);
	else
		mintf = new mi_4510_normal(this);

	m65ce02_device::device_start();

	save_item(NAME(map_offset));
	save_item(NAME(map_enable));
}

void m4510_device::device_reset()
{
	map_offset[0] = map_offset[1] = 0;
	map_enable = 0;
	nomap = true;
}

m4510_device::mi_4510_normal::mi_4510_normal(m4510_device *_base)
{
	base = _base;
}

UINT8 m4510_device::mi_4510_normal::read(UINT16 adr)
{
	return program->read_byte(base->map(adr));
}

UINT8 m4510_device::mi_4510_normal::read_direct(UINT16 adr)
{
	return direct->read_raw_byte(base->map(adr));
}

UINT8 m4510_device::mi_4510_normal::read_decrypted(UINT16 adr)
{
	return direct->read_decrypted_byte(base->map(adr));
}

void m4510_device::mi_4510_normal::write(UINT16 adr, UINT8 val)
{
	program->write_byte(base->map(adr), val);
}

m4510_device::mi_4510_nd::mi_4510_nd(m4510_device *_base) : mi_4510_normal(_base)
{
}

UINT8 m4510_device::mi_4510_nd::read_direct(UINT16 adr)
{
	return read(adr);
}

UINT8 m4510_device::mi_4510_nd::read_decrypted(UINT16 adr)
{
	return read(adr);
}

#include "cpu/m6502/m4510.inc"
