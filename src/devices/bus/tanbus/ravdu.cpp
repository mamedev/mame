// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Ralph Allen Colour VDU Card

    http://www.microtan.ukpc.net/6809/Video.pdf

    TODO:
    - use MC6845 screen_update to implement cursor, requires interlace
    - verify whether INHRAM and BE lines are used

**********************************************************************/


#include "emu.h"
#include "ravdu.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(TANBUS_RAVDU, tanbus_ravdu_device, "tanbus_ravdu", "Ralph Allen Colour VDU Card")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void tanbus_ravdu_device::device_add_mconfig(machine_config &config)
{
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(50);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	m_screen->set_size(1536, 624);
	m_screen->set_visarea(0, 972 - 1, 0, 500 - 1);
	//m_screen->set_screen_update("mc6845", FUNC(mc6845_device::screen_update)); // TODO: implement interlace in mc6845
	m_screen->set_screen_update("saa5055", FUNC(saa5055_device::screen_update));

	MC6845(config, m_crtc, DERIVED_CLOCK(1, 4));
	m_crtc->set_screen(nullptr);
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(12);
	m_crtc->out_vsync_callback().set(FUNC(tanbus_ravdu_device::vsync_changed));
	m_crtc->set_update_row_callback(FUNC(tanbus_ravdu_device::crtc_update_row), this);

	SAA5055(config, m_trom, DERIVED_CLOCK(1, 1));
	m_trom->d_cb().set(FUNC(tanbus_ravdu_device::videoram_r));
	m_trom->set_screen_size(81, 25, 81);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  tanbus_ravdu_device - constructor
//-------------------------------------------------

tanbus_ravdu_device::tanbus_ravdu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, TANBUS_RAVDU, tag, owner, clock)
	, device_tanbus_interface(mconfig, *this)
	, m_screen(*this, "screen")
	, m_crtc(*this, "mc6845")
	, m_trom(*this, "saa5055")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tanbus_ravdu_device::device_start()
{
	set_clock(m_tanbus->clock());
	m_videoram = std::make_unique<uint8_t[]>(0x800);
	memset(m_videoram.get(), 0xff, sizeof(uint8_t) * 0x800);

	save_pointer(NAME(m_videoram), 0x800);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void tanbus_ravdu_device::device_reset()
{
}

//-------------------------------------------------
//  read - card read
//-------------------------------------------------

uint8_t tanbus_ravdu_device::read(offs_t offset, int inhrom, int inhram, int be)
{
	uint8_t data = 0xff;

	if ((offset & 0xf800) == 0xe000)
	{
		data = m_videoram[offset & 0x7ff];
	}

	switch (offset)
	{
	case 0xe7fe:
		data = m_crtc->status_r();
		break;
	case 0xe7ff:
		data = m_crtc->register_r();
		break;
	}

	return data;
}

//-------------------------------------------------
//  write - card write
//-------------------------------------------------

void tanbus_ravdu_device::write(offs_t offset, uint8_t data, int inhrom, int inhram, int be)
{
	if ((offset & 0xf800) == 0xe000)
	{
		m_videoram[offset & 0x7ff] = data;
	}

	switch (offset)
	{
	case 0xe7fe:
		m_crtc->address_w(data);
		break;
	case 0xe7ff:
		m_crtc->register_w(data);
		break;
	}
}

//-------------------------------------------------
//  set_inhibit_lines
//-------------------------------------------------

void tanbus_ravdu_device::set_inhibit_lines(offs_t offset, int &inhram, int &inhrom)
{
	if ((offset & 0xf800) == 0xe000)
	{
		inhram = 1;
	}
};

//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

READ8_MEMBER(tanbus_ravdu_device::videoram_r)
{
	return m_videoram[offset & 0x7ff];
}

MC6845_UPDATE_ROW(tanbus_ravdu_device::crtc_update_row)
{
	uint32_t *p = &bitmap.pix32(y);

	m_trom->lose_w(1);
	m_trom->lose_w(0);

	for (int column = 0; column < x_count; column++)
	{
		m_trom->write(m_videoram[(ma + column) & 0x7ff]);

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

WRITE_LINE_MEMBER(tanbus_ravdu_device::vsync_changed)
{
	m_trom->dew_w(state);
}
