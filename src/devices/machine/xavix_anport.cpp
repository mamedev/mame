// license:BSD-3-Clause
// copyright-holders:David Haywood, MagikalUnicorn
/***************************************************************************

    Four XaviX input channels with immediate reads and optional movement
    counters for mice and trackballs. Counter mode accumulates signed input
    deltas with configurable saturation.

    TODO:
    - Confirm the physical relationship between these ports and the ADC.
    - Verify saturation across production revisions.  The current selection
      preserves software compatibility, not an established hardware variant.

***************************************************************************/

#include "emu.h"
#include "xavix_anport.h"

#include <algorithm>

#define VERBOSE 0
#include "logmacro.h"

DEFINE_DEVICE_TYPE(XAVIX_ANPORT, xavix_anport_device, "xavix_anport", "XaviX Analog Ports")

xavix_anport_device::xavix_anport_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, XAVIX_ANPORT, tag, owner, clock)
	, m_input_cb(*this, 0xff)
	, m_counter_mode{}
	, m_counter_saturation{}
	, m_counter_value{}
	, m_counter_last_input{}
{
}

void xavix_anport_device::map(address_map &map)
{
	map(0x00, 0x00).rw(FUNC(xavix_anport_device::data_r<0>), FUNC(xavix_anport_device::data_w<0>));
	map(0x01, 0x01).rw(FUNC(xavix_anport_device::data_r<1>), FUNC(xavix_anport_device::data_w<1>));
	map(0x10, 0x10).rw(FUNC(xavix_anport_device::data_r<2>), FUNC(xavix_anport_device::data_w<2>));
	map(0x11, 0x11).rw(FUNC(xavix_anport_device::data_r<3>), FUNC(xavix_anport_device::data_w<3>));
}

void xavix_anport_device::device_start()
{
	save_item(NAME(m_counter_value));
	save_item(NAME(m_counter_last_input));
	save_item(NAME(m_counter_valid));
}

void xavix_anport_device::device_reset()
{
	m_counter_valid.fill(false);
}

u8 xavix_anport_device::input_r(unsigned channel)
{
	return channel < 4 ? m_input_cb[channel]() : 0xff;
}

u8 xavix_anport_device::counter_r(unsigned channel)
{
	if (!machine().side_effects_disabled())
		LOG("%s: data_r (channel %u)\n", machine().describe_context(), channel);
	u8 const input = input_r(channel);
	if (!m_counter_mode[channel])
		return input;

	if (!m_counter_valid[channel])
	{
		if (!machine().side_effects_disabled())
		{
			m_counter_value[channel] = 0;
			m_counter_last_input[channel] = input;
			m_counter_valid[channel] = true;
		}
		return 0;
	}

	int const delta = s8(input - m_counter_last_input[channel]);
	int const result = s8(m_counter_value[channel]) + delta;
	u8 const value = m_counter_saturation[channel] ? std::clamp(result, -128, 127) : result;
	if (!machine().side_effects_disabled())
	{
		m_counter_value[channel] = value;
		m_counter_last_input[channel] = input;
	}
	return value;
}

void xavix_anport_device::counter_w(unsigned channel, u8 data)
{
	LOG("%s: data_w (channel %u) %02x\n", machine().describe_context(), channel, data);
	if (m_counter_mode[channel])
	{
		m_counter_value[channel] = data;
		m_counter_last_input[channel] = input_r(channel);
		m_counter_valid[channel] = true;
	}
}
