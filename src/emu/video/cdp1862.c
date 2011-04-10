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



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

static const int CDP1862_BACKGROUND_COLOR_SEQUENCE[] = { 2, 0, 1, 4 };



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// devices
const device_type CDP1862 = cdp1862_device_config::static_alloc_device_config;



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

GENERIC_DEVICE_CONFIG_SETUP(cdp1862, "CDP1862")


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void cdp1862_device_config::device_config_complete()
{
	// inherit a copy of the static data
	const cdp1862_interface *intf = reinterpret_cast<const cdp1862_interface *>(static_config());
	if (intf != NULL)
		*static_cast<cdp1862_interface *>(this) = *intf;

	// or initialize to defaults if none provided
	else
	{
		memset(&m_in_rd_func, 0, sizeof(m_in_rd_func));
		memset(&m_in_bd_func, 0, sizeof(m_in_bd_func));
		memset(&m_in_gd_func, 0, sizeof(m_in_gd_func));
	}
}



//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  initialize_palette - 
//-------------------------------------------------

inline void cdp1862_device::initialize_palette()
{
	int i;

	double res_total = m_config.m_chr_r + m_config.m_chr_g + m_config.m_chr_b + m_config.m_chr_bkg;

	int weight_r = (m_config.m_chr_r / res_total) * 100;
	int weight_g = (m_config.m_chr_g / res_total) * 100;
	int weight_b = (m_config.m_chr_b / res_total) * 100;
	int weight_bkg = (m_config.m_chr_bkg / res_total) * 100;

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

		palette_set_color_rgb(m_machine, i, r, g, b);
	}
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  cdp1862_device - constructor
//-------------------------------------------------

cdp1862_device::cdp1862_device(running_machine &_machine, const cdp1862_device_config &config)
    : device_t(_machine, config),
      m_config(config)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cdp1862_device::device_start()
{
	// resolve callbacks
	devcb_resolve_read_line(&m_in_rd_func, &m_config.m_in_rd_func, this);
	devcb_resolve_read_line(&m_in_bd_func, &m_config.m_in_bd_func, this);
	devcb_resolve_read_line(&m_in_gd_func, &m_config.m_in_gd_func, this);

	// find devices
	m_screen =  m_machine.device<screen_device>(m_config.m_screen_tag);
	m_bitmap = auto_bitmap_alloc(m_machine, m_screen->width(), m_screen->height(), m_screen->format());

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
		rd = devcb_call_read_line(&m_in_rd_func);
		bd = devcb_call_read_line(&m_in_bd_func);
		gd = devcb_call_read_line(&m_in_gd_func);
	}

	for (x = 0; x < 8; x++)
	{
		int color = CDP1862_BACKGROUND_COLOR_SEQUENCE[m_bgcolor] + 8;

		if (BIT(data, 7))
		{
			color = (gd << 2) | (bd << 1) | rd;
		}

		*BITMAP_ADDR16(m_bitmap, y, sx + x) = color;

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
//  update_screen - 
//-------------------------------------------------

void cdp1862_device::update_screen(bitmap_t *bitmap, const rectangle *cliprect)
{
	copybitmap(bitmap, m_bitmap, 0, 0, 0, 0, cliprect);
	bitmap_fill(m_bitmap, cliprect, CDP1862_BACKGROUND_COLOR_SEQUENCE[m_bgcolor] + 8);
}
