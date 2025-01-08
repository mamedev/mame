// license:BSD-3-Clause
// copyright-holders:Devin Acker

#ifndef MAME_CPU_ARM7_UPD800468_H
#define MAME_CPU_ARM7_UPD800468_H

#pragma once

#include "arm7.h"

#include "machine/gt913_kbd.h"
#include "machine/vic_pl192.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

class upd800468_timer_device : public device_t
{
public:
	upd800468_timer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto irq_cb() { return m_irq_cb.bind(); }

	u32 rate_r();
	void rate_w(u32);

	u8 control_r();
	void control_w(u8);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(irq_timer_tick);

private:
	devcb_write_line m_irq_cb;

	emu_timer *m_timer;
	u32 m_rate;
	u8 m_control;
};

class upd800468_device : public arm7_cpu_device
{
public:
	upd800468_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t);

	void upd800468_map(address_map &map) ATTR_COLD;

	template<offs_t i> auto adc_cb() { return m_adc_cb[i].bind(); }

	template<offs_t i> auto port_in_cb() { return m_in_cb[i].bind(); }
	template<offs_t i> auto port_out_cb() { return m_out_cb[i].bind(); }

	template<offs_t i> u8 port_ddr_r() { return port_ddr_r(i); }
	template<offs_t i> void port_ddr_w(u8 data) { port_ddr_w(i, data); }

	template<offs_t i> u8 port_r() { return port_r(i); }
	template<offs_t i> void port_w(u8 data) { port_w(i, data); }

	u32 ram_enable_r();
	void ram_enable_w(u32 data);

protected:
	u16 adc_r(offs_t);

	u8 port_ddr_r(offs_t);
	void port_ddr_w(offs_t, u8);

	u8 port_r(offs_t);
	void port_w(offs_t, u8);
	void port_update(offs_t);

	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual space_config_vector memory_space_config() const override;

private:
	address_space_config m_program_config;

	required_device<vic_upd800468_device> m_vic;
	required_device_array<upd800468_timer_device, 3> m_timer;
	required_device<gt913_kbd_hle_device> m_kbd;

	devcb_read16::array<8> m_adc_cb;
	devcb_read8::array<4> m_in_cb;
	devcb_write8::array<4> m_out_cb;
	u8 m_port_ddr[4], m_port_data[4];

	memory_view m_ram_view;
	u32 m_ram_enable;
};

// device type
DECLARE_DEVICE_TYPE(UPD800468_TIMER, upd800468_timer_device)
DECLARE_DEVICE_TYPE(UPD800468, upd800468_device)

#endif // MAME_CPU_ARM7_UPD800468_H
