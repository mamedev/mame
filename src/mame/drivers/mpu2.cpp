// license:BSD-3-Clause
// copyright-holders:David Haywood
/* There were also MPU0 and MPU1 units

 Information from 'MAGIK'

  Barcrest:

  Machine Processor Unit / MPU0 techincally
   -- No ram/roms, lots of discrete DTL logic

  MPU1
   -- First 6800 based unit, still running the earlier mech reels.

  MPU1 second revision
   -- revised board, same functionality?

  MPU2
   -- Early versions use mech reels, later versions use steppers.
   -- Later programs use the lamp output for the reels.
     -- Resulted in many MPU2 boards burning out the reel drive transistors because they aren't rated for it.


  MPU2 info from Highwayman (this doesn't seem to match up well with what the roms expect?)

    cartridge:0000-07ff
    not used: 0800-0fff
    eprom1: 1000-17ff
    rom: 1800-1fff
    eprom2: 2000-23ff
    not used: 2400-27ff
    sram (optional battery-backed 256x4 on d0-d3)
    2800-2fff eprom1
    3000-37ff rom
    3800-3fff eprom2

*/


#include "emu.h"
#include "cpu/m6800/m6800.h"


class mpu2_state : public driver_device
{
public:
	mpu2_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu")
	{ }

protected:

	// devices
	required_device<cpu_device> m_maincpu;
};

static ADDRESS_MAP_START( mpu2_basemap, AS_PROGRAM, 8, mpu2_state )
	ADDRESS_MAP_GLOBAL_MASK(0x3fff) // A14/A15 Not Connected
	AM_RANGE(0x1000, 0x17ff) AM_ROM AM_REGION("maskrom", 0)
	AM_RANGE(0x0800, 0x0fff) AM_ROM AM_REGION("romp1", 0)
	AM_RANGE(0x3800, 0x3fff) AM_ROM AM_REGION("romp2", 0)
ADDRESS_MAP_END

static INPUT_PORTS_START( mpu2 )
INPUT_PORTS_END



static MACHINE_CONFIG_START( mpu2, mpu2_state )
	MCFG_CPU_ADD("maincpu", M6800, 2000000) // ?
	MCFG_CPU_PROGRAM_MAP(mpu2_basemap)
MACHINE_CONFIG_END

// technically not a 'bios' because they're all on the same board.
#define MPU2_MASKROM \
	ROM_REGION( 0x800, "maskrom", 0 ) \
	ROM_LOAD( "rom1.bin", 0x0000, 0x0800, CRC(198d77ee) SHA1(ef466e539efd6e31c82ef01b09d63b7580f068fe) )

ROM_START( m2hilite )
	MPU2_MASKROM

	ROM_REGION( 0x800, "romp2", 0 )
	ROM_LOAD( "hl2-1.p2", 0x0000, 0x0800, CRC(48546c53) SHA1(f50f9b4fa4091510692f08a0d85c80c9803f2657) )

	ROM_REGION( 0x800, "romp1", 0 )
	ROM_LOAD( "hl2-1.p1", 0x0000, 0x0800, CRC(be46ab30) SHA1(54482157e82acc811fc7c1c95d5feacc472b2e10) )
ROM_END


ROM_START( m2svlite )
	MPU2_MASKROM

	ROM_REGION( 0x800, "romp2", 0 )
	ROM_LOAD( "sl2.bin", 0x0000, 0x0800, CRC(b3b1c3a8) SHA1(b4c1540f39cf27ed1312b00b0c7c1f3028c5ed2c) )

	ROM_REGION( 0x800, "romp1", 0 )
	ROM_LOAD( "sl1.bin", 0x0000, 0x0800, CRC(afe04b5a) SHA1(3b3385a9b039992279fda5b87926b5089a448581) )
ROM_END

GAME(198?,  m2hilite,  0, mpu2,  mpu2, driver_device,  0,  ROT0,  "Barcrest",    "Hi-Lights (Barcrest) (MPU2)",         MACHINE_IS_SKELETON_MECHANICAL)
GAME(198?,  m2svlite,  0, mpu2,  mpu2, driver_device,  0,  ROT0,  "Barcrest",    "Silver Lights (Barcrest) (MPU2)",     MACHINE_IS_SKELETON_MECHANICAL)
