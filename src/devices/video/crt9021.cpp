// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    SMC CRT9021 Video Attributes Controller (VAC) emulation

**********************************************************************/

/*

    TODO:

    - attributes
        - character blink
        - underline
        - full/half intensity
    - operation modes
        - wide graphics
        - thin graphics
        - character mode w/o underline
        - character mode w/underline
    - double height characters
    - double width characters
    - serial scan line
    - cursor
        - underline
        - blinking underline
        - reverse video
        - blinking reverse video
    - programmable character blink rate (75/25 duty)
    - programmable cursor blink rate (50/50 duty)
    - attribute latches

*/

#include "emu.h"
#include "crt9021.h"

#include "screen.h"

//#define VERBOSE 1
#include "logmacro.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(CRT9021, crt9021_device, "crt9021", "SMC CRT9021 VAC")



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  crt9021_device - constructor
//-------------------------------------------------

crt9021_device::crt9021_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, CRT9021, tag, owner, clock),
	device_video_interface(mconfig, *this),
	m_display_cb(*this),
	m_data(0),
	m_ms0(0),
	m_ms1(0),
	m_revid(0),
	m_chabl(0),
	m_blink(0),
	m_intin(0),
	m_atten(0),
	m_cursor(0),
	m_retbl(0),
	m_ld_sh(1),
	m_sld(1),
	m_slg(0),
	m_blc(0),
	m_bkc(0),
	m_sl0(0),
	m_sl1(0),
	m_sl2(0),
	m_sl3(0),
	m_vsync(0),
	m_sr(0),
	m_intout(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void crt9021_device::device_start()
{
	m_display_cb.resolve();

	// register bitmap
	screen().register_screen_bitmap(m_bitmap);

	// state saving
	save_item(NAME(m_data));
	save_item(NAME(m_ms0));
	save_item(NAME(m_ms1));
	save_item(NAME(m_revid));
	save_item(NAME(m_chabl));
	save_item(NAME(m_blink));
	save_item(NAME(m_intin));
	save_item(NAME(m_atten));
	save_item(NAME(m_cursor));
	save_item(NAME(m_retbl));
	save_item(NAME(m_ld_sh));
	save_item(NAME(m_sld));
	save_item(NAME(m_slg));
	save_item(NAME(m_blc));
	save_item(NAME(m_bkc));
	save_item(NAME(m_sl0));
	save_item(NAME(m_sl1));
	save_item(NAME(m_sl2));
	save_item(NAME(m_sl3));
	save_item(NAME(m_vsync));
	save_item(NAME(m_sr));
	save_item(NAME(m_intout));
	save_item(NAME(m_sl));
}


//-------------------------------------------------
//  ld_sh_w - load/shift
//-------------------------------------------------

WRITE_LINE_MEMBER( crt9021_device::ld_sh_w )
{
	LOG("CRT9021 LD/SH: %u\n", state);

	if (!m_ld_sh && state)
	{
		// shift in scanline data
		if (!m_slg)
		{
			m_sl >>= 1;
			m_sl |= m_sld << 3;
		}

		// latch data
		if (m_retbl)
		{
			m_sr = 0;
		}
		else
		{
			m_sr = m_chabl ? 0 : m_data;

			if (m_revid) m_sr ^= 0xff;
		}

		// latch attributes
		if (m_atten)
		{
			// TODO
		}

		m_display_cb(m_bitmap, screen().vpos(), screen().hpos(), m_sr, m_intout);
	}
}


//-------------------------------------------------
//  vsync_w - vertical sync
//-------------------------------------------------

WRITE_LINE_MEMBER( crt9021_device::vsync_w )
{
	LOG("CRT9021 VSYNC: %u\n", state);
}


//-------------------------------------------------
//  screen_update - update screen
//-------------------------------------------------

uint32_t crt9021_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_bitmap.fill(rgb_t::black(), cliprect);

	return 0;
}
