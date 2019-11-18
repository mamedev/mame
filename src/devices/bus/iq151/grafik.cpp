// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/***************************************************************************

    IQ151 grafik emulation

***************************************************************************/

#include "emu.h"
#include "grafik.h"
#include "screen.h"

#define LOG 0

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(IQ151_GRAFIK, iq151_grafik_device, "iq151_grafik", "IQ151 grafik")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  iq151_grafik_device - constructor
//-------------------------------------------------

iq151_grafik_device::iq151_grafik_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, IQ151_GRAFIK, tag, owner, clock)
	, device_iq151cart_interface(mconfig, *this)
	, m_ppi8255(*this, "ppi8255"), m_posx(0), m_posy(0), m_all(0), m_pen(0), m_fast(0), m_ev(0), m_ex(0), m_sel(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void iq151_grafik_device::device_start()
{
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void iq151_grafik_device::device_reset()
{
	// if required adjust screen size
	if (m_screen != nullptr && m_screen->visible_area().max_x < 64*8-1)
	{
		printf("adjusting screen size\n");
		m_screen->set_visible_area(0, 64*8-1, 0, 32*8-1);
	}

	memset(m_videoram, 0x00, sizeof(m_videoram));
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void iq151_grafik_device::device_add_mconfig(machine_config &config)
{
	I8255(config, m_ppi8255);
	m_ppi8255->out_pa_callback().set(FUNC(iq151_grafik_device::x_write));
	m_ppi8255->out_pb_callback().set(FUNC(iq151_grafik_device::y_write));
	m_ppi8255->out_pc_callback().set(FUNC(iq151_grafik_device::control_w));
}

//-------------------------------------------------
//  I8255 port a
//-------------------------------------------------

WRITE8_MEMBER(iq151_grafik_device::x_write)
{
	if (LOG) logerror("Grafik: set posx 0x%02x\n", data);

	m_posx = data & 0x3f;
}

//-------------------------------------------------
//  I8255 port b
//-------------------------------------------------

WRITE8_MEMBER(iq151_grafik_device::y_write)
{
	if (LOG) logerror("Grafik: set posy 0x%02x\n", data);

	m_posy = data;
}

//-------------------------------------------------
//  I8255 port c
//-------------------------------------------------

WRITE8_MEMBER(iq151_grafik_device::control_w)
{
	if (LOG) logerror("Grafik: control write 0x%02x\n", data);

	m_all  = BIT(data, 0);
	m_pen  = BIT(data, 1);
	m_fast = BIT(data, 2);
	m_ev   = BIT(data, 3);
	m_ex   = (data>>4) & 0x03;
	m_sel  = BIT(data, 7);
}


//-------------------------------------------------
//  IO read
//-------------------------------------------------

void iq151_grafik_device::io_read(offs_t offset, uint8_t &data)
{
	if (offset >= 0xd0 && offset < 0xd4)
	{
		data = m_ppi8255->read(offset & 3);
	}
	else if (offset == 0xd4)
	{
		if (LOG) logerror("Grafik: vram read 0x%04x\n", m_posx + 0x40 * m_posy);

		if (m_sel)
			data = m_videoram[m_posx + 0x40 * m_posy];
	}
}

//-------------------------------------------------
//  IO write
//-------------------------------------------------

void iq151_grafik_device::io_write(offs_t offset, uint8_t data)
{
	if (offset >= 0xd0 && offset < 0xd4)
	{
		m_ppi8255->write(offset & 3, data);
	}
	else if (offset == 0xd4)
	{
		if (m_sel)
		{
			if (LOG) logerror("Grafik: vram write 0x%04x 0x%02x\n", m_posx + 0x40 * m_posy, data);

			if (m_all)
			{
				m_videoram[m_posx + 0x40 * m_posy] = data;
			}
			else
			{
				if (m_pen)
					m_videoram[m_posx + 0x40 * m_posy] &= ~(1 << (data >> 5));
				else
					m_videoram[m_posx + 0x40 * m_posy] |= (1 << (data >> 5));
			}
		}
	}
}


//-------------------------------------------------
//  video update
//-------------------------------------------------

void iq151_grafik_device::video_update(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (m_ev)
	{
		for (int y = 0; y < 32*8; y++)
		{
			for (int x = 0; x < 64; x++)
			{
				for (int ra = 0; ra < 8; ra++)
				{
					bitmap.pix16(y, x*8 + ra) |= BIT(m_videoram[(32*8 -1 - y)*64 + x], ra);
				}
			}
		}
	}
}
