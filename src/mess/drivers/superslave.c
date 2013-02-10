/**************************************************************************

Monitor commands
Dxxxx,yyyy      = Dump memory
Fxxxx,yyyy,zz   = Fill memory
Gxxxx           = Goto
Ixx             = In port
Lxxxx           = Load
Mxxxx,yyyy,zzzz = Move x-y to z
Oxx,yy          = Out port
-               = Edit memory
.               = Edit memory


    TODO:

    - all

****************************************************************************/

#include "includes/superslave.h"



//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

//-------------------------------------------------
//  ADDRESS_MAP( superslave_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( superslave_mem, AS_PROGRAM, 8, superslave_state )
	AM_RANGE(0x0000, 0x07FF) AM_ROM AM_REGION(Z80_TAG, 0)
	AM_RANGE(0x0800, 0xFFFF) AM_RAM
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( superslave_io )
//-------------------------------------------------

static ADDRESS_MAP_START( superslave_io, AS_IO, 8, superslave_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ(port00_r) AM_DEVWRITE(TERMINAL_TAG, generic_terminal_device, write)
	AM_RANGE(0x01, 0x01) AM_READ(port01_r)
	AM_RANGE(0x1f, 0x1f) AM_READ(port1f_r)
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
	m_term_data = 0;
}

READ8_MEMBER( superslave_state::port00_r )
{
	UINT8 ret = m_term_data;
	m_term_data = 0;
	return ret;
}

READ8_MEMBER( superslave_state::port01_r )
{
	return (m_term_data) ? 5 : 4;
}

READ8_MEMBER( superslave_state::port1f_r )
{
	return 1;
}

WRITE8_MEMBER( superslave_state::kbd_put )
{
	m_term_data = data;
}

static GENERIC_TERMINAL_INTERFACE( terminal_intf )
{
	DEVCB_DRIVER_MEMBER(superslave_state, kbd_put)
};

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

	/* video hardware */
	MCFG_GENERIC_TERMINAL_ADD(TERMINAL_TAG, terminal_intf)
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

//    YEAR  NAME      PARENT  COMPAT  MACHINE     INPUT       CLASS          INIT    COMPANY                          FULLNAME        FLAGS
COMP( 1983, superslv, 0,      0,      superslave, superslave, driver_device, 0, "Advanced Digital Corporation", "Super Slave",  GAME_IS_SKELETON )
