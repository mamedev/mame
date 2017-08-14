// license:BSD-3-Clause
// copyright-holder:FelipeSanches

#include "emu.h"
#include "cpu/mcs51/mcs51.h"

class controlidx628_state : public driver_device
{
public:
	controlidx628_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
	{
	}
};


/*************************
* Memory map information *
*************************/

static ADDRESS_MAP_START( prog_map, AS_PROGRAM, 8, controlidx628_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( io_map, AS_IO, 8, controlidx628_state )
	AM_RANGE(0x8000, 0xffff) AM_RAM

//	/* Ports start here */
//	AM_RANGE(MCS51_PORT_P0, MCS51_PORT_P3) AM_RAM
ADDRESS_MAP_END


/*************************
*      Input ports       *
*************************/

//static INPUT_PORTS_START( controlidx628 )
//INPUT_PORTS_END


/*************************
*     Machine Driver     *
*************************/

static MACHINE_CONFIG_START( controlidx628 )
	// basic machine hardware
	MCFG_CPU_ADD("maincpu", I80C32, XTAL_11_0592MHz) /* Actually the board has an Atmel AT89S52 mcu. */
	MCFG_CPU_PROGRAM_MAP(prog_map)
	MCFG_CPU_IO_MAP(io_map)
MACHINE_CONFIG_END


/*************************
*        Rom Load        *
*************************/

ROM_START( cidx628 )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "controlid_x628.u1",   0x0000, 0x2000, CRC(500d79b4) SHA1(5522115f2da622db389e067fcdd4bccb7aa8561a) )
ROM_END

COMP(200?, cidx628, 0, 0, controlidx628, 0, driver_device, 0, "ControlID", "X628", MACHINE_NOT_WORKING|MACHINE_NO_SOUND)
