// license:BSD-3-Clause
// copyright-holders:Sven Schnelle

#include "emu.h"
#include "nereid.h"

#define VERBOSE 0
#include "logmacro.h"

DEFINE_DEVICE_TYPE(NEREID, nereid_device, "nereid", "HP Nereid ASIC")

nereid_device::nereid_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_palette_interface(mconfig, *this),
	m_red(0),
	m_green(0),
	m_blue(0),
	m_index(0),
	m_plane_mask(0)
{
}

nereid_device::nereid_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	nereid_device(mconfig, NEREID, tag, owner, clock)
{
}

void nereid_device::device_start()
{
	save_item(NAME(m_red));
	save_item(NAME(m_green));
	save_item(NAME(m_blue));
	save_item(NAME(m_index));
	save_item(NAME(m_plane_mask));
	save_item(NAME(m_overlay_ctl));
	save_item(NAME(m_overlay_index));
	save_item(NAME(m_unknown_a0));
}

void nereid_device::device_reset()
{
	for (int i = 0; i < palette_entries(); i++)
		set_pen_color(i, rgb_t(0, 0, 0));
	m_index = 0;
	m_plane_mask = 0;
	m_overlay_ctl = 0;
	m_red = 0;
	m_green = 0;
	m_blue = 0;
	m_unknown_a0 = 0;
	m_overlay_index = 0;
}

READ16_MEMBER(nereid_device::ctrl_r)
{
	LOG("NEREID ctrl_r: %02X\n", offset);

	switch (offset & 0x7f) {
	case NEREID_BUSY:
		return 0;

	case NEREID_RED_DATA:
		return 0xff00 | m_red;

	case NEREID_GREEN_DATA:
		return 0xff00 | m_green;

	case NEREID_BLUE_DATA:
		return 0xff00 | m_blue;

	case NEREID_INDEX:
		return 0xff00 | ~m_index;

	case NEREID_INDEX0:
		return 0xff00 | m_index;

	case NEREID_OVERLAY_CTL:
		return 0xff00 | m_overlay_ctl;

	case NEREID_WRITE_STROBE:
		return 0xff00;

	case NEREID_PLANE_MASK:
		return 0xff00 | m_plane_mask;

	case NEREID_UNKNOWN_A0:
		return 0xff00 | m_unknown_a0;

	case NEREID_OVERLAY_INDEX:
		return 0xff00 | m_overlay_index;

	case NEREID_REV:
		return 0xff01;

	default:
		LOG("NEREID ctrl_r: unknown register %04X\n", offset);
		return 0xffff;//space.unmap();
	}
	return 0xffff;
}

WRITE16_MEMBER(nereid_device::ctrl_w)
{
	LOG("NEREID: ctrl_w %02X = %02X\n", offset << 1, data);
	data &= 0xff;
	switch (offset & 0x7f) {
	case NEREID_RED_DATA:
		m_red = data;
		LOG("W NEREID_RED_DATA = %04X\n", m_red);
		break;

	case NEREID_GREEN_DATA:
		m_green = data;
		LOG("W NEREID_GREEN_DATA = %04X\n", m_green);
		break;

	case NEREID_BLUE_DATA:
		m_blue = data;
		LOG("W NEREID_BLUE_DATA = %04X\n", m_blue);
		break;

	case NEREID_INDEX:
		m_index = ~data;
		LOG("W NEREID_INDEX = %04X\n", m_index);
		break;

	case NEREID_INDEX0:
		m_index =data;
		LOG("W NEREID_INDEX0 = %04X\n", data);
		break;

	case NEREID_WRITE_STROBE:
		if (m_overlay_index & 8) {
			const int index = (m_overlay_index >> 1) & 0x3;
			LOG("NEREID: set overlay color index %u: rgb_t(%u,%u,%u)\n",
					index, m_red, m_green, m_blue);
			set_pen_color(index | 0x100, rgb_t(m_red, m_green, m_blue));
		} else {
			LOG("NEREID: set video color index %u: rgb_t(%u,%u,%u)\n",
					m_index, m_red, m_green, m_blue);
			set_pen_color(m_index, rgb_t(m_red, m_green, m_blue));
		}
		break;

	case NEREID_READ_STROBE:
	{
		rgb_t tmp = pen_color(m_index);
		m_red = tmp.r();
		m_green = tmp.g();
		m_blue = tmp.b();
		break;
	}
	case NEREID_PLANE_MASK:
		m_plane_mask = data;
		LOG("W NEREID_PLANE_MASK = %02x\n", m_plane_mask);
		break;

	case NEREID_OVERLAY_CTL:
		m_overlay_ctl = data;
		LOG("W NEREID_OVERLAY_CTL = %02x\n", m_overlay_ctl);
		break;

	case NEREID_UNKNOWN_A0:
		m_unknown_a0 = data;
		LOG("UW A0 = %04x\n", data);
		break;

	case NEREID_OVERLAY_INDEX:
		m_overlay_index = data;
		LOG("W OVERLAY_INDEX = %04x\n", data);
		break;

	default:
		LOG("nereid::ctrl_w: unknown %X = %04X (mask %04X)\n", offset << 1, data, mem_mask);
		break;
	}
}

rgb_t nereid_device::map_color(uint8_t input, uint8_t ovl)
{
	ovl &= m_overlay_ctl;

	if (ovl == 0) {
		return pen_color(input & m_plane_mask);
	} else {
		return pen_color((ovl & m_plane_mask) | 0x100);
	}
}
