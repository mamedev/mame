/***************************************************************************

    m6510.h

    6502 with 6 i/o pins, also known as 8500

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

#ifndef __M6510FAM_H__
#define __M6510FAM_H__

#include "m6502.h"

#define MCFG_M6510_PORT_CALLBACKS(_read, _write) \
	downcast<m6510_device *>(device)->set_callbacks(DEVCB2_##_read, DEVCB2_##_write);

#define MCFG_M6510_PORT_PULLS(_up, _down) \
	downcast<m6510_device *>(device)->set_pulls(_up, _down);

class m6510_device : public m6502_device {
public:
	m6510_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	m6510_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock);

	UINT8 get_port();
	void set_port(UINT8 val);
	void set_pulls(UINT8 pullup, UINT8 pulldown);

	template<class _read, class _write> void set_callbacks(_read rd, _write wr) {
		read_port.set_callback(rd);
		write_port.set_callback(wr);
	}

	static const disasm_entry disasm_entries[0x100];

	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options);
	virtual void do_exec_full();
	virtual void do_exec_partial();

protected:
	class mi_6510_normal : public memory_interface {
	public:
		m6510_device *base;

		mi_6510_normal(m6510_device *base);
		virtual ~mi_6510_normal() {}
		virtual UINT8 read(UINT16 adr);
		virtual UINT8 read_direct(UINT16 adr);
		virtual UINT8 read_decrypted(UINT16 adr);
		virtual void write(UINT16 adr, UINT8 val);
	};

	class mi_6510_nd : public mi_6510_normal {
	public:
		mi_6510_nd(m6510_device *base);
		virtual ~mi_6510_nd() {}
		virtual UINT8 read_direct(UINT16 adr);
		virtual UINT8 read_decrypted(UINT16 adr);
	};

	devcb2_read8  read_port;
	devcb2_write8 write_port;

	UINT8 pullup, floating, dir, port, drive;

	virtual void device_start();
	virtual void device_reset();

	UINT8 dir_r();
	void dir_w(UINT8 data);
	UINT8 port_r();
	void port_w(UINT8 data);

	void update_port();

#define O(o) void o ## _full(); void o ## _partial()

	// 6510 undocumented instructions in a C64 context
	// implementation follows what the test suites expect (usually an extra and)
	O(anc_10_imm);
	O(ane_10_imm);
	O(arr_10_imm);
	O(asr_10_imm);
	O(las_10_aby);
	O(lxa_10_imm);

#undef O
};

enum {
	M6510_IRQ_LINE = m6502_device::IRQ_LINE,
	M6510_NMI_LINE = m6502_device::NMI_LINE,
};

extern const device_type M6510;

#endif
