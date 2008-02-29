/*
    Jibun wo Migaku Culture School Mahjong Hen
    (c)1994 Face

    driver by Pierpaolo Prazzoli

    thanks to David Haywood for some precious advice

*/

#include "driver.h"
#include "sound/okim6295.h"

#define MCLK 16000000

static UINT8 *bg0_videoram;
static UINT8 *bg0_regs_x, *bg0_regs_y, *bg1_regs_x, *bg1_regs_y, *bg2_regs_x, *bg2_regs_y;
static tilemap *bg0_tilemap, *bg1_tilemap, *bg2_tilemap;

static int video_enable = 0;
static int irq_enable = 0;
static int bg1_bank = 0, bg2_bank = 0;

static TILE_GET_INFO( get_bg1_tile_info )
{
	UINT8 *region = memory_region(REGION_GFX3) + 0x200000 + 0x80000 * bg1_bank;
	int code = region[tile_index*2] + (region[tile_index*2+1] << 8);
	SET_TILE_INFO(2, code, 0, 0);
}

static TILE_GET_INFO( get_bg2_tile_info )
{
	UINT8 *region = memory_region(REGION_GFX2) + 0x200000 + 0x80000 * bg2_bank;
	int code = region[tile_index*2] + (region[tile_index*2+1] << 8);
	SET_TILE_INFO(1, code, 0, 0);
}

static TILE_GET_INFO( get_bg0_tile_info )
{
	int code = bg0_videoram[tile_index*2] + (bg0_videoram[tile_index*2+1] << 8);
	SET_TILE_INFO(0, code, 0, 0);
}

static VIDEO_START( cultures )
{
	bg2_tilemap = tilemap_create(get_bg2_tile_info,tilemap_scan_rows,8,8,512,512);
	bg1_tilemap = tilemap_create(get_bg1_tile_info,tilemap_scan_rows,8,8,512,512);
	bg0_tilemap = tilemap_create(get_bg0_tile_info,tilemap_scan_rows,8,8, 64,128);

	tilemap_set_transparent_pen(bg1_tilemap,0);
	tilemap_set_transparent_pen(bg0_tilemap,0);


	tilemap_set_scrolldx(bg0_tilemap, 502, 10);
	tilemap_set_scrolldx(bg1_tilemap, 502, 10);
	tilemap_set_scrolldx(bg2_tilemap, 502, 10);

	tilemap_set_scrolldy(bg0_tilemap, 255, 0);
	tilemap_set_scrolldy(bg1_tilemap, 255, 0);
	tilemap_set_scrolldy(bg2_tilemap, 255, 0);
}

static VIDEO_UPDATE( cultures )
{
	int attr;

	// tilemaps attributes

	attr = (bg0_regs_x[3] & 1 ? TILEMAP_FLIPX : 0) | (bg0_regs_y[3] & 1 ? TILEMAP_FLIPY : 0);
	tilemap_set_flip(bg0_tilemap, attr);

	attr = (bg1_regs_x[3] & 1 ? TILEMAP_FLIPX : 0) | (bg1_regs_y[3] & 1 ? TILEMAP_FLIPY : 0);
	tilemap_set_flip(bg1_tilemap, attr);

	attr = (bg2_regs_x[3] & 1 ? TILEMAP_FLIPX : 0) | (bg2_regs_y[3] & 1 ? TILEMAP_FLIPY : 0);
	tilemap_set_flip(bg2_tilemap, attr);

	// tilemaps scrolls

	tilemap_set_scrollx(bg0_tilemap, 0, (bg0_regs_x[2] << 8) + bg0_regs_x[0]);
	tilemap_set_scrollx(bg1_tilemap, 0, (bg1_regs_x[2] << 8) + bg1_regs_x[0]);
	tilemap_set_scrollx(bg2_tilemap, 0, (bg2_regs_x[2] << 8) + bg2_regs_x[0]);
	tilemap_set_scrolly(bg0_tilemap, 0, (bg0_regs_y[2] << 8) + bg0_regs_y[0]);
	tilemap_set_scrolly(bg1_tilemap, 0, (bg1_regs_y[2] << 8) + bg1_regs_y[0]);
	tilemap_set_scrolly(bg2_tilemap, 0, (bg2_regs_y[2] << 8) + bg2_regs_y[0]);

	if (video_enable)
	{
		tilemap_draw(bitmap, cliprect, bg2_tilemap, 0, 0);
		tilemap_draw(bitmap, cliprect, bg0_tilemap, 0, 0);
		tilemap_draw(bitmap, cliprect, bg1_tilemap, 0, 0);
	}
	else
		fillbitmap(bitmap, get_black_pen(machine), cliprect);

	return 0;
}

