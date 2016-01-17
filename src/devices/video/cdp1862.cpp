// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    RCA CDP1862 Video Display Controller emulation

**********************************************************************/

#include "cdp1862.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

static const int CDP1862_BACKGROUND_COLOR_SEQUENCE[] = { 2, 0, 1, 4 };



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

// device type definition
const device_type CDP1862 = &device_creator<cdp1862_device>;



//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  initialize_palette -
//-------------------------------------------------

inline void cdp1862_device::initialize_palette()
{
	int i;

	double res_total = m_chr_r + m_chr_g + m_chr_b + m_chr_bkg;

	int weight_r = (m_chr_r / res_total) * 100;
	int weight_g = (m_chr_g / res_total) * 100;
	int weight_b = (m_chr_b / res_total) * 100;
	int weight_bkg = (m_chr_bkg / res_total) * 100;

	for (i = 0; i < 16; i++)
	{
		int r, g, b, luma = 0;

		luma += (i & 4) ? weight_r : 0;
		luma += (i & 1) ? weight_g : 0;
		luma += (i & 2) ? weight_b : 0;
		luma += (i & 8) ? 0 : weight_bkg;

		luma = (luma * 0xff) / 100;

		r = (i & 4) ? luma : 0;
		g = (i & 1) ? luma : 0;
		b = (i & 2) ? luma : 0;

		m_palette[i] = rgb_t(r, g, b);
	}
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  cdp1862_device - constructor
//-------------------------------------------------

cdp1862_device::cdp1862_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, CDP1862, "CDP1862", tag, owner, clock, "cdp1862", __FILE__),
		device_video_interface(mconfig, *this),
		m_read_rd(*this),
		m_read_bd(*this),
		m_read_gd(*this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cdp1862_device::device_start()
{
	// resolve callbacks
	m_read_rd.resolve_safe(0);
	m_read_bd.resolve_safe(0);
	m_read_gd.resolve_safe(0);

	// find devices
	m_screen->register_screen_bitmap(m_bitmap);

	// init palette
	initialize_palette();

	// register for state saving
	save_item(NAME(m_bgcolor));
	save_item(NAME(m_con));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void cdp1862_device::device_reset()
{
	m_bgcolor = 0;
	m_con = 1;
}


//-------------------------------------------------
//  dma_w -
//-------------------------------------------------

WRITE8_MEMBER( cdp1862_device::dma_w )
{
	int rd = 1, bd = 1, gd = 1;
	int sx = m_screen->hpos() + 4;
	int y = m_screen->vpos();
	int x;

	if (!m_con)
	{
		rd = m_read_rd();
		bd = m_read_bd();
		gd = m_read_gd();
	}

	for (x = 0; x < 8; x++)
	{
		int color = CDP1862_BACKGROUND_COLOR_SEQUENCE[m_bgcolor] + 8;

		if (BIT(data, 7))
		{
			color = (gd << 2) | (bd << 1) | rd;
		}

		m_bitmap.pix32(y, sx + x) = m_palette[color];

		data <<= 1;
	}
}


//-------------------------------------------------
//  disp_on_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( cdp1862_device::bkg_w )
{
	if (state)
	{
		m_bgcolor++;

		if (m_bgcolor > 3)
		{
			m_bgcolor = 0;
		}
	}
}


//-------------------------------------------------
//  disp_off_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( cdp1862_device::con_w )
{
	if (!state)
	{
		m_con = 0;
	}
}


//-------------------------------------------------
//  screen_update -
//-------------------------------------------------

UINT32 cdp1862_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, m_bitmap, 0, 0, 0, 0, cliprect);

	m_bitmap.fill(m_palette[CDP1862_BACKGROUND_COLOR_SEQUENCE[m_bgcolor] + 8], cliprect);

	return 0;
}
