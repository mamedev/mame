// license: BSD-3-Clause
// copyright-holders: David Haywood, Dirk Best
/***************************************************************************

    Push-Over

    © 1981 Summit Coin

    Video Fruit Machine

    TODO:
    - Scrolling is wrong
    - Missing graphics at top and right (pound values)
    - Palette (currently handmade)
    - Sound (discrete?)
    - Better artwork

    Notes:
    - https://www.youtube.com/watch?v=2vtAwkP7JyM
    - https://www.youtube.com/watch?v=xx7b5Zd1Qvc
    - https://patents.google.com/patent/GB2106293A/en

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"
#include "speaker.h"

#include "summit.lh"


namespace {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class summit_state : public driver_device
{
public:
	summit_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_attr(*this, "attr"),
		m_vram(*this, "vram"),
		m_lamps(*this, "lamp_%u", 0U)
	{ }

	void summit(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_shared_ptr<uint8_t> m_attr;
	required_shared_ptr<uint8_t> m_vram;

	output_finder<16> m_lamps;

	void mainmap(address_map &map) ATTR_COLD;

	void sound_w(uint8_t data);
	void out2_w(uint8_t data);
	void out3_w(uint8_t data);
	void lamps1_w(uint8_t data);
	void lamps2_w(uint8_t data);

	void vram_w(offs_t offset, uint8_t data);

	void summit_palette(palette_device &palette) const;
	TILE_GET_INFO_MEMBER(tile_info);
	uint32_t screen_update_summit(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	tilemap_t *m_tilemap = nullptr;
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void summit_state::mainmap(address_map &map)
{
	map.global_mask(0x3fff);
	map(0x0000, 0x17ff).rom();
	map(0x2000, 0x23ff).ram().share("attr");
	map(0x2800, 0x2bff).ram().w(FUNC(summit_state::vram_w)).share("vram");
	map(0x3000, 0x31ff).ram();
	map(0x3800, 0x3800).portr("IN0");
	map(0x3800, 0x3800).w(FUNC(summit_state::sound_w));
	map(0x3880, 0x3880).w(FUNC(summit_state::out2_w));
	map(0x3900, 0x3900).portr("IN1");
	map(0x3900, 0x3900).w(FUNC(summit_state::out3_w));
	map(0x3980, 0x3980).w(FUNC(summit_state::lamps1_w));
	map(0x3a00, 0x3a00).portr("IN2");
	map(0x3a00, 0x3a00).w(FUNC(summit_state::lamps2_w));
	map(0x3b00, 0x3b00).portr("IN3");
}


//**************************************************************************
//  INPUT PORT DEFINITIONS
//**************************************************************************

static INPUT_PORTS_START( summit )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_CANCEL )
	PORT_DIPNAME( 0x02, 0x02, "IN0-02" )        // tilt?
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPNAME( 0x04, 0x04, "IN0-04" )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_DIPNAME( 0x40, 0x40, "Percentage 02" )     // payout percentage
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )   // 72
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )  // 70
	PORT_DIPNAME( 0x80, 0x80, "IN0-80" )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_NAME("Right / Gamble")
	PORT_DIPNAME( 0x02, 0x02, "IN1-02" )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_NAME("Left / Collect")
	PORT_DIPNAME( 0x08, 0x08, "IN1-08" )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_NAME("Down")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )  PORT_NAME("Up")
	PORT_DIPNAME( 0x40, 0x40, "Percentage 04" )     // payout percentage
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )   // 74
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )  // 70
	PORT_DIPNAME( 0x80, 0x80, "IN1-80" )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )

	PORT_START("IN2")
	PORT_DIPNAME( 0x01, 0x01, "Token Refill" )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPNAME( 0x02, 0x02, "IN2-02" )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_TOGGLE
	PORT_DIPNAME( 0x08, 0x08, "IN2-08" )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPNAME( 0x10, 0x10, "IN2-10" )        // output to 0x3a00 bit 3
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPNAME( 0x20, 0x20, "IN2-20" )        // output to 0x3880 bit 3, 0x3a00 bit 1
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPNAME( 0x40, 0x40, "Percentage 08" )     // payout percentage
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )   // 78
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )  // 70
	PORT_DIPNAME( 0x80, 0x80, "IN2-80" )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )  PORT_NAME("Start")
	PORT_DIPNAME( 0x02, 0x02, "IN3-02" )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )  PORT_NAME("50p")
	PORT_DIPNAME( 0x08, 0x08, "IN3-04" )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2     ) PORT_NAME("Token")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN3     ) PORT_NAME("Cash")
	PORT_DIPNAME( 0x40, 0x40, "Percentage 16" )     // payout percentage
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )   // 86
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )  // 70
	PORT_DIPNAME( 0x80, 0x80, "IN3-80" )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
INPUT_PORTS_END


//**************************************************************************
//  VIDEO EMULATION
//**************************************************************************

void summit_state::vram_w(offs_t offset, uint8_t data)
{
	if (offset < 0x20)
		m_tilemap->set_scrollx(offset, data);

	m_vram[offset] = data;
}

uint32_t summit_state::screen_update_summit(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_tilemap->mark_all_dirty();
	m_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}

TILE_GET_INFO_MEMBER(summit_state::tile_info)
{
	// attribute bit 0 used for both tile index and color?

	uint8_t attr = m_attr[(tile_index)];
	uint16_t tile = (BIT(attr, 0) << 8) | m_vram[tile_index];

	tileinfo.set(0, tile, attr & 0x0f, 0);
}

static const gfx_layout tiles8x8_layout =
{
	8, 8,
	RGN_FRAC(1,2),
	2,
	{ 0, RGN_FRAC(1,2) },
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( gfx_summit )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8_layout, 0, 16 )
GFXDECODE_END

static constexpr rgb_t summit_pens[64] =
{
	{ 0x00, 0x00, 0x00 }, // 00: black
	{ 0x00, 0x7f, 0x00 }, // 01: dark green
	{ 0xff, 0xff, 0xff }, // 02: white
	{ 0xff, 0x00, 0x00 }, // 03: red
	{ 0xff, 0x00, 0xff }, // 04: ?
	{ 0xff, 0x00, 0xff }, // 05: ?
	{ 0xff, 0x00, 0xff }, // 06: ?
	{ 0xff, 0x00, 0xff }, // 07: ?
	{ 0x00, 0x00, 0x00 }, // 08: black
	{ 0x7f, 0x00, 0x00 }, // 09: brown
	{ 0xff, 0xff, 0xff }, // 0a: white
	{ 0xff, 0x00, 0x00 }, // 0b: red
	{ 0x00, 0x00, 0x00 }, // 0c: black
	{ 0xff, 0x00, 0xff }, // 0d: ?
	{ 0xff, 0x00, 0x00 }, // 0e: red
	{ 0xff, 0x00, 0xff }, // 0f: ?
	{ 0x00, 0x00, 0x00 }, // 10: black
	{ 0xff, 0x7f, 0x00 }, // 11: orange
	{ 0xff, 0xff, 0xff }, // 12: white
	{ 0xff, 0xff, 0x7f }, // 13: light orange
	{ 0xff, 0x00, 0xff }, // 14: ?
	{ 0xff, 0x00, 0xff }, // 15: ?
	{ 0xff, 0x00, 0xff }, // 16: ?
	{ 0xff, 0x00, 0xff }, // 17: ?
	{ 0x00, 0x00, 0x00 }, // 18: black
	{ 0xff, 0x00, 0x00 }, // 19: red
	{ 0xff, 0xff, 0xff }, // 1a: white
	{ 0x00, 0x00, 0xff }, // 1b: blue
	{ 0x00, 0x00, 0x00 }, // 1c: black
	{ 0xff, 0x00, 0xff }, // 1d: ?
	{ 0x00, 0x00, 0xff }, // 1e: blue
	{ 0xff, 0x00, 0xff }, // 1f: ?
	{ 0x00, 0x00, 0x00 }, // 20: black
	{ 0xff, 0x00, 0x00 }, // 21: red
	{ 0xff, 0xff, 0x00 }, // 22: yellow
	{ 0x00, 0x00, 0xff }, // 23: blue
	{ 0xff, 0x00, 0xff }, // 24: ?
	{ 0xff, 0x00, 0xff }, // 25: ?
	{ 0xff, 0x00, 0xff }, // 26: ?
	{ 0xff, 0x00, 0xff }, // 27: ?
	{ 0x00, 0x00, 0x00 }, // 28: black
	{ 0x00, 0x00, 0xff }, // 29: blue
	{ 0xff, 0xff, 0xff }, // 2a: white
	{ 0xff, 0xff, 0x00 }, // 2b: yellow
	{ 0x00, 0x00, 0x00 }, // 2c: black
	{ 0x00, 0x00, 0x00 }, // 2d: black
	{ 0xff, 0xff, 0xff }, // 2e: white
	{ 0xff, 0xff, 0xff }, // 2f: white
	{ 0x00, 0x00, 0x00 }, // 30: black
	{ 0x00, 0x00, 0xff }, // 31: blue
	{ 0xff, 0xff, 0xff }, // 32: white
	{ 0x7f, 0x00, 0x00 }, // 33: brown
	{ 0xff, 0x00, 0xff }, // 34: ?
	{ 0xff, 0x00, 0xff }, // 35: ?
	{ 0xff, 0x00, 0xff }, // 36: ?
	{ 0xff, 0x00, 0xff }, // 37: ?
	{ 0x00, 0x00, 0x00 }, // 38: black
	{ 0xff, 0x00, 0x00 }, // 39: red
	{ 0x7f, 0x00, 0x00 }, // 3a: brown
	{ 0x00, 0x7f, 0x00 }, // 3b: dark green
	{ 0x00, 0x00, 0x00 }, // 3c: black
	{ 0xff, 0x00, 0xff }, // 3d: ?
	{ 0x00, 0x7f, 0x00 }, // 3e: dark green
	{ 0xff, 0x00, 0xff }  // 3f: ?
};

void summit_state::summit_palette(palette_device &palette) const
{
	palette.set_pen_colors(0, summit_pens);
}


//**************************************************************************
//  AUDIO EMULATION
//**************************************************************************

void summit_state::sound_w(uint8_t data)
{
	// likely connected to some kind of discrete audio hardware

	// 0x00 mute?
	// 0x14 reel bump
	// 0x42 go sound
	// 0x48 gamble/collect win
	// 0x80 mute?
}


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void summit_state::out2_w(uint8_t data)
{
	if (0)
		logerror("out2: %02x  %d %d %d %d  %d %d %d %d\n", data, BIT(data, 7), BIT(data, 6), BIT(data, 5), BIT(data, 4), BIT(data, 3), BIT(data, 2), BIT(data, 1), BIT(data, 0));
}

void summit_state::out3_w(uint8_t data)
{
	// 765-----  not used?
	// ---4----  coin lockout?
	// ----3---  cash in
	// -----2--  10p out
	// ------1-  toggles rapidly
	// -------0  unknown

//  machine().bookkeeping().coin_lockout_w(0, BIT(data, 4));
	machine().bookkeeping().coin_counter_w(0, BIT(data, 3));
	machine().bookkeeping().coin_counter_w(1, BIT(data, 2));
}

void summit_state::lamps1_w(uint8_t data)
{
	// 76------  not used?
	// --5-----  collect/win lamp
	// ---4----  collect/win lamp
	// ----3---  cancel lamp
	// -----2--  hold 3 lamp
	// ------1-  hold 2 lamp
	// -------0  hold 1 lamp

	for (int i  = 0; i < 8; i++)
		m_lamps[i] = BIT(data, i);
}

void summit_state::lamps2_w(uint8_t data)
{
	// 765-----  not used?
	// ---4----  gamble lamp
	// ----3---  active when IN2-10 active
	// -----2--  go lamp
	// ------1-  active when IN2-20 active
	// -------0  start lamp

	for (int i  = 0; i < 8; i++)
		m_lamps[8 + i] = BIT(data, i);
}

void summit_state::machine_start()
{
	m_lamps.resolve();

	m_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(summit_state::tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_tilemap->set_scroll_rows(32);
}


//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

void summit_state::summit(machine_config &config)
{
	Z80(config, m_maincpu, 4000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &summit_state::mainmap);
	m_maincpu->set_vblank_int("screen", FUNC(summit_state::irq0_line_hold));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(256, 256);
	screen.set_visarea(0, 256-1, 16, 256-16-1);
	screen.set_screen_update(FUNC(summit_state::screen_update_summit));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_summit);

	PALETTE(config, m_palette, FUNC(summit_state::summit_palette), 64);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	config.set_default_layout(layout_summit);
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( pushover )
	ROM_REGION( 0x1800, "maincpu", 0 )
	ROM_LOAD( "po40.bin", 0x0000, 0x0800, CRC(eb612cf7) SHA1(93d6b4d5a3c275cc27bece6f1fb1e8be7c6997bf) )
	ROM_LOAD( "po41.bin", 0x0800, 0x0800, CRC(268fd1b2) SHA1(a2421807cb38901ec7c24a8c6eecb34677235c21) )
	ROM_LOAD( "po42.bin", 0x1000, 0x0800, CRC(c0bb3f19) SHA1(8e773cc9503755a24428f9b0bbad1721a38e2c54) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "pu5.bin", 0x0000, 0x0800, CRC(79c07fbd) SHA1(9c36fa5d369dc5a177501ec302ced1ebaba1102a) )
	ROM_LOAD( "pu6.bin", 0x0800, 0x0800, CRC(3049e8b2) SHA1(becb09c8d3a25e5e2104aa46f8142e1954b1fede) )
	ROM_LOAD( "pu7.bin", 0x1000, 0x0800, CRC(d84fe540) SHA1(de7e66bc2f1d82a2db6a48525ed1802ec20cea44) )
	ROM_LOAD( "pu8.bin", 0x1800, 0x0800, CRC(e7da3be6) SHA1(b02aceef84860cdc9190b73ffe3fa8173e210682) )

	ROM_REGION( 0x1000, "proms", 0 )
	ROM_LOAD( "prom1.bin", 0x000, 0x400, CRC(2d947698) SHA1(85405a005fa6108e8d5ef5a7a12745ba05109661) )
	ROM_LOAD( "prom2.bin", 0x400, 0x400, CRC(87e11191) SHA1(59da09621f1a3c827bb0c158295ac1744a960aa4) )
	ROM_LOAD( "prom3.bin", 0x800, 0x400, CRC(fe0fb97a) SHA1(05c41988bd1fe18725023a1a67c1b3cae01b2028) )
	ROM_LOAD( "prom4.bin", 0xc00, 0x400, CRC(ce26b2c4) SHA1(a44b7a05c0c72592fe6083244c785e9ed1149909) )
ROM_END

} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME       PARENT  MACHINE  INPUT   CLASS         INIT        ROTATION  COMPANY        FULLNAME                   FLAGS
GAME( 1981, pushover,  0,      summit,  summit, summit_state, empty_init, ROT270,   "Summit Coin", "Push-Over (Summit Coin)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IMPERFECT_COLORS )
