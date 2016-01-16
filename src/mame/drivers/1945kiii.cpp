// license:BSD-3-Clause
// copyright-holders:David Haywood
/*

1945 K-3 driver
---------------

1945K-III
Oriental, 2000

This game is a straight rip-off of Psikyo's Strikers 1945 III.

PCB Layout
----------

ORIENTAL SOFT INC., -OPCX2-
|--------------------------------------------|
|    AD-65   SND-1.SU7            M16M-1.U62 |
|                     PAL                    |
|    AD-65   SND-2.SU4                       |
|                                 M16M-2.U63 |
|                                            |
|                    KM681000                |
|J                   KM681000     6116       |
|A                                           |
|M          62256    |-------|    6116       |
|M          62256    |SPR800E|               |
|A                   |OP-CX1 |    6116  6116 |
|    6116   PRG-1.U51|QFP208 |               |
|                    |-------|    6116  6116 |
|    6116   PRG-2.U52                        |
|                 |-----| |------|           |
|           PAL   |     | |QL2003| M16M-3.U61|
|           PAL   |68000| |PLCC84|           |
|DSW1 DSW2        |-----| |------| PAL       |
|             16MHz        27MHz             |
|--------------------------------------------|
Notes:
     68000 clock : 16.000MHz
    M6295 clocks : 1.000MHz (both), sample rate = 1000000 / 132
           VSync : 60Hz

*/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "sound/okim6295.h"

#define MASTER_CLOCK    XTAL_16MHz


class k3_state : public driver_device
{
public:
	k3_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
			m_oki1(*this, "oki1"),
			m_oki2(*this, "oki2") ,
		m_spriteram_1(*this, "spritera1"),
		m_spriteram_2(*this, "spritera2"),
		m_bgram(*this, "bgram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")  { }

	/* devices */
	required_device<okim6295_device> m_oki1;
	required_device<okim6295_device> m_oki2;
	/* memory pointers */
	required_shared_ptr<UINT16> m_spriteram_1;
	required_shared_ptr<UINT16> m_spriteram_2;
	required_shared_ptr<UINT16> m_bgram;

	/* video-related */
	tilemap_t  *m_bg_tilemap;

	DECLARE_WRITE16_MEMBER(k3_bgram_w);
	DECLARE_WRITE16_MEMBER(k3_scrollx_w);
	DECLARE_WRITE16_MEMBER(k3_scrolly_w);
	DECLARE_WRITE16_MEMBER(k3_soundbanks_w);
	TILE_GET_INFO_MEMBER(get_k3_bg_tile_info);
	virtual void machine_start() override;
	virtual void video_start() override;
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_k3(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};


WRITE16_MEMBER(k3_state::k3_bgram_w)
{
	COMBINE_DATA(&m_bgram[offset]);
	m_bg_tilemap->mark_tile_dirty(offset);
}

TILE_GET_INFO_MEMBER(k3_state::get_k3_bg_tile_info)
{
	int tileno = m_bgram[tile_index];
	SET_TILE_INFO_MEMBER(1, tileno, 0, 0);
}

void k3_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(k3_state::get_k3_bg_tile_info),this), TILEMAP_SCAN_ROWS, 16, 16, 32, 64);
}

void k3_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	gfx_element *gfx = m_gfxdecode->gfx(0);
	UINT16 *source = m_spriteram_1;
	UINT16 *source2 = m_spriteram_2;
	UINT16 *finish = source + 0x1000 / 2;

	while (source < finish)
	{
		int xpos, ypos;
		int tileno;
		xpos = (source[0] & 0xff00) >> 8;
		ypos = (source[0] & 0x00ff) >> 0;
		tileno = (source2[0] & 0x7ffe) >> 1;
		xpos |=  (source2[0] & 0x0001) << 8;
			gfx->transpen(bitmap,cliprect, tileno, 1, 0, 0, xpos, ypos, 0);
			gfx->transpen(bitmap,cliprect, tileno, 1, 0, 0, xpos, ypos - 0x100, 0); // wrap
			gfx->transpen(bitmap,cliprect, tileno, 1, 0, 0, xpos - 0x200, ypos, 0); // wrap
			gfx->transpen(bitmap,cliprect, tileno, 1, 0, 0, xpos - 0x200, ypos - 0x100, 0); // wrap

		source++;
		source2++;
	}
}

UINT32 k3_state::screen_update_k3(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	return 0;
}


WRITE16_MEMBER(k3_state::k3_scrollx_w)
{
	m_bg_tilemap->set_scrollx(0, data);
}

WRITE16_MEMBER(k3_state::k3_scrolly_w)
{
	m_bg_tilemap->set_scrolly(0, data);
}

WRITE16_MEMBER(k3_state::k3_soundbanks_w)
{
	m_oki1->set_bank_base((data & 4) ? 0x40000 : 0);
	m_oki2->set_bank_base((data & 2) ? 0x40000 : 0);
}

