/**********************************************************************

    RCA CDP1862 Video Display Controller emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

/*

    TODO:

    - calculate colors from luminance/chrominance resistors

*/

#include "emu.h"
#include "cdp1862.h"
#include "machine/devhelpr.h"


// device type definition
const device_type CDP1862 = &device_creator<cdp1862_device>;


//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

static const int CDP1862_BACKGROUND_COLOR_SEQUENCE[] = { 2, 0, 1, 4 };



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

		m_palette[i] = MAKE_RGB(r, g, b);
	}
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  cdp1862_device - constructor
//-------------------------------------------------

cdp1862_device::cdp1862_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
    : device_t(mconfig, CDP1862, "CDP1862", tag, owner, clock)
{
}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void cdp1862_device::device_config_complete()
{
	// inherit a copy of the static data
	const cdp1862_interface *intf = reinterpret_cast<const cdp1862_interface *>(static_config());
	if (intf != NULL)
		*static_cast<cdp1862_interface *>(this) = *intf;

	// or initialize to defaults if none provided
	else
	{
		memset(&m_in_rd_cb, 0, sizeof(m_in_rd_cb));
		memset(&m_in_bd_cb, 0, sizeof(m_in_bd_cb));
		memset(&m_in_gd_cb, 0, sizeof(m_in_gd_cb));
	}
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cdp1862_device::device_start()
{
	// resolve callbacks
	m_in_rd_func.resolve(m_in_rd_cb, *this);
	m_in_bd_func.resolve(m_in_bd_cb, *this);
	m_in_gd_func.resolve(m_in_gd_cb, *this);

	// find devices
	m_screen =  machine().device<screen_device>(m_screen_tag);
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
		rd = m_in_rd_func();
		bd = m_in_bd_func();
		gd = m_in_gd_func();
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
