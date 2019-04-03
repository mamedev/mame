// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Acorn 40 Column VDU Interface

    Part No. 200,002

    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Acorn_VDU.html

**********************************************************************/


#include "emu.h"
#include "vdu40.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ACORN_VDU40, acorn_vdu40_device, "acorn_vdu40", "Acorn 40 Column VDU Interface")

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void acorn_vdu40_device::device_add_mconfig(machine_config &config)
{
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(12_MHz_XTAL, 768, 132, 612, 311, 20, 270);
	m_screen->set_screen_update("mc6845", FUNC(mc6845_device::screen_update));

	PALETTE(config, "palette").set_entries(8);

	HD6845(config, m_crtc, 12_MHz_XTAL / 6);
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(12);
	m_crtc->out_vsync_callback().set(FUNC(acorn_vdu40_device::vsync_changed));
	m_crtc->set_update_row_callback(FUNC(acorn_vdu40_device::crtc_update_row), this);

	SAA5050(config, m_trom, 12_MHz_XTAL / 2);
	m_trom->set_screen_size(40, 25, 40);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  acorn_vdu40_device - constructor
//-------------------------------------------------

acorn_vdu40_device::acorn_vdu40_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ACORN_VDU40, tag, owner, clock)
	, device_acorn_bus_interface(mconfig, *this)
	, m_screen(*this, "screen")
	, m_crtc(*this, "mc6845")
	, m_trom(*this, "saa5050")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void acorn_vdu40_device::device_start()
{
	/* allocate m_videoram */
	m_videoram = std::make_unique<uint8_t[]>(0x0400);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void acorn_vdu40_device::device_reset()
{
	address_space &space = m_bus->memspace();

	space.install_ram(0x0400, 0x07ff, m_videoram.get());
	space.install_readwrite_handler(0x0800, 0x0800, read8smo_delegate(FUNC(mc6845_device::status_r), m_crtc.target()), write8smo_delegate(FUNC(mc6845_device::address_w), m_crtc.target()));
	space.install_readwrite_handler(0x0801, 0x0801, read8smo_delegate(FUNC(mc6845_device::register_r), m_crtc.target()), write8smo_delegate(FUNC(mc6845_device::register_w), m_crtc.target()));
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

MC6845_UPDATE_ROW(acorn_vdu40_device::crtc_update_row)
{
	uint32_t *p = &bitmap.pix32(y);

	m_trom->lose_w(1);
	m_trom->lose_w(0);

	for (int column = 0; column < x_count; column++)
	{
		m_trom->write(m_videoram[(ma + column) & 0x3ff]);

		m_trom->f1_w(1);
		m_trom->f1_w(0);

		for (int bit = 0; bit < 12; bit++)
		{
			m_trom->tr6_w(1);
			m_trom->tr6_w(0);

			int col = m_trom->get_rgb() ^ ((column == cursor_x) ? 7 : 0);

			int r = BIT(col, 0) * 0xff;
			int g = BIT(col, 1) * 0xff;
			int b = BIT(col, 2) * 0xff;

			*p++ = rgb_t(r, g, b);
		}
	}
}

WRITE_LINE_MEMBER(acorn_vdu40_device::vsync_changed)
{
	m_trom->dew_w(state);
}
