/*
    Jibun wo Migaku Culture School Mahjong Hen
    (c)1994 Face

    driver by Pierpaolo Prazzoli

    thanks to David Haywood for some precious advice

*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/okim6295.h"

#define MCLK 16000000

class cultures_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, cultures_state(machine)); }

	cultures_state(running_machine &machine) { }

	/* memory pointers */
	UINT8 *   bg0_videoram;
	UINT8 *   paletteram;
	UINT8 *   bg0_regs_x;
	UINT8 *   bg1_regs_x;
	UINT8 *   bg2_regs_x;
	UINT8 *   bg0_regs_y;
	UINT8 *   bg1_regs_y;
	UINT8 *   bg2_regs_y;

	/* video-related */
	tilemap_t  *bg0_tilemap, *bg1_tilemap, *bg2_tilemap;
	int      video_bank;
	int      irq_enable;
	int      bg1_bank, bg2_bank;
	int      old_bank;
};



static TILE_GET_INFO( get_bg1_tile_info )
{
	cultures_state *state = (cultures_state *)machine->driver_data;
	UINT8 *region = memory_region(machine, "gfx3") + 0x200000 + 0x80000 * state->bg1_bank;
	int code = region[tile_index * 2] + (region[tile_index * 2 + 1] << 8);
	SET_TILE_INFO(2, code, code >> 12, 0);
}

static TILE_GET_INFO( get_bg2_tile_info )
{
	cultures_state *state = (cultures_state *)machine->driver_data;
	UINT8 *region = memory_region(machine, "gfx2") + 0x200000 + 0x80000 * state->bg2_bank;
	int code = region[tile_index * 2] + (region[tile_index * 2 + 1] << 8);
	SET_TILE_INFO(1, code, code >> 12, 0);
}

static TILE_GET_INFO( get_bg0_tile_info )
{
	cultures_state *state = (cultures_state *)machine->driver_data;
	int code = state->bg0_videoram[tile_index * 2] + (state->bg0_videoram[tile_index * 2 + 1] << 8);
	SET_TILE_INFO(0, code, code >> 12, 0);
}

static VIDEO_START( cultures )
{
	cultures_state *state = (cultures_state *)machine->driver_data;
	state->bg0_tilemap = tilemap_create(machine, get_bg0_tile_info,tilemap_scan_rows, 8, 8, 64, 128);
	state->bg1_tilemap = tilemap_create(machine, get_bg1_tile_info,tilemap_scan_rows, 8, 8, 512, 512);
	state->bg2_tilemap = tilemap_create(machine, get_bg2_tile_info,tilemap_scan_rows, 8, 8, 512, 512);

	tilemap_set_transparent_pen(state->bg1_tilemap, 0);
	tilemap_set_transparent_pen(state->bg0_tilemap, 0);

	tilemap_set_scrolldx(state->bg0_tilemap, 502, 10);
	tilemap_set_scrolldx(state->bg1_tilemap, 502, 10);
	tilemap_set_scrolldx(state->bg2_tilemap, 502, 10);

	tilemap_set_scrolldy(state->bg0_tilemap, 255, 0);
	tilemap_set_scrolldy(state->bg1_tilemap, 255, 0);
	tilemap_set_scrolldy(state->bg2_tilemap, 255, 0);
}

