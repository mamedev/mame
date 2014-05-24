// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Sandy SuperQBoard emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "sandy_superqboard.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type SANDY_SUPERQBOARD = &device_creator<sandy_superqboard_t>;


//-------------------------------------------------
//  ROM( sandy_superqboard )
//-------------------------------------------------

ROM_START( sandy_superqboard )
	ROM_REGION( 0x8000, "rom", 0 )
	ROM_LOAD( "sandy_disk_controller_v1.18y_1984.rom", 0x0000, 0x8000, CRC(d02425be) SHA1(e730576e3e0c6a1acad042c09e15fc62a32d8fbd) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *sandy_superqboard_t::device_rom_region() const
{
	return ROM_NAME( sandy_superqboard );
}


//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( sandy_superqboard )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( sandy_superqboard )
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor sandy_superqboard_t::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( sandy_superqboard );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  sandy_superqboard_t - constructor
//-------------------------------------------------

sandy_superqboard_t::sandy_superqboard_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, SANDY_SUPERQBOARD, "SANDY_SUPERQBOARD", tag, owner, clock, "sandy_superqboard", __FILE__),
	device_ql_expansion_card_interface(mconfig, *this),
	m_rom(*this, "rom")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sandy_superqboard_t::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void sandy_superqboard_t::device_reset()
{
}
