// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Shugart SA1403D Winchester Disk Controller emulation

**********************************************************************/

#include "sa1403d.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type SA1403D = &device_creator<sa1403d_device>;


//-------------------------------------------------
//  ROM( sa1403d )
//-------------------------------------------------

ROM_START( sa1403d )
	ROM_REGION( 0x4000, "sa1403d", 0 )
	ROM_DEFAULT_BIOS( "as31" )
	ROM_SYSTEM_BIOS( 0, "as30", "AS30" )
	ROMX_LOAD( "i",   0x0000, 0x1000, NO_DUMP, ROM_BIOS(1) )
	ROMX_LOAD( "ii",  0x1000, 0x1000, NO_DUMP, ROM_BIOS(1) )
	ROMX_LOAD( "iii", 0x2000, 0x1000, NO_DUMP, ROM_BIOS(1) )
	ROMX_LOAD( "iv",  0x3000, 0x1000, NO_DUMP, ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "as31", "AS31" )
	ROMX_LOAD( "i",   0x0000, 0x1000, NO_DUMP, ROM_BIOS(2) )
	ROMX_LOAD( "ii",  0x1000, 0x1000, NO_DUMP, ROM_BIOS(2) )
	ROMX_LOAD( "iii", 0x2000, 0x1000, NO_DUMP, ROM_BIOS(2) )
	ROMX_LOAD( "iv",  0x3000, 0x1000, NO_DUMP, ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 2, "u50", "Diagnostic PROM set 12668" )
	ROMX_LOAD( "i",   0x0000, 0x1000, NO_DUMP, ROM_BIOS(3) )
	ROMX_LOAD( "ii",  0x1000, 0x1000, NO_DUMP, ROM_BIOS(3) )
	ROMX_LOAD( "iii", 0x2000, 0x1000, NO_DUMP, ROM_BIOS(3) )
	ROMX_LOAD( "iv",  0x3000, 0x1000, NO_DUMP, ROM_BIOS(3) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *sa1403d_device::device_rom_region() const
{
	return ROM_NAME( sa1403d );
}


//-------------------------------------------------
//  MACHINE_DRIVER( sa1403d )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( sa1403d )
	MCFG_HARDDISK_ADD("image")
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor sa1403d_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( sa1403d );
}


//-------------------------------------------------
//  INPUT_PORTS( sa1403d )
//-------------------------------------------------

INPUT_PORTS_START( sa1403d )
	PORT_INCLUDE(scsihle)

	PORT_START("2H")
	PORT_DIPNAME( 0xc0, 0x40, "LUN 0 Drive Type" ) PORT_DIPLOCATION("2H:7,8")
	PORT_DIPSETTING(    0x00, "SA1002" ) // 2 heads, 256 cylinders
	PORT_DIPSETTING(    0x40, "SA1004" ) // 4 heads, 256 cylinders
	PORT_DIPSETTING(    0x80, "SA800" ) // 1 head, 77 cylinders
	PORT_DIPSETTING(    0xc0, "SA850" ) // 2 heads, 77 cylinders
	PORT_DIPNAME( 0x30, 0x30, "LUN 1 Drive Type" ) PORT_DIPLOCATION("2H:5,6")
	PORT_DIPSETTING(    0x00, "SA1002" )
	PORT_DIPSETTING(    0x10, "SA1004" )
	PORT_DIPSETTING(    0x20, "SA800" )
	PORT_DIPSETTING(    0x30, "SA850" )
	PORT_DIPNAME( 0x0c, 0x0c, "LUN 2 Drive Type" ) PORT_DIPLOCATION("2H:3,4")
	PORT_DIPSETTING(    0x00, "SA1002" )
	PORT_DIPSETTING(    0x04, "SA1004" )
	PORT_DIPSETTING(    0x08, "SA800" )
	PORT_DIPSETTING(    0x0c, "SA850" )
	PORT_DIPNAME( 0x03, 0x03, "LUN 3 Drive Type" ) PORT_DIPLOCATION("2H:1,2")
	PORT_DIPSETTING(    0x00, "SA1002" )
	PORT_DIPSETTING(    0x01, "SA1004" )
	PORT_DIPSETTING(    0x02, "SA800" )
	PORT_DIPSETTING(    0x03, "SA850" )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor sa1403d_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( sa1403d );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  sa1403d_device - constructor
//-------------------------------------------------

sa1403d_device::sa1403d_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: scsihd_device(mconfig, SA1403D, "Shugart SA1403D", tag, owner, clock, "sa1403d", __FILE__)
{
}

void sa1403d_device::ExecCommand()
{
	switch( command[ 0 ] )
	{
	default:
		scsihd_device::ExecCommand();
		break;
	}
}

void sa1403d_device::WriteData( UINT8 *data, int dataLength )
{
	switch( command[ 0 ] )
	{
	default:
		scsihd_device::WriteData( data, dataLength );
		break;
	}
}
