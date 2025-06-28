// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Acorn 40 Column VDU Interface

    Part No. 200,002

    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Acorn_VDU.html

**********************************************************************/

#include "emu.h"
#include "vdu40.h"

#include "video/saa5050.h"
#include "video/mc6845.h"
#include "emupal.h"
#include "screen.h"


namespace {

class acorn_vdu40_device : public device_t, public device_acorn_bus_interface
{
public:
	acorn_vdu40_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, ACORN_VDU40, tag, owner, clock)
		, device_acorn_bus_interface(mconfig, *this)
		, m_videoram(*this, "videoram", 0x400, ENDIANNESS_LITTLE)
		, m_crtc(*this, "mc6845")
		, m_trom(*this, "saa5050")
	{
	}

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	MC6845_UPDATE_ROW(crtc_update_row);
	void vsync_changed(int state);

	memory_share_creator<uint8_t> m_videoram;
	required_device<mc6845_device> m_crtc;
	required_device<saa5050_device> m_trom;
};


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void acorn_vdu40_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(12_MHz_XTAL, 768, 132, 612, 311, 20, 270);
	screen.set_screen_update("mc6845", FUNC(mc6845_device::screen_update));

	PALETTE(config, "palette").set_entries(8);

	HD6845S(config, m_crtc, 12_MHz_XTAL / 6); // "MC6845S" on schematics
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(12);
	m_crtc->out_vsync_callback().set(FUNC(acorn_vdu40_device::vsync_changed));
	m_crtc->set_update_row_callback(FUNC(acorn_vdu40_device::crtc_update_row));

	SAA5050(config, m_trom, 12_MHz_XTAL / 2);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void acorn_vdu40_device::device_start()
{
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void acorn_vdu40_device::device_reset()
{
	address_space &space = m_bus->memspace();
	uint16_t m_blk0 = m_bus->blk0() << 12;

	space.install_ram(m_blk0 | 0x0400, m_blk0 | 0x07ff, m_videoram);
	space.install_readwrite_handler(m_blk0 | 0x0800, m_blk0 | 0x0800, emu::rw_delegate(*m_crtc, FUNC(mc6845_device::status_r)), emu::rw_delegate(*m_crtc, FUNC(mc6845_device::address_w)));
	space.install_readwrite_handler(m_blk0 | 0x0801, m_blk0 | 0x0801, emu::rw_delegate(*m_crtc, FUNC(mc6845_device::register_r)), emu::rw_delegate(*m_crtc, FUNC(mc6845_device::register_w)));
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

MC6845_UPDATE_ROW(acorn_vdu40_device::crtc_update_row)
{
	uint32_t *p = &bitmap.pix(y);

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

void acorn_vdu40_device::vsync_changed(int state)
{
	m_trom->dew_w(state);
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(ACORN_VDU40, device_acorn_bus_interface, acorn_vdu40_device, "acorn_vdu40", "Acorn 40 Column VDU Interface")
