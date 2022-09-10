// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    m6502mtu.h

    M6502 with MTU external banking hardware

***************************************************************************/

#ifndef MAME_CPU_M6502_M6502MTU_H
#define MAME_CPU_M6502_M6502MTU_H

#include "m6502.h"

class m6502mtu_device : public m6502_device {
public:
	m6502mtu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual void do_exec_full() override;
	virtual void do_exec_partial() override;

	void set_pbank(u32 bank) { pbank = bank << 16; }
	void set_dbank(u32 bank) { dbank = bank << 16; }

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	memory_access<18, 0, 0, ENDIANNESS_LITTLE>::specific cprogram;
	memory_access<18, 0, 0, ENDIANNESS_LITTLE>::specific csprogram;

	bool low_instruction, interrupt_mode;
	u32 pbank, dbank;

	uint8_t readd(uint16_t adr) { return cprogram.read_byte(low_instruction || interrupt_mode ? adr : adr | pbank); }
	void writed(uint16_t adr, uint8_t val) { cprogram.write_byte(low_instruction || interrupt_mode ? adr : adr | pbank, val); }
	uint8_t readi(uint16_t adr) { return cprogram.read_byte(adr | dbank); }
	void writei(uint16_t adr, uint8_t val) { cprogram.write_byte(adr | dbank, val); }
	uint8_t readil(uint16_t adr) { return cprogram.read_byte(low_instruction ? adr | dbank : adr); }
	void writeil(uint16_t adr, uint8_t val) { cprogram.write_byte(low_instruction ? adr | dbank : adr, val); }
	uint8_t readd_arg(uint16_t adr) { return readd(adr); }
	uint8_t readd_pc() { return readd(PC++); }
	uint8_t readd_pc_noinc() { return readd(PC); }
	void prefetchd();
	void prefetchd_noirq();

	void interrupt_mode_on() { interrupt_mode = true; }
	void interrupt_mode_off() { interrupt_mode = false; }

#define O(o) void o ## _full(); void o ## _partial()

	// NMOS 6502 opcodes
	//   documented opcodes
	O(adc_mtu_aba); O(adc_mtu_abx); O(adc_mtu_aby); O(adc_mtu_idx); O(adc_mtu_idy); O(adc_mtu_imm); O(adc_mtu_zpg); O(adc_mtu_zpx);
	O(and_mtu_aba); O(and_mtu_abx); O(and_mtu_aby); O(and_mtu_imm); O(and_mtu_idx); O(and_mtu_idy); O(and_mtu_zpg); O(and_mtu_zpx);
	O(asl_mtu_aba); O(asl_mtu_abx); O(asl_mtu_acc); O(asl_mtu_zpg); O(asl_mtu_zpx);
	O(bcc_mtu_rel);
	O(bcs_mtu_rel);
	O(beq_mtu_rel);
	O(bit_mtu_aba); O(bit_mtu_zpg);
	O(bmi_mtu_rel);
	O(bne_mtu_rel);
	O(bpl_mtu_rel);
	O(brk_mtu_imp);
	O(bvc_mtu_rel);
	O(bvs_mtu_rel);
	O(clc_mtu_imp);
	O(cld_mtu_imp);
	O(cli_mtu_imp);
	O(clv_mtu_imp);
	O(cmp_mtu_aba); O(cmp_mtu_abx); O(cmp_mtu_aby); O(cmp_mtu_idx); O(cmp_mtu_idy); O(cmp_mtu_imm); O(cmp_mtu_zpg); O(cmp_mtu_zpx);
	O(cpx_mtu_aba); O(cpx_mtu_imm); O(cpx_mtu_zpg);
	O(cpy_mtu_aba); O(cpy_mtu_imm); O(cpy_mtu_zpg);
	O(dec_mtu_aba); O(dec_mtu_abx); O(dec_mtu_zpg); O(dec_mtu_zpx);
	O(dex_mtu_imp);
	O(dey_mtu_imp);
	O(eor_mtu_aba); O(eor_mtu_abx); O(eor_mtu_aby); O(eor_mtu_idx); O(eor_mtu_idy); O(eor_mtu_imm); O(eor_mtu_zpg); O(eor_mtu_zpx);
	O(inc_mtu_aba); O(inc_mtu_abx); O(inc_mtu_zpg); O(inc_mtu_zpx);
	O(inx_mtu_imp);
	O(iny_mtu_imp);
	O(jmp_mtu_adr); O(jmp_mtu_ind);
	O(jsr_mtu_adr);
	O(lda_mtu_aba); O(lda_mtu_abx); O(lda_mtu_aby); O(lda_mtu_idx); O(lda_mtu_idy); O(lda_mtu_imm); O(lda_mtu_zpg); O(lda_mtu_zpx);
	O(ldx_mtu_aba); O(ldx_mtu_aby); O(ldx_mtu_imm); O(ldx_mtu_zpg); O(ldx_mtu_zpy);
	O(ldy_mtu_aba); O(ldy_mtu_abx); O(ldy_mtu_imm); O(ldy_mtu_zpg); O(ldy_mtu_zpx);
	O(lsr_mtu_aba); O(lsr_mtu_abx); O(lsr_mtu_acc); O(lsr_mtu_zpg); O(lsr_mtu_zpx);
	O(nop_mtu_imp);
	O(ora_mtu_aba); O(ora_mtu_abx); O(ora_mtu_aby); O(ora_mtu_imm); O(ora_mtu_idx); O(ora_mtu_idy); O(ora_mtu_zpg); O(ora_mtu_zpx);
	O(pha_mtu_imp);
	O(php_mtu_imp);
	O(pla_mtu_imp);
	O(plp_mtu_imp);
	O(rol_mtu_aba); O(rol_mtu_abx); O(rol_mtu_acc); O(rol_mtu_zpg); O(rol_mtu_zpx);
	O(ror_mtu_aba); O(ror_mtu_abx); O(ror_mtu_acc); O(ror_mtu_zpg); O(ror_mtu_zpx);
	O(rti_mtu_imp);
	O(rts_mtu_imp);
	O(sbc_mtu_aba); O(sbc_mtu_abx); O(sbc_mtu_aby); O(sbc_mtu_idx); O(sbc_mtu_idy); O(sbc_mtu_imm); O(sbc_mtu_zpg); O(sbc_mtu_zpx);
	O(sec_mtu_imp);
	O(sed_mtu_imp);
	O(sei_mtu_imp);
	O(sta_mtu_aba); O(sta_mtu_abx); O(sta_mtu_aby); O(sta_mtu_idx); O(sta_mtu_idy); O(sta_mtu_zpg); O(sta_mtu_zpx);
	O(stx_mtu_aba); O(stx_mtu_zpg); O(stx_mtu_zpy);
	O(sty_mtu_aba); O(sty_mtu_zpg); O(sty_mtu_zpx);
	O(tax_mtu_imp);
	O(tay_mtu_imp);
	O(tsx_mtu_imp);
	O(txa_mtu_imp);
	O(txs_mtu_imp);
	O(tya_mtu_imp);

