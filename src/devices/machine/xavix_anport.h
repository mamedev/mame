// license:BSD-3-Clause
// copyright-holders:David Haywood, MagikalUnicorn

#ifndef MAME_MACHINE_XAVIX_ANPORT_H
#define MAME_MACHINE_XAVIX_ANPORT_H

#pragma once

#include <array>

class xavix_anport_device : public device_t
{
public:
	xavix_anport_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	template <u32 Channel> auto input_callback() { return m_input_cb[Channel].bind(); }

	void map(address_map &map) ATTR_COLD;

	void set_counter_mode(u32 channel)
	{
		assert(channel < m_counter_mode.size());
		m_counter_mode[channel] = true;
	}

	void set_counter_saturation(u32 channel)
	{
		assert(channel < m_counter_saturation.size());
		m_counter_saturation[channel] = true;
	}

	template <u32 Channel> u8 data_r()
	{
		static_assert(Channel < 4);
		return counter_r(Channel);
	}

	template <u32 Channel> void data_w(u8 data)
	{
		static_assert(Channel < 4);
		counter_w(Channel, data);
	}

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	u8 input_r(u32 channel);
	u8 counter_r(u32 channel);
	void counter_w(u32 channel, u8 data);

	devcb_read8::array<4> m_input_cb;

	std::array<bool, 4> m_counter_mode;
	std::array<bool, 4> m_counter_saturation;
	std::array<u8, 4> m_counter_value;
	std::array<u8, 4> m_counter_last_input;
	std::array<bool, 4> m_counter_valid;
};

DECLARE_DEVICE_TYPE(XAVIX_ANPORT, xavix_anport_device)

#endif // MAME_MACHINE_XAVIX_ANPORT_H