static ADDRESS_MAP_START( k3_map, AS_PROGRAM, 16, k3_state )
	AM_RANGE(0x0009ce, 0x0009cf) AM_WRITENOP    // bug in code? (clean up log)
	AM_RANGE(0x0009d2, 0x0009d3) AM_WRITENOP    // bug in code? (clean up log)

	AM_RANGE(0x000000, 0x0fffff) AM_ROM // ROM
	AM_RANGE(0x100000, 0x10ffff) AM_RAM // Main Ram
	AM_RANGE(0x200000, 0x200fff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x240000, 0x240fff) AM_RAM AM_SHARE("spritera1")
	AM_RANGE(0x280000, 0x280fff) AM_RAM AM_SHARE("spritera2")
	AM_RANGE(0x2c0000, 0x2c0fff) AM_RAM_WRITE(k3_bgram_w) AM_SHARE("bgram")
	AM_RANGE(0x340000, 0x340001) AM_WRITE(k3_scrollx_w)
	AM_RANGE(0x380000, 0x380001) AM_WRITE(k3_scrolly_w)
	AM_RANGE(0x3c0000, 0x3c0001) AM_WRITE(k3_soundbanks_w)
	AM_RANGE(0x400000, 0x400001) AM_READ_PORT("INPUTS")
	AM_RANGE(0x440000, 0x440001) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x480000, 0x480001) AM_READ_PORT("DSW")
	AM_RANGE(0x4c0000, 0x4c0001) AM_DEVREADWRITE8("oki2", okim6295_device, read, write, 0xff00)
	AM_RANGE(0x500000, 0x500001) AM_DEVREADWRITE8("oki1", okim6295_device, read, write, 0xff00)
	AM_RANGE(0x8c0000, 0x8cffff) AM_RAM // not used?
ADDRESS_MAP_END

static INPUT_PORTS_START( k3 )
	PORT_START("INPUTS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xfff0, IP_ACTIVE_LOW, IPT_UNKNOWN )  /* Are these used at all? */

	PORT_START("DSW")
	PORT_DIPNAME( 0x007,  0x0007, DEF_STR( Coin_A ) )           PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(      0x0002, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0018, 0x0008, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW1:4,5")
	PORT_DIPSETTING(      0x0000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0060, 0x0060, DEF_STR( Lives ) )            PORT_DIPLOCATION("SW1:6,7")
	PORT_DIPSETTING(      0x0040, "2" )
	PORT_DIPSETTING(      0x0060, "3" )
	PORT_DIPSETTING(      0x0020, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SW1:8" )
	PORT_DIPNAME( 0x0100, 0x0000, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Yes ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x0400, 0x0400, "SW2:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0800, 0x0800, "SW2:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x1000, 0x1000, "SW2:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x2000, 0x2000, "SW2:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x4000, 0x4000, "SW2:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x8000, 0x8000, "SW2:8" )
INPUT_PORTS_END


static const gfx_layout k3_layout =
{
	16,16,
	RGN_FRAC(1,1),
	8,
	{ 0,1,2,3,4,5,6,7 },
	{ 0,8,16,24,32,40,48,56, 64, 72, 80, 88, 96, 104, 112, 120 },
	{ 0*128, 1*128, 2*128, 3*128, 4*128, 5*128, 6*128, 7*128,
		8*128, 9*128,10*128,11*128,12*128,13*128,14*128,15*128 },
	16*128
};

static GFXDECODE_START( 1945kiii )
	GFXDECODE_ENTRY( "gfx1", 0, k3_layout,   0x0, 2  ) /* bg tiles */
	GFXDECODE_ENTRY( "gfx2", 0, k3_layout,   0x0, 2  ) /* bg tiles */
GFXDECODE_END


void k3_state::machine_start()
{
}

static MACHINE_CONFIG_START( k3, k3_state )

	MCFG_CPU_ADD("maincpu", M68000, MASTER_CLOCK)
	MCFG_CPU_PROGRAM_MAP(k3_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", k3_state,  irq4_line_hold)


	MCFG_GFXDECODE_ADD("gfxdecode", "palette", 1945kiii)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 64*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 0*8, 28*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(k3_state, screen_update_k3)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 0x800)
	MCFG_PALETTE_FORMAT(xBBBBBGGGGGRRRRR)

	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_OKIM6295_ADD("oki1", MASTER_CLOCK/16, OKIM6295_PIN7_HIGH)  /* dividers? */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_OKIM6295_ADD("oki2", MASTER_CLOCK/16, OKIM6295_PIN7_HIGH) /* dividers? */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END



ROM_START( 1945kiii )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "prg-1.u51", 0x00001, 0x80000, CRC(6b345f27) SHA1(60867fa0e2ea7ebdd4b8046315ee0c83e5cf0d74) )
	ROM_LOAD16_BYTE( "prg-2.u52", 0x00000, 0x80000, CRC(ce09b98c) SHA1(a06bb712b9cf2249cc535de4055b14a21c68e0c5) )

	ROM_REGION( 0x080000, "oki1", 0 ) /* Samples */
	ROM_LOAD( "snd-2.su4", 0x00000, 0x80000, CRC(47e3952e) SHA1(d56524621a3f11981e4434e02f5fdb7e89fff0b4) )

	ROM_REGION( 0x080000, "oki2", 0 ) /* Samples */
	ROM_LOAD( "snd-1.su7", 0x00000, 0x80000, CRC(bbb7f0ff) SHA1(458cf3a0c2d42110bc2427db675226c6b8d30999) )

	ROM_REGION( 0x400000, "gfx1", 0 ) // sprites
	ROM_LOAD32_WORD( "m16m-1.u62", 0x000000, 0x200000, CRC(0b9a6474) SHA1(6110ecb17d0fef25935986af9a251fc6e88e3993) )
	ROM_LOAD32_WORD( "m16m-2.u63", 0x000002, 0x200000, CRC(368a8c2e) SHA1(4b1f360c4a3a86d922035774b2c712be810ec548) )

	ROM_REGION( 0x200000, "gfx2", 0 ) // bg tiles
	ROM_LOAD( "m16m-3.u61", 0x00000, 0x200000, CRC(32fc80dd) SHA1(bee32493a250e9f21997114bba26b9535b1b636c) )
ROM_END

GAME( 2000, 1945kiii, 0, k3, k3, driver_device, 0, ROT270, "Oriental Soft", "1945k III", MACHINE_SUPPORTS_SAVE )