	//   exceptions
	O(reset_mtu);

	//   undocumented reliable instructions
	O(dcp_mtu_aba); O(dcp_mtu_abx); O(dcp_mtu_aby); O(dcp_mtu_idx); O(dcp_mtu_idy); O(dcp_mtu_zpg); O(dcp_mtu_zpx);
	O(isb_mtu_aba); O(isb_mtu_abx); O(isb_mtu_aby); O(isb_mtu_idx); O(isb_mtu_idy); O(isb_mtu_zpg); O(isb_mtu_zpx);
	O(lax_mtu_aba); O(lax_mtu_aby); O(lax_mtu_idx); O(lax_mtu_idy); O(lax_mtu_zpg); O(lax_mtu_zpy);
	O(rla_mtu_aba); O(rla_mtu_abx); O(rla_mtu_aby); O(rla_mtu_idx); O(rla_mtu_idy); O(rla_mtu_zpg); O(rla_mtu_zpx);
	O(rra_mtu_aba); O(rra_mtu_abx); O(rra_mtu_aby); O(rra_mtu_idx); O(rra_mtu_idy); O(rra_mtu_zpg); O(rra_mtu_zpx);
	O(sax_mtu_aba); O(sax_mtu_idx); O(sax_mtu_zpg); O(sax_mtu_zpy);
	O(sbx_mtu_imm);
	O(sha_mtu_aby); O(sha_mtu_idy);
	O(shs_mtu_aby);
	O(shx_mtu_aby);
	O(shy_mtu_abx);
	O(slo_mtu_aba); O(slo_mtu_abx); O(slo_mtu_aby); O(slo_mtu_idx); O(slo_mtu_idy); O(slo_mtu_zpg); O(slo_mtu_zpx);
	O(sre_mtu_aba); O(sre_mtu_abx); O(sre_mtu_aby); O(sre_mtu_idx); O(sre_mtu_idy); O(sre_mtu_zpg); O(sre_mtu_zpx);

	//   undocumented unreliable instructions
	//     behaviour differs between visual6502 and online docs, which
	//     is a clear sign reliability is not to be expected
	//     implemented version follows visual6502
	O(anc_mtu_imm);
	O(ane_mtu_imm);
	O(arr_mtu_imm);
	O(asr_mtu_imm);
	O(las_mtu_aby);
	O(lxa_mtu_imm);

	//   nop variants
	O(nop_mtu_imm); O(nop_mtu_aba); O(nop_mtu_abx); O(nop_mtu_zpg); O(nop_mtu_zpx);

	//   system killers
	O(kil_mtu_non);

#undef O
};

DECLARE_DEVICE_TYPE(M6502MTU, m6502mtu_device);

#endif // MAME_CPU_M6502_M6502MTU_H
