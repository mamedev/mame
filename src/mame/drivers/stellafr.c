// license:BSD-3-Clause
// copyright-holders:David Haywood
/*

Stella
German Fruit Machines / Gambling Machines

Possibly related to ADP hardware?


*/


#include "emu.h"
#include "cpu/m68000/m68000.h"

class stellafr_state : public driver_device
{
public:
	stellafr_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu")
	{ }

protected:

	// devices
	required_device<cpu_device> m_maincpu;
};



static ADDRESS_MAP_START( stellafr_map, AS_PROGRAM, 16, stellafr_state )
	AM_RANGE(0x000000, 0x01ffff) AM_ROM
	AM_RANGE(0xff0000, 0xffffff) AM_RAM
ADDRESS_MAP_END


static INPUT_PORTS_START( stellafr )
INPUT_PORTS_END


static MACHINE_CONFIG_START( stellafr, stellafr_state )
	MCFG_CPU_ADD("maincpu", M68000, 10000000 ) //?
	MCFG_CPU_PROGRAM_MAP(stellafr_map)
MACHINE_CONFIG_END



ROM_START( st_ohla )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "oh_la_la_f1_1.bin", 0x00000, 0x010000, CRC(94583885) SHA1(5083d65da0347a37ffbb537f94d3b247241f1e8c) )
	ROM_LOAD16_BYTE( "oh_la_la_f1_2.bin", 0x00001, 0x010000, CRC(8ac647cd) SHA1(858f67d6121dde28477a5df8569e7ae92db6299e) )
ROM_END

ROM_START( st_vulkn )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "vulkan_f1_1.bin", 0x00000, 0x010000, CRC(06109bd5) SHA1(78f6b0cb3ae5873350fd50af8990fa38454c1183) )
	ROM_LOAD16_BYTE( "vulkan_f1_2.bin", 0x00001, 0x010000, CRC(951baf42) SHA1(1346043155ba85926b2bf9eef8136b377953abe1) )
ROM_END


GAME(199?,  st_ohla,   0,  stellafr,  stellafr, driver_device,  0,  ROT0,  "Stella",    "Oh La La (Stella)",    MACHINE_IS_SKELETON_MECHANICAL )
GAME(199?,  st_vulkn,  0,  stellafr,  stellafr, driver_device,  0,  ROT0,  "Stella",    "Vulkan (Stella)",      MACHINE_IS_SKELETON_MECHANICAL )
