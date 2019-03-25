// license:BSD-3-Clause
// copyright-holders:David Haywood, Pierpaolo Prazzoli
/*

Calorie-Kun
Sega, 1986

PCB Layout
----------

Top PCB


837-6077  171-5381
|--------------------------------------------|
|                    Z80             YM2149  |
|                                            |
|                    2016            YM2149  |
|                                            |
|10079    10077      10075           YM2149  |
|    10078    10076                          |
|                                           -|
|                                          |
|                    2114                   -|
|                                           1|
|                    2114                   8|
|                                           W|
|                    2114                   A|
|                                           Y|
|                                           -|
|                                          |
|                                    DSW2(8)-|
|           10082   10081   10080            |
|2016                                DSW1(8) |
|--------------------------------------------|

Notes:
      Z80 clock   : 3.000MHz
      YM2149 clock: 1.500MHz
      VSync       : 60Hz
      2016        : 2K x8 SRAM
      2114        : 1K x4 SRAM



Bottom PCB


837-6076  171-5380
|--------------------------------------------|
|                            12MHz           |
|                                            |
|                                            |
|                                            |
|                                            |
|                                            |
|                                            |
| 10074                               10071  |
|               NEC                          |
| 10073      D317-0004   4MHz         10070  |
|                                            |
| 10072                               10069  |
|                                            |
| 2016         2114      6148                |
|                                            |
| 2016         2114      6148                |
|                                            |
|                        6148                |
|                        6148                |
|--------------------------------------------|
Notes:
      317-0004 clock: 4.000MHz
      2016          : 2K x8 SRAM
      2114          : 1K x4 SRAM
      6148          : 1K x4 SRAM


 driver by David Haywood and Pierpaolo Prazzoli

*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/segacrp2_device.h"
#include "sound/ay8910.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


class calorie_state : public driver_device
{
public:
	calorie_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_fg_ram(*this, "fg_ram"),
		m_sprites(*this, "sprites"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_decrypted_opcodes(*this, "decrypted_opcodes")
	{ }

	/* memory pointers */
	required_shared_ptr<uint8_t> m_fg_ram;
	required_shared_ptr<uint8_t> m_sprites;

	/* video-related */
	tilemap_t  *m_bg_tilemap;
	tilemap_t  *m_fg_tilemap;
	uint8_t    m_bg_bank;
	DECLARE_WRITE8_MEMBER(fg_ram_w);
	DECLARE_WRITE8_MEMBER(bg_bank_w);
	DECLARE_WRITE8_MEMBER(calorie_flipscreen_w);
	DECLARE_READ8_MEMBER(calorie_soundlatch_r);
	DECLARE_WRITE8_MEMBER(bogus_w);
	void init_calorieb();
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_calorie(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;
	optional_shared_ptr<uint8_t> m_decrypted_opcodes;
	void caloriee(machine_config &config);
	void calorie(machine_config &config);
	void calorie_map(address_map &map);
	void calorie_sound_io_map(address_map &map);
	void calorie_sound_map(address_map &map);
	void decrypted_opcodes_map(address_map &map);
};


/*************************************
 *
 *  Video emulation
 *
 *************************************/

TILE_GET_INFO_MEMBER(calorie_state::get_bg_tile_info)
{
	uint8_t *src = memregion("user1")->base();
	int bg_base = (m_bg_bank & 0x0f) * 0x200;
	int code  = src[bg_base + tile_index] | (((src[bg_base + tile_index + 0x100]) & 0x10) << 4);
	int color = src[bg_base + tile_index + 0x100] & 0x0f;
	int flag  = src[bg_base + tile_index + 0x100] & 0x40 ? TILE_FLIPX : 0;

	SET_TILE_INFO_MEMBER(1, code, color, flag);
}

TILE_GET_INFO_MEMBER(calorie_state::get_fg_tile_info)
{
	int code  = ((m_fg_ram[tile_index + 0x400] & 0x30) << 4) | m_fg_ram[tile_index];
	int color = m_fg_ram[tile_index + 0x400] & 0x0f;

	SET_TILE_INFO_MEMBER(0, code, color, TILE_FLIPYX((m_fg_ram[tile_index + 0x400] & 0xc0) >> 6));
}


