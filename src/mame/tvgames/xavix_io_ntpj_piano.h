// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn

#ifndef MAME_TVGAMES_XAVIX_IO_NTPJ_PIANO_H
#define MAME_TVGAMES_XAVIX_IO_NTPJ_PIANO_H

#pragma once

#include "video/pwm.h"

class xavix_piano_controller_device : public device_t
{
public:
	xavix_piano_controller_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	void set_input_source(device_t &source) ATTR_COLD;
	auto event_callback() { return m_event_out_cb.bind(); }

	u8 data_r() const { return m_data; }
	void host_w(u8 data, u8 direction);
	void pio1_w(u8 data, u8 direction);
	void pio2_w(u8 data, u8 direction);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	static constexpr u8 GUIDE_KEYS[3][5] =
	{
		{ 1, 4, 7, 10, 13 },
		{ 0, 3, 6, 9, 12 },
		{ 14, 2, 5, 8, 11 }
	};

	TIMER_CALLBACK_MEMBER(event_timer);
	void light_w(offs_t offset, u8 data);

	devcb_write_line m_event_out_cb;
	required_ioport_array<3> m_keys;
	required_device<pwm_display_device> m_led_pwm;
	output_finder<14> m_key_lights;
	emu_timer *m_event_timer;

	u32 m_shift;
	u8 m_bit;
	u8 m_data;
	bool m_clock;
	bool m_scanning;
};

DECLARE_DEVICE_TYPE(XAVIX_PIANO_CONTROLLER, xavix_piano_controller_device)

#endif // MAME_TVGAMES_XAVIX_IO_NTPJ_PIANO_H
