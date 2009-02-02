/*

	MPU5

	Skeleton Driver
	
	 -- there are a wide range of titles running on this hardware, the recent ones are said to be encrypted
	 -- the driver does nothing, and currently only serves to act as a placeholder to document what existed on this hardware
	 
	 -- the main CPU is a 68340, which is a 32-bit 680xx variant with modified opcodes etc.

	 -- should there be a bios using an 8-bit cpu like MPU4, or is the 68340 the only CPU?

*/

#include "driver.h"
#include "cpu/m68000/m68000.h"


static ADDRESS_MAP_START( mpu5_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x000000, 0x2fffff) AM_ROM
ADDRESS_MAP_END

static INPUT_PORTS_START(  mpu5 )
INPUT_PORTS_END

static MACHINE_DRIVER_START( mpu5 )
	MDRV_CPU_ADD("main", M68EC020, 16000000)	 // ?
	MDRV_CPU_PROGRAM_MAP(mpu5_map,0)
	
	/* no video? */
	
	/* unknown sound */
MACHINE_DRIVER_END

ROM_START( m_honmon )
	ROM_REGION( 0x300000, "main", 0 ) /* Code */
	ROM_LOAD16_BYTE( "hmo_23s.p1", 0x000000, 0x80000, CRC(44bc1bd9) SHA1(8b72909c53b09b9287bf90bcd8970bdf9c1b8798) )
	ROM_LOAD16_BYTE( "hmo_23l.p2", 0x000001, 0x80000, CRC(625a311b) SHA1(38fa0d240b253fcc8dc89438582a9c446410b636) )
	ROM_LOAD16_BYTE( "hmo_23l.p3", 0x100000, 0x80000, CRC(44bc1bd9) SHA1(8b72909c53b09b9287bf90bcd8970bdf9c1b8798) )
	ROM_LOAD16_BYTE( "hmo_23l.p4", 0x100001, 0x80000, CRC(625a311b) SHA1(38fa0d240b253fcc8dc89438582a9c446410b636) )
	ROM_LOAD16_BYTE( "hmo_23l.p5", 0x200000, 0x80000, CRC(44bc1bd9) SHA1(8b72909c53b09b9287bf90bcd8970bdf9c1b8798) )
	ROM_LOAD16_BYTE( "hmo_23l.p6", 0x200001, 0x80000, CRC(625a311b) SHA1(38fa0d240b253fcc8dc89438582a9c446410b636) )
ROM_END

GAME( 199?, m_honmon,    0,         mpu5,     mpu5,    0, ROT0,  "Vivid", "Honey Money", GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK )
