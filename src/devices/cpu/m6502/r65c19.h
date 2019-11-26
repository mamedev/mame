// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_CPU_M6502_R65C19_H
#define MAME_CPU_M6502_R65C19_H

#pragma once

#include "r65c02.h"

class r65c19_device : public r65c02_device
{
public:
	enum {
		R65C19_W = M6502_IR + 1,
		R65C19_WL,
		R65C19_WH,
		R65C19_I
	};

	r65c19_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	r65c19_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, address_map_constructor internal_map);

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual void do_exec_full() override;
	virtual void do_exec_partial() override;

	virtual u16 get_irq_vector();

	void c19_init();

private:
	void do_add(u8 v);
	u16 do_accumulate(u16 v, u16 w);

#define O(o) void o ## _full(); void o ## _partial()

	O(add_imm);
	O(add_zpg);
	O(add_zpx);
	O(asr_acc);
	O(bar_amr);
	O(bas_amr);
	O(brk_r_imp);
	O(clw_imp);
	O(exc_zpx);
	O(ini_imp);
	O(jpi_ind);
	O(jsb_vec);
	O(jmp_r_ind);
	O(jsr_r_adr);
	O(lab_acc);
	O(lai_imp);
	O(lan_imp);
	O(lii_imp);
	O(mpa_imp);
	O(mpy_imp);
	O(neg_acc);
	O(nxt_imp);
	O(phi_imp);
	O(phw_imp);
	O(pia_imp);
	O(pli_imp);
	O(plw_imp);
	O(psh_imp);
	O(pul_imp);
	O(rba_ima);
	O(rnd_imp);
	O(rts_r_imp);
	O(sba_ima);
	O(sti_imz);
	O(taw_imp);
	O(tip_imp);
	O(twa_imp);
	O(reset_r);

#undef O

	u16 m_w;
	u16 m_i;
};

DECLARE_DEVICE_TYPE(R65C19, r65c19_device)

#endif // MAME_CPU_M6502_R65C19_H
