// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*
  Rather skeleton driver for a LG GP40NW10 usb dvd writer.

  Main cpu, a MT1839, seems to be a mcs51 variant.
*/

#include "emu.h"
#include "debugger.h"
#include "cpu/mcs51/mcs51.h"

class lg_dvd_state : public driver_device {
public:
	lg_dvd_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			maincpu(*this, "maincpu")
	{ }

	required_device<i80c52_device> maincpu;
};

static INPUT_PORTS_START( lg )
INPUT_PORTS_END

static ADDRESS_MAP_START( lg_dvd_map, AS_PROGRAM, 8, lg_dvd_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM AM_REGION("maincpu", 0)
ADDRESS_MAP_END

static MACHINE_CONFIG_START( lg, lg_dvd_state )
	MCFG_CPU_ADD( "maincpu", I80C52, XTAL_16MHz )
	MCFG_CPU_PROGRAM_MAP( lg_dvd_map )
MACHINE_CONFIG_END

ROM_START( lggp40 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_SYSTEM_BIOS( 0, "bios0", "1.00" )
	ROMX_LOAD( "firm-1.00.bin", 0x000000, 0x100000, CRC(c7f24f3b) SHA1(c2ce96c02ab419fb7e0b38703cdaeeccb2b7f34b), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "bios1", "1.01" )
	ROMX_LOAD( "firm-1.01.bin", 0x000000, 0x100000, CRC(28820e0c) SHA1(c5f2c1e14e6cff2e57c5196cabcebfaaff7284ce), ROM_BIOS(2) )
ROM_END

SYST( 2011, lggp40, 0, 0, lg, lg, driver_device, 0, "LG", "GP40NW10 dvd writer", MACHINE_NOT_WORKING|MACHINE_NO_SOUND_HW )