void calorie_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(calorie_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS, 16, 16, 16, 16);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(calorie_state::get_fg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_fg_tilemap->set_transparent_pen(0);
}

uint32_t calorie_state::screen_update_calorie(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int x;

	if (m_bg_bank & 0x10)
	{
		m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
		m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	}
	else
	{
		m_fg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
	}


	for (x = 0x400; x >= 0; x -= 4)
	{
		int xpos, ypos, tileno, color, flipx, flipy;

		tileno = m_sprites[x + 0];
		color = m_sprites[x + 1] & 0x0f;
		flipx = m_sprites[x + 1] & 0x40;
		flipy = 0;
		ypos = 0xff - m_sprites[x + 2];
		xpos = m_sprites[x + 3];

		if (flip_screen())
		{
			if (m_sprites[x + 1] & 0x10)
				ypos = 0xff - ypos + 32;
			else
				ypos = 0xff - ypos + 16;

			xpos = 0xff - xpos - 16;
			flipx = !flipx;
			flipy = !flipy;
		}

		if (m_sprites[x + 1] & 0x10)
		{
				/* 32x32 sprites */
			m_gfxdecode->gfx(3)->transpen(bitmap,cliprect, tileno | 0x40, color, flipx, flipy, xpos, ypos - 31, 0);
		}
		else
		{
			/* 16x16 sprites */
			m_gfxdecode->gfx(2)->transpen(bitmap,cliprect, tileno, color, flipx, flipy, xpos, ypos - 15, 0);
		}
	}
	return 0;
}

/*************************************
 *
 *  Memory handlers
 *
 *************************************/

WRITE8_MEMBER(calorie_state::fg_ram_w)
{
	m_fg_ram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset & 0x3ff);
}

WRITE8_MEMBER(calorie_state::bg_bank_w)
{
	if((m_bg_bank & ~0x10) != (data & ~0x10))
		m_bg_tilemap->mark_all_dirty();

	m_bg_bank = data;
}

WRITE8_MEMBER(calorie_state::calorie_flipscreen_w)
{
	flip_screen_set(data & 1);
}

READ8_MEMBER(calorie_state::calorie_soundlatch_r)
{
	uint8_t latch = m_soundlatch->read(space, 0);
	m_soundlatch->clear_w(space, 0, 0);
	return latch;
}

WRITE8_MEMBER(calorie_state::bogus_w)
{
	popmessage("written to 3rd sound chip: data = %02X port = %02X", data, offset);
}

/*************************************
 *
 *  Address maps
 *
 *************************************/

void calorie_state::calorie_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).rom();
	map(0xc000, 0xcfff).ram().share("ram");
	map(0xd000, 0xd7ff).ram().w(FUNC(calorie_state::fg_ram_w)).share("fg_ram");
	map(0xd800, 0xdbff).ram().share("sprites");
	map(0xdc00, 0xdcff).ram().w(m_palette, FUNC(palette_device::write8)).share("palette");
	map(0xde00, 0xde00).w(FUNC(calorie_state::bg_bank_w));
	map(0xf000, 0xf000).portr("P1");
	map(0xf001, 0xf001).portr("P2");
	map(0xf002, 0xf002).portr("SYSTEM");
	map(0xf004, 0xf004).portr("DSW1").w(FUNC(calorie_state::calorie_flipscreen_w));
	map(0xf005, 0xf005).portr("DSW2");
	map(0xf800, 0xf800).w(m_soundlatch, FUNC(generic_latch_8_device::write));
}

void calorie_state::decrypted_opcodes_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().share("decrypted_opcodes");
	map(0x8000, 0xbfff).rom();
	map(0xc000, 0xcfff).ram().share("ram");
}

void calorie_state::calorie_sound_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x8000, 0x87ff).ram();
	map(0xc000, 0xc000).r(FUNC(calorie_state::calorie_soundlatch_r));
}

