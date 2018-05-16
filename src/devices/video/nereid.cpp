// license:BSD-3-Clause
// copyright-holders:Sven Schnelle

#include "emu.h"
#include "nereid.h"

//#define VERBOSE 1
#include "logmacro.h"

DEFINE_DEVICE_TYPE(NEREID, nereid_device, "nereid", "HP Nereid ASIC")

nereid_device::nereid_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock),
	m_red(0),
	m_green(0),
	m_blue(0),
	m_index(0),
	m_plane_mask(0)
{
}

nereid_device::nereid_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nereid_device(mconfig, NEREID, tag, owner, clock)
{
}

void nereid_device::device_start()
{
	save_item(NAME(m_red));
	save_item(NAME(m_green));
	save_item(NAME(m_blue));
	save_item(NAME(m_index));
	save_item(NAME(m_palette));
}

void nereid_device::device_reset()
{
	memset(&m_palette, 0, sizeof(m_palette));
}

READ16_MEMBER(nereid_device::ctrl_r)
{
	uint8_t ret = 0;

	LOG("NEREID: %02X\n", offset);

	switch(offset) {
	case NEREID_RED_DATA:
		ret = m_red;
		break;
	case NEREID_GREEN_DATA:
		ret = m_red;
		break;
	case NEREID_BLUE_DATA:
		ret = m_red;
		break;
	case NEREID_INDEX:
		ret = m_index;
		break;
	case NEREID_STROBE:
		ret = 0;
		break;
	case NEREID_PLANE_MASK:
		ret = m_plane_mask;
		break;
	default:
		space.unmap();
	}
	return ret;
}

WRITE16_MEMBER(nereid_device::ctrl_w)
{
	LOG("NEREID: %02X = %02X\n", offset, data);
	data &= 0xff;
	switch(offset) {
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
	case NEREID_STROBE:
		LOG("NEREID: set color index %u: rgb_t(%u,%u,%u)\n",
		    m_index, m_red, m_green, m_blue);
		m_palette[m_index] = rgb_t(m_red, m_green, m_blue);
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
