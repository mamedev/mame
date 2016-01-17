// license:BSD-3-Clause
// copyright-holders:hap
/***************************************************************************

  Taito Capriccio Cyclone crane hardware

  Main PCB: NEC uPD30200GD-100-MBB VR4300, Galileo GT-64111 (system controller?), ...
  Sound PCB: Panasonic MN1020819 (has internal ROM), Zoom ZSG-2. No effects DSP!

  Like most other Taito Capriccio cabinets, it has two cranes. There are
  no 7seg leds on the cranes this time, some colour lamps instead.

TODO:
- everything

***************************************************************************/

#include "emu.h"
#include "cpu/mips/mips3.h"


class caprcyc_state : public driver_device
{
public:
	caprcyc_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	required_device<cpu_device> m_maincpu;
};


/***************************************************************************

  I/O

***************************************************************************/

static ADDRESS_MAP_START( caprcyc_map, AS_PROGRAM, 32, caprcyc_state ) // TODO...
	AM_RANGE(0x00000000, 0x0003ffff) AM_ROM
ADDRESS_MAP_END



/***************************************************************************

  Inputs

***************************************************************************/

static INPUT_PORTS_START( caprcyc ) // TODO...
INPUT_PORTS_END



/***************************************************************************

  Machine Config

***************************************************************************/

static MACHINE_CONFIG_START( caprcyc, caprcyc_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", VR4300BE, 100000000) // cpu configuration is unknown
	MCFG_CPU_PROGRAM_MAP(caprcyc_map)

	/* no video! */

	/* sound hardware */
	//..
MACHINE_CONFIG_END



/***************************************************************************

  Game drivers

***************************************************************************/

ROM_START( caprcyc )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "e69-08.bin",  0x000000, 0x040000, CRC(09336e82) SHA1(62e4337eabc920da57bc033a7ab177abfb637f53) )

	ROM_REGION( 0x10000, "sound_cpu", 0 ) /* Internal ROM :( */
	ROM_LOAD( "e68-01.ic2",  0x000000, 0x010000, NO_DUMP ) // E68-01 same label as Taito Type Zero - may be same ROM?

	ROM_REGION32_LE( 0x200000, "zsg2", 0 )
	ROM_LOAD( "e69-02.ic3",  0x000000, 0x200000, CRC(ca0ea2ed) SHA1(de2306207c8b8faa0dac3559ad93904cb957fa28) )
ROM_END


GAME (1999, caprcyc, 0, caprcyc, caprcyc, driver_device, 0, ROT0, "Taito", "Capriccio Cyclone", MACHINE_IS_SKELETON_MECHANICAL )
