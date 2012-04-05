/* Mirage Youjuu Mahjongden

TODO:
-eeprom emulation? Software settings all changes if you toggle the "flip screen" dip-switch

Notes:To enter into Test Mode you need to keep pressed the Mahjong A button at start-up.
*/

/*

Mirage Youjuu Mahjongden
(c)1994 Mitchell

MT4001-2
DEC-22V0
all custom chips are surface scratched, but I believe they are DECO156 and mates.

Sound: M6295x2
OSC  : 28.0000MHz

MBL-00.7A    [2e258b7b]

MBL-01.11A   [895be69a]
MBL-02.12A   [474f6104]

MBL-03.10A   [4a599703]

MBL-04.12K   [b533123d]

MR_00-.2A    [3a53f33d]
MR_01-.3A    [a0b758aa]

*/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "includes/decocrpt.h"
#include "includes/decoprot.h"
#include "video/deco16ic.h"
#include "sound/okim6295.h"
#include "video/bufsprite.h"
#include "video/decospr.h"

// mirage_state was also defined in mess/drivers/mirage.c
class miragemi_state : public driver_device
{
public:
	miragemi_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu"),
		  m_deco_tilegen1(*this, "tilegen1"),
		  m_oki_sfx(*this, "oki_sfx"),
		  m_oki_bgm(*this, "oki_bgm"),
		  m_spriteram(*this, "spriteram") { }

	/* memory pointers */
	UINT16 *  m_pf1_rowscroll;
	UINT16 *  m_pf2_rowscroll;
//  UINT16 *  m_paletteram;    // currently this uses generic palette handling (in decocomn.c)

	/* misc */
	UINT8 m_mux_data;

	/* devices */
	required_device<m68000_device> m_maincpu;
	required_device<deco16ic_device> m_deco_tilegen1;
	required_device<okim6295_device> m_oki_sfx;
	required_device<okim6295_device> m_oki_bgm;
	required_device<buffered_spriteram16_device> m_spriteram;
	DECLARE_WRITE16_MEMBER(mirage_mux_w);
	DECLARE_READ16_MEMBER(mirage_input_r);
	DECLARE_WRITE16_MEMBER(okim1_rombank_w);
	DECLARE_WRITE16_MEMBER(okim0_rombank_w);
};

static VIDEO_START( mirage )
{
	machine.device<decospr_device>("spritegen")->alloc_sprite_bitmap();
}

