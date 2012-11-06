/***************************************************************************

    m65c02.h

    Mostek 6502, CMOS variant with some additional instructions (but
    not the bitwise ones)

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

#ifndef __M65C02_H__
#define __M65C02_H__

#include "m6502.h"

class m65c02_device : public m6502_device {
public:
	m65c02_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	m65c02_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock);

	static const disasm_entry disasm_entries[0x100];

	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options);
	virtual void do_exec_full();
	virtual void do_exec_partial();

protected:
#define O(o) void o ## _full(); void o ## _partial()

	// 65c02 opcodes
	O(adc_c_aba); O(adc_c_abx); O(adc_c_aby); O(adc_c_idx); O(adc_c_idy); O(adc_c_imm); O(adc_c_zpg); O(adc_c_zpi); O(adc_c_zpx);
	O(and_zpi);
	O(asl_c_abx);
	O(bbr_zpb);
	O(bbs_zpb);
	O(bit_abx); O(bit_imm); O(bit_zpx);
	O(bra_rel);
	O(brk_c_imp);
	O(cmp_zpi);
	O(dec_acc);
	O(eor_zpi);
	O(inc_acc);
	O(jmp_c_ind); O(jmp_iax);
	O(lda_zpi);
	O(lsr_c_abx);
	O(nop_c_aba); O(nop_c_abx); O(nop_c_imp);
	O(ora_zpi);
	O(phx_imp);
	O(phy_imp);
	O(plx_imp);
	O(ply_imp);
	O(rmb_bzp);
	O(rol_c_abx);
	O(ror_c_abx);
	O(sbc_c_aba); O(sbc_c_abx); O(sbc_c_aby); O(sbc_c_idx); O(sbc_c_idy); O(sbc_c_imm); O(sbc_c_zpg); O(sbc_c_zpi); O(sbc_c_zpx);
	O(smb_bzp);
	O(stp_imp);
	O(sta_zpi);
	O(stz_aba); O(stz_abx); O(stz_zpg); O(stz_zpx);
	O(trb_aba); O(trb_zpg);
	O(tsb_aba); O(tsb_zpg);
	O(wai_imp);

#undef O
};

enum {
	M65C02_IRQ_LINE = m6502_device::IRQ_LINE,
	M65C02_NMI_LINE = m6502_device::NMI_LINE,
	M65C02_SET_OVERFLOW = m6502_device::V_LINE,
};

extern const device_type M65C02;

#endif
