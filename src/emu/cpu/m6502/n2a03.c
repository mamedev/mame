/***************************************************************************

    m6502.c

    6502, NES variant

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
#include "n2a03.h"

const device_type N2A03 = &device_creator<n2a03_device>;

n2a03_device::n2a03_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	m6502_device(mconfig, N2A03, "N2A03", tag, owner, clock)
{
}

offs_t n2a03_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	return disassemble_generic(buffer, pc, oprom, opram, options, disasm_entries);
}

void n2a03_device::device_start()
{
	if(direct_disabled)
		mintf = new mi_2a03_nd;
	else
		mintf = new mi_2a03_normal;

	init();
}

UINT8 n2a03_device::mi_2a03_normal::read(UINT16 adr)
{
	return program->read_byte(adr);
}

UINT8 n2a03_device::mi_2a03_normal::read_direct(UINT16 adr)
{
	return direct->read_raw_byte(adr);
}

UINT8 n2a03_device::mi_2a03_normal::read_decrypted(UINT16 adr)
{
	return direct->read_decrypted_byte(adr);
}

void n2a03_device::mi_2a03_normal::write(UINT16 adr, UINT8 val)
{
	program->write_byte(adr, val);
}

UINT8 n2a03_device::mi_2a03_nd::read(UINT16 adr)
{
	return program->read_byte(adr);
}

UINT8 n2a03_device::mi_2a03_nd::read_direct(UINT16 adr)
{
	return program->read_byte(adr);
}

UINT8 n2a03_device::mi_2a03_nd::read_decrypted(UINT16 adr)
{
	return program->read_byte(adr);
}

void n2a03_device::mi_2a03_nd::write(UINT16 adr, UINT8 val)
{
	program->write_byte(adr, val);
}

#include "cpu/m6502/n2a03.inc"
