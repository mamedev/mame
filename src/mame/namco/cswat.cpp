// license:BSD-3-Clause
// copyright-holders:hap
/***************************************************************************

Namco Cosmoswat, a lightgun game that uses a projector and a bunch of
mirrors and motors to move a spotlight(s) on a large projection screen,
similar to Namco Shoot Away/Shoot Away II.
The CRT is just used for showing status and keeping scores.

X1 18.432MHz
HD68A09EP   - Hitachi M6809 CPU
2 * M58725P - Mitsubishi 2KB SRAM
0737        - Namco custom DIP28, clock divider?
5275        - Namco custom DIP42, sample player

TODO:
- improve interrupts
- hook up namco 52xx sound
- simulate projector with external artwork?

***************************************************************************/

#include "emu.h"
#include "namco52.h"

#include "cpu/m6809/m6809.h"
#include "cpu/mb88xx/mb88xx.h"
#include "sound/sn76496.h"
#include "video/resnet.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class cswat_state : public driver_device
{
public:
	cswat_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_screen(*this, "screen"),
		m_sn(*this, "sn76489a"),
		m_videoram(*this, "videoram"),
		m_dips_inp(*this, "DIPS")
	{ }

	void cswat(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;
	required_device<sn76489a_device> m_sn;
	required_shared_ptr<uint8_t> m_videoram;
	required_ioport m_dips_inp;

	uint8_t m_nmi_enabled = 0;
	tilemap_t *m_tilemap;

	void videoram_w(offs_t offset, uint8_t data);
	void irq_ack_w(uint8_t data);
	uint8_t sensors_r();
	uint8_t dipswitch_r(offs_t offset);

	INTERRUPT_GEN_MEMBER(nmi_handler);

	void palette(palette_device &palette) const;
	TILEMAP_MAPPER_MEMBER(tilemap_scan_rows);
	TILE_GET_INFO_MEMBER(get_tile_info);
	uint32_t screen_update_cswat(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void cswat_map(address_map &map) ATTR_COLD;
};


/***************************************************************************

  Video

***************************************************************************/

void cswat_state::palette(palette_device &palette) const
{
	const uint8_t *color_prom = memregion("proms")->base();
	static constexpr int resistances[3] = { 1000, 470, 220 };

	// compute the color output resistor weights
	double rweights[3], gweights[3], bweights[2];
	compute_resistor_weights(0, 255, -1.0,
			3, &resistances[0], rweights, 0, 0,
			3, &resistances[0], gweights, 0, 0,
			2, &resistances[1], bweights, 0, 0);

	// initialize the palette with these colors
	for (int i = 0; i < 256; i++)
	{
		int bit0, bit1, bit2;

		// red component
		bit0 = BIT(color_prom[i], 0);
		bit1 = BIT(color_prom[i], 1);
		bit2 = BIT(color_prom[i], 2);
		int const r = combine_weights(rweights, bit0, bit1, bit2);

		// green component
		bit0 = BIT(color_prom[i], 3);
		bit1 = BIT(color_prom[i], 4);
		bit2 = BIT(color_prom[i], 5);
		int const g = combine_weights(gweights, bit0, bit1, bit2);

		// blue component
		bit0 = BIT(color_prom[i], 6);
		bit1 = BIT(color_prom[i], 7);
		int const b = combine_weights(bweights, bit0, bit1);

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}

TILEMAP_MAPPER_MEMBER(cswat_state::tilemap_scan_rows)
{
	// like with pacman, lower and upper parts of vram are left and right columns of the screen
	row += 2;
	col -= 2;
	if (col & 0x20)
		return row + ((col & 0x1f) << 5);
	else
		return col + (row << 5);
}

TILE_GET_INFO_MEMBER(cswat_state::get_tile_info)
{
	int color = m_videoram[tile_index | 0x400]; // high bits unused
	int attr = m_videoram[tile_index | 0x800]; // "
	int code = m_videoram[tile_index] | (attr << 8 & 0x300);
	int flags = TILE_FLIPYX(attr >> 2);

	tileinfo.set(0, code, color, flags);
}

void cswat_state::video_start()
{
	m_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(cswat_state::get_tile_info)), tilemap_mapper_delegate(*this, FUNC(cswat_state::tilemap_scan_rows)), 8, 8, 36, 28);
}

uint32_t cswat_state::screen_update_cswat(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}


/***************************************************************************

  Memory Map, I/O

***************************************************************************/

void cswat_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_tilemap->mark_tile_dirty(offset & 0x3ff);
}

void cswat_state::irq_ack_w(uint8_t data)
{
	// clear vblank irq and enable nmi?
	m_maincpu->set_input_line(0, CLEAR_LINE);
	m_nmi_enabled = data & 1;
}

uint8_t cswat_state::dipswitch_r(offs_t offset)
{
	uint16_t dips = m_dips_inp->read();
	return offset ? dips >> 8 : dips & 0xff;
}

uint8_t cswat_state::sensors_r()
{
	// ?
	return machine().rand();
}