static SCREEN_UPDATE_RGB32( mirage )
{
	miragemi_state *state = screen.machine().driver_data<miragemi_state>();
	UINT16 flip = deco16ic_pf_control_r(state->m_deco_tilegen1, 0, 0xffff);

	flip_screen_set(screen.machine(), BIT(flip, 7));

	screen.machine().device<decospr_device>("spritegen")->draw_sprites(bitmap, cliprect, state->m_spriteram->buffer(), 0x400);

	deco16ic_pf_update(state->m_deco_tilegen1, state->m_pf1_rowscroll, state->m_pf2_rowscroll);

	bitmap.fill(256, cliprect); /* not verified */

	deco16ic_tilemap_2_draw(state->m_deco_tilegen1, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
	screen.machine().device<decospr_device>("spritegen")->inefficient_copy_sprite_bitmap(bitmap, cliprect, 0x0800, 0x0800, 0x200, 0x1ff);
	deco16ic_tilemap_1_draw(state->m_deco_tilegen1, bitmap, cliprect, 0, 0);
	screen.machine().device<decospr_device>("spritegen")->inefficient_copy_sprite_bitmap(bitmap, cliprect, 0x0000, 0x0800, 0x200, 0x1ff);

	return 0;
}


WRITE16_MEMBER(miragemi_state::mirage_mux_w)
{
	m_mux_data = data & 0x1f;
}

READ16_MEMBER(miragemi_state::mirage_input_r)
{
	switch (m_mux_data & 0x1f)
	{
		case 0x01: return input_port_read(machine(), "KEY0");
		case 0x02: return input_port_read(machine(), "KEY1");
		case 0x04: return input_port_read(machine(), "KEY2");
		case 0x08: return input_port_read(machine(), "KEY3");
		case 0x10: return input_port_read(machine(), "KEY4");
	}

	return 0xffff;
}

WRITE16_MEMBER(miragemi_state::okim1_rombank_w)
{
	m_oki_sfx->set_bank_base(0x40000 * (data & 0x3));
}

WRITE16_MEMBER(miragemi_state::okim0_rombank_w)
{

	/*bits 4-6 used on POST? */
	m_oki_bgm->set_bank_base(0x40000 * (data & 0x7));
}

static ADDRESS_MAP_START( mirage_map, AS_PROGRAM, 16, miragemi_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	/* tilemaps */
	AM_RANGE(0x100000, 0x101fff) AM_DEVREADWRITE_LEGACY("tilegen1", deco16ic_pf1_data_r, deco16ic_pf1_data_w) // 0x100000 - 0x101fff tested
	AM_RANGE(0x102000, 0x103fff) AM_DEVREADWRITE_LEGACY("tilegen1", deco16ic_pf2_data_r, deco16ic_pf2_data_w) // 0x102000 - 0x102fff tested
	/* linescroll */
	AM_RANGE(0x110000, 0x110bff) AM_RAM AM_BASE(m_pf1_rowscroll)
	AM_RANGE(0x112000, 0x112bff) AM_RAM AM_BASE(m_pf2_rowscroll)
	AM_RANGE(0x120000, 0x1207ff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x130000, 0x1307ff) AM_RAM_WRITE(paletteram16_xBBBBBGGGGGRRRRR_word_w) AM_SHARE("paletteram")
	AM_RANGE(0x140000, 0x14000f) AM_DEVREADWRITE8("oki_sfx", okim6295_device, read, write, 0x00ff)
	AM_RANGE(0x150000, 0x15000f) AM_DEVREADWRITE8("oki_bgm", okim6295_device, read, write, 0x00ff)
//  AM_RANGE(0x140006, 0x140007) AM_READ_LEGACY(random_readers)
//  AM_RANGE(0x150006, 0x150007) AM_READNOP
	AM_RANGE(0x160000, 0x160001) AM_WRITENOP
	AM_RANGE(0x168000, 0x16800f) AM_DEVWRITE_LEGACY("tilegen1", deco16ic_pf_control_w)
	AM_RANGE(0x16a000, 0x16a001) AM_WRITENOP
	AM_RANGE(0x16c000, 0x16c001) AM_WRITE(okim1_rombank_w)
	AM_RANGE(0x16c002, 0x16c003) AM_WRITE(okim0_rombank_w)
	AM_RANGE(0x16c004, 0x16c005) AM_WRITE(mirage_mux_w)
	AM_RANGE(0x16c006, 0x16c007) AM_READ(mirage_input_r)
	AM_RANGE(0x16e000, 0x16e001) AM_WRITENOP
	AM_RANGE(0x16e002, 0x16e003) AM_READ_PORT("SYSTEM_IN")
	AM_RANGE(0x170000, 0x173fff) AM_RAM
ADDRESS_MAP_END


static INPUT_PORTS_START( mirage )
	PORT_START("SYSTEM_IN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_SERVICE( 0x0008, IP_ACTIVE_LOW )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_VBLANK )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("KEY0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0xffc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0xffe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY2")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP )
	PORT_BIT( 0xfff7, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY3")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT( 0xffe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY4")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0xfff0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static const gfx_layout tile_8x8_layout =
{
	8,8,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+8,RGN_FRAC(1,2)+0,RGN_FRAC(0,2)+8,RGN_FRAC(0,2)+0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	8*16
};

static const gfx_layout tile_16x16_layout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+8,RGN_FRAC(1,2)+0,RGN_FRAC(0,2)+8,RGN_FRAC(0,2)+0 },
	{ 256,257,258,259,260,261,262,263,0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,8*16,9*16,10*16,11*16,12*16,13*16,14*16,15*16 },
	32*16
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 24,8,16,0 },
	{ 512,513,514,515,516,517,518,519, 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
	  8*32, 9*32,10*32,11*32,12*32,13*32,14*32,15*32},
	32*32
};

static GFXDECODE_START( mirage )
	GFXDECODE_ENTRY("gfx1", 0, tile_8x8_layout,     0x000, 32)	/* Tiles (8x8) */
	GFXDECODE_ENTRY("gfx1", 0, tile_16x16_layout,   0x000, 32)	/* Tiles (16x16) */
	GFXDECODE_ENTRY("gfx2", 0, spritelayout,        0x200, 32)	/* Sprites (16x16) */
GFXDECODE_END


static int mirage_bank_callback( const int bank )
{
	return ((bank >> 4) & 0x7) * 0x1000;
}

