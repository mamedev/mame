// license:BSD-3-Clause
// copyright-holders:David Haywood

// the hardware is quite galaxian-like (background + sprites + bullets)
// but tiles are 3bpp, not 2bpp, and there is extra sound hardware
// 
// TODO:
// - sound
// - bullets sometimes get stuck on screen (and need verifying)
// - sometimes killed by something invisible?
// - remaining dipswitches

#include "emu.h"

#include "galaxian.h"

#include "speaker.h"

#include "cpu/z80/z80.h"

namespace {

class sega119_state : public galaxian_state
{
public:
	sega119_state(const machine_config &mconfig, device_type type, const char *tag) :
		galaxian_state(mconfig, type, tag)
	{ }

	void sega119(machine_config &config);

	void init_119();

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void extend_sprite_info(const u8 *base, u8 *sx, u8 *sy, u8 *flipx, u8 *flipy, u16 *code, u8 *color);
	void extend_tile_info(u16 *code, u8 *color, u8 attrib, u8 x, u8 y);

	void tilebanks_flipscreen_w(u8 data);

	void prg_map(address_map &map) ATTR_COLD;

	u8 m_bankdata;
};

void sega119_state::prg_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x83ff).mirror(0x0400).ram().w(FUNC(galaxian_state::galaxian_videoram_w)).share("videoram");
	map(0x9000, 0x90ff).mirror(0x0700).ram().w(FUNC(galaxian_state::galaxian_objram_w)).share("spriteram");

	map(0xa000, 0xa000).nopw(); // soundlatch?

	map(0xb000, 0xb000).portr("IN0").w(FUNC(sega119_state::tilebanks_flipscreen_w));
	map(0xb001, 0xb001).portr("IN1");
	map(0xb002, 0xb002).portr("DSW1");
	map(0xb003, 0xb003).portr("DSW2");

	map(0xe000, 0xefff).ram();
}

void sega119_state::tilebanks_flipscreen_w(u8 data)
{
	// ---- bbff
	// bb = sprite and tile banks, uncertain which is which
	// ff = flipscreen (could be separate sprite/tile flip, or separate x/y flip, we treat it as the latter)

	if (data & 0xf0)
		popmessage("%02x", data);

	m_bankdata = data;
	m_bg_tilemap->mark_all_dirty();

	m_flipscreen_x = data & 0x01;
	m_flipscreen_y = data & 0x02;
	m_bg_tilemap->set_flip((m_flipscreen_x ? TILEMAP_FLIPX : 0) | (m_flipscreen_y ? TILEMAP_FLIPY : 0));

}

static INPUT_PORTS_START( sega119 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SERVICE1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank)) // game speed is not driven by interrupts, polls bit in port

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "DSW1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x0c, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x00, "Free" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x05, "5" )
	PORT_DIPSETTING(    0x06, "6" )
	PORT_DIPSETTING(    0x07, "7" )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x08, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x18, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x28, "5" )
	PORT_DIPSETTING(    0x30, "6" )
	PORT_DIPSETTING(    0x38, "7" )
	PORT_DIPNAME( 0x40, 0x40, "DSW2" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(0,3), RGN_FRAC(1,3), RGN_FRAC(2,3) },
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8*8
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(0,3), RGN_FRAC(1,3), RGN_FRAC(2,3) },
	{ STEP8(0,1), STEP8(8*8,1) },
	{ STEP8(0,8), STEP8(16*8,8) },
	16*16
};

static GFXDECODE_START( gfx_sega119 )
	GFXDECODE_SCALE( "tiles", 0, charlayout, 0, 4, GALAXIAN_XSCALE, 1)
	GFXDECODE_SCALE( "tiles", 0, spritelayout, 0, 4, GALAXIAN_XSCALE, 1)
GFXDECODE_END

void sega119_state::machine_start()
{
	galaxian_state::machine_start();
	m_bankdata = 0;
	save_item(NAME(m_bankdata));
}

void sega119_state::extend_sprite_info(const u8 *base, u8 *sx, u8 *sy, u8 *flipx, u8 *flipy, u16 *code, u8 *color)
{
	*code += 0x40;
	if (m_bankdata & 0x04)
		*code += 0x80;
}

void sega119_state::extend_tile_info(u16 *code, u8 *color, u8 attrib, u8 x, u8 y)
{
	// might not be correct, see level 3
	if (m_bankdata & 0x08)
		*code |= 0x200;
}