static VIDEO_UPDATE( cultures )
{
	cultures_state *state = (cultures_state *)screen->machine->driver_data;
	int attr;

	// tilemaps attributes
	attr = (state->bg0_regs_x[3] & 1 ? TILEMAP_FLIPX : 0) | (state->bg0_regs_y[3] & 1 ? TILEMAP_FLIPY : 0);
	tilemap_set_flip(state->bg0_tilemap, attr);

	attr = (state->bg1_regs_x[3] & 1 ? TILEMAP_FLIPX : 0) | (state->bg1_regs_y[3] & 1 ? TILEMAP_FLIPY : 0);
	tilemap_set_flip(state->bg1_tilemap, attr);

	attr = (state->bg2_regs_x[3] & 1 ? TILEMAP_FLIPX : 0) | (state->bg2_regs_y[3] & 1 ? TILEMAP_FLIPY : 0);
	tilemap_set_flip(state->bg2_tilemap, attr);

	// tilemaps scrolls
	tilemap_set_scrollx(state->bg0_tilemap, 0, (state->bg0_regs_x[2] << 8) + state->bg0_regs_x[0]);
	tilemap_set_scrollx(state->bg1_tilemap, 0, (state->bg1_regs_x[2] << 8) + state->bg1_regs_x[0]);
	tilemap_set_scrollx(state->bg2_tilemap, 0, (state->bg2_regs_x[2] << 8) + state->bg2_regs_x[0]);
	tilemap_set_scrolly(state->bg0_tilemap, 0, (state->bg0_regs_y[2] << 8) + state->bg0_regs_y[0]);
	tilemap_set_scrolly(state->bg1_tilemap, 0, (state->bg1_regs_y[2] << 8) + state->bg1_regs_y[0]);
	tilemap_set_scrolly(state->bg2_tilemap, 0, (state->bg2_regs_y[2] << 8) + state->bg2_regs_y[0]);

	tilemap_draw(bitmap, cliprect, state->bg2_tilemap, 0, 0);
	tilemap_draw(bitmap, cliprect, state->bg0_tilemap, 0, 0);
	tilemap_draw(bitmap, cliprect, state->bg1_tilemap, 0, 0);

	return 0;
}

static WRITE8_HANDLER( cpu_bankswitch_w )
{
	cultures_state *state = (cultures_state *)space->machine->driver_data;
	memory_set_bank(space->machine, "bank1", data & 0x0f);
	state->video_bank = ~data & 0x20;
}

static WRITE8_HANDLER( bg0_videoram_w )
{
	cultures_state *state = (cultures_state *)space->machine->driver_data;
	if (state->video_bank == 0)
	{
		int r, g, b, datax;
		state->paletteram[offset] = data;
		offset >>= 1;
		datax = state->paletteram[offset * 2] + 256 * state->paletteram[offset * 2 + 1];

		r = ((datax >> 7) & 0x1e) | ((datax & 0x4000) ? 0x1 : 0);
		g = ((datax >> 3) & 0x1e) | ((datax & 0x2000) ? 0x1 : 0);
		b = ((datax << 1) & 0x1e) | ((datax & 0x1000) ? 0x1 : 0);

		palette_set_color_rgb(space->machine, offset, pal5bit(r), pal5bit(g), pal5bit(b));
	}
	else
	{
		state->bg0_videoram[offset] = data;
		tilemap_mark_tile_dirty(state->bg0_tilemap, offset >> 1);
	}
}

static WRITE8_HANDLER( misc_w )
{
	cultures_state *state = (cultures_state *)space->machine->driver_data;
	int new_bank = data & 0xf;

	if (state->old_bank != new_bank)
	{
		// oki banking
		UINT8 *src = memory_region(space->machine, "oki") + 0x40000 + 0x20000 * new_bank;
		UINT8 *dst = memory_region(space->machine, "oki") + 0x20000;
		memcpy(dst, src, 0x20000);

		state->old_bank = new_bank;
	}

	state->irq_enable = data & 0x80;
}

static WRITE8_HANDLER( bg_bank_w )
{
	cultures_state *state = (cultures_state *)space->machine->driver_data;
	if (state->bg1_bank != (data & 3))
	{
		state->bg1_bank = data & 3;
		tilemap_mark_all_tiles_dirty(state->bg1_tilemap);
	}

	if (state->bg2_bank != ((data & 0xc) >> 2))
	{
		state->bg2_bank = (data & 0xc) >> 2;
		tilemap_mark_all_tiles_dirty(state->bg2_tilemap);
	}
	coin_counter_w(space->machine, 0, data & 0x10);
}

