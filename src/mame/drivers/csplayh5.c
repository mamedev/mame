/***************************************************************************

	'High Rate DVD' HW (c) 1998 Nichibutsu

	preliminary driver by Angelo Salese

	TODO:
	- V9958 timings are screwed, they basically don't work for this.

***************************************************************************/

#include "emu.h"
#include "deprecat.h"
#include "cpu/m68000/m68000.h"
#include "machine/tmp68301.h"
#include "video/v9938.h"

static bitmap_t *vdp0_bitmap;

// from MSX2 driver, may be not accurate for this HW
#define MSX2_XBORDER_PIXELS		16
#define MSX2_YBORDER_PIXELS		28
#define MSX2_TOTAL_XRES_PIXELS		256 * 2 + (MSX2_XBORDER_PIXELS * 2)
#define MSX2_TOTAL_YRES_PIXELS		212 * 2 + (MSX2_YBORDER_PIXELS * 2)
#define MSX2_VISIBLE_XBORDER_PIXELS	8 * 2
#define MSX2_VISIBLE_YBORDER_PIXELS	14 * 2

static void csplayh5_vdp0_interrupt(running_machine *machine, int i)
{
	/* this is not used as the v9938 interrupt callbacks are broken
       interrupts seem to be fired quite randomly */
}

static VIDEO_START( csplayh5 )
{
	vdp0_bitmap = machine->primary_screen->alloc_compatible_bitmap();
	v9938_init (machine, 0, *machine->primary_screen, vdp0_bitmap, MODEL_V9958, 0x20000, csplayh5_vdp0_interrupt);
	v9938_reset(0);
}

static VIDEO_UPDATE( csplayh5 )
{
	bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine));

	copybitmap(bitmap, vdp0_bitmap, 0, 0, 0, 0, cliprect);

	return 0;
}

static READ16_HANDLER( test_r )
{
	return 0xff;//mame_rand(space->machine);
}

static ADDRESS_MAP_START( csplayh5_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x200000, 0x200001) AM_READ(test_r)
	AM_RANGE(0x200200, 0x200201) AM_READ(test_r) AM_WRITENOP // input multiplexer
	AM_RANGE(0x200400, 0x200401) AM_READ(test_r)

	AM_RANGE(0x200600, 0x200601) AM_READWRITE8(v9938_0_vram_r, v9938_0_vram_w,0xff)
	AM_RANGE(0x200602, 0x200603) AM_READWRITE8(v9938_0_status_r, v9938_0_command_w,0xff)
	AM_RANGE(0x200604, 0x200605) AM_WRITE8(v9938_0_palette_w,0xff)
	AM_RANGE(0x200606, 0x200607) AM_WRITE8(v9938_0_register_w,0xff)

	AM_RANGE(0x800000, 0xbfffff) AM_ROM AM_REGION("blit_gfx",0) // GFX ROM routes here

	AM_RANGE(0xc80000, 0xcfffff) AM_RAM // work RAM

	AM_RANGE(0xfffc00, 0xffffff) AM_READWRITE(tmp68301_regs_r, tmp68301_regs_w)	// TMP68301 Registers
ADDRESS_MAP_END


static INPUT_PORTS_START( csplayh5 )
INPUT_PORTS_END

static GFXDECODE_START( csplayh5 )
GFXDECODE_END

static MACHINE_RESET( csplayh5 )
{

}

static INTERRUPT_GEN( scanline_irq )
{
	v9938_set_sprite_limit(0, 0);
	v9938_set_resolution(0, RENDER_HIGH);
	v9938_interrupt(device->machine, 0);
}

static INTERRUPT_GEN( csplayh5_irq )
{
	cpu_set_input_line_and_vector(device, 1, HOLD_LINE,0x100/4);
}


static MACHINE_CONFIG_START( csplayh5, driver_device )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu",M68000,16000000) /* TMP68301 */
	MDRV_CPU_PROGRAM_MAP(csplayh5_map)
	MDRV_CPU_VBLANK_INT("screen", csplayh5_irq )
	MDRV_CPU_PERIODIC_INT(scanline_irq,572) // unknown timing

	MDRV_MACHINE_RESET(csplayh5)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)

	MDRV_SCREEN_ADD("screen",RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))

	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(MSX2_TOTAL_XRES_PIXELS, MSX2_TOTAL_YRES_PIXELS)
	MDRV_SCREEN_VISIBLE_AREA(MSX2_XBORDER_PIXELS - MSX2_VISIBLE_XBORDER_PIXELS, MSX2_TOTAL_XRES_PIXELS - MSX2_XBORDER_PIXELS + MSX2_VISIBLE_XBORDER_PIXELS - 1, MSX2_YBORDER_PIXELS - MSX2_VISIBLE_YBORDER_PIXELS, MSX2_TOTAL_YRES_PIXELS - MSX2_YBORDER_PIXELS + MSX2_VISIBLE_YBORDER_PIXELS - 1)
	MDRV_PALETTE_LENGTH(512)
	MDRV_PALETTE_INIT( v9958 )

	MDRV_VIDEO_START(csplayh5)
	MDRV_VIDEO_UPDATE(csplayh5)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")
	/* ... */
