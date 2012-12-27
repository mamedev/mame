/***************************************************************************

    m740.c

    Mitsubishi M740 series (M507xx/M509xx)

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
#include "m740.h"

const device_type M740 = &device_creator<m740_device>;

m740_device::m740_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	m6502_device(mconfig, M740, "M740", tag, owner, clock)
{
}

m740_device::m740_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock) :
	m6502_device(mconfig, type, name, tag, owner, clock)
{
}

offs_t m740_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	return disassemble_generic(buffer, pc, oprom, opram, options, disasm_entries);
}

void m740_device::device_reset()
{
	inst_state = STATE_RESET;
	inst_substate = 0;
	nmi_state = false;
	irq_state = false;
	apu_irq_state = false;
	irq_taken = false;
	v_state = false;
	end_cycles = 0;
	sync = false;
	inhibit_interrupts = false;
	SP = 0x00ff;
}

UINT8 m740_device::do_clb(UINT8 in, UINT8 bit)
{
	return in & ~(1<<bit);
}

UINT8 m740_device::do_seb(UINT8 in, UINT8 bit)
{
	return in | (1<<bit);
}

// swap the two nibbles of the input (Rotate Right Four bits)
// doesn't affect the flags
UINT8 m740_device::do_rrf(UINT8 in)
{
	return ((in&0xf)<<4) | ((in&0xf0)>>4);
}

#include "cpu/m6502/m740.inc"
