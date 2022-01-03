// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Falco TS-28 (aka FM-II or Endura?)

    VT100/VT102/VT132 terminal

    Hardware:
    - Z80A (Z8400A PS)
	- 6116
	- SY2128-3 x2 (and two empty sockets)
	- 2x 2764 (and one empty socket)
	- 1x 2732 (chargen)
	- Z80ACTC
	- SCN2672 with SCB2673
	- Z80ADART
	- XTAL unreadable (xx.xxx MHz)
	- Bell

    TODO:
    - Everything

    Notes:
    - Coretron keyboard?

***************************************************************************/

#include "emu.h"
#include "bus/rs232/rs232.h"
#include "cpu/z80/z80.h"
#include "machine/z80ctc.h"
#include "machine/z80sio.h"
#include "video/scn2674.h"
#include "emupal.h"
#include "screen.h"


namespace {


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class ts28_state : public driver_device
{
public:
	ts28_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_pvtc(*this, "pvtc"),
		m_chargen(*this, "chargen")
	{ }

	void ts28(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	required_device<z80_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<scn2672_device> m_pvtc;
	required_region_ptr<uint8_t> m_chargen;

	void mem_map(address_map &map);
	void io_map(address_map &map);

	void char_map(address_map &map);
	void attr_map(address_map &map);
	SCN2672_DRAW_CHARACTER_MEMBER(draw_character);
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void ts28_state::mem_map(address_map &map)
{
	map(0x0000, 0x3fff).rom().region("maincpu", 0);
	map(0x8000, 0x8800).ram();
	map(0xa000, 0xafff).ram().share("charram");
	map(0xb000, 0xbfff).ram().share("attrram");
}

void ts28_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0xe8, 0xef).rw(m_pvtc, FUNC(scn2672_device::read), FUNC(scn2672_device::write));
}


//**************************************************************************
//  VIDEO EMULATION
//**************************************************************************

void ts28_state::char_map(address_map &map)
{
	map(0x0000, 0x0fff).ram().share("charram");
}

void ts28_state::attr_map(address_map &map)
{
	map(0x0000, 0x0fff).ram().share("attrram");
}

SCN2672_DRAW_CHARACTER_MEMBER( ts28_state::draw_character )
{
	uint16_t data = m_chargen[charcode << 4 | linecount];
	const pen_t *const pen = m_palette->pens();

	if (cursor)
		data = ~data;

	// foreground/background colors
	rgb_t fg = pen[1];
	rgb_t bg = pen[0];

	// draw 9 pixels of the character
	for (int i = 0; i < 9; i++)
		bitmap.pix(y, x + i) = BIT(data, i) ? fg : bg;
}

static const gfx_layout char_layout =
{
	8, 16,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	{ STEP16(0, 8) },
	8 * 16
};

static GFXDECODE_START(chars)
	GFXDECODE_ENTRY("chargen", 0, char_layout, 0, 1)
GFXDECODE_END


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void ts28_state::machine_start()
{
}

void ts28_state::machine_reset()
{
}


//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

void ts28_state::ts28(machine_config &config)
{
	Z80(config, m_maincpu, 4000000); // unknown clock
	m_maincpu->set_addrmap(AS_PROGRAM, &ts28_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &ts28_state::io_map);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_color(rgb_t::amber()); // unknown color
	m_screen->set_raw(16537000, 864, 0, 720, 319, 0, 300); // unknown clock
	m_screen->set_screen_update(m_pvtc, FUNC(scn2672_device::screen_update));

	PALETTE(config, m_palette, palette_device::MONOCHROME);

	GFXDECODE(config, "gfxdecode", m_palette, chars);

	SCN2672(config, m_pvtc, 1500000); // unknown clock
	m_pvtc->set_screen("screen");
	m_pvtc->set_character_width(9); // unknown
	m_pvtc->set_display_callback(FUNC(ts28_state::draw_character));
	m_pvtc->set_addrmap(0, &ts28_state::char_map);
	m_pvtc->set_addrmap(1, &ts28_state::attr_map);
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( ts28 )
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD("endura_v100_0.e27", 0x0000, 0x2000, CRC(2fd1ea7f) SHA1(711e09b843ecf2e463bd2b193a66842261512fa4))
	ROM_LOAD("endura_v100_1.e28", 0x2000, 0x2000, CRC(4de83d27) SHA1(a1c0858debfb0c1e82b44c620855a13dc1626302))

	ROM_REGION(0x1000, "chargen", 0)
	ROM_LOAD("chargen.f3", 0x0000, 0x1000, CRC(f0764798) SHA1(3d4826bfe1f1e533ee2134ad9ae6c2512edbb069))

	ROM_REGION(0x40, "prom", 0)
	ROM_LOAD("82s123.f3", 0x00, 0x20, CRC(862b9680) SHA1(0d19266cb9680e0e5dd92d230b64a5ee86bf6046))
	ROM_LOAD("82s123.h7", 0x00, 0x20, CRC(275a4436) SHA1(e986454f6f0f93f72b1c43a49ff3ded41630e38b))
ROM_END


} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS       INIT        COMPANY  FULLNAME  FLAGS
COMP( 1983, ts28, 0,       0,     ts28,    0,     ts28_state, empty_init, "Falco", "TS-28",  MACHINE_IS_SKELETON )
