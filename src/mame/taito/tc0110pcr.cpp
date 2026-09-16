// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/*
Taito TC0110PCR
---------
Interface to palette RAM, and simple tilemap/sprite priority handler. The
priority order seems to be fixed.
The data bus is 16 bits wide.

000  W selects palette RAM address
002 RW read/write palette RAM
004  W unknown, often written to
*/

#include "emu.h"
#include "tc0110pcr.h"

#define LOG_UNKNOWN (1 << 1)

#define VERBOSE (0)

#include "logmacro.h"

DEFINE_DEVICE_TYPE(TC0110PCR, tc0110pcr_device, "tc0110pcr", "Taito TC0110PCR")

tc0110pcr_device::tc0110pcr_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, TC0110PCR, tag, owner, clock)
	, device_palette_interface(mconfig, *this)
	, m_ram(nullptr)
	, m_addr(0)
	, m_shift(0)
	, m_color_cb(*this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tc0110pcr_device::device_start()
{
	m_color_cb.resolve_safe(rgb_t(0, 0, 0));

	m_ram = make_unique_clear<uint16_t[]>(TC0110PCR_RAM_SIZE);

	save_pointer(NAME(m_ram), TC0110PCR_RAM_SIZE);
	save_item(NAME(m_addr));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void tc0110pcr_device::device_reset()
{
}

//-------------------------------------------------
//  device_post_load - device-specific postload
//-------------------------------------------------

void tc0110pcr_device::device_post_load()
{
	restore_colors();
}

/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

void tc0110pcr_device::restore_colors()
{
	for (int i = 0; i < palette_entries(); i++)
	{
		set_pen_color(i, m_color_cb(m_ram[i]));
	}
}


u16 tc0110pcr_device::word_r(offs_t offset)
{
	switch (offset)
	{
		case 1:
			return m_ram[m_addr];

		default:
			if (!machine().side_effects_disabled())
				LOGMASKED(LOG_UNKNOWN, "%s: warning - read TC0110PCR address %02x\n", machine().describe_context(), offset);
			return 0xff;
	}
}

void tc0110pcr_device::word_w(offs_t offset, u16 data)
{
	switch (offset)
	{
		case 0:
			m_addr = (data >> m_shift) & 0xfff;
			if ((data >> m_shift) >= palette_entries())
				LOGMASKED(LOG_UNKNOWN, "%s: Write to palette index > %x\n", machine().describe_context(), (palette_entries() << m_shift) - 1);
			break;

		case 1:
			m_ram[m_addr] = data & 0xffff;
			set_pen_color(m_addr, m_color_cb(data));
			break;

		default:
			LOGMASKED(LOG_UNKNOWN, "%s: warning - write %04x to TC0110PCR address %02x\n", machine().describe_context(), data, offset);
			break;
	}
}
