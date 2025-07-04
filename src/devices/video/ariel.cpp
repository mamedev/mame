// license:BSD-3-Clause
// copyright-holders:R. Belmont, Patrick Mackinlay
/*
    Apple "Ariel" video RAMDAC, part # 343S1045 (slow), 343S1069 (faster)
    by R. Belmont, based on bt450.cpp by Patrick Mackinlay

    This is Apple's first in-house DAC, and it's very Brooktree flavored.

    Control bits are unknown except that bits 0-2 are color depth
    and bit 3 is set to indicate the chip is a master and cleared to
    tell it that it's a slave.

    The key color register is never used by Mac OS, but it was intended for
    a somewhat crude video overlay.  You have two Ariel chips, one master
    and one slave.   The master gets pixels from main VRAM, the slave
    gets pixels from a video input or whatever.  You set bit 3 of the control
    register to 1 on the master and 0 on the slave, and connect pin 19 of the
    master and slave together with a small pull-up resistor.

    When the master Ariel sees a color index matching the key, it will pull down
    pin 19 and tri-state its own output for that pixel clock period.  In response,
    the slave will assert its own RGB output for that pixel clock period.

    Because MAME RAMDACs almost never do the actual drawing for performance
    reasons, we don't implement any of the scaffolding to support that feature.
*/

#include "emu.h"
#include "ariel.h"

#define LOG_REGISTERS   (1U << 1)

#define VERBOSE (0)

#include "logmacro.h"

DEFINE_DEVICE_TYPE(ARIEL, ariel_device, "ariel", "Apple Ariel 256 Color RAMDAC")

ariel_device::ariel_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, ARIEL, tag, owner, clock)
	, device_palette_interface(mconfig, *this)
	, m_address(0)
	, m_address_rgb(0)
	, m_control(0)
	, m_key_color(0)
	, m_color_ram(nullptr)
{
}

void ariel_device::device_start()
{
	m_color_ram = std::make_unique<std::array<u8, 3>[]>(palette_entries());

	save_item(NAME(m_address));
	save_item(NAME(m_address_rgb));
	save_item(NAME(m_control));
	save_item(NAME(m_key_color));

	save_pointer(NAME(m_color_ram), palette_entries());
}

u8 ariel_device::read(offs_t offset)
{
	switch (offset)
	{
		case 0: return address_r();
		case 1: return palette_r();
		case 2: return control_r();
		case 3: return key_color_r();
	}
	return 0;
}

void ariel_device::write(offs_t offset, u8 data)
{
	switch (offset)
	{
		case 0:
			address_w(data);
			break;

		case 1:
			palette_w(data);
			break;

		case 2:
			control_w(data);
			break;

		case 3:
			key_color_w(data);
			break;
		}
}

u8 ariel_device::address_r()
{
	LOGMASKED(LOG_REGISTERS, "%s: read address @ 0x%02x (%s)\n", tag(), m_address, machine().describe_context());

	if (!machine().side_effects_disabled())
	{
		m_address_rgb = 0;
	}

	return m_address;
}

void ariel_device::address_w(u8 data)
{
	LOGMASKED(LOG_REGISTERS, "%s: 0x%02x to address (%s)\n", tag(), data, machine().describe_context());

	m_address_rgb = 0;
	m_address = data;
}

u8 ariel_device::palette_r()
{
	u8 const data = (m_address < palette_entries()) ? m_color_ram[m_address][m_address_rgb] : 0xff;

	LOGMASKED(LOG_REGISTERS, "%s: read palette @ 0x%02x (%s)\n", tag(), data, machine().describe_context());

	if (!machine().side_effects_disabled())
	{
		m_address_rgb = (m_address_rgb + 1) % 3;
		if (m_address_rgb == 0)
		{
			m_address++;
		}
	}

	return data;
}

void ariel_device::palette_w(u8 data)
{
	LOGMASKED(LOG_REGISTERS, "%s: write 0x%02x to palette (%s)\n", tag(), data, machine().describe_context());

	if (m_address < palette_entries())
	{
		m_color_ram[m_address][m_address_rgb] = data;

		// Sync the MAME palette so this can be used directly with ind16 screens if desired
		if (m_address_rgb == 2)
			set_pen_color(m_address, rgb_t(
				m_color_ram[m_address][0],
				m_color_ram[m_address][1],
				m_color_ram[m_address][2]));
	}

	m_address_rgb = (m_address_rgb + 1) % 3;
	if (m_address_rgb == 0)
	{
		m_address++;
	}
}

u8 ariel_device::control_r()
{
	return m_control;
}

void ariel_device::control_w(u8 data)
{
	LOGMASKED(LOG_REGISTERS, "%s: write 0x%02x to control (%s)\n", tag(), data, machine().describe_context());
	m_control = data;
}

u8 ariel_device::key_color_r()
{
	return m_control;
}

void ariel_device::key_color_w(u8 data)
{
	LOGMASKED(LOG_REGISTERS, "%s: write 0x%02x to key color (%s)\n", tag(), data, machine().describe_context());
	m_control = data;
}
