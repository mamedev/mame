/***************************************************************************

    m6509.h

    6502 with banking and extended address bus

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

#ifndef __M6509_H__
#define __M6509_H__

#include "m6502.h"

class m6509_device : public m6502_device {
public:
	m6509_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	static const disasm_entry disasm_entries[0x100];

	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options);
	virtual void do_exec_full();
	virtual void do_exec_partial();

protected:
	class mi_6509_normal : public memory_interface {
	public:
		m6509_device *base;

		mi_6509_normal(m6509_device *base);
		virtual ~mi_6509_normal() {}
		virtual UINT8 read(UINT16 adr);
		virtual UINT8 read_9(UINT16 adr);
		virtual UINT8 read_direct(UINT16 adr);
		virtual UINT8 read_decrypted(UINT16 adr);
		virtual void write(UINT16 adr, UINT8 val);
		virtual void write_9(UINT16 adr, UINT8 val);
	};

	class mi_6509_nd : public mi_6509_normal {
	public:
		mi_6509_nd(m6509_device *base);
		virtual ~mi_6509_nd() {}
		virtual UINT8 read_direct(UINT16 adr);
		virtual UINT8 read_decrypted(UINT16 adr);
	};

	virtual void device_start();
	virtual void device_reset();
	virtual void state_export(const device_state_entry &entry);

	UINT32 XPC;

	UINT8 bank_i, bank_y;

	UINT8 bank_i_r() { return bank_i; }
	UINT8 bank_y_r() { return bank_y; }
	void bank_i_w(UINT8 data) { bank_i = data; }
	void bank_y_w(UINT8 data) { bank_y = data; }

	UINT32 adr_in_bank_i(UINT16 adr) { return adr | ((bank_i & 0xf) << 16); }
	UINT32 adr_in_bank_y(UINT16 adr) { return adr | ((bank_y & 0xf) << 16); }

#define O(o) void o ## _full(); void o ## _partial()

	// 6509 opcodes
	O(lda_9_idy);
	O(sta_9_idy);

#undef O
};

enum {
	M6509_IRQ_LINE = m6502_device::IRQ_LINE,
	M6509_NMI_LINE = m6502_device::NMI_LINE,
	M6509_SET_OVERFLOW = m6502_device::V_LINE,
};

enum {
	M6509_BI = M6502_IR+1,
	M6509_BY
};

extern const device_type M6509;

#endif
