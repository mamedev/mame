// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Robbbert

#include "emu.h"
#include "cpu/z80/z80.h"
#include "softlist.h"

class dps1_state : public driver_device
{
public:
	dps1_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag)
	{ }
};

static ADDRESS_MAP_START( dps1_mem, AS_PROGRAM, 8, dps1_state )
	AM_RANGE(0x0000, 0x03ff) AM_ROM AM_REGION("maincpu", 0)
	AM_RANGE(0x2000, 0x2fff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( dps1_io, AS_IO, 8, dps1_state )
ADDRESS_MAP_END

static INPUT_PORTS_START( dps1 )
INPUT_PORTS_END

static MACHINE_CONFIG_START( dps1, dps1_state )
	// basic machine hardware
	MCFG_CPU_ADD("maincpu", Z80, 4000000)
	MCFG_CPU_PROGRAM_MAP(dps1_mem)
	MCFG_CPU_IO_MAP(dps1_io)

	// software lists
	MCFG_SOFTWARE_LIST_ADD("flop_list", "dps1")
MACHINE_CONFIG_END

ROM_START( dps1 )
	ROM_REGION( 0x400, "maincpu", 0 )
	ROM_LOAD( "boot 1280", 0x000, 0x400, CRC(9c2e98fa) SHA1(78e6c9d00aa6e8f6c4d3c65984cfdf4e99434c66) ) // actually on the FDC-2 board
ROM_END

COMP( 1979, dps1, 0, 0, dps1, dps1, driver_device, 0, "Ithaca InterSystems", "DPS-1", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
