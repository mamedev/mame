// license:BSD-3-Clause
// copyright-holders:Roberto Zandona'
/* Unknown - Poker (morugem bootleg)
   Unknown - Slot

driver by Roberto Zandona'
thanks to Angelo Salese for some precious advice

TO DO:
- check sound

Anno: 1982
Produttore:
N.revisione:

CPU:
1x unknown DIP40 (1ef)(missing)
1x TBA820 (14e)(sound)
1x oscillator 12.000 (2f)

ROMs:
2x TMS2532 (5b,5e)
1x TMS2516 (8b)
1x PROM SN74S288N (8a)
1x RAM MWS5101AEL2 (11e)
4x RAM AM9114EPC (2b,3b,8e,9e)

Note:
1x 22x2 edge connector
1x trimmer (volume)
1x 8x2 switches dip
1x empty DIP14 socket (close to sound)

Funzionamento: Non Funzionante

Dumped: 06/04/2009 f205v
*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/dac.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class murogmbl_state : public driver_device
{
public:
	murogmbl_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_video(*this, "video")
	{ }

	void murogmbl(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_video;

	void murogmbl_palette(palette_device &palette) const;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void murogmbl_map(address_map &map) ATTR_COLD;
};

class slotunbl_state : public driver_device
{
public:
	slotunbl_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_video(*this, "video")
	{ }

	void slotunbl(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_video;

	void slotunbl_palette(palette_device &palette) const;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void slotunbl_map(address_map &map) ATTR_COLD;
};

void murogmbl_state::murogmbl_palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();

	for (int i = 0; i < 0x20; ++i)
	{
		int bit0, bit1, bit2;

		bit0 = BIT(color_prom[i], 0);
		bit1 = BIT(color_prom[i], 1);
		bit2 = BIT(color_prom[i], 2);
		int const r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		bit0 = BIT(color_prom[i], 3);
		bit1 = BIT(color_prom[i], 4);
		bit2 = BIT(color_prom[i], 5);
		int const g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		bit0 = 0;
		bit1 = BIT(color_prom[i], 6);
		bit2 = BIT(color_prom[i], 7);
		int const b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}

void slotunbl_state::slotunbl_palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();

	for (int i = 0; i < 0x20; ++i)
	{
		int bit0, bit1, bit2;

		bit0 = BIT(color_prom[i], 0);
		bit1 = BIT(color_prom[i], 1);
		bit2 = BIT(color_prom[i], 2);
		int const r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		bit0 = BIT(color_prom[i], 3);
		bit1 = BIT(color_prom[i], 4);
		bit2 = BIT(color_prom[i], 5);
		int const g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		bit0 = 0;
		bit1 = BIT(color_prom[i], 6);
		bit2 = BIT(color_prom[i], 7);
		int const b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}

void murogmbl_state::murogmbl_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x4000, 0x43ff).ram();
	map(0x4800, 0x4bff).ram();
	map(0x5800, 0x5bff).ram().share("video");
	map(0x5c00, 0x5fff).ram();
	map(0x6000, 0x6000).portr("IN0");
	map(0x6800, 0x6800).portr("DSW");
	map(0x7000, 0x7000).portr("IN1");
	map(0x7800, 0x7800).nopr().w("dac", FUNC(dac_byte_interface::data_w)); /* read is always discarded */
}

void slotunbl_state::slotunbl_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x4000, 0x43ff).ram();
	map(0x4800, 0x4fff).ram();
	map(0x5000, 0x53ff).ram().share("video");
//  map(0x5400, 0x5fff).ram();
	map(0x6000, 0x6000).portr("IN0");
	map(0x6800, 0x6800).portr("DSW");
	map(0x7000, 0x7000).portr("IN1");
	map(0x7800, 0x7800).nopr().w("dac", FUNC(dac_byte_interface::data_w)); /* read is always discarded */
}

void murogmbl_state::video_start()
{
}

void slotunbl_state::video_start()
{
}

uint32_t murogmbl_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	gfx_element *gfx = m_gfxdecode->gfx(0);
	int count = 0;

	int y, x;

	for (y = 0; y < 32; y++)
	{
		for (x = 0; x < 32; x++)
		{
			int tile = m_video[count];
				gfx->opaque(bitmap,cliprect, tile, 0, 0, 0, x * 8, y * 8);

			count++;
		}
	}
	return 0;
}

uint32_t slotunbl_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	gfx_element *gfx = m_gfxdecode->gfx(0);
	int count = 0;

	int y, x;

	for (y = 0; y < 32; y++)
	{
		for (x = 0; x < 32; x++)
		{
			int tile = m_video[count];
				gfx->opaque(bitmap,cliprect, tile, 0, 0, 0, x * 8, y * 8);

			count++;
		}
	}
	return 0;
}

