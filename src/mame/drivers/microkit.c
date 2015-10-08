// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*

    RCA COSMAC Microkit

    http://www.vintagecomputer.net/browse_thread.cfm?id=511

*/

#include "emu.h"
#include "cpu/cosmac/cosmac.h"

class microkit_state : public driver_device
{
public:
	microkit_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
	{ }
};

static ADDRESS_MAP_START( microkit_mem, AS_PROGRAM, 8, microkit_state )
	AM_RANGE(0x0000, 0x01ff) AM_ROM AM_REGION("maincpu", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( microkit_io, AS_IO, 8, microkit_state )
ADDRESS_MAP_END

static INPUT_PORTS_START( microkit )
INPUT_PORTS_END

static MACHINE_CONFIG_START( microkit, microkit_state )
	// basic machine hardware
	MCFG_CPU_ADD("maincpu", CDP1801, 2000000)
	MCFG_CPU_PROGRAM_MAP(microkit_mem)
	MCFG_CPU_IO_MAP(microkit_io)
	MCFG_COSMAC_WAIT_CALLBACK(VCC)
MACHINE_CONFIG_END

ROM_START( microkit )
	ROM_REGION( 0x200, "maincpu", ROMREGION_INVERT )
	ROM_LOAD( "3.2b", 0x000, 0x100, CRC(6799357e) SHA1(c46e3322b8b1b6534a7da04806be29fa265951b7) )
	ROM_LOAD( "4.2a", 0x100, 0x100, CRC(27267bad) SHA1(838df9be2dc175584a1a6ee1770039118e49482e) )
ROM_END

COMP( 1975, microkit,    0,      0,      microkit,        microkit, driver_device, 0,      "RCA",  "COSMAC Microkit",  MACHINE_IS_SKELETON | MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
