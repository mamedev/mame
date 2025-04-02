// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * Brooktree Bt450 66MHz Monolithic CMOS 16x12 Color Palette RAMDAC
 *
 * Sources:
 *  - Product Databook 1991, Brooktree Corporation
 *
 */

#include "emu.h"
#include "bt450.h"

#define LOG_READS   (1U << 1)
#define LOG_WRITES  (1U << 2)

#define VERBOSE (LOG_WRITES)

#include "logmacro.h"

DEFINE_DEVICE_TYPE(BT450, bt450_device, "bt450", "Brooktree Bt450 16x12 Color RAMDAC")

bt450_device::bt450_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, BT450, tag, owner, clock)
	, device_palette_interface(mconfig, *this)
	, m_address(0)
	, m_address_rgb(0)
	, m_color_ram(nullptr)
{
}

void bt450_device::device_start()
{
	m_color_ram = std::make_unique<std::array<u8, 3>[]>(palette_entries());

	save_item(NAME(m_address));
	save_item(NAME(m_address_rgb));

	save_pointer(NAME(m_color_ram), palette_entries());
}

u8 bt450_device::address_r()
{
	LOGMASKED(LOG_READS, "address_r 0x%02x (%s)\n", m_address, machine().describe_context());

	if (!machine().side_effects_disabled())
		m_address_rgb = 0;

	return m_address;

}

void bt450_device::address_w(u8 data)
{
	LOGMASKED(LOG_WRITES, "address_w 0x%02x (%s)\n", data, machine().describe_context());

	m_address_rgb = 0;
	m_address = data;
}

u8 bt450_device::palette_r(address_space &space)
{
	u8 const data = (m_address < palette_entries()) ? m_color_ram[m_address][m_address_rgb] : space.unmap();

	LOGMASKED(LOG_READS, "palette_r 0x%02x (%s)\n", data, machine().describe_context());

	if (!machine().side_effects_disabled())
	{
		// increment component index and address register
		m_address_rgb = (m_address_rgb + 1) % 3;
		if (m_address_rgb == 0)
			m_address++;
	}

	return data;
}

void bt450_device::palette_w(u8 data)
{
	LOGMASKED(LOG_WRITES, "palette_w 0x%02x (%s)\n", data, machine().describe_context());

	if (m_address < palette_entries())
	{
		m_color_ram[m_address][m_address_rgb] = data & 0x0f;

		// update the mame palette to match the device
		if (m_address_rgb == 2)
			set_pen_color(m_address, rgb_t(
				m_color_ram[m_address][0] << 4,
				m_color_ram[m_address][1] << 4,
				m_color_ram[m_address][2] << 4));
	}

	// increment component index and address register
	m_address_rgb = (m_address_rgb + 1) % 3;
	if (m_address_rgb == 0)
		m_address++;
}