static WRITE8_HANDLER( cpu_bankswitch_w )
{
	memory_set_bankptr(1, memory_region(REGION_CPU1) + 0x4000 * (data & 0xf));
	video_enable = ~data & 0x20;
}

static WRITE8_HANDLER( bg0_videoram_w )
{
	bg0_videoram[offset] = data;
	tilemap_mark_tile_dirty(bg0_tilemap, offset >> 1);
}

static WRITE8_HANDLER( misc_w )
{
	static int old_bank = -1;
	int new_bank = data & 0xf;

	if(old_bank != new_bank)
	{
		// oki banking
		UINT8 *src = memory_region(REGION_SOUND1) + 0x40000 + 0x20000 * new_bank;
		UINT8 *dst = memory_region(REGION_SOUND1) + 0x20000;
		memcpy(dst, src, 0x20000);

		old_bank = new_bank;
	}

	irq_enable = data & 0x80;
}

static WRITE8_HANDLER( bg_bank_w )
{
	if(bg1_bank != (data & 3))
	{
		bg1_bank = data & 3;
		tilemap_mark_all_tiles_dirty(bg1_tilemap);
	}

	if(bg2_bank != ((data & 0xc) >> 2))
	{
		bg2_bank = (data & 0xc) >> 2;
		tilemap_mark_all_tiles_dirty(bg2_tilemap);
	}

	coin_counter_w(0,data & 0x10);
}

