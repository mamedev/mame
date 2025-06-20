// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Acorn 80x25 VDU Interface

    Part No. 400,019

    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Acorn_80x25VDUIF.html

**********************************************************************/

#include "emu.h"
#include "vdu80.h"

#include "video/mc6845.h"
#include "emupal.h"
#include "screen.h"


namespace {

class acorn_vdu80_device : public device_t, public device_acorn_bus_interface
{
public:
	acorn_vdu80_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, ACORN_VDU80, tag, owner, clock)
		, device_acorn_bus_interface(mconfig, *this)
		, m_chargen(*this, "chargen")
		, m_videoram(*this, "videoram", 0x800, ENDIANNESS_LITTLE)
		, m_crtc(*this, "mc6845")
		, m_palette(*this, "palette")
		, m_links(*this, "LINKS")
	{
	}

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	required_memory_region m_chargen;
	memory_share_creator<uint8_t> m_videoram;
	required_device<mc6845_device> m_crtc;
	required_device<palette_device> m_palette;
	required_ioport m_links;

	MC6845_UPDATE_ROW(crtc_update_row);
};


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

INPUT_PORTS_START( vdu80 )
	PORT_START("LINKS")
	PORT_CONFNAME(0x01, 0x00, "Address Selection (RAM/CRTC)")
	PORT_CONFSETTING(0x00, "A: &1000-&17FF/&1840-&1841")
	PORT_CONFSETTING(0x01, "B: &F000-&F7FF/&E840-&E841")
	PORT_CONFNAME(0x02, 0x00, "LK2: Display Inversion")
	PORT_CONFSETTING(0x00, DEF_STR(No))
	PORT_CONFSETTING(0x02, DEF_STR(Yes))
INPUT_PORTS_END

ioport_constructor acorn_vdu80_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( vdu80 );
}


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

ROM_START( vdu80 )
	ROM_REGION(0x0800, "chargen", 0)
	ROM_LOAD("80chvdu.ic13", 0x0000, 0x0800, CRC(a943f01b) SHA1(04c326a745ba6a78185ebc2adeecb53a166a6ab3))
ROM_END

const tiny_rom_entry *acorn_vdu80_device::device_rom_region() const
{
	return ROM_NAME( vdu80 );
}


//-------------------------------------------------
//  gfx_layout acorn_vdu80_charlayout
//-------------------------------------------------

static const gfx_layout acorn_vdu80_charlayout =
{
	6, 10,                  /* 6 x 10 characters */
	128,                    /* 128 characters */
	1,                      /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5 },
	/* y offsets */
	{ 0 * 8,  1 * 8,  2 * 8,  3 * 8,  4 * 8,  5 * 8,  6 * 8,  7 * 8, 8 * 8,  9 * 8 },
	8 * 16                  /* every char takes 16 bytes */
};

//-------------------------------------------------
//  GFXDECODE( gfx_acorn_vdu80 )
//-------------------------------------------------

static GFXDECODE_START(gfx_acorn_vdu80)
	GFXDECODE_ENTRY("chargen", 0, acorn_vdu80_charlayout, 0, 1)
GFXDECODE_END


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void acorn_vdu80_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_color(rgb_t::white());
	screen.set_raw(12_MHz_XTAL, 768, 132, 612, 312, 20, 270);
	screen.set_screen_update("mc6845", FUNC(mc6845_device::screen_update));

	GFXDECODE(config, "gfxdecode", m_palette, gfx_acorn_vdu80);
	PALETTE(config, m_palette, palette_device::MONOCHROME);

	HD6845S(config, m_crtc, 2_MHz_XTAL); // "46505" on schematics (variant not verified)
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(6);
	m_crtc->set_update_row_callback(FUNC(acorn_vdu80_device::crtc_update_row));
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void acorn_vdu80_device::device_start()
{
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void acorn_vdu80_device::device_reset()
{
	address_space &space = m_bus->memspace();

	if (m_links->read() & 0x01)
	{
		space.install_ram(0xf000, 0x0f7ff, m_videoram);

		space.install_readwrite_handler(0xe840, 0xe840, 0, 0x3f, 0, emu::rw_delegate(*m_crtc, FUNC(mc6845_device::status_r)), emu::rw_delegate(*m_crtc, FUNC(mc6845_device::address_w)));
		space.install_readwrite_handler(0xe841, 0xe841, 0, 0x3e, 0, emu::rw_delegate(*m_crtc, FUNC(mc6845_device::register_r)), emu::rw_delegate(*m_crtc, FUNC(mc6845_device::register_w)));
	}
	else
	{
		space.install_ram(0x1000, 0x017ff, m_videoram);

		space.install_readwrite_handler(0x1840, 0x1840, 0, 0x3f, 0, emu::rw_delegate(*m_crtc, FUNC(mc6845_device::status_r)), emu::rw_delegate(*m_crtc, FUNC(mc6845_device::address_w)));
		space.install_readwrite_handler(0x1841, 0x1841, 0, 0x3e, 0, emu::rw_delegate(*m_crtc, FUNC(mc6845_device::register_r)), emu::rw_delegate(*m_crtc, FUNC(mc6845_device::register_w)));
	}
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

MC6845_UPDATE_ROW(acorn_vdu80_device::crtc_update_row)
{
	uint8_t invert = BIT(m_links->read(), 1);
	uint32_t *p = &bitmap.pix(y);

	for (int column = 0; column < x_count; column++)
	{
		uint8_t code = m_videoram[(ma + column) & 0x7ff];
		uint16_t addr = (code << 4) | (ra & 0x0f);
		uint8_t data = m_chargen->base()[addr & 0x7ff];

		if (column == cursor_x)
		{
			data = 0xff;
		}

		for (int bit = 0; bit < 6; bit++)
		{
			*p++ = m_palette->pen(BIT(data, 7) ^ invert);

			data <<= 1;
		}
	}
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(ACORN_VDU80, device_acorn_bus_interface, acorn_vdu80_device, "acorn_vdu80", "Acorn 80x25 VDU Interface")
