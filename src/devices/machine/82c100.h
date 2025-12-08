// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Chips 82C100 IBM PS/2 Model 30 and Super XT

**********************************************************************/

#ifndef MAME_MACHINE_82C100_H
#define MAME_MACHINE_82C100_H

#pragma once

#include "machine/am9517a.h"
#include "machine/i8255.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"


class f82c100_device : public device_t
{
public:
	// construction/destruction
	f82c100_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	template <typename X>
	f82c100_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, X &&clock2)
		: f82c100_device(mconfig, tag, owner, clock)
	{
		set_clock2(std::forward<X>(clock2));
	}

	void set_clock2(uint32_t clock) { m_clock2 = clock; }
	void set_clock2(const XTAL &xtal) { set_clock2(xtal.value()); }

	void map(address_map &map) ATTR_COLD;

	template <typename T> void set_cpu_tag(T &&tag) { m_maincpu.set_tag(std::forward<T>(tag)); }

	// cpu interface
	auto intr() { return m_intr_callback.bind(); }
	auto nmi() { return m_nmi_callback.bind(); }

	// dma and interrupt controller interface
	auto memr() { return m_in_memr_callback.bind(); }
	auto memw() { return m_out_memw_callback.bind(); }
	template <unsigned C> auto ior() { return m_in_ior_callback[C].bind(); }
	template <unsigned C> auto iow() { return m_out_iow_callback[C].bind(); }
	template <unsigned C> auto dack() { return m_out_dack_callback[C].bind(); }
	auto tc() { return m_tc_callback.bind(); }
	void irq1_w(int state) { m_pic8259->ir1_w(state); }
	void irq2_w(int state) { m_pic8259->ir2_w(state); }
	void irq3_w(int state) { m_pic8259->ir3_w(state); }
	void irq4_w(int state) { m_pic8259->ir4_w(state); }
	void irq5_w(int state) { m_pic8259->ir5_w(state); }
	void irq6_w(int state) { m_pic8259->ir6_w(state); }
	void irq7_w(int state) { m_pic8259->ir7_w(state); }
	void drq1_w(int state) { m_dma8237->dreq1_w(state); }
	void drq2_w(int state) { m_dma8237->dreq2_w(state); }
	void drq3_w(int state) { m_dma8237->dreq3_w(state); }
	void npnmi_w(int state);
	void rtcnmi_w(int state);
	void pwrnmi_w(int state);

	IRQ_CALLBACK_MEMBER(inta_cb) { return m_pic8259->acknowledge(); }

	// keyboard and speaker interface
	auto spkdata() { return m_spkdata_callback.bind(); }
	void kbclk_w(int state);
	void kbdata_w(int state);
	int kbclk_r();
	int kbdata_r();

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	u8 config_r(offs_t offset);
	void config_w(offs_t offset, u8 data);

	void pit8253_out2_changed(int state);
	void set_spkrdata(int state);

	u8 ppi_porta_r();
	void ppi_portb_w(u8 data);
	u8 ppi_portc_r();

	void update_nmi();

	u8 nmi_control_r();
	void nmi_control_w(u8 data);
	u8 nmi_status_r();
	void nmi_status_w(u8 data);
	u8 pwr_control_r();
	void pwr_control_w(u8 data);
	void nmi_mask_w(u8 data);
	void set_dack(u8 channel, int state);
	void dma_page_w(offs_t offset, u8 data);

	required_device<cpu_device>     m_maincpu;
	required_device<am9517a_device> m_dma8237;
	required_device<pic8259_device> m_pic8259;
	required_device<pit8254_device> m_pit8254;
	required_device<i8255_device>   m_ppi8255;

	devcb_write_line m_intr_callback;
	devcb_write_line m_nmi_callback;

	devcb_read8  m_in_memr_callback;
	devcb_write8 m_out_memw_callback;
	devcb_read8::array<4>  m_in_ior_callback;
	devcb_write8::array<4> m_out_iow_callback;
	devcb_write_line::array<4> m_out_dack_callback;
	devcb_write_line m_tc_callback;
	devcb_write_line m_spkdata_callback;

	u32 m_clock2;

	u8 m_cfg_regs[256] { 0x00 };
	u8 m_cfg_indx;

	u8 m_ppi_portb;
	int m_pit_out2;
	int m_spkdata;

	bool m_nmi_enable;
	u8 m_nmi_control;
	u8 m_nmi_status;
	u8 m_pwr_control;
	u8 m_dma_page[4];
	u8 m_dma_channel;

	int m_kbclklo;
	int m_kbclk;
	int m_kbdata;
	int m_scan_bit;
	u8 m_scan_code;
};


DECLARE_DEVICE_TYPE(F82C100, f82c100_device)

#endif // MAME_MACHINE_82C100_H
