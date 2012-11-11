/***************************************************************************

    m4510.h

    65ce02 with a mmu and a port

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

#ifndef __M4510_H__
#define __M4510_H__

#include "m65ce02.h"

class m4510_device : public m65ce02_device {
public:
	m4510_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	static const disasm_entry disasm_entries[0x100];

	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options);
	virtual void do_exec_full();
	virtual void do_exec_partial();

	bool get_nomap() const { return nomap; }

protected:
	UINT32 map_offset[2];
	UINT8 map_enable;
	bool nomap;

	class mi_4510_normal : public memory_interface {
	public:
		m4510_device *base;

		mi_4510_normal(m4510_device *base);
		virtual ~mi_4510_normal() {}
		virtual UINT8 read(UINT16 adr);
		virtual UINT8 read_direct(UINT16 adr);
		virtual UINT8 read_decrypted(UINT16 adr);
		virtual void write(UINT16 adr, UINT8 val);
	};

	class mi_4510_nd : public mi_4510_normal {
	public:
		mi_4510_nd(m4510_device *base);
		virtual ~mi_4510_nd() {}
		virtual UINT8 read_direct(UINT16 adr);
		virtual UINT8 read_decrypted(UINT16 adr);
	};

	virtual void device_start();
	virtual void device_reset();

	inline UINT32 map(UINT16 adr) {
		if(map_enable & (1 << (adr >> 13))) {
			nomap = false;
			return adr + map_offset[adr >> 15];
		}
		nomap = true;
		return adr;
	}

#define O(o) void o ## _full(); void o ## _partial()

	// 4510 opcodes
	O(eom_imp);
	O(map_imp);

#undef O
};

enum {
	M4510_IRQ_LINE = m6502_device::IRQ_LINE,
	M4510_NMI_LINE = m6502_device::NMI_LINE,
};

extern const device_type M4510;

#endif