void calorie_state::calorie_sound_io_map(address_map &map)
{
	map.global_mask(0xff);
	// 3rd ?
	map(0x00, 0xff).w(FUNC(calorie_state::bogus_w));

	map(0x00, 0x01).w("ay1", FUNC(ym2149_device::address_data_w));
	map(0x01, 0x01).r("ay1", FUNC(ym2149_device::data_r));
	map(0x10, 0x11).w("ay2", FUNC(ym2149_device::address_data_w));
	map(0x11, 0x11).r("ay2", FUNC(ym2149_device::data_r));
}


/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( calorie )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_COCKTAIL PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_COCKTAIL PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x0c, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0xc0, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x40, "4" )
	PORT_DIPSETTING(    0x80, "5" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPSETTING(    0x01, "20,000 Only" )
	PORT_DIPSETTING(    0x03, "20,000 and 60,000" ) /* No listed value for 0x02 (Off / On) */
	PORT_DIPNAME( 0x04, 0x00, "Number of Bombs" ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPNAME( 0x08, 0x00, "Difficulty - Mogura Nian" ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x30, 0x00, "Difficulty - Select of Mogura" ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Hardest ) )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW2:7" )
	PORT_DIPNAME( 0x80, 0x00, "Infinite Lives" ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END



/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(0,3),RGN_FRAC(1,3),RGN_FRAC(2,3) },
	{ 0, 1,2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout tiles16x16_layout =
{
	16,16,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(0,3),RGN_FRAC(1,3),RGN_FRAC(2,3) },
	{ 0, 1,2, 3, 4, 5, 6, 7, 8*8+0,8*8+1,8*8+2,8*8+3,8*8+4,8*8+5,8*8+6,8*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
		16*8, 17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8 },
	8*32
};

static const gfx_layout tiles32x32_layout =
{
	32,32,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(0,3),RGN_FRAC(1,3),RGN_FRAC(2,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
			8*8+0, 8*8+1, 8*8+2, 8*8+3, 8*8+4, 8*8+5, 8*8+6, 8*8+7,
			32*8+0, 32*8+1, 32*8+2, 32*8+3, 32*8+4, 32*8+5, 32*8+6, 32*8+7,
			40*8+0, 40*8+1, 40*8+2, 40*8+3, 40*8+4, 40*8+5, 40*8+6, 40*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			16*8, 17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8,
			64*8, 65*8, 66*8, 67*8, 68*8, 69*8, 70*8, 71*8,
			80*8, 81*8, 82*8, 83*8, 84*8, 85*8, 86*8, 87*8 },
	128*8
};

static GFXDECODE_START( gfx_calorie )
	GFXDECODE_ENTRY( "gfx2", 0, tiles8x8_layout,   0, 16 )
	GFXDECODE_ENTRY( "gfx3", 0, tiles16x16_layout, 0, 16 )
	GFXDECODE_ENTRY( "gfx1", 0, tiles16x16_layout, 0, 16 )
	GFXDECODE_ENTRY( "gfx1", 0, tiles32x32_layout, 0, 16 )
GFXDECODE_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

void calorie_state::machine_start()
{
	save_item(NAME(m_bg_bank));
}

void calorie_state::machine_reset()
{
	m_bg_bank = 0;
}


MACHINE_CONFIG_START(calorie_state::calorie)

	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", Z80,4000000)         /* 4 MHz */
	MCFG_DEVICE_PROGRAM_MAP(calorie_map)
	MCFG_DEVICE_OPCODES_MAP(decrypted_opcodes_map)
	MCFG_DEVICE_VBLANK_INT_DRIVER("screen", calorie_state,  irq0_line_hold)

	MCFG_DEVICE_ADD("audiocpu", Z80,3000000)        /* 3 MHz */
	MCFG_DEVICE_PROGRAM_MAP(calorie_sound_map)
	MCFG_DEVICE_IO_MAP(calorie_sound_io_map)
	MCFG_DEVICE_PERIODIC_INT_DRIVER(calorie_state, irq0_line_hold,  64)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(256, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 256-1, 16, 256-16-1)
	MCFG_SCREEN_UPDATE_DRIVER(calorie_state, screen_update_calorie)
	MCFG_SCREEN_PALETTE(m_palette)

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_calorie);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_444, 0x100);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);

	YM2149(config, "ay1", 1500000).add_route(ALL_OUTPUTS, "mono", 0.8);

	YM2149(config, "ay2", 1500000).add_route(ALL_OUTPUTS, "mono", 0.8);

	YM2149(config, "ay3", 1500000).add_route(ALL_OUTPUTS, "mono", 0.8);
MACHINE_CONFIG_END

void calorie_state::caloriee(machine_config &config)
{
	calorie(config);
	sega_317_0004_device &maincpu(SEGA_317_0004(config.replace(), m_maincpu, 4000000));         /* 4 MHz */
	maincpu.set_addrmap(AS_PROGRAM, &calorie_state::calorie_map);
	maincpu.set_addrmap(AS_OPCODES, &calorie_state::decrypted_opcodes_map);
	maincpu.set_vblank_int("screen", FUNC(calorie_state::irq0_line_hold));
	maincpu.set_decrypted_tag(m_decrypted_opcodes);
}

/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( calorie )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr10072.1j", 0x00000, 0x4000, CRC(ade792c1) SHA1(6ea5afb00a87037d502c17adda7e4060d12680d7) )
	ROM_LOAD( "epr10073.1k", 0x04000, 0x4000, CRC(b53e109f) SHA1(a41c5affe917232e7adf40d7c15cff778b197e90) )
	ROM_LOAD( "epr10074.1m", 0x08000, 0x4000, CRC(a08da685) SHA1(69f9cfebc771312dbb1726350c2d9e9e8c46353f) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "epr10075.4d", 0x0000, 0x4000, CRC(ca547036) SHA1(584a65482f2b92a4c08c37560450d6db68a56c7b) )

	ROM_REGION( 0x2000, "user1", 0 ) /* background tilemaps */
	ROM_LOAD( "epr10079.8d", 0x0000, 0x2000, CRC(3c61a42c) SHA1(68ea6b5d2f3c6a9e5308c08dde20424f20021a73) )

	ROM_REGION( 0xc000, "gfx1", 0 ) /* sprites */
	ROM_LOAD( "epr10071.7m", 0x0000, 0x4000, CRC(5f55527a) SHA1(ec1ba8f95ac47a0c783e117ef4af6fe0ab5925b5) )
	ROM_LOAD( "epr10070.7k", 0x4000, 0x4000, CRC(97f35a23) SHA1(869553a334e1b3ba900a8b9c9eaf25fbc6ab31dd) )
	ROM_LOAD( "epr10069.7j", 0x8000, 0x4000, CRC(c0c3deaf) SHA1(8bf2e2146b794a330a079dd080f0586500964b1a) )

	ROM_REGION( 0x6000, "gfx2", 0 ) /* tiles  */
	ROM_LOAD( "epr10082.5r", 0x0000, 0x2000, CRC(5984ea44) SHA1(010011b5b8dfa593c6fc7d2366f8cf82fcc8c978) )
	ROM_LOAD( "epr10081.4r", 0x2000, 0x2000, CRC(e2d45dd8) SHA1(5e11089680b574ea4cbf64510e51b0a945f79174) )
	ROM_LOAD( "epr10080.3r", 0x4000, 0x2000, CRC(42edfcfe) SHA1(feba7b1daffcad24d4c24f55ab5466f8cebf31ad) )

	ROM_REGION( 0xc000, "gfx3", 0 ) /* tiles */
	ROM_LOAD( "epr10078.7d", 0x0000, 0x4000, CRC(5b8eecce) SHA1(e7eee82081939b361edcbb9587b072b4b9a162f9) )
	ROM_LOAD( "epr10077.6d", 0x4000, 0x4000, CRC(01bcb609) SHA1(5d01fa75f214d34483284aaaef985ab92a606505) )
	ROM_LOAD( "epr10076.5d", 0x8000, 0x4000, CRC(b1529782) SHA1(8e0e92aae4c8dd8720414372aa767054cc316a0f) )