static const deco16ic_interface mirage_deco16ic_tilegen1_intf =
{
	"screen",
	0, 1,
	0x0f, 0x0f,	/* trans masks (default values) */
	0, 16, /* color base (default values) */
	0x0f, 0x0f,	/* color masks (default values) */
	mirage_bank_callback,
	mirage_bank_callback,
	0,1,
};


static MACHINE_START( mirage )
{
	miragemi_state *state = machine.driver_data<miragemi_state>();

	state->save_item(NAME(state->m_mux_data));
}

static MACHINE_RESET( mirage )
{
	miragemi_state *state = machine.driver_data<miragemi_state>();

	state->m_mux_data = 0;
}

static MACHINE_CONFIG_START( mirage, miragemi_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 28000000/2)
	MCFG_CPU_PROGRAM_MAP(mirage_map)
	MCFG_CPU_VBLANK_INT("screen", irq6_line_hold)

	MCFG_MACHINE_START(mirage)
	MCFG_MACHINE_RESET(mirage)

	/* video hardware */
	MCFG_BUFFERED_SPRITERAM16_ADD("spriteram")

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(58)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(529))
	MCFG_SCREEN_SIZE(40*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 1*8, 31*8-1)
	MCFG_SCREEN_UPDATE_STATIC(mirage)
	MCFG_SCREEN_VBLANK_DEVICE("spriteram", buffered_spriteram16_device, vblank_copy_rising)

	MCFG_VIDEO_START(mirage)

	MCFG_GFXDECODE(mirage)
	MCFG_PALETTE_LENGTH(1024)

	MCFG_DECO16IC_ADD("tilegen1", mirage_deco16ic_tilegen1_intf)
	MCFG_DEVICE_ADD("spritegen", DECO_SPRITE, 0)
	decospr_device::set_gfx_region(*device, 2);

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_OKIM6295_ADD("oki_bgm", 2000000, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)

	MCFG_OKIM6295_ADD("oki_sfx", 1000000, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.70)
MACHINE_CONFIG_END


ROM_START( mirage )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "mr_00-.2a", 0x00000, 0x40000, CRC(3a53f33d) SHA1(0f654021dcd64202b41e0ef5ef3cdf5dd274f8a5) )
	ROM_LOAD16_BYTE( "mr_01-.3a", 0x00001, 0x40000, CRC(a0b758aa) SHA1(7fb5faf6fb57cd72a3ac24b8af1f33e504ac8398) )

	ROM_REGION( 0x100000, "gfx1", 0 ) /* Tiles - Encrypted */
	ROM_LOAD( "mbl-00.7a", 0x000000, 0x100000, CRC(2e258b7b) SHA1(2dbd7d16a1eda97ae3de149b67e80e511aa9d0ba) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* Sprites */
	ROM_LOAD16_BYTE( "mbl-01.11a", 0x000001, 0x200000, CRC(895be69a) SHA1(541d8f37fb4cf99312b80a0eb0d729fbbeab5f4f) )
	ROM_LOAD16_BYTE( "mbl-02.12a", 0x000000, 0x200000, CRC(474f6104) SHA1(ff81b32b90192c3d5f27c436a9246aa6caaeeeee) )

	ROM_REGION( 0x200000, "oki_bgm_data", 0 )
	ROM_LOAD( "mbl-03.10a", 0x000000, 0x200000, CRC(4a599703) SHA1(b49e84faa2d6acca952740d30fc8d1a33ac47e79) )

	ROM_REGION( 0x200000, "oki_bgm", 0 )
	ROM_COPY( "oki_bgm_data", 0x000000, 0x000000, 0x080000 )
	ROM_COPY( "oki_bgm_data", 0x100000, 0x080000, 0x080000 ) // - banks 2,3 and 4,5 are swapped, PAL address shuffle
	ROM_COPY( "oki_bgm_data", 0x080000, 0x100000, 0x080000 ) // /
	ROM_COPY( "oki_bgm_data", 0x180000, 0x180000, 0x080000 )

	ROM_REGION( 0x100000, "oki_sfx", 0 )	/* M6295 samples */
	ROM_LOAD( "mbl-04.12k", 0x000000, 0x100000, CRC(b533123d) SHA1(2cb2f11331d00c2d282113932ed2836805f4fc6e) )
ROM_END

static DRIVER_INIT( mirage )
{
	deco56_decrypt_gfx(machine, "gfx1");
}

GAME( 1994, mirage, 0,     mirage, mirage, mirage, ROT0, "Mitchell", "Mirage Youjuu Mahjongden (Japan)", GAME_SUPPORTS_SAVE )
