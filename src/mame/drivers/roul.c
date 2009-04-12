/* Super Lucky Roulette?

driver by Roberto Zandona'
thanks to Angelo Salese for some precious advice

Has 36 pin Cherry master looking edge connector 

.u12 2764 stickered 1 
.u19 27256 stickered 2 
.u15 tibpal16l8-25 (checksum was 0) 
.u56 tibpal16l8-25 (checksum was 0) 
.u38 82s123 
.u53 82s123 

Z80 x2 
Altera Ep1810LC-45 
20.000 MHz crystal
video 464p10 x4 (board silcksreeend 4416) 
AY-3-8912A

ROM text showed SUPER LUCKY ROULETTE LEISURE ENT 
*/

#include "driver.h"
#include "cpu/z80/z80.h"


static READ8_HANDLER( testf5_r )
{
logerror("Read unknown port $f5 at %04x\n",cpu_get_pc(space->cpu));
return mame_rand(space->machine) & 0x00f0;
	return 0xc0;
}

static READ8_HANDLER( testf8_r )
{
logerror("Read unknown port $f8 at %04x\n",cpu_get_pc(space->cpu));
return mame_rand(space->machine) & 0x00f0;
	return 0xc0;
}

static READ8_HANDLER( testfa_r )
{
logerror("Read unknown port $fa at %04x\n",cpu_get_pc(space->cpu));
return mame_rand(space->machine) & 0x00f0;
	return 0xc0;
}

static READ8_HANDLER( testfd_r )
{
logerror("Read unknown port $fd at %04x\n",cpu_get_pc(space->cpu));
return mame_rand(space->machine) & 0x00f0;
	return 0xc0;
}

static WRITE8_HANDLER( testfx_w )
{
logerror("Write [%02x] -> %02x\n",offset,data);
}



static ADDRESS_MAP_START( roul_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x8fff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( roul_cpu_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0xf0, 0xff) AM_WRITE(testfx_w)
	AM_RANGE(0xf5, 0xf5) AM_READ(testf5_r)
	AM_RANGE(0xf8, 0xf8) AM_READ(testf8_r)
	AM_RANGE(0xfa, 0xfa) AM_READ(testfa_r)
	AM_RANGE(0xfd, 0xfd) AM_READ(testfd_r)
ADDRESS_MAP_END

static VIDEO_START(roul)
{

}

static VIDEO_UPDATE(roul)
{
	return 0;
}

static INPUT_PORTS_START( roul )
INPUT_PORTS_END

static MACHINE_DRIVER_START( roul )
	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80, 1000000) /* Z80? */
	MDRV_CPU_PROGRAM_MAP(roul_map, 0)
	MDRV_CPU_IO_MAP(roul_cpu_io_map,0)

 	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 32*8-1)

	MDRV_PALETTE_LENGTH(0x100)

	MDRV_VIDEO_START(roul)
	MDRV_VIDEO_UPDATE(roul)
MACHINE_DRIVER_END

ROM_START(roul)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("roul.u19",	0x0000, 0x8000, CRC(1ec37876) SHA1(c2877646dad9daebc55db57d513ad448b1f4c923) )
ROM_END

GAME( 1982, roul,  0,   roul, roul, 0, ROT0, "bootleg", "Super Lucky Roulette?", GAME_NOT_WORKING | GAME_NO_SOUND )