ROM_END

ROM_START( calorieb )
	ROM_REGION( 0x1c000, "maincpu", 0 )
	ROM_LOAD( "12.bin",      0x10000, 0x4000, CRC(cf5fa69e) SHA1(520d5652e93a672a1fc147295fbd63b873967885) )
	ROM_CONTINUE(            0x00000, 0x4000 )
	ROM_LOAD( "13.bin",      0x14000, 0x4000, CRC(52e7263f) SHA1(4d684c9e3f08ddb18b0b3b982aba82d3c809a633) )
	ROM_CONTINUE(            0x04000, 0x4000 )
	ROM_LOAD( "epr10074.1m", 0x08000, 0x4000, CRC(a08da685) SHA1(69f9cfebc771312dbb1726350c2d9e9e8c46353f) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "epr10075.4d", 0x0000, 0x4000, CRC(ca547036) SHA1(584a65482f2b92a4c08c37560450d6db68a56c7b) )

	ROM_REGION( 0x2000, "user1", 0 ) /* background tilemaps */
	ROM_LOAD( "epr10079.8d", 0x0000, 0x2000, CRC(3c61a42c) SHA1(68ea6b5d2f3c6a9e5308c08dde20424f20021a73) )

	ROM_REGION( 0xc000, "gfx1", 0 ) /* sprites */
	ROM_LOAD( "epr10071.7m", 0x0000, 0x4000, CRC(5f55527a) SHA1(ec1ba8f95ac47a0c783e117ef4af6fe0ab5925b5) )
	ROM_LOAD( "epr10070.7k", 0x4000, 0x4000, CRC(97f35a23) SHA1(869553a334e1b3ba900a8b9c9eaf25fbc6ab31dd) )
	ROM_LOAD( "epr10069.7j", 0x8000, 0x4000, CRC(c0c3deaf) SHA1(8bf2e2146b794a330a079dd080f0586500964b1a) )

	ROM_REGION( 0x6000, "gfx2", 0 ) /* tiles  */
	ROM_LOAD( "epr10082.5r", 0x0000, 0x2000, CRC(5984ea44) SHA1(010011b5b8dfa593c6fc7d2366f8cf82fcc8c978) )
	ROM_LOAD( "epr10081.4r", 0x2000, 0x2000, CRC(e2d45dd8) SHA1(5e11089680b574ea4cbf64510e51b0a945f79174) )
	ROM_LOAD( "epr10080.3r", 0x4000, 0x2000, CRC(42edfcfe) SHA1(feba7b1daffcad24d4c24f55ab5466f8cebf31ad) )

	ROM_REGION( 0xc000, "gfx3", 0 ) /* tiles */
	ROM_LOAD( "epr10078.7d", 0x0000, 0x4000, CRC(5b8eecce) SHA1(e7eee82081939b361edcbb9587b072b4b9a162f9) )
	ROM_LOAD( "epr10077.6d", 0x4000, 0x4000, CRC(01bcb609) SHA1(5d01fa75f214d34483284aaaef985ab92a606505) )
	ROM_LOAD( "epr10076.5d", 0x8000, 0x4000, CRC(b1529782) SHA1(8e0e92aae4c8dd8720414372aa767054cc316a0f) )
ROM_END


/*************************************
 *
 *  Driver initialization
 *
 *************************************/



void calorie_state::init_calorieb()
{
	memcpy(m_decrypted_opcodes, memregion("maincpu")->base() + 0x10000, 0x8000);
}


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

/* Note: the bootleg is identical to the original once decrypted */
GAME( 1986, calorie,  0,       caloriee,calorie, calorie_state, empty_init,    ROT0, "Sega",    "Calorie Kun vs Moguranian",           MACHINE_SUPPORTS_SAVE )
GAME( 1986, calorieb, calorie, calorie, calorie, calorie_state, init_calorieb, ROT0, "bootleg", "Calorie Kun vs Moguranian (bootleg)", MACHINE_SUPPORTS_SAVE )
