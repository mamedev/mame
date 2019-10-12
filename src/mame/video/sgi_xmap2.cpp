// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * Silicon Graphics Multiplexed Multimode Graphics Processor (XMAP2).
 */

#include "emu.h"
#include "sgi_xmap2.h"

#define LOG_GENERAL   (1U << 0)

//#define VERBOSE       (LOG_GENERAL)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(SGI_XMAP2, sgi_xmap2_device, "sgi_xmap2", "SGI XMAP2")

sgi_xmap2_device::sgi_xmap2_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, SGI_XMAP2, tag, owner, clock)
	, device_palette_interface(mconfig, *this)
	, m_options_port(*this, "^options")
	, m_map_select(false)
{
}

void sgi_xmap2_device::device_start()
{
	// save state
	save_item(NAME(m_addr));
	save_item(NAME(m_color));
	save_item(NAME(m_overlay));
	save_item(NAME(m_mode));
	save_item(NAME(m_wid_aux));
	save_item(NAME(m_map_select));
}

void sgi_xmap2_device::device_reset()
{
	m_options = m_options_port->read();
}

u8 sgi_xmap2_device::reg_r(offs_t offset)
{
	switch (offset)
	{
	case 0: // nop
		break;

	case 1: // blue data
		if (m_addr & 0x1000)
			return m_color[m_map_select ? m_addr : (m_addr & 0xfff)].b();
		else if (m_addr < 0x10)
			return m_overlay[m_addr].b();
		break;

	case 2: // green data
		if (m_addr & 0x1000)
			return m_color[m_map_select ? m_addr : (m_addr & 0xfff)].g();
		else if (m_addr < 0x10)
			return m_overlay[m_addr].g();
		break;

	case 3: // red data
		if (m_addr & 0x1000)
			return m_color[m_map_select ? m_addr : (m_addr & 0xfff)].r();
		else if (m_addr < 0x10)
			return m_overlay[m_addr].r();
		break;

	case 4: // increment address
		// TODO: should reading increment the address register?
		//m_addr = (m_addr + 1) & 0x1fff;
		LOG("read address increment\n");
		break;

	case 5: // other data
		if (m_addr < 0x20)
		{
			u16 const mode = m_mode[(m_addr >> 1) & 0xf];

			return BIT(m_addr, 0) ? (mode >> 8) : u8(mode);
		}
		else if (m_addr == 0x20)
			return m_wid_aux;
		else if (m_addr == 0x21)
			return m_options;
		break;

	case 6: // address msb
		return m_addr >> 8;

	case 7: // address lsb
		return u8(m_addr);
	}

	return 0;
}

void sgi_xmap2_device::reg_w(offs_t offset, u8 data)
{
	switch (offset)
	{
	case 0: // nop
		break;

	case 1: // blue data
		if (m_addr & 0x1000)
		{
			unsigned const index = m_map_select ? m_addr : (m_addr & 0xfff);

			m_color[index].set_b(data);
			set_pen_blue_level(index & 0xfff, data);
		}
		else if (m_addr < 0x10)
		{
			m_overlay[m_addr].set_b(data);
			set_pen_blue_level(0x1000 + m_addr, data);
		}
		break;

	case 2: // green data
		if (m_addr & 0x1000)
		{
			unsigned const index = m_map_select ? m_addr : (m_addr & 0xfff);

			m_color[index].set_g(data);
			set_pen_green_level(index & 0xfff, data);
		}
		else if (m_addr < 0x10)
		{
			m_overlay[m_addr].set_g(data);
			set_pen_green_level(0x1000 + m_addr, data);
		}
		break;

	case 3: // red data
		if (m_addr & 0x1000)
		{
			unsigned const index = m_map_select ? m_addr : (m_addr & 0xfff);

			m_color[index].set_r(data);
			set_pen_red_level(index & 0xfff, data);
		}
		else if (m_addr < 0x10)
		{
			m_overlay[m_addr].set_r(data);
			set_pen_red_level(0x1000 + m_addr, data);
		}
		break;

	case 4: // increment address
		m_addr = (m_addr + 1) & 0x1fff;
		break;

	case 5: // other data
		if (m_addr < 0x20)
		{
			u16 &mode = m_mode[(m_addr >> 1) & 0xf];

			if (BIT(m_addr, 0))
				mode = (u16(data & 0x3f) << 8) | (mode & 0x00ff);
			else
				mode = (mode & 0x3f00) | data;
		}
		else if (m_addr == 0x20)
			m_wid_aux = BIT(data, 0);
		break;

	case 6: // address msb
		m_addr = u16((data & 0x1f) << 8) | (m_addr & 0x00ff);
		break;

	case 7: // address lsb
		m_addr = (m_addr & 0x1f00) | data;
		break;
	}
}

void sgi_xmap2_device::map_select_w(int state)
{
	if (m_map_select ^ bool(state))
	{
		m_map_select = bool(state);

		// update mame palette
		unsigned const base = m_map_select ? 4096 : 0;
		for (unsigned i = 0; i < 4096; i++)
			set_pen_color(i, m_color[base + i]);
	}
}