void sega119_state::sega119(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, GALAXIAN_PIXEL_CLOCK/3/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &sega119_state::prg_map);
	// nmi is unused (just returns) timing is done by polling vblank
	
	//WATCHDOG_TIMER(config, "watchdog").set_vblank_count("screen", 8); // does it exist on this hardware?

	// video hardware
	GFXDECODE(config, m_gfxdecode, m_palette, gfx_sega119);
	PALETTE(config, m_palette, FUNC(sega119_state::galaxian_palette), 32);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(GALAXIAN_PIXEL_CLOCK, GALAXIAN_HTOTAL, GALAXIAN_HBEND, GALAXIAN_HBSTART, GALAXIAN_VTOTAL, GALAXIAN_VBEND, GALAXIAN_VBSTART);
	m_screen->set_screen_update(FUNC(sega119_state::screen_update_galaxian));
	m_screen->screen_vblank().set(FUNC(sega119_state::vblank_interrupt_w));

	// sound hardware
	SPEAKER(config, "speaker").front_center();

	set_left_sprite_clip(0);
	set_right_sprite_clip(0);
}

void sega119_state::init_119()
{
	common_init(&galaxian_state::galaxian_draw_bullet, &galaxian_state::galaxian_draw_background, nullptr, nullptr);

	m_extend_sprite_info_ptr = extend_sprite_info_delegate(&sega119_state::extend_sprite_info, this);
	m_extend_tile_info_ptr = extend_tile_info_delegate(&sega119_state::extend_tile_info, this);
}

ROM_START( sega119 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "119_4.bin",   0x0000, 0x2000, CRC(b614229e) SHA1(06d0b17f5ff12222c74ff9325c21268bef25446e) )
	ROM_LOAD( "119_3.bin",   0x2000, 0x2000, CRC(d2a984bf) SHA1(d2d5a83deff894978394461f8779d296b855971f) )
	ROM_LOAD( "119_2.bin",   0x4000, 0x2000, CRC(d96611bc) SHA1(fffb516f00e747931941844b5358fe46d656bfb8) )
	ROM_LOAD( "119_1.bin",   0x6000, 0x2000, CRC(368723e2) SHA1(515724eed41138e2e852f53d63f9a226584126f5) )

	ROM_REGION( 0x6000, "tiles", 0 )
	ROM_LOAD( "119_5.bin",   0x0000, 0x2000, CRC(1b08c881) SHA1(b372d614ec41cff49d6ff1c2256170c15069bd55) )
	ROM_LOAD( "119_6.bin",   0x4000, 0x2000, CRC(1a7490a4) SHA1(e74141b04ffb63e5cc434fbce89ac0c51e79330f) )
	ROM_LOAD( "119_7.bin",   0x2000, 0x2000, CRC(fcff7f59) SHA1(87a4668ef0c28091c895b0aeae4d4c486396e549) )

	ROM_REGION( 0x6000, "audiocpu", 0 ) // another z80
	ROM_LOAD( "119_8.bin",   0x0000, 0x2000, CRC(6570149c) SHA1(b139edbe7bd2f965804b0c850f87e2ef8e418256) )

	ROM_REGION( 0x6000, "samples", 0 ) // samples for MC1408P8
	ROM_LOAD( "119_9.bin",   0x0000, 0x2000, BAD_DUMP CRC(b917e2c2) SHA1(8acd598b898204e18a4cfccc40720d149f401b42) ) //  FIXED BITS (xxxx1xxx) (but always reads the same?)

	ROM_REGION( 0x20, "proms", 0 )
	ROM_LOAD( "119_6331.bin",   0x00, 0x20, CRC(b73e79f3) SHA1(8345d45699c51a90c1d2743623b923531a577993) )

	ROM_REGION( 0x20, "proms2", 0 )
	ROM_LOAD( "119_7502.bin",   0x00, 0x20, CRC(52bdbe39) SHA1(e6f126e22944b698bea599760a79bd5cfa8f0d1f) ) // ?? hopefully just video timing, not a bad read
ROM_END

} // anonymous namespace

// all tiles are upside down in ROM, but handled by flipscreen
GAME( 1986, sega119, 0, sega119, sega119, sega119_state, init_119, ROT0, "Sega / Coreland", "119", MACHINE_NOT_WORKING )
