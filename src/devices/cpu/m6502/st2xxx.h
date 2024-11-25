// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_CPU_M6502_ST2XXX_H
#define MAME_CPU_M6502_ST2XXX_H

#pragma once

#include "w65c02s.h"

class st2xxx_device : public w65c02s_device {
public:
	enum {
		ST_PAOUT = M6502_IR + 1,
		ST_PBOUT,
		ST_PCOUT,
		ST_PDOUT,
		ST_PEOUT,
		ST_PFOUT,
		ST_PLOUT,
		ST_PCA,
		ST_PCB,
		ST_PCC,
		ST_PCD,
		ST_PCE,
		ST_PCF,
		ST_PCL,
		ST_PSA,
		ST_PSB,
		ST_PSC,
		ST_PSD,
		ST_PSE,
		ST_PSF,
		ST_PSL,
		ST_PFC,
		ST_PFD,
		ST_PMCR,
		ST_PSGC,
		ST_BTEN,
		ST_BTSR,
		ST_PRS,
		ST_SYS,
		ST_IRR,
		ST_PRR,
		ST_DRR,
		ST_DMR,
		ST_MISC,
		ST_IREQ,
		ST_IENA,
		ST_LSSA,
		ST_LVPW,
		ST_LXMAX,
		ST_LYMAX,
		ST_LPAN,
		ST_LCTR,
		ST_LCKR,
		ST_LFRA,
		ST_LAC,
		ST_LPWM,
		ST_SCTR,
		ST_SCKR,
		ST_SSR,
		ST_SMOD,
		ST_UCTR,
		ST_USR,
		ST_IRCTR,
		ST_BCTR,
		ST_BRS,
		ST_BDIV
	};