static ADDRESS_MAP_START( cultures_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x7fff) AM_ROMBANK("bank1")
	AM_RANGE(0x8000, 0xbfff) AM_RAM_WRITE(bg0_videoram_w) AM_BASE_MEMBER(cultures_state, bg0_videoram)
	AM_RANGE(0xc000, 0xdfff) AM_RAM
	AM_RANGE(0xf000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( cultures_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x03) AM_RAM
	AM_RANGE(0x10, 0x13) AM_RAM
	AM_RANGE(0x20, 0x23) AM_RAM AM_BASE_MEMBER(cultures_state, bg0_regs_x)
	AM_RANGE(0x30, 0x33) AM_RAM AM_BASE_MEMBER(cultures_state, bg0_regs_y)
	AM_RANGE(0x40, 0x43) AM_RAM AM_BASE_MEMBER(cultures_state, bg1_regs_x)
	AM_RANGE(0x50, 0x53) AM_RAM AM_BASE_MEMBER(cultures_state, bg1_regs_y)
	AM_RANGE(0x60, 0x63) AM_RAM AM_BASE_MEMBER(cultures_state, bg2_regs_x)
	AM_RANGE(0x70, 0x73) AM_RAM AM_BASE_MEMBER(cultures_state, bg2_regs_y)
	AM_RANGE(0x80, 0x80) AM_WRITE(cpu_bankswitch_w)
	AM_RANGE(0x90, 0x90) AM_WRITE(misc_w)
	AM_RANGE(0xa0, 0xa0) AM_WRITE(bg_bank_w)
	AM_RANGE(0xc0, 0xc0) AM_DEVREADWRITE("oki", okim6295_r, okim6295_w)
	AM_RANGE(0xd0, 0xd0) AM_READ_PORT("SW1_A")
	AM_RANGE(0xd1, 0xd1) AM_READ_PORT("SW1_B")
	AM_RANGE(0xd2, 0xd2) AM_READ_PORT("SW2_A")
	AM_RANGE(0xd3, 0xd3) AM_READ_PORT("SW2_B")
	AM_RANGE(0xe0, 0xe0) AM_READ_PORT("KEY0")
	AM_RANGE(0xe1, 0xe1) AM_READ_PORT("KEY1")
	AM_RANGE(0xe2, 0xe2) AM_READ_PORT("KEY2")
	AM_RANGE(0xe3, 0xe3) AM_READ_PORT("KEY3")
	AM_RANGE(0xe4, 0xe4) AM_READ_PORT("KEY4")
	AM_RANGE(0xe5, 0xe5) AM_READ_PORT("START")
	AM_RANGE(0xf0, 0xf0) AM_READ_PORT("UNUSED1")
	AM_RANGE(0xf1, 0xf1) AM_READ_PORT("UNUSED2")
	AM_RANGE(0xf2, 0xf2) AM_READ_PORT("UNUSED3")
	AM_RANGE(0xf3, 0xf3) AM_READ_PORT("UNUSED4")
	AM_RANGE(0xf7, 0xf7) AM_READ_PORT("COINS")
ADDRESS_MAP_END


static INPUT_PORTS_START( cultures )
	PORT_START("SW1_A")
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

	PORT_START("SW1_B")
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

	PORT_START("SW2_A")
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

	PORT_START("SW2_B")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x02, "Machihai Display" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Yes ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "SW2:7" ) // "always off"
	PORT_SERVICE_DIPLOC( 0x08, IP_ACTIVE_LOW, "SW2:8" )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0xf8, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("START")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("UNUSED1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("UNUSED2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("UNUSED3")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("UNUSED4")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("COINS")
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
	GFXDECODE_ENTRY("gfx1", 0, gfxlayout, 0x0000, 0x10 )
	GFXDECODE_ENTRY("gfx2", 0, gfxlayout, 0x1000, 0x10 )
	GFXDECODE_ENTRY("gfx3", 0, gfxlayout, 0x1000, 0x10 )
GFXDECODE_END

static INTERRUPT_GEN( cultures_interrupt )
{
	cultures_state *state = (cultures_state *)device->machine->driver_data;
	if (state->irq_enable)
		cpu_set_input_line(device, 0, HOLD_LINE);
}

static MACHINE_START( cultures )
{
	cultures_state *state = (cultures_state *)machine->driver_data;
	UINT8 *ROM = memory_region(machine, "maincpu");

	memory_configure_bank(machine, "bank1", 0, 16, &ROM[0x0000], 0x4000);

	state->paletteram = auto_alloc_array(machine, UINT8, 0x4000);
	state_save_register_global_pointer(machine, state->paletteram, 0x4000);

	state_save_register_global(machine, state->old_bank);
	state_save_register_global(machine, state->video_bank);
	state_save_register_global(machine, state->irq_enable);
	state_save_register_global(machine, state->bg1_bank);
	state_save_register_global(machine, state->bg2_bank);
}

static MACHINE_RESET( cultures )
{
	cultures_state *state = (cultures_state *)machine->driver_data;
	state->old_bank = -1;
	state->video_bank = 0;
	state->irq_enable = 0;
	state->bg1_bank = 0;
	state->bg2_bank = 0;
}

static MACHINE_DRIVER_START( cultures )

	/* driver data */
	MDRV_DRIVER_DATA(cultures_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80, MCLK/2) /* 8.000 MHz */
	MDRV_CPU_PROGRAM_MAP(cultures_map)
	MDRV_CPU_IO_MAP(cultures_io_map)
	MDRV_CPU_VBLANK_INT("screen", cultures_interrupt)

	MDRV_MACHINE_START(cultures)
	MDRV_MACHINE_RESET(cultures)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 48*8-1, 0*8, 30*8-1)

	MDRV_GFXDECODE(culture)
	MDRV_PALETTE_LENGTH(0x2000)

	MDRV_VIDEO_START(cultures)
	MDRV_VIDEO_UPDATE(cultures)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("oki", OKIM6295, (MCLK/1024)*132)
	MDRV_SOUND_CONFIG(okim6295_interface_pin7high) // clock frequency & pin 7 not verified
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
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "ma01.u12",     0x000000, 0x040000, CRC(f57417b3) SHA1(9a2a50222f54e5da9bc5c66863b8be16e33b171f) )

	ROM_REGION( 0x300000, "gfx1", 0 )
	ROM_LOAD( "bg0c.u45",     0x000000, 0x200000, CRC(ad2e1263) SHA1(b28a3d82aaa0421a7b4df837814147b109e7d1a5) )
	ROM_LOAD( "bg0c2.u46",    0x200000, 0x100000, CRC(97c71c09) SHA1(ffbcee1d9cb39d0824f3aa652c3a24579113cf2e) )

	ROM_REGION( 0x400000, "gfx2", ROMREGION_ERASE00 )
	ROM_LOAD( "bg1c.u80",     0x000000, 0x200000, CRC(9ab99bd9) SHA1(bce41b6f5d83c8262ba8d37b2dfcd5d7a5e7ace7) )
	ROM_LOAD( "bg2t.u79",     0x200000, 0x100000, CRC(0610a79f) SHA1(9fc6b2e5c573ed682b2f7fa462c8f42ff99da5ba) )
	/* 0x300000 - 0x3fffff empty */

	ROM_REGION( 0x400000, "gfx3", ROMREGION_ERASE00 )
	ROM_LOAD( "bg2c.u68",     0x000000, 0x200000, CRC(fa598644) SHA1(532249e456c34f18a787d5a028df82f2170f604d) )
	ROM_LOAD( "bg1t.u67",     0x200000, 0x100000, CRC(d2e594ee) SHA1(a84b5ab62dec1867d433ccaeb1381e7593958cf0) )
	/* 0x300000 - 0x3fffff empty */

	ROM_REGION( 0x240000, "oki", 0 )
	ROM_LOAD( "pcm.u87",      0x040000, 0x200000, CRC(84206475) SHA1(d1423bd5c7425e121fb4e7845cf57801e9afa7b3) )
	ROM_RELOAD(               0x000000, 0x020000 )
ROM_END


GAME( 1994, cultures, 0, cultures, cultures, 0, ROT0, "Face", "Jibun wo Migaku Culture School Mahjong Hen", GAME_SUPPORTS_SAVE )