MACHINE_CONFIG_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( csplayh5 )
	ROM_REGION( 0x40000, "maincpu", 0 ) // tmp68301 prg
	ROM_LOAD16_BYTE( "2.ic3",   0x00000, 0x20000, CRC(980bf3b0) SHA1(89da7354552f30aaa9d46442972c060b4b0f8979) )
	ROM_LOAD16_BYTE( "1.ic2",   0x00001, 0x20000, CRC(81ca49a4) SHA1(601b6802ab85be61f45a64f5b4c7e1f1ae5ee887) )

	ROM_REGION( 0x20000, "subcpu", 0 ) //m68k-based
	ROM_LOAD( "daughter.bin",   0x00000, 0x20000, CRC(36135792) SHA1(1b9c50bd02df8227b228b35cc485efd5a13ec639) )

	ROM_REGION( 0x20000, "audiocpu", 0 ) // z80
	ROM_LOAD( "11.ic51",   0x00000, 0x20000, CRC(0b920806) SHA1(95f50ebfb296ba29aaa8079a41f5362cb9e879cc) )

	ROM_REGION( 0x400000, "blit_gfx", ROMREGION_ERASEFF ) // blitter based gfxs
	ROM_LOAD16_BYTE( "3.ic40",   0x00000, 0x80000, CRC(895b5e1f) SHA1(9398ee95d391f74d62fe641cb75311f31d4d1c8d) )
	ROM_LOAD16_BYTE( "4.ic41",   0x00001, 0x80000, CRC(113d7e96) SHA1(f3fb9c719544417a6a018b82f07c65bf73de21ff) )
	// 0x100000 - 0x3fffff empty sockets

	DISK_REGION( "dvd" )
	DISK_IMAGE( "csplayh5", 0, SHA1(ce4883ce1351ce5299e41bfbd9a5ae8078b82b8c) )
ROM_END

// 1998
// 01 : Mahjong Gal-pri (World Gal-con Grandprix) : Nichibutsu/Just&Just
// 02 : Sengoku Mahjong Kurenai Otome-tai : Nichibutsu/Just&Just
// 03 : Jyunai - Manatsu no First Kiss : Nichibutsu/eic
/* 04 */ GAME( 1998, csplayh5,  0,   csplayh5,  csplayh5,  0, ROT0, "Nichibutsu", "Cosplay Heaven 5 (Japan)", GAME_NOT_WORKING | GAME_NO_SOUND )
// 05 : Jyunai2 - White Love Story : Nichibutsu/eic
// 06 : Mahjong Mogitate : Nichibutsu/Just&Just/NVS/Astro System/AV Japan

// 1999
// 07 : Mahjong Maina - Kairakukan he Youkoso : Sphinx/Just&Just
// 08 : Renai Mahjong Idol Gakuen : Nichibutsu/eic
// 09 : BiKiNikko - Okinawa de Ippai Shityaimashita : Nichibutsu/eic
// 10 : Mahjong Hanafuda Cospure Tengoko 6 - Jyunai hen : Nichibutsu/eic
// 11 : The Nanpa : Nichibutsu/Love Factory/eic
// 12 : PokoaPoka Onsen de CHU - Bijin 3 Simai ni kiwotukete : Nichibutsu/eic
// 13 : Cospure Tengoku 7 - Super Co-gal Grandprix : Nichibutsu/eic
// 14 : Ai-mode - Pet Shiiku : Nichibutsu/eic

// 2000
// 15 : Fu-dol : Nichbutsu/eic
// 16 : Nurete Mitaino... - Net Idol Hen : Nichibutsu/Love Factory
// 17 : Tuugakuro no Yuuwaku : Nichibutsu/Love Factory/Just&Just
// 18 : Toraretyattano - AV Kantoku Hen : Nichibutsu/Love Factory/M Friend

// 2001
// 19 : Konnano Hajimete! : Nichibutsu/Love Factory
// 20 : Uwasa no Deai-kei Sight : Nichibutsu/Love Factory/eic

