/***************************************************************************************

very preliminary driver based on djboy.c



Heavy Unit
Kaneko/Taito 1988

This game runs on Kaneko hardware. The game is similar to R-Type.

PCB Layout
----------
M6100391A
M6100392A  880210204
KNA-001
|----------------------------------------------------|
|                                                    |
|           6116                          6116       |
|      15Mhz                              6116       |
|                                 PAL                |
|           B73_09.2P             B73_11.5P          |
|                                                    |
|                                                    |
|                                 Z80-1   DSW1 DSW2 J|
|                                                   A|
|     16MHz                                         M|
|                                                   M|
|       12MHz                       6264  MERMAID   A|
| B73_05.1H                                          |
| B73_04.1F B73_08.2F  6116              Z80-2       |
| B73_03.1D                       Z80-3  B73_12.7E   |
| B73_02.1C B73_07.2C  PANDORA    B73_10.5C  6116    |
| B73_01.1B B73_06.2B 4164 4164   6264 PAL  YM3014   |
|                     4164 4164   PAL       YM2203   |
|----------------------------------------------------|

Notes:
      Z80-1 clock  : 6.000MHz
      Z80-2 clock  : 6.000MHz
      Z80-3 clock  : 6.000MHz
      YM2203 clock : 3.000MHz
      VSync        : 58Hz
      HSync        : 15.59kHz
               \-\ : KANEKO 1988. DIP40 chip, probably 8751 MCU (clock pins match)
      MERMAID    | : pin 18,19 = 6.000MHz (main clock)
                 | : pin 30 = 1.000MHz (prog/ale)
               /-/ : pin 22 = 111.48Hz (port 2 bit 1)

      PANDORA      : KANEKO PX79480FP-3 PANDORA-CHIP (C) KANEKO 1988


***************************************************************************************/

#include "driver.h"
#include "deprecat.h"
#include "cpu/z80/z80.h"
#include "sound/2203intf.h"
#include "video/kan_pand.h"


static WRITE8_HANDLER( mermaid_data_w )
{

}



static READ8_HANDLER( mermaid_data_r )
{
		return mame_rand(space->machine);
}



static READ8_HANDLER( mermaid_status_r )
{
		return mame_rand(space->machine);
}

static VIDEO_START(hvyunit)
{
	pandora_start(machine,0,0,0);
}

static VIDEO_UPDATE(hvyunit)
{
	bitmap_fill(bitmap,cliprect,get_black_pen(screen->machine));
	pandora_update(screen->machine,bitmap,cliprect);
	return 0;
}

static VIDEO_EOF(hvyunit)
{
	pandora_eof(machine);
}

static WRITE8_HANDLER( trigger_nmi_on_sound_cpu2 )
{
	soundlatch_w(space,0,data);
	cpu_set_input_line(space->machine->cpu[2], INPUT_LINE_NMI, PULSE_LINE);
}


static WRITE8_HANDLER( trigger_nmi_on_sub_cpu)
{
	cpu_set_input_line(space->machine->cpu[0], INPUT_LINE_NMI, PULSE_LINE);
}

static WRITE8_HANDLER( main_bankswitch_w )
{
	unsigned char *ROM = memory_region(space->machine, "maincpu");
	int bank=data&7;

	ROM = &ROM[0x4000 * bank];

	memory_set_bankptr(space->machine, 1,ROM);
}

