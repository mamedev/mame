// license:BSD-3-Clause
// copyright-holders:Enik Land
/**********************************************************************

    The Sega 3-D Glasses / SegaScope 3-D Glasses emulation


Release data from the Sega Retro project:

  Year: 1987    Country/region: JP    Model code: ?
  Year: 1987    Country/region: US    Model code: 3073
  Year: 1987    Country/region: EU    Model code: MK-3073-50
  Year: 1989    Country/region: BR    Model code: ? ("Oculos 3D")
  Year: 1989    Country/region: KR    Model code: ?
  Year: 19??    Country/region: AR    Model code: ? ("Anteojos 3D")

**********************************************************************/

#include "emu.h"
#include "s3dglass.h"

#include "s3dglass.lh"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(SMS_3D_GLASSES, sms_3d_glasses_device, "sms_3d_glasses", "Sega 3-D Glasses")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  sms_3d_glasses_device - constructor
//-------------------------------------------------

sms_3d_glasses_device::sms_3d_glasses_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, SMS_3D_GLASSES, tag, owner, clock),
	device_sms_3d_port_interface(mconfig, *this),
	m_left_lcd(*this, "left_lcd"),
	m_right_lcd(*this, "right_lcd"),
	m_port_binocular(*this, "BINOCULAR")
{
}


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

static INPUT_PORTS_START( sms_3d_glasses )
	PORT_START("BINOCULAR")
	PORT_CONFNAME( 0x03, 0x00, "Binocular Hack" )
	PORT_CONFSETTING( 0x00, DEF_STR( Off ) )
	PORT_CONFSETTING( 0x01, "Left Lens" )
	PORT_CONFSETTING( 0x02, "Right Lens" )
	PORT_CONFSETTING( 0x03, "Both Lens" )
INPUT_PORTS_END

ioport_constructor sms_3d_glasses_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( sms_3d_glasses );
}


// For consistency, use the same template used by the SMS drivers to workaround a
// rounding issue in set_raw() that causes the ratio between CPU and pixel clocks
// to not be exactly 2/3. The SMS VDP emulation controls some counters/flags through
// screen timing and if the ratio is different the CPU will encounter unexpected
// state.

template <typename X>
void sms_3d_glasses_device::screen_sms_ntsc_raw_params(screen_device &screen, X &&pixelclock)
{
	screen.set_raw(std::forward<X>(pixelclock),
		sega315_5124_device::WIDTH,
		sega315_5124_device::LBORDER_START + sega315_5124_device::LBORDER_WIDTH - 2,
		sega315_5124_device::LBORDER_START + sega315_5124_device::LBORDER_WIDTH + 256 + 10,
		sega315_5124_device::HEIGHT_NTSC,
		sega315_5124_device::TBORDER_START + sega315_5124_device::NTSC_224_TBORDER_HEIGHT,
		sega315_5124_device::TBORDER_START + sega315_5124_device::NTSC_224_TBORDER_HEIGHT + 224);
	screen.set_refresh_hz(pixelclock / (sega315_5124_device::WIDTH * sega315_5124_device::HEIGHT_NTSC));
}

