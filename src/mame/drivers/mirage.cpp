// license:BSD-3-Clause
// copyright-holders:Angelo Salese
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
		m_spriteram(*this, "spriteram") ,
		m_pf1_rowscroll(*this, "pf1_rowscroll"),
		m_pf2_rowscroll(*this, "pf2_rowscroll"),
		m_sprgen(*this, "spritegen")
	{ }


	/* misc */
	UINT8 m_mux_data;

	/* devices */
	required_device<m68000_device> m_maincpu;
	required_device<deco16ic_device> m_deco_tilegen1;
	required_device<okim6295_device> m_oki_sfx;
	required_device<okim6295_device> m_oki_bgm;
	required_device<buffered_spriteram16_device> m_spriteram;
	/* memory pointers */
	required_shared_ptr<UINT16> m_pf1_rowscroll;
	required_shared_ptr<UINT16> m_pf2_rowscroll;
	optional_device<decospr_device> m_sprgen;

	DECLARE_WRITE16_MEMBER(mirage_mux_w);
	DECLARE_READ16_MEMBER(mirage_input_r);
	DECLARE_WRITE16_MEMBER(okim1_rombank_w);
	DECLARE_WRITE16_MEMBER(okim0_rombank_w);
	DECLARE_DRIVER_INIT(mirage);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	UINT32 screen_update_mirage(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	DECO16IC_BANK_CB_MEMBER(bank_callback);
};

void miragemi_state::video_start()
{
	m_sprgen->alloc_sprite_bitmap();
}

