// license:BSD-3-Clause
// copyright-holders:David Haywood

/*****************************************************************************
Mogura Desse, Konami 1991
Hardware info by Guru
---------------------

This is a small test board. The game is basically a whack-a-mole type
game and has basic sound but the game is purely a bonus. The board is
primarily for testing the controls and setting up the screen and may
have been provided in non-dedicated empty Konami cabinets from the era
as there is (or was?) a Japanese law stating that arcade cabinets
could not be sold without a game included. If this is true then there
were likely many of these boards manufactured, but due to the simple
game may have been thrown away by arcade operators so how many survive
today is unknown.

PCB Layout
----------

GX141 PWB452660 (parts side)
GX141 PWB452660A (solder side)
|-------------------------------------------------------|
|CN2    MB3714 MB3714  X XXXXXXX X X  LS194 LS02  LS10  |
|                                                       |
|-|  LED-5    2SD687   X XXXXXXX X X  LS175 LS74  LS00  |
  |   LED+12  2SD687                                    |
|-|      LED+5       LM358    HC273   LS194       LS74  |
|   X X X                                               |
|                  X X XXX XXXX X X   LS161       LS74  |
|                                                       |
|                      X X X  LS06    LS175       LS74  |
|                                                       |
|J  005273           X X X X  LS06    24MHz       LS161 |
|A  005273                                              |
|M  005273          X X  TBP18S030.J7 LS367       LS161 |
|M  005273   X  X  PST572D                              |
|A  005273   X  X  X  |-------------| LS175       LS161 |
|   005273   X  X     |   Z80A      |                   |
|   005273 TEST       |-------------| LS138       LS161 |
|   005273                         S13                  |
|   005273  S1 S14 S2 27C256.N5 LS153   LS153           |
|   005273                                        6264  |
|-| CN4           LS244         LS153   LS153           |
  |                                               6264  |
|-| CN3           LS253  LS253  LS153   LS153           |
|                                                       |
|                 LS253  LS253  LS153   LS153     LS273 |
|-------------------------------------------------------|
Notes: (all ICs shown)
         Z80A - Sharp LH0080A Z80A-compatible CPU. Clock 3.000MHz (24/8)
 TBP18S030.J7 - Texas Instruments TBP18S030 256-bit (32x8-bit) Bi-Polar PROM (color PROM)
       MB3714 - Fujitsu MB3714 Audio Power Amplifier
        LM358 - Texas Instruments LM358 Dual Operational Amplifier
       005273 - Konami 005273 custom resistor array used for inputs
         TEST - Push button test switch
S1/S2/S13/S14 - 0-ohm jumpers. S14 open, S1 1-3, S2 1-2, S13 1-2
    27C256.N5 - 27C256 32kBx8-bit EPROM (main program)
         6264 - 8kBx8-bit SRAM
          CN2 - 4-pin connector for right speaker audio output
      CN3/CN4 - Connectors for player 3 and player 4 controls/buttons
            X - Resistors (some used for R2R Audio DAC)
        HSync - 15.6256kHz
        VSync - 59.1881Hz
*****************************************************************************/

#include "emu.h"

#include "konamipt.h"

#include "cpu/z80/z80.h"
#include "sound/dac.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class mogura_state : public driver_device
{
public:
	mogura_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ldac(*this, "ldac"),
		m_rdac(*this, "rdac"),
		m_gfxdecode(*this, "gfxdecode"),
		m_gfxram(*this, "gfxram"),
		m_tileram(*this, "tileram")
	{ }

	void mogura(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<dac_byte_interface> m_ldac;
	required_device<dac_byte_interface> m_rdac;
	required_device<gfxdecode_device> m_gfxdecode;
	required_shared_ptr<uint8_t> m_gfxram;
	required_shared_ptr<uint8_t> m_tileram;

	tilemap_t *m_tilemap = nullptr;

	void tileram_w(offs_t offset, uint8_t data);
	void dac_w(uint8_t data);
	void gfxram_w(offs_t offset, uint8_t data);
	TILE_GET_INFO_MEMBER(get_tile_info);
	void palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void prg_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
};


void mogura_state::palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();
	for (int i = 0; i < 0x20; i++)
	{
		int bit0, bit1, bit2;

		// red component
		bit0 = BIT(color_prom[i], 0);
		bit1 = BIT(color_prom[i], 1);
		bit2 = BIT(color_prom[i], 2);
		int const r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		// green component
		bit0 = BIT(color_prom[i], 3);
		bit1 = BIT(color_prom[i], 4);
		bit2 = BIT(color_prom[i], 5);
		int const g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		// blue component
		bit0 = 0;
		bit1 = BIT(color_prom[i], 6);
		bit2 = BIT(color_prom[i], 7);
		int const b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette.set_pen_color(bitswap<5>(i, 2, 1, 0, 4, 3), rgb_t(r, g, b));
	}
}


