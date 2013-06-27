/***************************************************************************

    deco16.h

    6502, reverse-engineered DECO variant

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

#ifndef __DECO16_H__
#define __DECO16_H__

#include "m6502.h"

class deco16_device : public m6502_device {
public:
	deco16_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	static const disasm_entry disasm_entries[0x100];

	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options);
	virtual void do_exec_full();
	virtual void do_exec_partial();

protected:
	address_space *io;
	address_space_config io_config;

	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const;
	virtual void device_start();

#define O(o) void o ## _full(); void o ## _partial()

	O(brk_16_imp);
	O(ill_non);
	O(u0B_zpg);
	O(u13_zpg);
	O(u23_zpg);
	O(u3F_zpg);
	O(u4B_zpg);
	O(u87_zpg);
	O(u8F_zpg);
	O(uA3_zpg);
	O(uAB_zpg);
	O(uBB_zpg);
	O(vbl_zpg);

	O(reset_16);

#undef O
};

enum {
	DECO16_IRQ_LINE = m6502_device::IRQ_LINE,
	DECO16_NMI_LINE = m6502_device::NMI_LINE,
	DECO16_SET_OVERFLOW = m6502_device::V_LINE,
};

extern const device_type DECO16;

#endif
