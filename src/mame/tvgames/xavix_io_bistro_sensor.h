// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn

#ifndef MAME_TVGAMES_XAVIX_IO_BISTRO_SENSOR_H
#define MAME_TVGAMES_XAVIX_IO_BISTRO_SENSOR_H

#pragma once

class xavix_bistro_sensor_device : public device_t
{
public:
	xavix_bistro_sensor_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	int sense_r() const { return m_sense; }
	void excite_w(int state);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	static constexpr u8 RESPONSE_HALF_PERIOD_TICKS[10] = { 0x00, 0x77, 0x92, 0x85, 0x98, 0x5b, 0x43, 0x68, 0x4f, 0x38 };

	TIMER_CALLBACK_MEMBER(response_tick);

	required_ioport m_ingredient;
	emu_timer *m_response_timer;
	u8 m_edges_remaining;
	bool m_excite;
	bool m_sense;
};

DECLARE_DEVICE_TYPE(XAVIX_BISTRO_SENSOR, xavix_bistro_sensor_device)

#endif // MAME_TVGAMES_XAVIX_IO_BISTRO_SENSOR_H
