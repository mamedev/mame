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

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual void do_exec_full() override;
	virtual void do_exec_partial() override;

	virtual u16 get_irq_vector();

	void c19_init();

	u8 cir_r();
	void cir_w(u8 data);
	u8 page1_seg_r(offs_t offset);
	void page1_seg_w(offs_t offset, u8 data);

private:
	void do_add(u8 v);
	u16 do_accumulate(u16 v, u16 w);

#define O(o) void o ## _full(); void o ## _partial()

	O(adc_ipx);
	O(add_imm);
	O(add_zpg);
	O(add_zpx);
	O(and_ipx);
	O(asr_acc);
	O(bar_amr);
	O(bas_amr);
	O(brk_r_imp);
	O(clw_imp);
	O(cmp_ipx);
	O(eor_ipx);
	O(exc_zpx);
	O(ini_imp);
	O(jpi_ind);
	O(jsb_vec);
	O(jmp_r_ind);
	O(jsr_r_adr);
	O(lab_acc);
	O(lai_imp);
	O(lan_imp);
	O(lda_ipx);
	O(lii_imp);
	O(mpa_imp);
	O(mpy_imp);
	O(neg_acc);
	O(nxt_imp);
	O(ora_ipx);
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
	O(sbc_ipx);
	O(sta_ipx);
	O(sti_imz);
	O(taw_imp);
	O(tip_imp);
	O(twa_imp);
	O(reset_r);

#undef O

	u16 m_w;
	u16 m_i;

	optional_shared_ptr<u8> m_page1_ram;
	u8 m_cir;
};

class c39_device : public r65c19_device
{
public:
	enum {
		C39_BSR0 = R65C19_I + 1,
		C39_BSR1, C39_BSR2, C39_BSR3, C39_BSR4, C39_BSR5, C39_BSR6, C39_BSR7
	};

protected:
	c39_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, address_map_constructor internal_map);

	virtual space_config_vector memory_space_config() const override;

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	u8 pbs_r();
	void pbs_w(u8 data);
	u8 bsr_r(offs_t offset);
	void bsr_w(offs_t offset, u8 data);
	u8 expansion_r(offs_t offset);
	void expansion_w(offs_t offset, u8 data);

private:
	class mi_banked : public memory_interface {
	public:
		virtual u8 read(u16 adr) override;
		virtual u8 read_sync(u16 adr) override;
		virtual u8 read_arg(u16 adr) override;
		virtual void write(u16 adr, u8 val) override;

		u8 exp_read(u16 adr);
		u8 exp_read_cached(u16 adr);
		void exp_write(u16 adr, u8 val);
		u8 es4_read(u16 adr);
		void es4_write(u16 adr, u8 val);

		memory_access<21, 0, 0, ENDIANNESS_LITTLE>::cache escache;
		memory_access<21, 0, 0, ENDIANNESS_LITTLE>::specific exp;
		memory_access< 9, 0, 0, ENDIANNESS_LITTLE>::specific es4;

		u8 bsr[8];
		u8 pbs;
	};

	address_space_config m_exp_config;
	address_space_config m_es4_config;
};

class l2800_device : public c39_device
{
public:
	l2800_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

private:
	void internal_map(address_map &map) ATTR_COLD;
};

DECLARE_DEVICE_TYPE(R65C19, r65c19_device)
DECLARE_DEVICE_TYPE(L2800, l2800_device)

#endif // MAME_CPU_M6502_R65C19_H
