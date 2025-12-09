// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    F.A.C.C. EF9345 80 Column Card

    https://site.acornatom.nl/hardware/video-boards/ef9345-80-kolomskaart/

**********************************************************************/

#include "emu.h"
#include "vdu80.h"

#include "machine/timer.h"
#include "video/ef9345.h"
#include "screen.h"


namespace {

class atom_vdu80_device : public device_t, public device_acorn_bus_interface
{
public:
	atom_vdu80_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, ATOM_VDU80, tag, owner, clock)
		, device_acorn_bus_interface(mconfig, *this)
		, m_ef9345(*this, "ef9345")
	{
	}

	TIMER_DEVICE_CALLBACK_MEMBER(update_scanline);

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	required_device<ef9345_device> m_ef9345;
};


//-------------------------------------------------
//  gfx_layout cms_4080term_charlayout
//-------------------------------------------------

static const gfx_layout atom_vdu80_charlayout =
{
	8, 10,                  /* 8 x 10 characters */
	256,                    /* 256 characters */
	1,                      /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	/* y offsets */
	{ 0, 8, 2 * 8, 3 * 8, 4 * 8, 5 * 8, 6 * 8, 7 * 8, 8 * 8, 9 * 8 },
	8 * 16                  /* every char takes 16 bytes */
};

//-------------------------------------------------
//  GFXDECODE( gfx_atom_vdu80 )
//-------------------------------------------------

static GFXDECODE_START(gfx_atom_vdu80)
	GFXDECODE_ENTRY("ef9345", 0x2000, atom_vdu80_charlayout, 0, 4)
GFXDECODE_END


//-------------------------------------------------
//  ROM( atom_vdu80 )
//-------------------------------------------------

ROM_START(atom_vdu80)
	ROM_REGION(0x4000, "ef9345", 0)
	ROM_LOAD("charset.rom", 0x0000, 0x2000, BAD_DUMP CRC(b2f49eb3) SHA1(d0ef530be33bfc296314e7152302d95fdf9520fc)) // from dcvg5k
ROM_END

const tiny_rom_entry *atom_vdu80_device::device_rom_region() const
{
	return ROM_NAME( atom_vdu80 );
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void atom_vdu80_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(12_MHz_XTAL, 768, 0, 492, 312, 0, 270);
	screen.set_screen_update("ef9345", FUNC(ef9345_device::screen_update));

	GFXDECODE(config, "gfxdecode", "palette", gfx_atom_vdu80);
	PALETTE(config, "palette", palette_device::RGB_3BIT);

	EF9345(config, m_ef9345, 0);
	m_ef9345->set_screen("screen");
	m_ef9345->set_palette_tag("palette");

	TIMER(config, "scantimer").configure_scanline(FUNC(atom_vdu80_device::update_scanline), "screen", 0, 10);
}




//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void atom_vdu80_device::device_start()
{
	address_space &space = m_bus->memspace();

	space.install_readwrite_handler(0xbe20, 0xbe2f, emu::rw_delegate(*m_ef9345, FUNC(ef9345_device::data_r)), emu::rw_delegate(*m_ef9345, FUNC(ef9345_device::data_w)));

	uint8_t *FNT = memregion("ef9345")->base();
	uint16_t dest = 0x2000;

	/* Unscramble the chargen rom as the format is too complex for gfxdecode to handle unaided */
	for (uint16_t a = 0; a < 8192; a += 4096)
		for (uint16_t b = 0; b < 2048; b += 64)
			for (uint16_t c = 0; c < 4; c++)
				for (uint16_t d = 0; d < 64; d += 4)
					FNT[dest++] = FNT[a | b | c | d];
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

TIMER_DEVICE_CALLBACK_MEMBER(atom_vdu80_device::update_scanline)
{
	m_ef9345->update_scanline((uint16_t)param);
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(ATOM_VDU80, device_acorn_bus_interface, atom_vdu80_device, "atom_vdu80", "Atom EF9345 80 Column Card")
