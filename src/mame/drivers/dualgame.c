/*

15.zip

The title Dual Games appears in several places, I assume this is the actual title

Some kind of Demo / Prototype gambling game (no payout?) according to label on PCB
and strings found in ROM

*/

#include "emu.h"
#include "cpu/m68000/m68000.h"


static ADDRESS_MAP_START( dualgame_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x100000, 0x10ffff) AM_RAM
	ADDRESS_MAP_END

static INPUT_PORTS_START( dualgame )
INPUT_PORTS_END

static const gfx_layout dualgame_layout =
{
	8,8,
	RGN_FRAC(1,2),
	8,
	{ 0,1,2,3,4,5,6,7 },
	{ 0,RGN_FRAC(1,2)+0, 8,RGN_FRAC(1,2)+8,  16,RGN_FRAC(1,2)+16,24,RGN_FRAC(1,2)+24 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	8*32
};

static GFXDECODE_START( dualgame )
	GFXDECODE_ENTRY( "gfx1", 0, dualgame_layout,   0x0, 2  )
GFXDECODE_END

VIDEO_START(dualgame)
{

}

VIDEO_UPDATE(dualgame)
{
	return 0;
}

class dualgame_state : public driver_device
{
public:
	dualgame_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

};



static MACHINE_CONFIG_START( dualgame, dualgame_state )

	MDRV_CPU_ADD("maincpu", M68000, 16000000) //?
	MDRV_CPU_PROGRAM_MAP(dualgame_map)
	MDRV_CPU_VBLANK_INT("screen", irq3_line_hold) // lv 2 & 3?

	MDRV_GFXDECODE(dualgame)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 48*8-1, 0*8, 30*8-1)

	MDRV_PALETTE_LENGTH(0x200)

	MDRV_VIDEO_START(dualgame)
	MDRV_VIDEO_UPDATE(dualgame)
MACHINE_CONFIG_END


ROM_START( dualgame )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "mpduga_0.01-a.u27", 0x00000, 0x20000, CRC(57b87596) SHA1(b31d83f5dbd0ad25564c876e2995bba61e1f425f) )
	ROM_LOAD16_BYTE( "mpduga_0.01-b.u28", 0x00001, 0x20000, CRC(e441d895) SHA1(c026b6ebeaedece303b9361bd92c69150ea63b0a) )

	ROM_REGION( 0x80000, "snd", 0 ) /* Samples */
	ROM_LOAD( "bank_2.31-g.u17", 0x00000, 0x80000, CRC(37f5862d) SHA1(8053c9ea30bb304982ef7e2c67d94454df520dfd) )

	ROM_REGION( 0x100000, "gfx1", 0 ) // appears blitter-based
	ROM_LOAD( "mpduga_0.01-c.u68", 0x00000, 0x80000, CRC(bc5b4738) SHA1(69bcc15d3e7524ba26dad0e29919461fbd0a8736) )
	ROM_LOAD( "mpduga_0.01-d.u69", 0x80000, 0x80000, CRC(2f65e87e) SHA1(ded9d75ebb46e061615dac408f86dad14df9d30b) )
ROM_END

GAME( 1995, dualgame,    0,        dualgame,    dualgame,    0, ROT0,  "Labtronix Technologies", "Dual Games (prototype)", GAME_NOT_WORKING | GAME_NO_SOUND ) // 15th September 1995
