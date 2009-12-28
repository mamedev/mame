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

#include "driver.h"
#include "cpu/m68000/m68000.h"
#include "includes/decocrpt.h"
#include "includes/decoprot.h"
#include "includes/deco16ic.h"
#include "sound/okim6295.h"

static void draw_sprites(running_machine *machine, bitmap_t *bitmap,const rectangle *cliprect, int pri)
{
	UINT16 *spriteram16 = machine->generic.spriteram.u16;
	int offs;

	for (offs = 0;offs < 0x400;offs += 4)
	{
		int x,y,sprite,colour,multi,fx,fy,inc,flash,mult;

		sprite = spriteram16[offs+1];
		if (!sprite) continue;

		y = spriteram16[offs];
		flash=y&0x1000;

		if (flash && (video_screen_get_frame_number(machine->primary_screen) & 1)) continue;

		if (pri != ((y & 0x8000)>>15)) continue;

		x = spriteram16[offs+2];
		colour = (x >>9) & 0x1f;

		fx = y & 0x2000;
		fy = y & 0x4000;

		multi = (1 << ((y & 0x0600) >> 9)) - 1;	/* 1x, 2x, 4x, 8x height */

		x = x & 0x01ff;
		y = y & 0x01ff;
		if (x >= 320) x -= 512;
		if (y >= 256) y -= 512;
		y = 240 - y;
        x = 304 - x;

		if (x>320) continue;

		sprite &= ~multi;
		if (fy)
			inc = -1;
		else
		{
			sprite += multi;
			inc = 1;
		}

		if (flip_screen_get(machine))
		{
			y=240-y;
			x=304-x;
			if (fx) fx=0; else fx=1;
			if (fy) fy=0; else fy=1;
			mult=16;
		}
		else mult=-16;

		while (multi >= 0)
		{
			drawgfx_transpen(bitmap,cliprect,machine->gfx[2],
					sprite - multi * inc,
					colour,
					fx,fy,
					x,y + mult * multi,0);

			multi--;
		}
	}
}

static int mirage_bank_callback(const int bank)
{
	return ((bank>>4)&0x7) * 0x1000;
}

static VIDEO_START(mirage)
{
	deco16_1_video_init(machine);

	deco16_set_tilemap_bank_callback(0, mirage_bank_callback);
	deco16_set_tilemap_bank_callback(1, mirage_bank_callback);
}

static VIDEO_UPDATE(mirage)
{
	flip_screen_set(screen->machine,  deco16_pf12_control[0]&0x80 );
	deco16_pf12_update(deco16_pf1_rowscroll,deco16_pf2_rowscroll);

	bitmap_fill(bitmap,cliprect,256); /* not verified */

	deco16_tilemap_2_draw(screen,bitmap,cliprect,TILEMAP_DRAW_OPAQUE,0);
	draw_sprites(screen->machine,bitmap,cliprect,1);
	deco16_tilemap_1_draw(screen,bitmap,cliprect,0,0);
	draw_sprites(screen->machine,bitmap,cliprect,0);

	return 0;
}

static UINT32 mux_data;

static WRITE16_HANDLER( mirage_mux_w )
{
	mux_data = data & 0x1f;
}

static READ16_HANDLER( mirage_input_r )
{
	switch(mux_data & 0x1f)
	{
		case 0x01: return input_port_read(space->machine, "KEY0");
		case 0x02: return input_port_read(space->machine, "KEY1");
		case 0x04: return input_port_read(space->machine, "KEY2");
		case 0x08: return input_port_read(space->machine, "KEY3");
		case 0x10: return input_port_read(space->machine, "KEY4");
	}

	return 0xffff;
}

static WRITE16_HANDLER( okim1_rombank_w )
{
	okim6295_set_bank_base(devtag_get_device(space->machine, "oki_sfx"), 0x40000 * (data & 0x3));
}

static WRITE16_HANDLER( okim0_rombank_w )
{
	/*bits 4-6 used on POST? */

	okim6295_set_bank_base(devtag_get_device(space->machine, "oki_bgm"), 0x40000 * (data & 0x7));
}

static ADDRESS_MAP_START( mirage_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	/* tilemaps */
	AM_RANGE(0x100000, 0x101fff) AM_RAM_WRITE(deco16_pf1_data_w) AM_BASE(&deco16_pf1_data) // 0x100000 - 0x101fff tested
	AM_RANGE(0x102000, 0x103fff) AM_RAM_WRITE(deco16_pf2_data_w) AM_BASE(&deco16_pf2_data) // 0x102000 - 0x102fff tested
	/* linescroll */
	AM_RANGE(0x110000, 0x110bff) AM_RAM AM_BASE(&deco16_pf1_rowscroll)
	AM_RANGE(0x112000, 0x112bff) AM_RAM AM_BASE(&deco16_pf2_rowscroll)
	AM_RANGE(0x120000, 0x1207ff) AM_RAM AM_BASE_GENERIC(spriteram)
	AM_RANGE(0x130000, 0x1307ff) AM_RAM_WRITE(paletteram16_xBBBBBGGGGGRRRRR_word_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0x140000, 0x14000f) AM_DEVREADWRITE8("oki_sfx", okim6295_r, okim6295_w, 0x00ff)
	AM_RANGE(0x150000, 0x15000f) AM_DEVREADWRITE8("oki_bgm", okim6295_r, okim6295_w, 0x00ff)
//  AM_RANGE(0x140006, 0x140007) AM_READ(random_readers)
//  AM_RANGE(0x150006, 0x150007) AM_READNOP
	AM_RANGE(0x160000, 0x160001) AM_WRITENOP
	AM_RANGE(0x168000, 0x16800f) AM_WRITEONLY AM_BASE(&deco16_pf12_control)
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



static MACHINE_DRIVER_START( mirage )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, 28000000/2)
	MDRV_CPU_PROGRAM_MAP(mirage_map)
	MDRV_CPU_VBLANK_INT("screen", irq6_line_hold)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(58)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(529))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(40*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 1*8, 31*8-1)

	MDRV_GFXDECODE(mirage)
	MDRV_PALETTE_LENGTH(1024)

	MDRV_VIDEO_START(mirage)
	MDRV_VIDEO_UPDATE(mirage)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("oki_bgm", OKIM6295, 2000000)
	MDRV_SOUND_CONFIG(okim6295_interface_pin7high) // clock frequency & pin 7 not verified
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)

	MDRV_SOUND_ADD("oki_sfx", OKIM6295, 1000000)
	MDRV_SOUND_CONFIG(okim6295_interface_pin7high) // clock frequency & pin 7 not verified
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.70)
MACHINE_DRIVER_END


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

GAME( 1994, mirage, 0,        mirage, mirage, mirage, ROT0, "Mitchell", "Mirage Youjuu Mahjongden (Japan)", 0 )
