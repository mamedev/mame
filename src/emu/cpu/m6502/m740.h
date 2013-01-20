/***************************************************************************

    m740.h

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

#ifndef __M740_H__
#define __M740_H__

#include "m6502.h"

class m740_device : public m6502_device {
public:
	m740_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	m740_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock);

	virtual void device_reset();

	static const disasm_entry disasm_entries[0x100];

	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options);
	virtual void do_exec_full();
	virtual void do_exec_partial();

protected:
#define O(o) void o ## _full(); void o ## _partial()

	UINT8 do_clb(UINT8 in, UINT8 bit);
	UINT8 do_seb(UINT8 in, UINT8 bit);
	UINT8 do_rrf(UINT8 in);

	// m740 opcodes
	O(clt_imp);
	O(set_imp);
	O(ldm_imz);
	O(jsr_spg);
	O(reset740);
	O(seb_biz); O(seb_acc);
	O(clb_biz); O(clb_acc);
	O(bbc_bzr); O(bbc_acc);
	O(bbs_bzr); O(bbs_acc);
	O(rrf_zpg);
	O(bra_rel);

#undef O
};

enum {
	M740_IRQ_LINE = m6502_device::IRQ_LINE,
	M740_NMI_LINE = m6502_device::NMI_LINE,
	M740_SET_OVERFLOW = m6502_device::V_LINE,
};

extern const device_type M740;

#endif
