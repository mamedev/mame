/*

    TODO:

    - all

*/

#include "includes/superslave.h"



//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

//-------------------------------------------------
//  ADDRESS_MAP( superslave_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( superslave_mem, AS_PROGRAM, 8, superslave_state )
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( superslave_io )
//-------------------------------------------------

static ADDRESS_MAP_START( superslave_io, AS_IO, 8, superslave_state )
ADDRESS_MAP_END



//**************************************************************************
//  INPUT PORTS
//**************************************************************************

//-------------------------------------------------
//  INPUT_PORTS( superslave )
//-------------------------------------------------

static INPUT_PORTS_START( superslave )
INPUT_PORTS_END



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  z80_daisy_config superslave_daisy_chain
//-------------------------------------------------

static const z80_daisy_config superslave_daisy_chain[] =
{
	{ NULL }
};



//**************************************************************************
//  MACHINE INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  MACHINE_START( superslave )
//-------------------------------------------------

void superslave_state::machine_start()
{
}


//-------------------------------------------------
//  MACHINE_RESET( superslave )
//-------------------------------------------------

void superslave_state::machine_reset()
{
}



//**************************************************************************
//  MACHINE DRIVERS
//**************************************************************************

//-------------------------------------------------
//  MACHINE_CONFIG( superslave )
//-------------------------------------------------

static MACHINE_CONFIG_START( superslave, superslave_state )
	// basic machine hardware
	MCFG_CPU_ADD(Z80_TAG, Z80, 4000000)
	MCFG_CPU_PROGRAM_MAP(superslave_mem)
	MCFG_CPU_IO_MAP(superslave_io)
	MCFG_CPU_CONFIG(superslave_daisy_chain)
MACHINE_CONFIG_END



//**************************************************************************
//  ROMS
//**************************************************************************

//-------------------------------------------------
//  ROM( superslv )
//-------------------------------------------------

ROM_START( superslv )
	ROM_REGION( 0x800, Z80_TAG, 0 )
	ROM_LOAD( "adcs6_slave_v3.2.bin", 0x000, 0x800, CRC(7f39322d) SHA1(2e9621e09378a1bb6fc05317bb58ae7865e52744) )
ROM_END



//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    INIT    COMPANY                          FULLNAME        FLAGS
COMP( 1983, superslv,  0,      0,      superslave,  superslave, driver_device,  0,      "Advanced Digital Corporation",	"Super Slave",	GAME_IS_SKELETON )
