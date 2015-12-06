// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli
/*

Dr. Tomy    -   (c) 1993 Playmark

A rip-off of Dr. Mario by Playmark, using some code from Gaelco's Big Karnak on
similar hardware.

*/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "sound/okim6295.h"

class drtomy_state : public driver_device
{
public:
	drtomy_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram_fg(*this, "videorafg"),
		m_videoram_bg(*this, "videorabg"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_oki(*this, "oki"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<UINT16> m_videoram_fg;
	required_shared_ptr<UINT16> m_videoram_bg;
	required_shared_ptr<UINT16> m_spriteram;

	/* video-related */
	tilemap_t   *m_tilemap_bg;
	tilemap_t   *m_tilemap_fg;

	/* misc */
	int       m_oki_bank;
	DECLARE_WRITE16_MEMBER(drtomy_vram_fg_w);
	DECLARE_WRITE16_MEMBER(drtomy_vram_bg_w);
	DECLARE_WRITE16_MEMBER(drtomy_okibank_w);
	TILE_GET_INFO_MEMBER(get_tile_info_fg);
	TILE_GET_INFO_MEMBER(get_tile_info_bg);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	UINT32 screen_update_drtomy(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	required_device<cpu_device> m_maincpu;
	required_device<okim6295_device> m_oki;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};



TILE_GET_INFO_MEMBER(drtomy_state::get_tile_info_fg)
{
	int code  = m_videoram_fg[tile_index] & 0xfff;
	int color = (m_videoram_fg[tile_index] & 0xf000) >> 12;
	SET_TILE_INFO_MEMBER(2, code, color, 0);
}


TILE_GET_INFO_MEMBER(drtomy_state::get_tile_info_bg)
{
	int code  = m_videoram_bg[tile_index] & 0xfff;
	int color = (m_videoram_bg[tile_index] & 0xf000) >> 12;
	SET_TILE_INFO_MEMBER(1, code, color, 0);
}


/*
    Sprite Format (almost like gaelco.c)
    -------------

    Word | Bit(s)            | Description
    -----+-FEDCBA98-76543210-+--------------------------
      0  | -------- xxxxxxxx | y position
      0  | ----x--- -------- | sprite size
      0  | -x------ -------- | flipx
      0  | x------- -------- | flipy
      1  | xxxxxxxx xxxxxxxx | not used
      2  | -------x xxxxxxxx | x position
      2  | ---xxxx- -------- | sprite color
      3  | -------- ------xx | sprite code (8x8 cuadrant)
      3  | xxxxxxxx xxxxxx-- | sprite code
*/

void drtomy_state::draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	int i, x, y, ex, ey;
	gfx_element *gfx = m_gfxdecode->gfx(0);

	static const int x_offset[2] = {0x0, 0x2};
	static const int y_offset[2] = {0x0, 0x1};

	for (i = 3; i < 0x1000 / 2; i += 4)
	{
		int sx = m_spriteram[i + 2] & 0x01ff;
		int sy = (240 - (m_spriteram[i] & 0x00ff)) & 0x00ff;
		int number = m_spriteram[i + 3];
		int color = (m_spriteram[i + 2] & 0x1e00) >> 9;
		int attr = (m_spriteram[i] & 0xfe00) >> 9;

		int xflip = attr & 0x20;
		int yflip = attr & 0x40;
		int spr_size;

		if (attr & 0x04)
			spr_size = 1;
		else
		{
			spr_size = 2;
			number &= (~3);
		}

		for (y = 0; y < spr_size; y++)
		{
			for (x = 0; x < spr_size; x++)
			{
				ex = xflip ? (spr_size - 1 - x) : x;
				ey = yflip ? (spr_size - 1 - y) : y;

				gfx->transpen(bitmap,cliprect,number + x_offset[ex] + y_offset[ey],
						color,xflip,yflip,
						sx-0x09+x*8,sy+y*8,0);
			}
		}
	}
}

void drtomy_state::video_start()
{
	m_tilemap_bg = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(drtomy_state::get_tile_info_bg),this), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_tilemap_fg = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(drtomy_state::get_tile_info_fg),this), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);

	m_tilemap_fg->set_transparent_pen(0);
}