	auto in_pa_callback() { return m_in_port_cb[0].bind(); }
	auto out_pa_callback() { return m_out_port_cb[0].bind(); }
	auto in_pb_callback() { return m_in_port_cb[1].bind(); }
	auto out_pb_callback() { return m_out_port_cb[1].bind(); }
	auto in_pc_callback() { return m_in_port_cb[2].bind(); }
	auto out_pc_callback() { return m_out_port_cb[2].bind(); }
	auto in_pd_callback() { return m_in_port_cb[3].bind(); }
	auto out_pd_callback() { return m_out_port_cb[3].bind(); }
	auto in_pe_callback() { return m_in_port_cb[4].bind(); }
	auto out_pe_callback() { return m_out_port_cb[4].bind(); }
	auto in_pf_callback() { return m_in_port_cb[5].bind(); }
	auto out_pf_callback() { return m_out_port_cb[5].bind(); }
	auto in_pl_callback() { return m_in_port_cb[6].bind(); }
	auto out_pl_callback() { return m_out_port_cb[6].bind(); }

protected:
	st2xxx_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, address_map_constructor internal_map, int data_bits, bool has_banked_ram);

	virtual space_config_vector memory_space_config() const override;
	virtual void device_reset() override ATTR_COLD;

	virtual u16 st2xxx_ireq_mask() const = 0;
	virtual const char *st2xxx_irq_name(int i) const = 0;
	virtual u8 st2xxx_pmcr_mask() const = 0;
	virtual unsigned st2xxx_bt_divider(int n) const = 0;
	virtual u8 st2xxx_prs_mask() const = 0;
	virtual void st2xxx_tclk_start() { }
	virtual void st2xxx_tclk_stop() { }
	virtual u8 st2xxx_sys_mask() const = 0;
	virtual u8 st2xxx_misc_mask() const = 0;
	virtual bool st2xxx_wdten_on_reset() const { return false; }
	virtual bool st2xxx_has_dma() const { return false; }
	virtual u8 st2xxx_lpan_mask() const = 0;
	virtual u8 st2xxx_lctr_mask() const = 0;
	virtual u8 st2xxx_lckr_mask() const = 0;
	virtual u8 st2xxx_lpwm_mask() const = 0;
	virtual unsigned st2xxx_lfr_clocks() const = 0;
	virtual bool st2xxx_has_spi() const = 0;
	virtual bool st2xxx_spi_iis() const { return false; }
	virtual u8 st2xxx_uctr_mask() const = 0;
	virtual u8 st2xxx_bctr_mask() const = 0;

	class mi_st2xxx : public memory_interface {
	public:
		virtual u8 read_vector(u16 adr) = 0;

		memory_access<26, 0, 0, ENDIANNESS_LITTLE>::cache dcache;
		memory_access<26, 0, 0, ENDIANNESS_LITTLE>::specific data;

		bool irq_service;
		bool irr_enable;
		u16 irr;
		u16 prr;
		u16 drr;
	};

	void init_base_timer(u16 ireq);
	void init_lcd_timer(u16 ireq);
	void save_common_registers();

	virtual u8 read_vector(u16 adr) override;
	virtual void end_interrupt() override { set_irq_service(false); }

	void set_irq_service(bool state) { downcast<mi_st2xxx &>(*mintf).irq_service = state; }
	void update_irq_state() { irq_state = (m_ireq & m_iena) != 0; }
	u8 active_irq_level() const;

	TIMER_CALLBACK_MEMBER(bt_interrupt);
	TIMER_CALLBACK_MEMBER(lcd_interrupt);

	u8 pdata_r(offs_t offset);
	void pdata_w(offs_t offset, u8 data);
	u8 pctrl_r(offs_t offset);
	void pctrl_w(offs_t offset, u8 data);
	u8 pl_r();
	void pl_w(u8 data);
	u8 pcl_r();
	void pcl_w(u8 data);
	u8 psc_r();
	void psc_w(u8 data);
	u8 pse_r();
	void pse_w(u8 data);
	u8 pfc_r();
	void pfc_w(u8 data);
	u8 pfd_r();
	void pfd_w(u8 data);
	u8 pmcr_r();
	void pmcr_w(u8 data);

	u8 sys_r();
	void sys_w(u8 data);
	u8 misc_r();
	void misc_w(u8 data);

	u8 irrl_r();
	void irrl_w(u8 data);
	u8 irrh_r();
	void irrh_w(u8 data);
	u8 prrl_r();
	void prrl_w(u8 data);
	u8 prrh_r();
	void prrh_w(u8 data);
	u8 drrl_r();
	void drrl_w(u8 data);
	u8 drrh_r();
	void drrh_w(u8 data);

	u8 bten_r();
	void bten_w(u8 data);
	u8 btsr_r();
	void btclr_w(u8 data);
	void btclr_all_w(u8 data);

	u16 pres_count() const;
	u8 prs_r();
	void prs_w(u8 data);

	u8 ireql_r();
	void ireql_w(u8 data);
	u8 ireqh_r();
	void ireqh_w(u8 data);
	u8 ienal_r();
	void ienal_w(u8 data);
	u8 ienah_r();
	void ienah_w(u8 data);

	void lssal_w(u8 data);
	void lssah_w(u8 data);
	void lvpw_w(u8 data);
	u8 lxmax_r();
	void lxmax_w(u8 data);
	u8 lymax_r();
	void lymax_w(u8 data);
	u8 lpan_r();
	void lpan_w(u8 data);
	u8 lctr_r();
	void lctr_w(u8 data);
	void lckr_w(u8 data);
	void lfra_w(u8 data);
	void lfr_recalculate_period();
	u8 lac_r();
	void lac_w(u8 data);
	u8 lpwm_r();
	void lpwm_w(u8 data);

	u8 sctr_r();
	void sctr_w(u8 data);
	u8 sckr_r();
	void sckr_w(u8 data);
	u8 ssr_r();
	void ssr_w(u8 data);
	u8 smod_r();
	void smod_w(u8 data);

	u8 uctr_r();
	void uctr_w(u8 data);
	u8 usr_r();
	void ustr_trg_w(u8 data);
	void usr_clr_w(u8 data);
	u8 irctr_r();
	void irctr_w(u8 data);
	u8 udata_r();
	void udata_w(u8 data);
	u8 bctr_r();
	void bctr_w(u8 data);
	u8 brs_r();
	void brs_w(u8 data);
	u8 bdiv_r();
	void bdiv_w(u8 data);

	address_space_config m_data_config;

	devcb_read8::array<7> m_in_port_cb;
	devcb_write8::array<7> m_out_port_cb;

	const u16 m_prr_mask;
	const u16 m_drr_mask;

	u8 m_pdata[7];
	u8 m_pctrl[7];
	u8 m_psel[7];
	u8 m_pfun[2];
	u8 m_pmcr;

	u8 m_bten;
	u8 m_btsr;
	emu_timer *m_base_timer[8];
	u8 m_bt_mask;
	u16 m_bt_ireq;

	u16 m_pres_base;
	attotime m_pres_started;
	u8 m_prs;

	u8 m_sys;
	u8 m_misc;

	u16 m_ireq;
	u16 m_iena;
	u8 m_irq_level;

	u16 m_lssa;
	u8 m_lvpw;
	u8 m_lxmax;
	u8 m_lymax;
	u8 m_lpan;
	u8 m_lctr;
	u8 m_lckr;
	u8 m_lfra;
	u8 m_lac;
	u8 m_lpwm;
	u16 m_lcd_ireq;
	emu_timer *m_lcd_timer;

	u8 m_sctr;
	u8 m_sckr;
	u8 m_ssr;
	u8 m_smod;

	u8 m_uctr;
	u8 m_usr;
	u8 m_irctr;
	u8 m_bctr;
	u8 m_brs;
	u8 m_bdiv;
};

#endif // MAME_CPU_M6502_ST2XXX_H