static INPUT_PORTS_START( murogmbl )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_POKER_CANCEL ) PORT_NAME("Clear")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_GAMBLE_BET ) PORT_NAME("Replay")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_GAMBLE_DEAL )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_POKER_HOLD5 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Coin clear") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_NAME("Coin 1")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_NAME("Coin 2")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_POKER_HOLD1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_POKER_HOLD2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_POKER_HOLD3 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_POKER_HOLD4 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x08, "10 credits" )
	PORT_DIPSETTING(    0x00, "5 credits" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( slotunbl )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_POKER_CANCEL ) PORT_NAME("Clear")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_GAMBLE_BET ) PORT_NAME("Replay")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_GAMBLE_DEAL )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_POKER_HOLD5 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Coin clear") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_NAME("Coin 1")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_NAME("Coin 2")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_POKER_HOLD1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_POKER_HOLD2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_POKER_HOLD3 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_POKER_HOLD4 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x08, "10 credits" )
	PORT_DIPSETTING(    0x00, "5 credits" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static const gfx_layout layout8x8x2 =
{
	8,8,
	RGN_FRAC(1,2),
	2,
	{
		RGN_FRAC(0,2),RGN_FRAC(1,2)
	},
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8*8
};

static GFXDECODE_START( gfx_murogmbl )
	GFXDECODE_ENTRY( "gfx1", 0, layout8x8x2,  0x0, 1 )
GFXDECODE_END

static GFXDECODE_START( gfx_slotunbl )
	GFXDECODE_ENTRY( "gfx1", 0, layout8x8x2,  0x0, 1 )
GFXDECODE_END

void murogmbl_state::murogmbl(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 1000000); /* Z80? */
	m_maincpu->set_addrmap(AS_PROGRAM, &murogmbl_state::murogmbl_map);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_murogmbl);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 0*8, 32*8-1);
	screen.set_screen_update(FUNC(murogmbl_state::screen_update));
	screen.set_palette(m_palette);

	PALETTE(config, m_palette, FUNC(murogmbl_state::murogmbl_palette), 0x100);

	SPEAKER(config, "speaker").front_center();
	DAC_8BIT_R2R(config, "dac", 0).add_route(ALL_OUTPUTS, "speaker", 0.5); // unknown DAC
}

void slotunbl_state::slotunbl(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 1000000); /* Z80? */
	m_maincpu->set_addrmap(AS_PROGRAM, &slotunbl_state::slotunbl_map);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_slotunbl);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 0*8, 32*8-1);
	screen.set_screen_update(FUNC(slotunbl_state::screen_update));
	screen.set_palette(m_palette);

	PALETTE(config, m_palette, FUNC(slotunbl_state::slotunbl_palette), 0x100);

	SPEAKER(config, "speaker").front_center();
	DAC_8BIT_R2R(config, "dac", 0).add_route(ALL_OUTPUTS, "speaker", 0.5); // unknown DAC
}

ROM_START(murogmbl)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("2532.5e", 0x0000, 0x1000, CRC(093d4560) SHA1(d5401b5f7a2ebe5099fefc5b09f8710886e243b2) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD("2532.8b", 0x0000, 0x0800, CRC(4427ffc0) SHA1(45f5fd0ae967cdb6abbf2e6c6d12d787556488ef) )
	ROM_CONTINUE(0x0000, 0x0800)
	ROM_LOAD("2516.5b",     0x0800, 0x0800, CRC(496ad48c) SHA1(28380c9d02b64e7d5ef2763de92cd2ca8861eceb) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "74s288.a8",  0x0000, 0x0020, CRC(fc35201c) SHA1(4549e228c48992e0d10957f029b89a547392e72b) )
ROM_END

ROM_START(slotunbl)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("2532.e7", 0x0000, 0x1000, CRC(031929a7) SHA1(a140e1450b6196bbe9f130e7f3b7cfe2f876b86d) )
	ROM_LOAD("2532.e7", 0x1000, 0x1000, CRC(031929a7) SHA1(a140e1450b6196bbe9f130e7f3b7cfe2f876b86d) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD("2516.d3", 0x0000, 0x0800, CRC(708764d8) SHA1(f914ae3318558c2530f007a427ec98a529984bb4) )
	ROM_LOAD("2516.d4", 0x0800, 0x0800, CRC(2cffb600) SHA1(d7c8ea6395e9cba43b9fe0cb9ddf45b96c442544) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "74s288.a8",  0x0000, 0x0020, NO_DUMP )
ROM_END

} // anonymous namespace


GAME( 1982, murogmbl,  murogem,   murogmbl, murogmbl, murogmbl_state, empty_init, ROT0, "bootleg?", "Muroge Monaco (bootleg?)", MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )

GAME( 1982, slotunbl,  0,   slotunbl, slotunbl, slotunbl_state, empty_init, ROT0, "bootleg?", "Slot (unknown bootleg?)", MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
