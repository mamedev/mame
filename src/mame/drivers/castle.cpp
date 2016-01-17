// license:BSD-3-Clause
// copyright-holders:David Haywood
/* Castle Mach2000 Hardware

  Mechanical Fruit Machines with DMD display

  motherboard pic:
  http://mamedev.emulab.it/haze/source_refs/mach2000.jpg

  videos:
  http://www.youtube.com/watch?v=jNx1OwwS58I
  http://www.youtube.com/watch?v=m1QKaYh64-o

*/


/*
  There are two distinctly different revisions of this PCB, which have incompatible cartridges / hardware (physical connector is very different)

  CASTLE. MACH2000 MCU BOARD VERSION 1 REVISION E (C)DJP

  CASTLE. MACH2000 MCU BOARD VERSION 2 REVISION A (C)DJP

  One significant difference is that the V1 REV E board has *2* 6303 CPUs
  whereas the V2 REV A board only has a single 6303

              CPUs       PIA?                  SOUND           CMOSRAM    RTC       REEL CONTROLLER
    V1 rv E - 2x6303YP + HD6321 +              2xAY-3-8912A +  2xTC5516 + ICM7170 + 68705
    V2 rv A - 1x6303YP + HD468821P/HD68821P +  2xAY-3-8912A +   TC5??4  + none?   + 68705

  In both cases the 68705 is marked the same way
  'REEL CONTROLLER V1 (C) CASTLE (1987) MACH 2000 SYSTEM'
  It is not dumped

  It is said that one of these boards would go on to become the spACE system, although I don't know if that means exactly the same board
  was used, or if modifications were made.  Based on the CPU configuraiton it looks most likely that it was the V2 rv A board.

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
	castle_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu")
	{ }

protected:

	// devices
	required_device<cpu_device> m_maincpu;
};


static ADDRESS_MAP_START( V1rvE_mastermap, AS_PROGRAM, 8, castle_state )
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( V1rvE_slavemap, AS_PROGRAM, 8, castle_state )
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END



static INPUT_PORTS_START( castrev )
INPUT_PORTS_END


static MACHINE_CONFIG_START( castle_V1rvE, castle_state )
	MCFG_CPU_ADD("maincpu", HD6303Y, 1000000)
	MCFG_CPU_PROGRAM_MAP(V1rvE_mastermap)

	MCFG_CPU_ADD("slavecpu", HD6303Y, 1000000)
	MCFG_CPU_PROGRAM_MAP(V1rvE_slavemap)
MACHINE_CONFIG_END



static ADDRESS_MAP_START( V2rvA_map, AS_PROGRAM, 8, castle_state )
	AM_RANGE(0x2000, 0xffff) AM_ROM
ADDRESS_MAP_END


static MACHINE_CONFIG_START( castle_V2rvA, castle_state )
	MCFG_CPU_ADD("maincpu", HD6303Y, 1000000)
	MCFG_CPU_PROGRAM_MAP(V2rvA_map)
MACHINE_CONFIG_END


ROM_START( castrev )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "castlerevolutionp2.bin", 0x8000, 0x8000, CRC(cb3bea7f) SHA1(0bffbf53f52ad6dbd9832cae05ed8b55d0cc11d4) )

	ROM_REGION( 0x10000, "slavecpu", 0 )
	ROM_LOAD( "castlerevolutionp1.bin", 0x8000, 0x8000, CRC(99ecc2ff) SHA1(8d66c81da13c88dc9ed10d75d24a2eaf448d6c6d) )
ROM_END

ROM_START( castfpt )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "fp2.bin", 0x0000, 0x008000, CRC(c49138d1) SHA1(e7813703e57b2f356aff7a17152c03af70b00683) )
	ROM_LOAD( "fp1.bin", 0x8000, 0x008000, CRC(fb458850) SHA1(71343099c7b83db1a4e093f546e5061b9761aa3a) )
ROM_END


// 4.00 JACKPOT. VERSION 1 (for revision E CPU) Written by and copyright of David John Powell - 25th February 1987
GAME( 1987, castrev,    0,  castle_V1rvE, castrev, driver_device, 0, ROT0, "Castle","Revolution (Castle) (MACH2000 V1rvE)",MACHINE_IS_SKELETON_MECHANICAL )

// I'm *guessing* this is on MACH2000 V2rvA hardware, it contains strings saying 'MACH 2000 test' and is designed for a single CPU.
GAME( 198?, castfpt,    0,  castle_V2rvA, castrev, driver_device, 0, ROT0, "Castle","Fortune Pot (Castle) (MACH2000 V2rvA)",MACHINE_IS_SKELETON_MECHANICAL )