TILE_GET_INFO_MEMBER(mogura_state::get_tile_info)
{
	int code = m_tileram[tile_index];
	int attr = m_tileram[tile_index + 0x800];

	tileinfo.set(0,
			code,
			(attr >> 1) & 7,
			0);
}


void mogura_state::video_start()
{
	m_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(mogura_state::get_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
}

uint32_t mogura_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	const rectangle &visarea = screen.visible_area();

	// tilemap layout is a bit strange ...
	rectangle clip = visarea;
	clip.max_x = 256 - 1;
	m_tilemap->set_scrollx(0, 256);
	m_tilemap->draw(screen, bitmap, clip, 0, 0);

	clip.min_x = 256;
	clip.max_x = 512 - 1;
	m_tilemap->set_scrollx(0, -128);
	m_tilemap->draw(screen, bitmap, clip, 0, 0);

	return 0;
}

void mogura_state::tileram_w(offs_t offset, uint8_t data)
{
	m_tileram[offset] = data;
	m_tilemap->mark_tile_dirty(offset & 0x7ff);
}

void mogura_state::dac_w(uint8_t data)
{
	m_ldac->write(data >> 4);
	m_rdac->write(data & 15);
}


void mogura_state::gfxram_w(offs_t offset, uint8_t data)
{
	m_gfxram[offset] = data ;

	m_gfxdecode->gfx(0)->mark_dirty(offset / 16);
}


void mogura_state::prg_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0xc000, 0xdfff).ram(); // main RAM
	map(0xe000, 0xefff).ram().w(FUNC(mogura_state::gfxram_w)).share(m_gfxram); // RAM based characters
	map(0xf000, 0xffff).ram().w(FUNC(mogura_state::tileram_w)).share(m_tileram);
}

void mogura_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).nopw();    // ??
	map(0x08, 0x08).portr("SYSTEM");
	map(0x0c, 0x0c).portr("P1");
	map(0x0d, 0x0d).portr("P2");
	map(0x0e, 0x0e).portr("P3");
	map(0x0f, 0x0f).portr("P4");
	map(0x10, 0x10).portr("SERVICE");
	map(0x14, 0x14).w(FUNC(mogura_state::dac_w)); // 4 bit DAC x 2. MSB = left, LSB = right
}

static INPUT_PORTS_START( mogura )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE4 )

	PORT_START("P1")
	KONAMI8_B123_START(1)

	PORT_START("P2")
	KONAMI8_B123_START(2)

	PORT_START("P3")
	KONAMI8_B123_START(3)

	PORT_START("P4")
	KONAMI8_B123_START(4)

	PORT_START("SERVICE")
	PORT_SERVICE_NO_TOGGLE( 0x01, IP_ACTIVE_LOW)
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END



static const gfx_layout tiles8x8_layout =
{
	8,8,
	0x1000*8/(16*8),
	2,
	{ 0, 1 },
	{ 0, 2, 4, 6, 8, 10, 12, 14 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8
};

static GFXDECODE_START( gfx_mogura )
	GFXDECODE_RAM( "gfxram", 0, tiles8x8_layout, 0, 8 )
GFXDECODE_END


void mogura_state::mogura(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 24_MHz_XTAL / 8 ); // 3 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &mogura_state::prg_map);
	m_maincpu->set_addrmap(AS_IO, &mogura_state::io_map);
	m_maincpu->set_vblank_int("screen", FUNC(mogura_state::irq0_line_hold));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(59.1881); // ?
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(512, 512);
	screen.set_visarea(0, 320-1, 0, 256-1);
	screen.set_screen_update(FUNC(mogura_state::screen_update));
	screen.set_palette("palette");

	GFXDECODE(config, m_gfxdecode, "palette", gfx_mogura);
	PALETTE(config, "palette", FUNC(mogura_state::palette), 32);

	// sound hardware
	SPEAKER(config, "speaker", 2).front();
	DAC_4BIT_R2R(config, m_ldac, 0).add_route(ALL_OUTPUTS, "speaker", 0.25, 0);
	DAC_4BIT_R2R(config, m_rdac, 0).add_route(ALL_OUTPUTS, "speaker", 0.25, 1);
}


ROM_START( mogura )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "gx141.5n", 0x0000, 0x8000, CRC(98e6120d) SHA1(45cdb2d78224a7c44fff8cd3487f33c57669a06c)  )

	ROM_REGION( 0x20, "proms", 0 )
	ROM_LOAD( "gx141.7j", 0x00, 0x20, CRC(b21c5d5f) SHA1(6913c840dd69a7d4687f4c4cbe3ff12300f62bc2) )
ROM_END

} // anonymous namespace


GAME( 1991, mogura, 0, mogura, mogura, mogura_state, empty_init, ROT0, "Konami", "Mogura Desse (Japan)", MACHINE_SUPPORTS_SAVE )