UINT32 drtomy_state::screen_update_drtomy(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_tilemap_bg->draw(screen, bitmap, cliprect, 0, 0);
	m_tilemap_fg->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	return 0;
}

WRITE16_MEMBER(drtomy_state::drtomy_vram_fg_w)
{
	COMBINE_DATA(&m_videoram_fg[offset]);
	m_tilemap_fg->mark_tile_dirty(offset);
}

WRITE16_MEMBER(drtomy_state::drtomy_vram_bg_w)
{
	COMBINE_DATA(&m_videoram_bg[offset]);
	m_tilemap_bg->mark_tile_dirty(offset);
}

WRITE16_MEMBER(drtomy_state::drtomy_okibank_w)
{
	if (m_oki_bank != (data & 3))
	{
		m_oki_bank = data & 3;
		m_oki->set_bank_base(m_oki_bank * 0x40000);
	}

	/* unknown bit 2 -> (data & 4) */
}

static ADDRESS_MAP_START( drtomy_map, AS_PROGRAM, 16, drtomy_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM /* ROM */
	AM_RANGE(0x100000, 0x100fff) AM_RAM_WRITE(drtomy_vram_fg_w) AM_SHARE("videorafg")   /* Video RAM FG */
	AM_RANGE(0x101000, 0x101fff) AM_RAM_WRITE(drtomy_vram_bg_w) AM_SHARE("videorabg") /* Video RAM BG */
	AM_RANGE(0x200000, 0x2007ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x440000, 0x440fff) AM_RAM AM_SHARE("spriteram") /* Sprite RAM */
	AM_RANGE(0x700000, 0x700001) AM_READ_PORT("DSW1")
	AM_RANGE(0x700002, 0x700003) AM_READ_PORT("DSW2")
	AM_RANGE(0x700004, 0x700005) AM_READ_PORT("P1")
	AM_RANGE(0x700006, 0x700007) AM_READ_PORT("P2")
	AM_RANGE(0x70000c, 0x70000d) AM_WRITE(drtomy_okibank_w) /* OKI banking */
	AM_RANGE(0x70000e, 0x70000f) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0x00ff) /* OKI 6295*/
	AM_RANGE(0xffc000, 0xffffff) AM_RAM /* Work RAM */
ADDRESS_MAP_END

static const gfx_layout tilelayout8=
{
	8,8,                                    /* 8x8 tiles */
	RGN_FRAC(1,4),                          /* number of tiles */
	4,                                      /* bitplanes */
	{ RGN_FRAC(0,4), RGN_FRAC(1,4), RGN_FRAC(2,4), RGN_FRAC(3,4) }, /* plane offsets */
	{ 0,1,2,3,4,5,6,7 },
	{ 0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8 },
	8*8
};

static const gfx_layout tilelayout16 =
{
	16,16,                                  /* 16x16 tiles */
	RGN_FRAC(1,4),                          /* number of tiles */
	4,                                      /* bitplanes */
	{ RGN_FRAC(0,4), RGN_FRAC(1,4), RGN_FRAC(2,4), RGN_FRAC(3,4) }, /* plane offsets */
	{ 0,1,2,3,4,5,6,7, 16*8+0,16*8+1,16*8+2,16*8+3,16*8+4,16*8+5,16*8+6,16*8+7 },
	{ 0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8, 8*8,9*8,10*8,11*8,12*8,13*8,14*8,15*8 },
	32*8
};

static GFXDECODE_START( drtomy )
	GFXDECODE_ENTRY( "gfx1", 0, tilelayout8,  0x100, 16 ) /* Sprites */
	GFXDECODE_ENTRY( "gfx1", 0, tilelayout16, 0x000, 16 ) /* BG */
	GFXDECODE_ENTRY( "gfx1", 0, tilelayout16, 0x200, 16 ) /* FG */
GFXDECODE_END


