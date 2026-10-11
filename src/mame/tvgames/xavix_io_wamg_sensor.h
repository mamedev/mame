// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn

#ifndef MAME_TVGAMES_XAVIX_IO_WAMG_SENSOR_H
#define MAME_TVGAMES_XAVIX_IO_WAMG_SENSOR_H

#pragma once

class xavix_wamg_sensor_device : public device_t
{
public:
	xavix_wamg_sensor_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	void set_input_source(device_t &source) ATTR_COLD;
	u8 adc_r();
	void emitter_w(int state) { m_emitter = bool(state); }
	void vblank_w(int state);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	static constexpr u8 PULSE_COUNTS[7] = { 0, 0x80, 0x50, 0x2c, 0x1e, 0x12, 0x08 };
	static constexpr u32 CLOCKS_PER_COUNT = 11'712;

	TIMER_CALLBACK_MEMBER(pulse_end);

	required_ioport m_strength;
	emu_timer *m_pulse_timer;
	u8 m_peak;
	bool m_armed;
	bool m_pulse_active;
	bool m_emitter;
};

DECLARE_DEVICE_TYPE(XAVIX_WAMG_SENSOR, xavix_wamg_sensor_device)

#endif // MAME_TVGAMES_XAVIX_IO_WAMG_SENSOR_H
