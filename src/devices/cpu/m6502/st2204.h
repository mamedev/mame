// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    Sitronix ST2202 8-Bit Integrated Microcontroller
    Sitronix ST2204 8-Bit Integrated Microcontroller

**********************************************************************/

#ifndef MAME_CPU_M6502_ST2204_H
#define MAME_CPU_M6502_ST2204_H

#pragma once

#include "st2xxx.h"

class st2204_device : public st2xxx_device
{
public:
	enum {
		ST_T0M = ST_BDIV + 1,
		ST_T0C,
		ST_T1M,
		ST_T1C,
		ST_PSG0,
		ST_PSG1,
		ST_VOL,
		ST_DAC,
		ST_DMS,
		ST_DMD
	};

	st2204_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	auto dac_callback() { return m_dac_callback.bind(); }

protected:
	st2204_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, address_map_constructor int_map);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual u16 st2xxx_ireq_mask() const override { return 0x0f7f; }
	virtual const char *st2xxx_irq_name(int i) const override;
	virtual u8 st2xxx_pmcr_mask() const override { return 0xff; }
	virtual unsigned st2xxx_bt_divider(int n) const override;
	virtual u8 st2xxx_prs_mask() const override { return 0xe0; }
	virtual void st2xxx_tclk_start() override;
	virtual void st2xxx_tclk_stop() override;
	virtual u8 st2xxx_sys_mask() const override { return 0xff; }
	virtual u8 st2xxx_misc_mask() const override { return 0x1f; }
	virtual bool st2xxx_has_dma() const override { return true; }
	virtual u8 st2xxx_lpan_mask() const override { return 0x07; }
	virtual u8 st2xxx_lctr_mask() const override { return 0xff; }
	virtual u8 st2xxx_lckr_mask() const override { return 0x1f; }
	virtual u8 st2xxx_lpwm_mask() const override { return 0x3f; }
	virtual unsigned st2xxx_lfr_clocks() const override;
	virtual bool st2xxx_has_spi() const override { return true; }
	virtual u8 st2xxx_uctr_mask() const override { return 0x0f; }
	virtual u8 st2xxx_bctr_mask() const override { return 0x87; }

	void common_map(address_map &map) ATTR_COLD;

private:
	class mi_st2204 : public mi_st2xxx {
	public:
		virtual u8 read(u16 adr) override;
		virtual u8 read_sync(u16 adr) override;
		virtual u8 read_arg(u16 adr) override;
		u8 read_dma(u16 adr);
		virtual u8 read_vector(u16 adr) override;
		virtual void write(u16 adr, u8 val) override;

		u8 pread(u16 adr);
		u8 preadc(u16 adr);
		void pwrite(u16 adr, u8 val);
		u8 dread(u16 adr);
		u8 dreadc(u16 adr);
		void dwrite(u16 adr, u8 val);

		u16 dmr;
	};

	u32 tclk_pres_div(u8 mode) const;
	TIMER_CALLBACK_MEMBER(t0_interrupt);
	TIMER_CALLBACK_MEMBER(t1_interrupt);
	void timer_start_from_tclk(int t);
	void t1_start_from_oscx();
	u8 t0m_r();
	void t0m_w(u8 data);
	u8 t0c_r();
	void t0c_w(u8 data);
	u8 t1m_r();
	void t1m_w(u8 data);
	u8 t1c_r();
	void t1c_w(u8 data);

	TIMER_CALLBACK_MEMBER(psg_interrupt);
	void psg_timer_reload();
	u8 psg_r(offs_t offset);
	void psg_w(offs_t offset, u8 data);
	u8 psgc_r();
	void psgc_w(u8 data);
	u8 vol_r();
	void vol_w(u8 data);
	u8 dac_r();
	void dac_w(u8 data);

	u8 dmsl_r();
	void dmsl_w(u8 data);
	u8 dmsh_r();
	void dmsh_w(u8 data);
	u8 dmdl_r();
	void dmdl_w(u8 data);
	u8 dmdh_r();
	void dmdh_w(u8 data);
	void dcntl_w(u8 data);
	void dcnth_w(u8 data);
	u8 dmrl_r();
	void dmrl_w(u8 data);
	u8 dmrh_r();
	void dmrh_w(u8 data);

	u8 pmem_r(offs_t offset);
	void pmem_w(offs_t offset, u8 data);
	u8 dmem_r(offs_t offset);
	void dmem_w(offs_t offset, u8 data);

	void int_map(address_map &map) ATTR_COLD;

	devcb_write8 m_dac_callback;

	u8 m_tmode[2];
	u8 m_tcntr[2];
	u8 m_tload[2];
	emu_timer *m_timer[2];
	u16 m_psg[2];
	u8 m_psgc;
	u8 m_vol;
	u8 m_dac;
	emu_timer *m_psg_timer;
	u16 m_dms;
	u16 m_dmd;
	u8 m_dcnth;
};

class st2202_device : public st2204_device
{
public:
	st2202_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual u8 st2xxx_misc_mask() const override { return 0x0f; }
	virtual u8 st2xxx_lctr_mask() const override { return 0xe0; }

private:
	void int_map(address_map &map) ATTR_COLD;
};

DECLARE_DEVICE_TYPE(ST2202, st2202_device)
DECLARE_DEVICE_TYPE(ST2204, st2204_device)

#endif // MAME_MACHINE_M6502_ST2204_H
