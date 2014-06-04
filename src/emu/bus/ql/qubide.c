// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Qubbesoft QubIDE emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "qubide.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type QUBIDE = &device_creator<qubide_t>;


//-------------------------------------------------
//  ROM( qubide )
//-------------------------------------------------

ROM_START( qubide )
	ROM_REGION( 0x4000, "rom", 0 )
	ROM_DEFAULT_BIOS("v201")
	ROM_SYSTEM_BIOS( 0, "v201", "v2.01" )
	ROMX_LOAD( "qb201_16k.rom", 0x0000, 0x4000, CRC(6f1d62a6) SHA1(1708d85397422e2024daa1a3406cac685f46730d), ROM_BIOS(1) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *qubide_t::device_rom_region() const
{
	return ROM_NAME( qubide );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  qubide_t - constructor
//-------------------------------------------------

qubide_t::qubide_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, QUBIDE, "QubIDE", tag, owner, clock, "ql_qubide", __FILE__),
	device_ql_expansion_card_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void qubide_t::device_start()
{
}


//-------------------------------------------------
//  read -
//-------------------------------------------------

UINT8 qubide_t::read(address_space &space, offs_t offset, UINT8 data)
{
	return data;
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

void qubide_t::write(address_space &space, offs_t offset, UINT8 data)
{
}
