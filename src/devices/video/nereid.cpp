// license:BSD-3-Clause
// copyright-holders:Sven Schnelle

#include "emu.h"
#include "nereid.h"

#define VERBOSE 1
#include "logmacro.h"

DEFINE_DEVICE_TYPE(NEREID, nereid_device, "nereid", "HP Nereid ASIC")

nereid_device::nereid_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
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
	save_item(NAME(m_palette));
	save_item(NAME(m_plane_mask));
}

void nereid_device::device_reset()
{
	memset(&m_palette, 0, sizeof(m_palette));
}

READ16_MEMBER(nereid_device::ctrl_r)
{
	LOG("NEREID ctrl_r: %02X\n", offset);

	switch(offset & 0x7f) {
	case NEREID_BUSY:
		return 0;
	case NEREID_RED_DATA:
		return 0xff00 | m_red;
	case NEREID_GREEN_DATA:
		return 0xff00 | m_green;
	case NEREID_BLUE_DATA:
		return 0xff00 | m_blue;
	case NEREID_INDEX:
		return 0xff00 | m_index;
	case NEREID_WRITE_STROBE:
		return 0xff00;
	case NEREID_PLANE_MASK:
		return 0xff00 | m_plane_mask;
	default:
		return space.unmap();
	}
	return 0xffff;
}

WRITE16_MEMBER(nereid_device::ctrl_w)
{
	LOG("NEREID: ctrl_w %02X = %02X\n", offset, data);
	data &= 0xff;
	switch(offset & 0x7f) {
	case NEREID_RED_DATA:
		m_red = data;
		break;
	case NEREID_GREEN_DATA:
		m_green = data;
		break;
	case NEREID_BLUE_DATA:
		m_blue = data;
		break;
	case NEREID_INDEX:
		m_index = ~data;
		break;
	case NEREID_WRITE_STROBE:
		LOG("NEREID: set color index %u: rgb_t(%u,%u,%u)\n",
			m_index, m_red, m_green, m_blue);
		m_palette[m_index] = rgb_t(m_red, m_green, m_blue);
		break;
	case NEREID_READ_STROBE:
		m_red = m_palette[m_index].r();
		m_green = m_palette[m_index].g();
		m_blue = m_palette[m_index].b();
		break;
	case NEREID_PLANE_MASK:
		m_plane_mask = data;
		break;
	default:
		LOG("nereid::ctrl_w: %X = %04X (mask %04X)\n", offset, data, mem_mask);
		break;
	}
}

rgb_t nereid_device::map_color(uint8_t input)
{
	return m_palette[input & m_plane_mask];
}