static INPUT_PORTS_START( drtomy )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW1:8,7,6,5")
	PORT_DIPSETTING(    0x0a, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x00, "5 Coins/4 Credits" )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW1:4,3,2,1")
	PORT_DIPSETTING(    0xa0, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x00, "5 Coins/4 Credits" )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "Time" )          PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x00, "Less" )
	PORT_DIPSETTING(    0x01, "More" )
	PORT_DIPNAME( 0x02, 0x02, "Number of Virus" )       PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x02, "Less" )
	PORT_DIPSETTING(    0x00, "More" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x08, IP_ACTIVE_LOW, "SW2:5" )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Language ) )     PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x20, DEF_STR( English ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Italian ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )
INPUT_PORTS_END


void drtomy_state::machine_start()
{
	save_item(NAME(m_oki_bank));
}

void drtomy_state::machine_reset()
{
	m_oki_bank = 0;
}

static MACHINE_CONFIG_START( drtomy, drtomy_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000,24000000/2)          /* ? MHz */
	MCFG_CPU_PROGRAM_MAP(drtomy_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", drtomy_state,  irq6_line_hold)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(32*16, 32*16)
	MCFG_SCREEN_VISIBLE_AREA(0, 320-1, 16, 256-1)
	MCFG_SCREEN_UPDATE_DRIVER(drtomy_state, screen_update_drtomy)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", drtomy)
	MCFG_PALETTE_ADD("palette", 1024)
	MCFG_PALETTE_FORMAT(xRRRRRGGGGGBBBBB)


	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_OKIM6295_ADD("oki", 26000000/16, OKIM6295_PIN7_LOW)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.8)
MACHINE_CONFIG_END


ROM_START( drtomy )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "15.u21", 0x00001, 0x20000, CRC(0b8d763b) SHA1(082005985a2de7b941ea227bbf6e761a197132e6) )
	ROM_LOAD16_BYTE( "16.u22", 0x00000, 0x20000, CRC(206f4d65) SHA1(f4a28bc6041981d50a03477e63e90d5ff8ffb765) )

	ROM_REGION( 0x400000, "gfx1", 0 )
	ROM_LOAD( "20.u80", 0x000000, 0x40000, CRC(4d4d86ff) SHA1(60df0bf8ba62fea42ff756cd7c5485b57f597098) )
	ROM_LOAD( "19.u81", 0x100000, 0x40000, CRC(49ecbfe2) SHA1(16889663bdd3b7d0a350d5b18e221480413f6b4f) )
	ROM_LOAD( "18.u82", 0x200000, 0x40000, CRC(8ee5c921) SHA1(6ba43eeb3b633c3db22f7b18b8fe91f250da2242) )
	ROM_LOAD( "17.u83", 0x300000, 0x40000, CRC(42044b1c) SHA1(6fd01911932e0fb800ffefec595a9e7c524faa8f) )

	ROM_REGION( 0x80000, "user1", 0 ) /* OKIM6295 samples */
	ROM_LOAD( "14.u23", 0x00000, 0x80000, CRC(479614ec) SHA1(b6300b344422f0a64146b853411f5285eaaada28) )

	ROM_REGION( 0x100000, "oki", 0 )
	ROM_COPY( "user1", nullptr, 0x00000, 0x20000)
	ROM_COPY( "user1", nullptr, 0x20000, 0x20000)
	ROM_COPY( "user1", nullptr, 0x40000, 0x20000)
	ROM_COPY( "user1", 0x20000, 0x60000, 0x20000)
	ROM_COPY( "user1", nullptr, 0x80000, 0x20000)
	ROM_COPY( "user1", 0x40000, 0xa0000, 0x20000)
	ROM_COPY( "user1", nullptr, 0xc0000, 0x20000)
	ROM_COPY( "user1", 0x60000, 0xe0000, 0x20000)
ROM_END


GAME( 1993, drtomy, 0, drtomy, drtomy, driver_device, 0, ROT0, "Playmark", "Dr. Tomy", MACHINE_SUPPORTS_SAVE )
