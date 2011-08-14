/* Castle Mach2000 Hardware

  Mechanical Fruit Machines with DMD display

  motherboard pic:
  http://mamedev.emulab.it/haze/source_refs/mach2000.jpg

  videos:
  http://www.youtube.com/watch?v=jNx1OwwS58I
  http://www.youtube.com/watch?v=m1QKaYh64-o

  Note: MB contains an undumped 68705P3, used as the
  reel controller!

  CPU on the MB is a HD6303YP

*/

/*
  Known but not dumped:
  Cashbolt

*/

#include "emu.h"
#include "cpu/m6800/m6800.h"

class castle_state : public driver_device
{
public:
	castle_state(const machine_config &mconfig, device_type type, const char *tag)
	: driver_device(mconfig, type, tag) { }
};

static ADDRESS_MAP_START( mastermap, AS_PROGRAM, 8 )
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( slavemap, AS_PROGRAM, 8 )
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END



static INPUT_PORTS_START( castrev )
INPUT_PORTS_END


static MACHINE_CONFIG_START( castle, castle_state )
	MCFG_CPU_ADD("maincpu", HD6301, 1000000)
	MCFG_CPU_PROGRAM_MAP(mastermap)

	MCFG_CPU_ADD("slavecpu", HD6301, 1000000)
	MCFG_CPU_PROGRAM_MAP(slavemap)
MACHINE_CONFIG_END


ROM_START( castrev )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "castlerevolutionp2.bin", 0x8000, 0x8000, CRC(cb3bea7f) SHA1(0bffbf53f52ad6dbd9832cae05ed8b55d0cc11d4) )

	ROM_REGION( 0x10000, "slavecpu", 0 )
	ROM_LOAD( "castlerevolutionp1.bin", 0x8000, 0x8000, CRC(99ecc2ff) SHA1(8d66c81da13c88dc9ed10d75d24a2eaf448d6c6d) )
ROM_END

// 4.00 JACKPOT. VERSION 1 (for revision E CPU) Written by and copyright of David John Powell - 25th February 1987
GAME( 1987, castrev,	0,	castle, castrev, 0, ROT0, "Castle","Revolution (Castle, Fruit Machine)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
