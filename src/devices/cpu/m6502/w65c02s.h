// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    w65c02s.h

    WDC W65C02S, CMOS variant with bitwise instructions, BE, ML, VP pins
    and cleaner fetch patterns

***************************************************************************/

#ifndef MAME_CPU_M6502_W65C02S_H
#define MAME_CPU_M6502_W65C02S_H

#include "m65c02.h"

class w65c02s_device : public m65c02_device {
public:
	w65c02s_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
	virtual void do_exec_full() override;
	virtual void do_exec_partial() override;

protected:
	w65c02s_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t read_vector(uint16_t adr) { return mintf->read_arg(adr); }
	virtual void end_interrupt() { }

#define O(o) void o ## _full(); void o ## _partial()

	O(adc_s_abx); O(adc_s_aby); O(adc_s_idx); O(adc_s_idy); O(adc_s_zpx);
	O(and_s_abx); O(and_s_aby); O(and_s_idx); O(and_s_idy); O(and_s_zpx);
	O(asl_s_abx); O(asl_s_zpg); O(asl_s_zpx);
	O(bit_s_abx); O(bit_s_zpx);
	O(brk_s_imp);
	O(cmp_s_abx); O(cmp_s_aby); O(cmp_s_idx); O(cmp_s_idy); O(cmp_s_zpx);
	O(dec_s_abx); O(dec_s_zpg); O(dec_s_zpx);
	O(eor_s_abx); O(eor_s_aby); O(eor_s_idx); O(eor_s_idy); O(eor_s_zpx);
	O(inc_s_abx); O(inc_s_zpg); O(inc_s_zpx);
	O(jmp_s_iax); O(jmp_s_ind);
	O(lda_s_abx); O(lda_s_aby); O(lda_s_idx); O(lda_s_idy); O(lda_s_zpx);
	O(ldx_s_aby); O(ldx_s_zpy);
	O(ldy_s_abx); O(ldy_s_zpx);
	O(lsr_s_abx); O(lsr_s_zpg); O(lsr_s_zpx);
	O(nop_s_abx); O(nop_s_zpx);
	O(ora_s_abx); O(ora_s_aby); O(ora_s_idx); O(ora_s_idy); O(ora_s_zpx);
	O(pla_s_imp);
	O(plp_s_imp);
	O(plx_s_imp);
	O(ply_s_imp);
	O(rol_s_abx); O(rol_s_zpg); O(rol_s_zpx);
	O(ror_s_abx); O(ror_s_zpg); O(ror_s_zpx);
	O(rti_s_imp);
	O(rts_s_imp);
	O(sbc_s_abx); O(sbc_s_aby); O(sbc_s_idx); O(sbc_s_idy); O(sbc_s_zpx);
	O(sta_s_abx); O(sta_s_aby); O(sta_s_idx); O(sta_s_idy); O(sta_s_zpx);
	O(stx_s_zpy);
	O(sty_s_zpx);
	O(stz_s_abx); O(stz_s_zpx);
	O(trb_s_zpg);
	O(tsb_s_zpg);
	O(reset_s);

#undef O
};

DECLARE_DEVICE_TYPE(W65C02S, w65c02s_device)

#endif // MAME_CPU_M6502_W65C02S_H