static ADDRESS_MAP_START( main_memory, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_READ(SMH_BANK1)
	AM_RANGE(0xc000, 0xcfff) AM_READWRITE( pandora_spriteram_r, pandora_spriteram_w )
 	AM_RANGE(0xd000, 0xdfff) AM_RAM
	AM_RANGE(0xe000, 0xffff) AM_RAM AM_SHARE(1)
ADDRESS_MAP_END

static ADDRESS_MAP_START(main_io, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_WRITE(main_bankswitch_w)
	AM_RANGE(0x01, 0x01) AM_WRITE(trigger_nmi_on_sub_cpu)
ADDRESS_MAP_END

static WRITE8_HANDLER( sub_bankswitch_w )
{
	unsigned char *ROM = memory_region(space->machine, "sub");
	int bank=data&0x3;

	ROM = &ROM[0x4000 * bank];

	memory_set_bankptr(space->machine, 2,ROM);
}


static ADDRESS_MAP_START( sub_memory, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_READ(SMH_BANK2)
	AM_RANGE(0xc000, 0xcfff) AM_RAM // videoram?
	AM_RANGE(0xd000, 0xdfff) AM_RAM
	AM_RANGE(0xe000, 0xffff) AM_RAM AM_SHARE(1)
ADDRESS_MAP_END

static ADDRESS_MAP_START(sub_io, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_WRITE(sub_bankswitch_w)
	AM_RANGE(0x02, 0x02) AM_WRITE(trigger_nmi_on_sound_cpu2)
	AM_RANGE(0x04, 0x04) AM_READ(mermaid_data_r) AM_WRITE(mermaid_data_w)
	AM_RANGE(0x0c, 0x0c) AM_READ(mermaid_status_r)
	AM_RANGE(0x0e, 0x0e) AM_WRITENOP //coins?
ADDRESS_MAP_END

static WRITE8_HANDLER( sound_bankswitch_w )
{
	unsigned char *ROM = memory_region(space->machine, "soundcpu");
	int bank=data&0x3;

	ROM = &ROM[0x4000 * bank];

	memory_set_bankptr(space->machine, 3,ROM);
}

static ADDRESS_MAP_START( sound_memory, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_READ(SMH_BANK3)
	AM_RANGE(0xc000, 0xc7ff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START(sound_io, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_WRITE(sound_bankswitch_w)
	AM_RANGE(0x02, 0x03) AM_DEVREADWRITE(SOUND, "ym", ym2203_r, ym2203_w)
	AM_RANGE(0x04, 0x04) AM_READ(soundlatch_r)
ADDRESS_MAP_END


static INPUT_PORTS_START( hvyunit )
	PORT_START("IN0")

	PORT_START("IN1")

	PORT_START("IN2")

	PORT_START("DSW1")

	PORT_START("DSW2")

INPUT_PORTS_END

/****************** Graphics ************************/

static const gfx_layout tile_layout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{
		0*4,1*4,2*4,3*4,4*4,5*4,6*4,7*4,
		8*32+0*4,8*32+1*4,8*32+2*4,8*32+3*4,8*32+4*4,8*32+5*4,8*32+6*4,8*32+7*4
	},
	{
		0*32,1*32,2*32,3*32,4*32,5*32,6*32,7*32,
		16*32+0*32,16*32+1*32,16*32+2*32,16*32+3*32,16*32+4*32,16*32+5*32,16*32+6*32,16*32+7*32
	},
	4*8*32
};

static GFXDECODE_START( hvyunit )
	GFXDECODE_ENTRY( "gfx1", 0, tile_layout, 0x100, 16 ) /* sprite bank */
	GFXDECODE_ENTRY( "gfx2", 0, tile_layout, 0x000, 16 ) /* background tiles */
GFXDECODE_END

/****************** Machine ************************/


static INTERRUPT_GEN( hvyunit_interrupt )
{
	static int addr = 0xff;
	addr ^= 0x02;
	cpu_set_input_line_and_vector(device, 0, HOLD_LINE, addr);
}


static MACHINE_DRIVER_START( hvyunit )

	MDRV_CPU_ADD("maincpu", Z80,6000000)
	MDRV_CPU_PROGRAM_MAP(main_memory,0)
	MDRV_CPU_IO_MAP(main_io,0)
	MDRV_CPU_VBLANK_INT_HACK(hvyunit_interrupt,2)

	MDRV_CPU_ADD("sub", Z80,6000000)
	MDRV_CPU_PROGRAM_MAP(sub_memory,0)
	MDRV_CPU_IO_MAP(sub_io,0)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	MDRV_CPU_ADD("soundcpu", Z80, 6000000)
	MDRV_CPU_PROGRAM_MAP(sound_memory,0)
	MDRV_CPU_IO_MAP(sound_io,0)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	MDRV_QUANTUM_TIME(HZ(6000))

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(58)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(256, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 256-1, 16, 256-1)

	MDRV_GFXDECODE(hvyunit)
	MDRV_PALETTE_LENGTH(0x200)

	MDRV_VIDEO_START(hvyunit)
	MDRV_VIDEO_UPDATE(hvyunit)
	MDRV_VIDEO_EOF(hvyunit)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ym", YM2203, 3000000)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)
MACHINE_DRIVER_END



ROM_START( hvyunit )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "b73.13",  0x00000, 0x20000, CRC(e2874601) SHA1(7f7f3287113b8622eb365d04135d2d9c35d70554) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "b73.14",  0x00000, 0x10000, CRC(0dfb51d4) SHA1(0e6f3b3d4558f12fe1b1620f57a0f4ac2065fd1a) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "b73.12",  0x000000, 0x010000, CRC(d1d24fab) SHA1(ed0312535d0b136d79aa885b9e6eea19ebde6409))

	ROM_REGION( 0x02000, "mermaid", 0 )
	ROM_LOAD( "mermaid.i8751_mcu",  0x000000, 0x02000, NO_DUMP )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "b73.08",  0x000000, 0x080000, CRC(f83dd808) SHA1(09d5f1e86fad3a0d2d3ac1845103d3f2833c6793) )
	ROM_LOAD( "b73.01",  0x080000, 0x010000, CRC(3a8a4489) SHA1(a01d7300015f90ce6dd571ad93e7a58270a99e47) )
	ROM_LOAD( "b73.02",  0x090000, 0x010000, CRC(025c536c) SHA1(075e95cc39e792049ae656404e7f7440df064391) )
	ROM_LOAD( "b73.03",  0x0a0000, 0x010000, CRC(ec6020cf) SHA1(2973aa2dc3deb2f27c9f1bad07a7664bad95b3f2) )
	ROM_LOAD( "b73.04",  0x0b0000, 0x010000, CRC(f7badbb2) SHA1(d824ab4aba94d7ca02401f4f6f34213143c282ec) )
	ROM_LOAD( "b73.05",  0x0c0000, 0x010000, CRC(b8e829d2) SHA1(31102358500d7b58173d4f18647decf5db744416) )
	ROM_LOAD( "b73.06",  0x0d0000, 0x010000, CRC(a98e4aea) SHA1(560fef03ad818894c9c7578c6282d55b646e8129) )
	ROM_LOAD( "b73.07",  0x0e0000, 0x010000, CRC(5cffa42c) SHA1(687e047345039479b35d5099e56dbc1d57284ed9) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "b73.09",  0x000000, 0x080000, CRC(537c647f) SHA1(941c0f4e251bc68e53d62e70b033a3a6c145bb7e) )
ROM_END

ROM_START( hvyunita )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "b73_10.5c",  0x00000, 0x20000, CRC(ca52210f) SHA1(346951962aa5bbad641117dbd66f035dddc7c0bf) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "b73_11.5p",  0x00000, 0x10000, CRC(cb451695) SHA1(116fd59f96a54c22fae65eea9ee5e58cb9ce5074) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "b73.12",  0x000000, 0x010000, CRC(d1d24fab) SHA1(ed0312535d0b136d79aa885b9e6eea19ebde6409) )

	ROM_REGION( 0x02000, "mermaid", 0 )
	ROM_LOAD( "mermaid.i8751_mcu",  0x000000, 0x02000, NO_DUMP )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "b73.08",  0x000000, 0x080000, CRC(f83dd808) SHA1(09d5f1e86fad3a0d2d3ac1845103d3f2833c6793) )
	ROM_LOAD( "b73.01",  0x080000, 0x010000, CRC(3a8a4489) SHA1(a01d7300015f90ce6dd571ad93e7a58270a99e47) )
	ROM_LOAD( "b73.02",  0x090000, 0x010000, CRC(025c536c) SHA1(075e95cc39e792049ae656404e7f7440df064391) )
	ROM_LOAD( "b73.03",  0x0a0000, 0x010000, CRC(ec6020cf) SHA1(2973aa2dc3deb2f27c9f1bad07a7664bad95b3f2) )
	ROM_LOAD( "b73.04",  0x0b0000, 0x010000, CRC(f7badbb2) SHA1(d824ab4aba94d7ca02401f4f6f34213143c282ec) )
	ROM_LOAD( "b73.05",  0x0c0000, 0x010000, CRC(b8e829d2) SHA1(31102358500d7b58173d4f18647decf5db744416) )
	ROM_LOAD( "b73.06",  0x0d0000, 0x010000, CRC(a98e4aea) SHA1(560fef03ad818894c9c7578c6282d55b646e8129) )
	ROM_LOAD( "b73.07",  0x0e0000, 0x010000, CRC(5cffa42c) SHA1(687e047345039479b35d5099e56dbc1d57284ed9) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "b73.09",  0x000000, 0x080000, CRC(537c647f) SHA1(941c0f4e251bc68e53d62e70b033a3a6c145bb7e) )
ROM_END

ROM_START( hvyunitb )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "b73-30.bin",  0x00000, 0x20000, CRC(600af545) SHA1(c52b9be2bae28848ad0818c296f000a1bda4fa4f) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "b73.14",  0x00000, 0x10000, CRC(0dfb51d4) SHA1(0e6f3b3d4558f12fe1b1620f57a0f4ac2065fd1a) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "b73.12",  0x000000, 0x010000, CRC(d1d24fab) SHA1(ed0312535d0b136d79aa885b9e6eea19ebde6409))

	ROM_REGION( 0x02000, "mermaid", 0 )
	ROM_LOAD( "mermaid.i8751_mcu",  0x000000, 0x02000, NO_DUMP )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "b73.08",  0x000000, 0x080000, CRC(f83dd808) SHA1(09d5f1e86fad3a0d2d3ac1845103d3f2833c6793) )
	ROM_LOAD( "b73.01",  0x080000, 0x010000, CRC(3a8a4489) SHA1(a01d7300015f90ce6dd571ad93e7a58270a99e47) )
	ROM_LOAD( "b73.02",  0x090000, 0x010000, CRC(025c536c) SHA1(075e95cc39e792049ae656404e7f7440df064391) )
	ROM_LOAD( "b73.03",  0x0a0000, 0x010000, CRC(ec6020cf) SHA1(2973aa2dc3deb2f27c9f1bad07a7664bad95b3f2) )
	ROM_LOAD( "b73.04",  0x0b0000, 0x010000, CRC(f7badbb2) SHA1(d824ab4aba94d7ca02401f4f6f34213143c282ec) )
	ROM_LOAD( "b73.05",  0x0c0000, 0x010000, CRC(b8e829d2) SHA1(31102358500d7b58173d4f18647decf5db744416) )
	ROM_LOAD( "b73.06",  0x0d0000, 0x010000, CRC(a98e4aea) SHA1(560fef03ad818894c9c7578c6282d55b646e8129) )
	ROM_LOAD( "b73.07",  0x0e0000, 0x010000, CRC(5cffa42c) SHA1(687e047345039479b35d5099e56dbc1d57284ed9) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "b73.09",  0x000000, 0x080000, CRC(537c647f) SHA1(941c0f4e251bc68e53d62e70b033a3a6c145bb7e) )
ROM_END

GAME( 1988, hvyunit, 0,        hvyunit, hvyunit, 0, ROT0, "Kaneko / Taito", "Heavy Unit (set 1)" ,GAME_NOT_WORKING )
GAME( 1988, hvyunita, hvyunit, hvyunit, hvyunit, 0, ROT0, "Kaneko / Taito", "Heavy Unit (set 2)" ,GAME_NOT_WORKING )
GAME( 1988, hvyunitb, hvyunit, hvyunit, hvyunit, 0, ROT0, "Kaneko / Taito", "Heavy Unit (set 3)" ,GAME_NOT_WORKING )


