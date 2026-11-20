// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "c2color_companion.h"

DEFINE_DEVICE_TYPE(C2_COLOR_COMPANION, c2_color_companion_device, "c2_color_companion", "C2 Color companion (HLE)")

c2_color_companion_device::c2_color_companion_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, C2_COLOR_COMPANION, tag, owner, clock)
	, i2c_hle_interface(mconfig, *this, 0x53)
{
}

void c2_color_companion_device::device_start()
{
	clear_state();
	save_item(NAME(m_challenge));
	save_item(NAME(m_received));
	save_item(NAME(m_response));
}

void c2_color_companion_device::clear_state()
{
	std::fill(std::begin(m_challenge), std::end(m_challenge), 0);
	m_received = m_response = 0;
}

void c2_color_companion_device::device_reset()
{
	clear_state();
}

void c2_color_companion_device::write_data(u16 offset, u8 data)
{
	// Command 1 is followed by three challenge bytes.
	if (offset == 1)
		m_received = 0;
	if ((offset >= 1) && (offset <= 3) && (offset == m_received + 1))
	{
		m_challenge[m_received++] = data;
		if (m_received == 3)
		{
			u8 const a = bitswap<8>(m_challenge[0], 3, 2, 1, 0, 7, 6, 5, 4) ^ 0x19;
			u8 const b = bitswap<8>(m_challenge[1], 0, 1, 2, 3, 4, 5, 6, 7) ^ 0xac;
			u8 const c = bitswap<8>(m_challenge[2], 6, 7, 4, 5, 2, 3, 0, 1) ^ 0x58;
			m_response = a + b + c;
		}
	}
}

u8 c2_color_companion_device::read_data(u16 offset)
{
	// Firmware writes command 2, then issues STOP and a two-byte read.
	if (m_received == 3)
	{
		if (offset == 0)
			return 1;
		if (offset == 1)
			return m_response;
	}
	return 0xff;
}