void cswat_state::cswat_map(address_map &map)
{
	map(0x0000, 0x0bff).ram().w(FUNC(cswat_state::videoram_w)).share(m_videoram);
	map(0x0c00, 0x0fff).ram();
//  map(0x1800, 0x1800).nopr(); // ? reads here after writing to $4000
	map(0x2000, 0x2000).w(FUNC(cswat_state::irq_ack_w)); // writes 1 at end of vblank irq, 0 at gamestart
	map(0x2000, 0x2001).r(FUNC(cswat_state::dipswitch_r));
	map(0x2002, 0x2002).w(FUNC(cswat_state::irq_ack_w)); // writes 0 at start of vblank irq
	map(0x2002, 0x2002).r(FUNC(cswat_state::sensors_r));
	map(0x2003, 0x2003).portr("IN0");
	map(0x4000, 0x4000).w(m_sn, FUNC(sn76489a_device::write));
//  map(0x4001, 0x4009).noprw(); // ?
	map(0x8000, 0xffff).rom();
}


/***************************************************************************

  Inputs

***************************************************************************/

static INPUT_PORTS_START( cswat )
	PORT_START("DIPS") // SW1 & SW2
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0302, 0x0302, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0102, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x0302, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0202, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x0404, 0x0404, "Extend Point" )
	PORT_DIPSETTING(      0x0404, "700, 3300, 9000" )
	PORT_DIPSETTING(      0x0400, "800, 3500, 9500" )
	PORT_DIPSETTING(      0x0004, "900, 3800, 10000" )
	PORT_DIPSETTING(      0x0000, "1000, 4000, 10400" )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x3020, 0x3020, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x1020, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x3020, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x2020, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x4040, 0x4040, "Player Ticket" )
	PORT_DIPSETTING(      0x4040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) ) // dupe
	PORT_DIPSETTING(      0x0000, "Grade 3 or 4" )
	PORT_DIPSETTING(      0x0040, "Grade 4" )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, "Speed" )
	PORT_DIPSETTING(      0x0800, "Slow" )
	PORT_DIPSETTING(      0x0000, "Fast" )
	PORT_DIPNAME( 0x8000, 0x8000, "Freeze" )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )
INPUT_PORTS_END


/***************************************************************************

  Machine Config

***************************************************************************/

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ 0, 4 },
	{ STEP4(8*8,1), STEP4(0*8,1) },
	{ STEP8(0*8,8) },
	16*8
};

static GFXDECODE_START( gfx_cswat )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout, 0, 64 )
GFXDECODE_END

INTERRUPT_GEN_MEMBER(cswat_state::nmi_handler)
{
	if (m_nmi_enabled)
		device.execute().pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

void cswat_state::machine_reset()
{
	m_nmi_enabled = 0;
}

void cswat_state::machine_start()
{
	save_item(NAME(m_nmi_enabled));
}

void cswat_state::cswat(machine_config &config)
{
	// basic machine hardware
	MC6809E(config, m_maincpu, 18.432_MHz_XTAL / 3 / 4); // 1.536MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &cswat_state::cswat_map);
	m_maincpu->set_vblank_int("screen", FUNC(cswat_state::irq0_line_assert));
	m_maincpu->set_periodic_int(FUNC(cswat_state::nmi_handler), attotime::from_hz(300)); // 16V?

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	m_screen->set_size(36*8, 28*8);
	m_screen->set_visarea_full();
	m_screen->set_palette(m_palette);
	m_screen->set_screen_update(FUNC(cswat_state::screen_update_cswat));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_cswat);
	PALETTE(config, m_palette, FUNC(cswat_state::palette), 256);

	// sound hardware
	SN76489A(config, m_sn, 18.432_MHz_XTAL / 3 / 2); // 1H
	m_sn->add_route(ALL_OUTPUTS, "speaker", 0.50);

	SPEAKER(config, "speaker").front_center();
}


/***************************************************************************

  Game drivers

***************************************************************************/

ROM_START( cswat )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cs2-1.5k",  0x8000, 0x2000, CRC(896604a8) SHA1(a7b2e2253eea8a4db6f6ea049d7a21050ea71d4b) )
	ROM_LOAD( "cs2-2.5j",  0xa000, 0x2000, CRC(746f5afb) SHA1(ebb5ad49d97ea3e4e25104c67168644cb833bd87) )
	ROM_LOAD( "cs2-3.5h",  0xc000, 0x2000, CRC(52af8d0b) SHA1(1087b1dbf8b7734f0497e0ad1f8a7a10a25f4e7b) )
	ROM_LOAD( "cs2-4.5f",  0xe000, 0x2000, CRC(2f88074c) SHA1(a8864b6123f76c0d1f372b2196d4e436bac75c21) )

	ROM_REGION( 0x4000, "gfx1", ROMREGION_INVERT )
	ROM_LOAD( "cs2-5.3l",  0x0000, 0x2000, CRC(909c8903) SHA1(876fdaec70fff7709dbd70ed4f9601c26fa3ce9b) )
	ROM_LOAD( "cs2-6.2l",  0x2000, 0x2000, CRC(95e8180f) SHA1(1d819649b62b194e10decb04d6be68c99c419a93) )

	ROM_REGION( 0x2000, "52xx", 0 ) // digitised speech
	ROM_LOAD( "cs2-7.2b",  0x0000, 0x2000, CRC(f57c4a00) SHA1(9d258026b7a829a6f685e5b48656e2142d4cc0ad) )

	ROM_REGION( 0x0200, "proms", 0 ) // color prom
	ROM_LOAD( "cs2-1.1p",  0x0000, 0x0200, CRC(2b34d9f2) SHA1(d64c60a43cf98c5e2778509f5a9b912aeda68b8f) )
ROM_END

} // anonymous namespace


GAME( 1984, cswat, 0, cswat, cswat, cswat_state, empty_init, ROT0, "Namco", "Cosmoswat", MACHINE_SUPPORTS_SAVE | MACHINE_MECHANICAL | MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )
