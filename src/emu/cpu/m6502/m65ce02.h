// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    m65ce02.h

    6502 with Z register and some more stuff

***************************************************************************/

#ifndef __M65CE02_H__
#define __M65CE02_H__

#include "m65c02.h"

class m65ce02_device : public m65c02_device {
public:
	m65ce02_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	m65ce02_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	static const disasm_entry disasm_entries[0x100];

	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options);
	virtual void do_exec_full();
	virtual void do_exec_partial();

protected:
	UINT16  TMP3;                   /* temporary internal values */
	UINT8   Z;                      /* Z index register */
	UINT16  B;                      /* Zero page base address (always xx00) */

	virtual void init();
	virtual void device_start();
	virtual void device_reset();
	virtual void state_import(const device_state_entry &entry);
	virtual void state_export(const device_state_entry &entry);
	virtual void state_string_export(const device_state_entry &entry, std::string &str);

	inline void dec_SP_ce() { if(P & F_E) SP = set_l(SP, SP-1); else SP--; }
	inline void inc_SP_ce() { if(P & F_E) SP = set_l(SP, SP+1); else SP++; }

#define O(o) void o ## _full(); void o ## _partial()

	// 65ce02 opcodes
	O(adc_ce_aba); O(adc_ce_abx); O(adc_ce_aby); O(adc_ce_idx); O(adc_ce_idy); O(adc_idz); O(adc_ce_imm); O(adc_ce_zpg); O(adc_ce_zpx);
	O(and_ce_abx); O(and_ce_aby); O(and_ce_idx); O(and_ce_idy); O(and_idz); O(and_ce_zpg); O(and_ce_zpx);
	O(asl_ce_aba); O(asl_ce_abx); O(asl_ce_acc); O(asl_ce_zpg); O(asl_ce_zpx);
	O(asr_acc); O(asr_zpg); O(asr_zpx);
	O(asw_aba);
	O(aug_iw3);
	O(bbr_ce_zpb);
	O(bbs_ce_zpb);
	O(bcc_ce_rel); O(bcc_rw2);
	O(bcs_ce_rel); O(bcs_rw2);
	O(beq_ce_rel); O(beq_rw2);
	O(bit_ce_abx); O(bit_ce_imm); O(bit_ce_zpg); O(bit_ce_zpx);
	O(bmi_ce_rel); O(bmi_rw2);
	O(bne_ce_rel); O(bne_rw2);
	O(bpl_ce_rel); O(bpl_rw2);
	O(bra_ce_rel); O(bra_rw2);
	O(brk_ce_imp);
	O(bsr_rw2);
	O(bvc_ce_rel); O(bvc_rw2);
	O(bvs_ce_rel); O(bvs_rw2);
	O(clc_ce_imp);
	O(cld_ce_imp);
	O(cle_imp);
	O(cli_ce_imp);
	O(clv_ce_imp);
	O(cmp_ce_abx); O(cmp_ce_aby); O(cmp_ce_idx); O(cmp_ce_idy); O(cmp_idz); O(cmp_ce_zpg); O(cmp_ce_zpx);
	O(cpx_ce_zpg);
	O(cpy_ce_zpg);
	O(cpz_aba); O(cpz_imm); O(cpz_zpg);
	O(dec_ce_aba); O(dec_ce_abx); O(dec_ce_acc); O(dec_ce_zpg); O(dec_ce_zpx);
	O(dew_zpg);
	O(dex_ce_imp);
	O(dey_ce_imp);
	O(dez_imp);
	O(eor_ce_abx); O(eor_ce_aby); O(eor_ce_idx); O(eor_ce_idy); O(eor_idz); O(eor_ce_zpg); O(eor_ce_zpx);
	O(inc_ce_aba); O(inc_ce_abx); O(inc_ce_acc); O(inc_ce_zpg); O(inc_ce_zpx);
	O(inw_zpg);
	O(inx_ce_imp);
	O(iny_ce_imp);
	O(inz_imp);
	O(jmp_ce_iax); O(jmp_ce_ind);
	O(jsr_ce_adr); O(jsr_ind); O(jsr_iax);
	O(lda_ce_abx); O(lda_ce_aby); O(lda_ce_idx); O(lda_ce_idy); O(lda_idz); O(lda_isy); O(lda_ce_zpg); O(lda_ce_zpx);
	O(ldx_ce_aby); O(ldx_ce_zpg); O(ldx_ce_zpy);
	O(ldy_ce_abx); O(ldy_ce_zpg); O(ldy_ce_zpx);
	O(ldz_aba); O(ldz_abx); O(ldz_imm);
	O(lsr_ce_aba); O(lsr_ce_abx); O(lsr_ce_acc); O(lsr_ce_zpg); O(lsr_ce_zpx);
	O(neg_acc);
	O(ora_ce_abx); O(ora_ce_aby); O(ora_ce_idx); O(ora_ce_idy); O(ora_idz); O(ora_ce_zpg); O(ora_ce_zpx);
	O(pha_ce_imp);
	O(php_ce_imp);
	O(phw_aba); O(phw_iw2);
	O(phx_ce_imp);
	O(phy_ce_imp);
	O(phz_imp);
	O(pla_ce_imp);
	O(plp_ce_imp);
	O(plx_ce_imp);
	O(ply_ce_imp);
	O(plz_imp);
	O(rmb_ce_bzp);
	O(rol_ce_aba); O(rol_ce_abx); O(rol_ce_acc); O(rol_ce_zpg); O(rol_ce_zpx);
	O(ror_ce_aba); O(ror_ce_abx); O(ror_ce_acc); O(ror_ce_zpg); O(ror_ce_zpx);
	O(row_aba);
	O(rti_ce_imp);
	O(rtn_imm);
	O(rts_ce_imp);
	O(sbc_ce_aba); O(sbc_ce_abx); O(sbc_ce_aby); O(sbc_ce_idx); O(sbc_ce_idy); O(sbc_idz); O(sbc_ce_imm); O(sbc_ce_zpg); O(sbc_ce_zpx);
	O(sec_ce_imp);
	O(sed_ce_imp);
	O(see_imp);
	O(sei_ce_imp);
	O(smb_ce_bzp);
	O(sta_ce_abx); O(sta_ce_aby); O(sta_ce_idx); O(sta_ce_idy); O(sta_idz); O(sta_isy); O(sta_ce_zpg); O(sta_ce_zpx);
	O(stx_aby); O(stx_ce_zpg); O(stx_ce_zpy);
	O(sty_abx); O(sty_ce_zpg); O(sty_ce_zpx);
	O(stz_ce_aba); O(stz_ce_abx); O(stz_ce_zpg); O(stz_ce_zpx);
	O(tab_imp);
	O(tax_ce_imp);
	O(tay_ce_imp);
	O(taz_imp);
	O(tba_imp);
	O(trb_ce_aba); O(trb_ce_zpg);
	O(tsb_ce_aba); O(tsb_ce_zpg);
	O(tsx_ce_imp);
	O(tsy_imp);
	O(txa_ce_imp);
	O(txs_ce_imp);
	O(tys_imp);
	O(tya_ce_imp);
	O(tza_imp);

#undef O
};

enum {
	M65CE02_IRQ_LINE = m6502_device::IRQ_LINE,
	M65CE02_NMI_LINE = m6502_device::NMI_LINE
};


enum {
	M65CE02_Z = M6502_IR+1,
	M65CE02_B
};

extern const device_type M65CE02;

#endif
