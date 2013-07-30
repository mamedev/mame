#include "emu.h"
#include "cpu/z80/z80.h"

class dsb46_state : public driver_device
{
public:
	dsb46_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
	{ }
};

static ADDRESS_MAP_START( dsb46_mem, AS_PROGRAM, 8, dsb46_state )
	AM_RANGE(0x0000, 0x07ff) AM_ROM AM_REGION("maincpu", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( dsb46_io, AS_IO, 8, dsb46_state )
ADDRESS_MAP_END

static INPUT_PORTS_START( dsb46 )
INPUT_PORTS_END

static MACHINE_CONFIG_START( dsb46, dsb46_state )
	// basic machine hardware
	MCFG_CPU_ADD("maincpu", Z80, 4000000)
	MCFG_CPU_PROGRAM_MAP(dsb46_mem)
	MCFG_CPU_IO_MAP(dsb46_io)
MACHINE_CONFIG_END

ROM_START( dsb46 )
	ROM_REGION( 0x800, "maincpu", 0 )
	ROM_LOAD( "1538a.bin", 0x000, 0x800, CRC(65b3e26e) SHA1(afe1f03f266b7d13fdb1f1bc6762df5e0aa5c764) )

	ROM_REGION( 0x4000, "ades", 0 )
	ROM_LOAD( "ades.bin", 0x0000, 0x4000, CRC(d374abf0) SHA1(331f51a2bb81375aeffbe63c1ebc1d7cd779b9c3) )
ROM_END

COMP( 198?, dsb46,    0,      0,      dsb46,        dsb46, driver_device, 0,      "Davidge",  "DSB-4/6",  GAME_NOT_WORKING | GAME_NO_SOUND_HW )
