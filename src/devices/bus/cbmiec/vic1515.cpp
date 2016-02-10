// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore VIC-1515 Printer emulation

**********************************************************************/

#include "vic1515.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type VIC1515 = &device_creator<vic1515_t>;


//-------------------------------------------------
//  ROM( vic1515 )
//-------------------------------------------------

ROM_START( vic1515 )
	ROM_REGION( 0x1000, "rom", 0 )
	ROM_LOAD( "805-5.p4", 0x0000, 0x1000, CRC(05a99a5a) SHA1(035c23dc83923eea34feea260445356a909fbd98) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *vic1515_t::device_rom_region() const
{
	return ROM_NAME( vic1515 );
}


//-------------------------------------------------
//  ADDRESS_MAP( vic1515_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( vic1515_mem, AS_PROGRAM, 8, vic1515_t )
	AM_RANGE(0x0000, 0x0fff) AM_ROM AM_REGION("rom", 0)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( vic1515_io )
//-------------------------------------------------

static ADDRESS_MAP_START( vic1515_io, AS_IO, 8, vic1515_t )
ADDRESS_MAP_END


//-------------------------------------------------
//  MACHINE_DRIVER( vic1515 )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( vic1515 )
	MCFG_CPU_ADD("maincpu", I8039, XTAL_6MHz)
	MCFG_CPU_PROGRAM_MAP(vic1515_mem)
	MCFG_CPU_IO_MAP(vic1515_io)
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor vic1515_t::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( vic1515 );
}


//-------------------------------------------------
//  INPUT_PORTS( vic1515 )
//-------------------------------------------------

static INPUT_PORTS_START( vic1515 )
	PORT_START("ADDRESS")
	PORT_DIPNAME( 0x03, 0x00, "Device Address" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x02, "T" )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor vic1515_t::device_input_ports() const
{
	return INPUT_PORTS_NAME( vic1515 );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  vic1515_t - constructor
//-------------------------------------------------

vic1515_t::vic1515_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, VIC1515, "VIC-1515", tag, owner, clock, "vic1515", __FILE__),
	device_cbm_iec_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vic1515_t::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void vic1515_t::device_reset()
{
}


//-------------------------------------------------
//  cbm_iec_atn -
//-------------------------------------------------

void vic1515_t::cbm_iec_atn(int state)
{
}


//-------------------------------------------------
//  cbm_iec_data -
//-------------------------------------------------

void vic1515_t::cbm_iec_data(int state)
{
}


//-------------------------------------------------
//  cbm_iec_reset -
//-------------------------------------------------

void vic1515_t::cbm_iec_reset(int state)
{
	if (!state)
	{
		device_reset();
	}
}
