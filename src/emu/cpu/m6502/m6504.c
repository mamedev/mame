/***************************************************************************

    m6504.c

    Mostek 6502, NMOS variant with reduced address bus

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
#include "m6504.h"

const device_type M6504 = &device_creator<m6504_device>;

m6504_device::m6504_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	m6502_device(mconfig, M6504, "M6504", tag, owner, clock, "m6504", __FILE__)
{
	program_config.m_addrbus_width = 13;
}

void m6504_device::device_start()
{
	if(direct_disabled)
		mintf = new mi_6504_nd;
	else
		mintf = new mi_6504_normal;

	init();
}

UINT8 m6504_device::mi_6504_normal::read(UINT16 adr)
{
	return program->read_byte(adr & 0x1fff);
}

UINT8 m6504_device::mi_6504_normal::read_direct(UINT16 adr)
{
	return direct->read_raw_byte(adr & 0x1fff);
}

UINT8 m6504_device::mi_6504_normal::read_decrypted(UINT16 adr)
{
	return direct->read_decrypted_byte(adr & 0x1fff);
}

void m6504_device::mi_6504_normal::write(UINT16 adr, UINT8 val)
{
	program->write_byte(adr & 0x1fff, val);
}

UINT8 m6504_device::mi_6504_nd::read_direct(UINT16 adr)
{
	return read(adr);
}

UINT8 m6504_device::mi_6504_nd::read_decrypted(UINT16 adr)
{
	return read(adr);
}
