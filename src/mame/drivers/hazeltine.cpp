// license:BSD-3-Clause
// copyright-holders:
/***********************************************************************************************************************************

2017-11-02 Skeleton

Hazeltine Esprit terminals.

2 zipfiles were found: "Hazeltine_Esprit" and "Hazeltine_EspritIII".


************************************************************************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"

class hazeltine_state : public driver_device
{
public:
	hazeltine_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_p_chargen(*this, "chargen")
	{ }

private:
	required_device<cpu_device> m_maincpu;
	required_region_ptr<u8> m_p_chargen;
};

static ADDRESS_MAP_START( mem_map, AS_PROGRAM, 8, hazeltine_state )
	ADDRESS_MAP_GLOBAL_MASK (0x7fff)
	AM_RANGE(0x0000,0x6fff) AM_RAM
	AM_RANGE(0x7000,0x7fff) AM_ROM AM_REGION("roms", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( mem3_map, AS_PROGRAM, 8, hazeltine_state )
	AM_RANGE(0x0000,0xdfff) AM_RAM
	AM_RANGE(0xe000,0xffff) AM_ROM AM_REGION("roms", 0)
ADDRESS_MAP_END

static INPUT_PORTS_START( hazeltine )
INPUT_PORTS_END

static MACHINE_CONFIG_START( hazeltine )
	MCFG_CPU_ADD("maincpu", M6502, 1000000) // no idea of clock
	MCFG_CPU_PROGRAM_MAP(mem_map)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( hazeltine3 )
	MCFG_CPU_ADD("maincpu", M6502, 1000000) // no idea of clock
	MCFG_CPU_PROGRAM_MAP(mem3_map)
MACHINE_CONFIG_END

ROM_START( hazeltine )
	// Esprit
	ROM_REGION( 0x1000, "roms", 0 )
	ROM_LOAD( "hazeltine_esprit.u19",    0x0000, 0x1000, CRC(6fdec792) SHA1(a1d1d68c8793e7e15ab5cd17682c299dff3985cb) )
	ROM_REGION( 0x1000, "chargen", 0 )
	ROM_LOAD( "hazeltine_esprit.u26",    0x0000, 0x0804, CRC(93f45f13) SHA1(1f493b44124c348759469e24fdfa8b7c52fe6fac) )
ROM_END

ROM_START( hazeltine3 )
	// Esprit III
	ROM_REGION( 0x10000, "roms", 0 )
	ROM_LOAD( "hazeltine_espritiii.u5",  0x0000, 0x2000, CRC(fd63dad1) SHA1(b2a3e7db8480b28cab2b2834ad89fb6257f13cba) )
	ROM_REGION( 0x1000, "chargen", 0 )
	ROM_LOAD( "hazeltine_espritiii.u19", 0x0000, 0x1000, CRC(33e4a8ef) SHA1(e19c84a3c5f94812928ea84bab3ede7970dd5e72) )
ROM_END

COMP( 1981, hazeltine,  0,         0, hazeltine,  hazeltine, hazeltine_state, 0, "Hazeltine", "Esprit", MACHINE_IS_SKELETON )
COMP( 1981, hazeltine3, hazeltine, 0, hazeltine3, hazeltine, hazeltine_state, 0, "Hazeltine", "Esprit III", MACHINE_IS_SKELETON )
