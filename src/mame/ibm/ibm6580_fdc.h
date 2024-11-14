// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
#ifndef MAME_IBM_IBM6580_FDC_H
#define MAME_IBM_IBM6580_FDC_H

#pragma once

#include "cpu/mcs48/mcs48.h"
#include "machine/i8255.h"
#include "machine/upd765.h"


class dw_fdc_device :  public device_t
{
public:
	dw_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto out_data_handler() { return m_out_data.bind(); }
	auto out_clock_handler() { return m_out_clock.bind(); }
	auto out_strobe_handler() { return m_out_strobe.bind(); }

	void reset_w(int state);
	void ack_w(int state);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(assert_reset_line);

private:
	uint8_t m_bus, m_t0, m_t1, m_p1, m_p2;
	emu_timer *m_reset_timer;

	devcb_write_line m_out_data;
	devcb_write_line m_out_clock;
	devcb_write_line m_out_strobe;
	required_device<i8048_device> m_mcu;

	void bus_w(uint8_t data);
	uint8_t bus_r();
	void p1_w(uint8_t data);
	void p2_w(uint8_t data);
	uint8_t p2_r();
	int t0_r();
	int t1_r();
};

DECLARE_DEVICE_TYPE(DW_FDC, dw_fdc_device)

#endif // MAME_IBM_IBM6580_FDC_H
