// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Mator Systems SHARK Intelligent Winchester Disc Subsystem emulation

**********************************************************************/

#include "shark.h"
#include "bus/rs232/rs232.h"
#include "cpu/i8085/i8085.h"
#include "imagedev/harddriv.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define I8085_TAG       "i8085"
#define RS232_TAG       "rs232"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type SHARK = &device_creator<mshark_device>;


//-------------------------------------------------
//  ROM( mshark )
//-------------------------------------------------

ROM_START( mshark )
	ROM_REGION( 0x5000, I8085_TAG, 0 )
	ROM_LOAD( "pch488 3450 v22.1 #1", 0x0000, 0x1000, CRC(03bff9d7) SHA1(ac506df6509e1b2185a69f9f8f44b8b456aa9834) )
	ROM_LOAD( "pch488 3450 v22.1 #2", 0x1000, 0x1000, CRC(c14fa5fe) SHA1(bcfd1dd65d692c76b90e6134b85f22c39c049430) )
	ROM_LOAD( "pch488 3450 v22.1 #3", 0x2000, 0x1000, CRC(4dfaa482) SHA1(fe2c44bb650572616c8bdad6358032fe64b1e363) )
	ROM_LOAD( "pch488 3450 v22.1 #4", 0x3000, 0x1000, CRC(aef665e9) SHA1(80a4c00b717100b4e22fa3704e34060fffce2bc3) )
	ROM_LOAD( "pch488 3450 v22.1 #5", 0x4000, 0x1000, CRC(f30adf60) SHA1(96c15264d5a9b52e1d238921880c48a797a6da1e) )

	ROM_REGION( 0x800, "micro", 0 ) // address decoder
	ROM_LOAD( "micro p3450 v1.3", 0x000, 0x800, CRC(0e69202e) SHA1(3b384951ff54c4b45a3a778a88966d13e2c9d57a) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *mshark_device::device_rom_region() const
{
	return ROM_NAME( mshark );
}


//-------------------------------------------------
//  ADDRESS_MAP( mshark_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( mshark_mem, AS_PROGRAM, 8, mshark_device )
	AM_RANGE(0x0000, 0x4fff) AM_ROM AM_REGION(I8085_TAG, 0)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( mshark_io )
//-------------------------------------------------

static ADDRESS_MAP_START( mshark_io, AS_IO, 8, mshark_device )
ADDRESS_MAP_END


//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( mshark )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( mshark )
	// basic machine hardware
	MCFG_CPU_ADD(I8085_TAG, I8085A, 1000000)
	MCFG_CPU_PROGRAM_MAP(mshark_mem)
	MCFG_CPU_IO_MAP(mshark_io)

	// devices
	MCFG_HARDDISK_ADD("harddisk1")
	MCFG_RS232_PORT_ADD(RS232_TAG, default_rs232_devices, nullptr)
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor mshark_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( mshark );
}


//-------------------------------------------------
//  INPUT_PORTS( mshark )
//-------------------------------------------------

INPUT_PORTS_START( mshark )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor mshark_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( mshark );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  mshark_device - constructor
//-------------------------------------------------

mshark_device::mshark_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, SHARK, "Mator SHARK", tag, owner, clock, "mshark", __FILE__),
		device_ieee488_interface(mconfig, *this),
		m_maincpu(*this, I8085_TAG)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mshark_device::device_start()
{
}