static ADDRESS_MAP_START( cultures_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x7fff) AM_ROMBANK(1)
	AM_RANGE(0x8000, 0xbfff) AM_RAM AM_WRITE(bg0_videoram_w) AM_BASE(&bg0_videoram)
	AM_RANGE(0xc000, 0xdfff) AM_RAM
	AM_RANGE(0xf000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( cultures_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_FLAGS( AMEF_ABITS(8) )
	AM_RANGE(0x00, 0x03) AM_RAM //?
	AM_RANGE(0x10, 0x13) AM_RAM //?
	AM_RANGE(0x20, 0x23) AM_RAM AM_BASE(&bg0_regs_x)
	AM_RANGE(0x30, 0x33) AM_RAM AM_BASE(&bg0_regs_y)
	AM_RANGE(0x40, 0x43) AM_RAM AM_BASE(&bg1_regs_x)
	AM_RANGE(0x50, 0x53) AM_RAM AM_BASE(&bg1_regs_y)
	AM_RANGE(0x60, 0x63) AM_RAM AM_BASE(&bg2_regs_x)
	AM_RANGE(0x70, 0x73) AM_RAM AM_BASE(&bg2_regs_y)
	AM_RANGE(0x80, 0x80) AM_WRITE(cpu_bankswitch_w)
	AM_RANGE(0x90, 0x90) AM_WRITE(misc_w)
	AM_RANGE(0xa0, 0xa0) AM_WRITE(bg_bank_w)
	AM_RANGE(0xc0, 0xc0) AM_READWRITE(OKIM6295_status_0_r, OKIM6295_data_0_w)
	AM_RANGE(0xd0, 0xd0) AM_READ(input_port_0_r)
	AM_RANGE(0xd1, 0xd1) AM_READ(input_port_1_r)
	AM_RANGE(0xd2, 0xd2) AM_READ(input_port_2_r)
	AM_RANGE(0xd3, 0xd3) AM_READ(input_port_3_r)
	AM_RANGE(0xe0, 0xe0) AM_READ(input_port_4_r)
	AM_RANGE(0xe1, 0xe1) AM_READ(input_port_5_r)
	AM_RANGE(0xe2, 0xe2) AM_READ(input_port_6_r)
	AM_RANGE(0xe3, 0xe3) AM_READ(input_port_7_r)
	AM_RANGE(0xe4, 0xe4) AM_READ(input_port_8_r)
	AM_RANGE(0xe5, 0xe5) AM_READ(input_port_9_r)
	AM_RANGE(0xf0, 0xf0) AM_READ(input_port_10_r)
	AM_RANGE(0xf1, 0xf1) AM_READ(input_port_11_r)
	AM_RANGE(0xf2, 0xf2) AM_READ(input_port_12_r)
	AM_RANGE(0xf3, 0xf3) AM_READ(input_port_13_r)
	AM_RANGE(0xf7, 0xf7) AM_READ(input_port_14_r)
ADDRESS_MAP_END


static INPUT_PORTS_START( cultures )
	PORT_START
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0x00, "10 Coins / 1 Credit" )
	PORT_DIPSETTING(    0x01, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START
	PORT_DIPNAME( 0x01, 0x01, "Auto Mode After Reach" ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x02, "Attract Mode" ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x02, "Partial" )
	PORT_DIPSETTING(    0x00, "Full" )
	PORT_DIPNAME( 0x04, 0x04, "Open Hands After Noten" ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x08, 0x08, "Datsui Count After Continue" ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x08, "Not Cleared" )
	PORT_DIPSETTING(    0x00, "Cleared" )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Easy ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Game Background Music" ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes ) )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x02, "Machihai Display" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:7") // "always off"
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x08, IP_ACTIVE_LOW, "SW2:8" )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START //5
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0xf8, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START
	PORT_BIT( 0x1f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE2 ) // "Test"
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )
INPUT_PORTS_END

/*** GFX Decode ***/


static const gfx_layout gfxlayout =
{
	8,8,
	RGN_FRAC(1,1),
	8,
	{ 0,1,2,3,4,5,6,7 },
	{ 1*8, 0*8, 3*8, 2*8, 5*8, 4*8, 7*8, 6*8 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64 },
	8*64,
};

static GFXDECODE_START( culture )
	GFXDECODE_ENTRY(REGION_GFX1, 0, gfxlayout, 0, 1 )
	GFXDECODE_ENTRY(REGION_GFX2, 0, gfxlayout, 0, 1 )
	GFXDECODE_ENTRY(REGION_GFX3, 0, gfxlayout, 0, 1 )
GFXDECODE_END

//WRONG!
static PALETTE_INIT( cultures )
{
	int c,x;

	for (c = 0; c < 256; c++)
	{
 		int r,g,b,i;

 		i = c & 0x03;
 		r = ((c & 0x04) >> 0) | ((c & 0x10) >> 1) | i;
 		g = ((c & 0x20) >> 3) | ((c & 0x40) >> 3) | i;
 		b = ((c & 0x08) >> 1) | ((c & 0x80) >> 4) | i;

		/*
        r = ((c & 0x0c) >> 0)  | i;
        g = ((c & 0x30) >> 2) | i;
        b = ((c & 0xc0) >> 4) | i;
*/

		x = ((c >> 4) & 0x0f);

		x = ((x & 4) << 1) | ((x & 8) >> 1) | (x & 1);

 		palette_set_color_rgb(machine, x | (((c << 4) & 0xf0) ^ 0), pal4bit(r), pal4bit(g), pal4bit(b));
	}
}


static INTERRUPT_GEN( cultures_interrupt )
{
	if (irq_enable)
		cpunum_set_input_line(machine, 0, 0, HOLD_LINE);
}