UINT32 miragemi_state::screen_update_mirage(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	address_space &space = machine().driver_data()->generic_space();
	UINT16 flip = m_deco_tilegen1->pf_control_r(space, 0, 0xffff);

	flip_screen_set(BIT(flip, 7));

	m_sprgen->draw_sprites(bitmap, cliprect, m_spriteram->buffer(), 0x400);

	m_deco_tilegen1->pf_update(m_pf1_rowscroll, m_pf2_rowscroll);

	bitmap.fill(256, cliprect); /* not verified */

	m_deco_tilegen1->tilemap_2_draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
	m_sprgen->inefficient_copy_sprite_bitmap(bitmap, cliprect, 0x0800, 0x0800, 0x200, 0x1ff);
	m_deco_tilegen1->tilemap_1_draw(screen, bitmap, cliprect, 0, 0);
	m_sprgen->inefficient_copy_sprite_bitmap(bitmap, cliprect, 0x0000, 0x0800, 0x200, 0x1ff);

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
		case 0x01: return ioport("KEY0")->read();
		case 0x02: return ioport("KEY1")->read();
		case 0x04: return ioport("KEY2")->read();
		case 0x08: return ioport("KEY3")->read();
		case 0x10: return ioport("KEY4")->read();
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
	AM_RANGE(0x100000, 0x101fff) AM_DEVREADWRITE("tilegen1", deco16ic_device, pf1_data_r, pf1_data_w) // 0x100000 - 0x101fff tested
	AM_RANGE(0x102000, 0x103fff) AM_DEVREADWRITE("tilegen1", deco16ic_device, pf2_data_r, pf2_data_w) // 0x102000 - 0x102fff tested
	/* linescroll */
	AM_RANGE(0x110000, 0x110bff) AM_RAM AM_SHARE("pf1_rowscroll")
	AM_RANGE(0x112000, 0x112bff) AM_RAM AM_SHARE("pf2_rowscroll")
	AM_RANGE(0x120000, 0x1207ff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x130000, 0x1307ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x140000, 0x14000f) AM_DEVREADWRITE8("oki_sfx", okim6295_device, read, write, 0x00ff)
	AM_RANGE(0x150000, 0x15000f) AM_DEVREADWRITE8("oki_bgm", okim6295_device, read, write, 0x00ff)
//  AM_RANGE(0x140006, 0x140007) AM_READ(random_readers)
//  AM_RANGE(0x150006, 0x150007) AM_READNOP
	AM_RANGE(0x160000, 0x160001) AM_WRITENOP
	AM_RANGE(0x168000, 0x16800f) AM_DEVWRITE("tilegen1", deco16ic_device, pf_control_w)
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
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")
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
	GFXDECODE_ENTRY("gfx1", 0, tile_8x8_layout,     0x000, 32)  /* Tiles (8x8) */
	GFXDECODE_ENTRY("gfx1", 0, tile_16x16_layout,   0x000, 32)  /* Tiles (16x16) */
	GFXDECODE_ENTRY("gfx2", 0, spritelayout,        0x200, 32)  /* Sprites (16x16) */
GFXDECODE_END


DECO16IC_BANK_CB_MEMBER(miragemi_state::bank_callback)
{
	return ((bank >> 4) & 0x7) * 0x1000;
}

void miragemi_state::machine_start()
{
	save_item(NAME(m_mux_data));
}

void miragemi_state::machine_reset()
{
	m_mux_data = 0;
}

static MACHINE_CONFIG_START( mirage, miragemi_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 28000000/2)
	MCFG_CPU_PROGRAM_MAP(mirage_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", miragemi_state,  irq6_line_hold)


	/* video hardware */
	MCFG_BUFFERED_SPRITERAM16_ADD("spriteram")

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(58)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(529))
	MCFG_SCREEN_SIZE(40*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 1*8, 31*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(miragemi_state, screen_update_mirage)
	MCFG_SCREEN_VBLANK_DEVICE("spriteram", buffered_spriteram16_device, vblank_copy_rising)


	MCFG_GFXDECODE_ADD("gfxdecode", "palette", mirage)
	MCFG_PALETTE_ADD("palette", 1024)
	MCFG_PALETTE_FORMAT(xBBBBBGGGGGRRRRR)

	MCFG_DEVICE_ADD("tilegen1", DECO16IC, 0)
	MCFG_DECO16IC_SPLIT(0)
	MCFG_DECO16IC_WIDTH12(1)
	MCFG_DECO16IC_PF1_TRANS_MASK(0x0f)
	MCFG_DECO16IC_PF2_TRANS_MASK(0x0f)
	MCFG_DECO16IC_PF1_COL_BANK(0x00)
	MCFG_DECO16IC_PF2_COL_BANK(0x10)
	MCFG_DECO16IC_PF1_COL_MASK(0x0f)
	MCFG_DECO16IC_PF2_COL_MASK(0x0f)
	MCFG_DECO16IC_BANK1_CB(miragemi_state, bank_callback)
	MCFG_DECO16IC_BANK2_CB(miragemi_state, bank_callback)
	MCFG_DECO16IC_PF12_8X8_BANK(0)
	MCFG_DECO16IC_PF12_16X16_BANK(1)
	MCFG_DECO16IC_GFXDECODE("gfxdecode")
	MCFG_DECO16IC_PALETTE("palette")

	MCFG_DEVICE_ADD("spritegen", DECO_SPRITE, 0)
	MCFG_DECO_SPRITE_GFX_REGION(2)
	MCFG_DECO_SPRITE_GFXDECODE("gfxdecode")
	MCFG_DECO_SPRITE_PALETTE("palette")

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

	ROM_REGION( 0x100000, "oki_sfx", 0 )    /* M6295 samples */
	ROM_LOAD( "mbl-04.12k", 0x000000, 0x100000, CRC(b533123d) SHA1(2cb2f11331d00c2d282113932ed2836805f4fc6e) )
ROM_END

DRIVER_INIT_MEMBER(miragemi_state,mirage)
{
	deco56_decrypt_gfx(machine(), "gfx1");
}

GAME( 1994, mirage, 0,     mirage, mirage, miragemi_state, mirage, ROT0, "Mitchell", "Mirage Youjuu Mahjongden (Japan)", MACHINE_SUPPORTS_SAVE )
