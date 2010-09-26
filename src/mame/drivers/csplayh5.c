/***************************************************************************

	skeleton driver for Nichibutsu 'High Rate DVD' HW

***************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/tmp68301.h"

static VIDEO_START( csplayh5 )
{

}

static VIDEO_UPDATE( csplayh5 )
{
	static int test_x, test_y, start_offs;
	int x,y,count;
	const UINT8 *blit_ram = memory_region(screen->machine,"blit_gfx");

	if(input_code_pressed(screen->machine, KEYCODE_Z))
		test_x++;

	if(input_code_pressed(screen->machine, KEYCODE_X))
		test_x--;

	if(input_code_pressed(screen->machine, KEYCODE_A))
		test_y++;

	if(input_code_pressed(screen->machine, KEYCODE_S))
		test_y--;

	if(input_code_pressed(screen->machine, KEYCODE_Q))
		start_offs+=0x200;

	if(input_code_pressed(screen->machine, KEYCODE_W))
		start_offs-=0x200;

	if(input_code_pressed(screen->machine, KEYCODE_E))
		start_offs++;

	if(input_code_pressed(screen->machine, KEYCODE_R))
		start_offs--;

	popmessage("%d %d %04x",test_x,test_y,start_offs);

	bitmap_fill(bitmap,cliprect,get_black_pen(screen->machine));

	count = (start_offs);

	for(y=0;y<test_y;y++)
	{
		for(x=0;x<test_x;x++)
		{
			UINT32 color;

			color = (blit_ram[count] & 0xff)>>0;

			if((x)<screen->visible_area().max_x && ((y)+0)<screen->visible_area().max_y)
				*BITMAP_ADDR32(bitmap, y, x) = screen->machine->pens[color];

			count++;
		}
	}

	return 0;
}

static READ16_HANDLER( test_r )
{
	return 0xff;//mame_rand(space->machine);
}

static READ16_HANDLER( blit_r )
{
	return 0xfe;
}

static ADDRESS_MAP_START( csplayh5_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x200000, 0x200001) AM_READ(test_r)
	AM_RANGE(0x200200, 0x200201) AM_READ(test_r) AM_WRITENOP // input multiplexer
	AM_RANGE(0x200400, 0x200401) AM_READ(test_r) // dsw 1

	AM_RANGE(0x200600, 0x200601) AM_WRITENOP
	AM_RANGE(0x200602, 0x200603) AM_READ(blit_r) AM_WRITENOP //blitter
	AM_RANGE(0x200604, 0x200605) AM_WRITENOP
	AM_RANGE(0x200606, 0x200607) AM_WRITENOP

	AM_RANGE(0x800000, 0xbfffff) AM_RAM // GFX ROM / RAM?

	AM_RANGE(0xc80000, 0xcbffff) AM_RAM // work RAM

	AM_RANGE(0xfffc00, 0xffffff) AM_READWRITE(tmp68301_regs_r, tmp68301_regs_w)	// TMP68301 Registers
ADDRESS_MAP_END


static INPUT_PORTS_START( csplayh5 )
INPUT_PORTS_END

static GFXDECODE_START( csplayh5 )
GFXDECODE_END

static MACHINE_RESET( csplayh5 )
{

}

static INTERRUPT_GEN( csplayh5_irq )
{
	cpu_set_input_line_and_vector(device, 1, HOLD_LINE,0x100/4);
}

static MACHINE_CONFIG_START( csplayh5, driver_device )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu",M68000,8000000) /* TMP68301 */
	MDRV_CPU_PROGRAM_MAP(csplayh5_map)
	MDRV_CPU_VBLANK_INT("screen", csplayh5_irq )

	MDRV_MACHINE_RESET(csplayh5)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 32*8-1)
	MDRV_GFXDECODE(csplayh5)
	MDRV_PALETTE_LENGTH(512)

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

	ROM_REGION( 0x100000, "blit_gfx", 0 ) // blitter based gfxs
	ROM_LOAD16_BYTE( "4.ic41",   0x00000, 0x80000, CRC(113d7e96) SHA1(f3fb9c719544417a6a018b82f07c65bf73de21ff) )
	ROM_LOAD16_BYTE( "3.ic40",   0x00001, 0x80000, CRC(895b5e1f) SHA1(9398ee95d391f74d62fe641cb75311f31d4d1c8d) )

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