template <typename X>
void sms_3d_glasses_device::screen_sms_3d_glasses_raw_params(screen_device &screen, sms_3d_port_device *port, X &&pixelclock)
{
	screen.set_raw(std::forward<X>(pixelclock),
		port->m_screen->width(),
		port->m_screen->visible_area().min_x,
		port->m_screen->visible_area().max_x + 1,
		port->m_screen->height(),
		port->m_screen->visible_area().min_y,
		port->m_screen->visible_area().max_y + 1);
	screen.set_refresh_hz(pixelclock / (port->m_screen->width() * port->m_screen->height()));
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sms_3d_glasses_device::device_start()
{
	screen_sms_3d_glasses_raw_params(*m_left_lcd, m_port, ATTOSECONDS_TO_HZ(m_port->m_screen->refresh_attoseconds() / (m_port->m_screen->width() * m_port->m_screen->height())));
	screen_sms_3d_glasses_raw_params(*m_right_lcd, m_port, ATTOSECONDS_TO_HZ(m_port->m_screen->refresh_attoseconds() / (m_port->m_screen->width() * m_port->m_screen->height())));

	m_port->m_screen->set_video_attributes(VIDEO_ALWAYS_UPDATE);
	m_port->m_screen->register_screen_bitmap(m_tmpbitmap);

	m_left_lcd->register_screen_bitmap(m_prevleft_bitmap);
	m_right_lcd->register_screen_bitmap(m_prevright_bitmap);

	save_item(NAME(m_prevleft_bitmap));
	save_item(NAME(m_prevright_bitmap));

	// Allow screens to have crosshair, useful for the game missil3d
	machine().crosshair().get_crosshair(0).set_screen(CROSSHAIR_SCREEN_ALL);
}


//-------------------------------------------------
//  device_reset - device-specific startup
//-------------------------------------------------

void sms_3d_glasses_device::device_reset()
{
	uint8_t binocular_hack = m_port_binocular->read();

	if (binocular_hack & 0x01)
		m_prevleft_bitmap.fill(rgb_t::black());
	if (binocular_hack & 0x02)
		m_prevright_bitmap.fill(rgb_t::black());
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void sms_3d_glasses_device::device_add_mconfig(machine_config &config)
{
	SCREEN(config, m_left_lcd, SCREEN_TYPE_LCD);
	m_left_lcd->set_screen_update(FUNC(sms_3d_glasses_device::screen_update_left));

	SCREEN(config, m_right_lcd, SCREEN_TYPE_LCD);
	m_right_lcd->set_screen_update(FUNC(sms_3d_glasses_device::screen_update_right));

	// initial timings to pass validation and set correct aspect ratio,
	// they are overridden with main screen parameters in device_start().
	screen_sms_ntsc_raw_params(*m_left_lcd, XTAL(10'738'635)/2);
	screen_sms_ntsc_raw_params(*m_right_lcd, XTAL(10'738'635)/2);

	config.set_default_layout(layout_s3dglass);
}


void sms_3d_glasses_device::update_displayed_range()
{
	m_left_lcd->update_partial(m_left_lcd->vpos());
	m_right_lcd->update_partial(m_right_lcd->vpos());
}


void sms_3d_glasses_device::blit_bitmap(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int min_y = std::max(cliprect.min_y, m_port->m_screen->visible_area().min_y);
	int max_y = std::min(cliprect.max_y, m_port->m_screen->visible_area().max_y);

	m_port->m_screen->pixels(&m_tmpbitmap.pix(0));

	for (int y = min_y; y <= max_y; y++)
	{
		int scr_visible_width = m_port->m_screen->visible_area().max_x - m_port->m_screen->visible_area().min_x + 1;
		int min_x = std::max(cliprect.min_x, m_port->m_screen->visible_area().min_x);
		int max_x = std::min(cliprect.max_x, m_port->m_screen->visible_area().max_x);

		uint32_t *const linedst = &bitmap.pix(y);
		uint32_t const *const line = &m_tmpbitmap.pix(0) + ((y - min_y) * scr_visible_width) - min_x;

		for (int x = min_x; x <= max_x; x++)
		{
			linedst[x] = line[x];
		}
	}
}


uint32_t sms_3d_glasses_device::screen_update_left(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (m_port->m_sscope_state)
	{
		// state 0 = left screen OFF, right screen ON

		blit_bitmap(bitmap, cliprect);

		// HACK: fake 3D->2D handling (if enabled, it repeats each frame twice on the selected lens)
		// save a copy of current bitmap for the binocular hack
		if (BIT(m_port_binocular->read(), 0))
			copybitmap(m_prevleft_bitmap, bitmap, 0, 0, 0, 0, cliprect);
	}
	else
	{
		// HACK: fake 3D->2D handling (if enabled, it repeats each frame twice on the selected lens)
		// use the copied bitmap for the binocular hack
		if (BIT(m_port_binocular->read(), 0))
		{
			copybitmap(bitmap, m_prevleft_bitmap, 0, 0, 0, 0, cliprect);
			return 0;
		}
		bitmap.fill(rgb_t::black(), cliprect);
	}

	return 0;
}

uint32_t sms_3d_glasses_device::screen_update_right(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (!m_port->m_sscope_state)
	{
		// state 1 = left screen ON, right screen OFF

		blit_bitmap(bitmap, cliprect);

		// HACK: fake 3D->2D handling (if enabled, it repeats each frame twice on the selected lens)
		// save a copy of current bitmap for the binocular hack
		if (BIT(m_port_binocular->read(), 1))
			copybitmap(m_prevright_bitmap, bitmap, 0, 0, 0, 0, cliprect);
	}
	else
	{
		// HACK: fake 3D->2D handling (if enabled, it repeats each frame twice on the selected lens)
		// use the copied bitmap for the binocular hack
		if (BIT(m_port_binocular->read(), 1))
		{
			copybitmap(bitmap, m_prevright_bitmap, 0, 0, 0, 0, cliprect);
			return 0;
		}
		bitmap.fill(rgb_t::black(), cliprect);
	}

	return 0;
}