static MACHINE_DRIVER_START( cultures )

	/* basic machine hardware */
	MDRV_CPU_ADD(Z80, MCLK/2) /* 8.000 MHz */
	MDRV_CPU_PROGRAM_MAP(cultures_map,0)
	MDRV_CPU_IO_MAP(cultures_io_map,0)
	MDRV_CPU_VBLANK_INT("main", cultures_interrupt)

	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 48*8-1, 0*8, 30*8-1)

	MDRV_GFXDECODE(culture)
	MDRV_PALETTE_LENGTH(256)

	MDRV_PALETTE_INIT(cultures)

	MDRV_VIDEO_START(cultures)
	MDRV_VIDEO_UPDATE(cultures)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD(OKIM6295, (MCLK/1024)*132)
	MDRV_SOUND_CONFIG(okim6295_interface_region_1_pin7high) // clock frequency & pin 7 not verified
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)
MACHINE_DRIVER_END

/*

Jibun wo Migaku Culture School Mahjong Hen
(c)1994 Face

CPU: Z80
Sound: M6295
OSC: 16.000MHz
EEPROM: 93C46
Custom: FACE AV44 4HA0G (x3)

ROMs:
MA01.U12
PCM.U87
BG0C.U45
BG0C2.U46
BG1C.U80
BG1T.U67
BG2C.U68
BG2T.U79

-----mahjong connector-----
                      empty
amp OSC               BG0C
6295 GAL              BG0C2
G PCM     SS        S
A MA01 G  RR        R
L      A  AA        A
 Z80   L  MM custom M

D GGG
I AAA
P LLL
D
I custom BG2C         BG1C
P custom      custom
93C46    BG1T         BG2T

*/

ROM_START( cultures )
	ROM_REGION( 0x40000, REGION_CPU1, 0 )
	ROM_LOAD( "ma01.u12",     0x000000, 0x040000, CRC(f57417b3) SHA1(9a2a50222f54e5da9bc5c66863b8be16e33b171f) )

	ROM_REGION( 0x300000, REGION_GFX1, 0 )
	ROM_LOAD( "bg0c.u45",     0x000000, 0x200000, CRC(ad2e1263) SHA1(b28a3d82aaa0421a7b4df837814147b109e7d1a5) )
	ROM_LOAD( "bg0c2.u46",    0x200000, 0x100000, CRC(97c71c09) SHA1(ffbcee1d9cb39d0824f3aa652c3a24579113cf2e) )

	ROM_REGION( 0x400000, REGION_GFX2, ROMREGION_ERASE00 )
	ROM_LOAD( "bg1c.u80",     0x000000, 0x200000, CRC(9ab99bd9) SHA1(bce41b6f5d83c8262ba8d37b2dfcd5d7a5e7ace7) )
	ROM_LOAD( "bg2t.u79",     0x200000, 0x100000, CRC(0610a79f) SHA1(9fc6b2e5c573ed682b2f7fa462c8f42ff99da5ba) )
	/* 0x300000 - 0x3fffff empty */

	ROM_REGION( 0x400000, REGION_GFX3, ROMREGION_ERASE00 )
	ROM_LOAD( "bg2c.u68",     0x000000, 0x200000, CRC(fa598644) SHA1(532249e456c34f18a787d5a028df82f2170f604d) )
	ROM_LOAD( "bg1t.u67",     0x200000, 0x100000, CRC(d2e594ee) SHA1(a84b5ab62dec1867d433ccaeb1381e7593958cf0) )
	/* 0x300000 - 0x3fffff empty */

	ROM_REGION( 0x240000, REGION_SOUND1, 0 )
	ROM_LOAD( "pcm.u87",      0x040000, 0x200000, CRC(84206475) SHA1(d1423bd5c7425e121fb4e7845cf57801e9afa7b3) )
	ROM_RELOAD(               0x000000, 0x020000 )
ROM_END

GAME( 1994, cultures, 0, cultures, cultures, 0, ROT0, "Face", "Jibun wo Migaku Culture School Mahjong Hen", GAME_WRONG_COLORS )
