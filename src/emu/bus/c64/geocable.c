	// license:BSD-3-Clause
// copyright-holders:Curt Coder, smf
/**********************************************************************

    geoCable Centronics Cable emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "geocable.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define CENTRONICS_TAG "centronics"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type C64_GEOCABLE = &device_creator<c64_geocable_device>;


//-------------------------------------------------
//  centronics_interface centronics_intf
//-------------------------------------------------

WRITE_LINE_MEMBER( c64_geocable_device::busy_w )
{
	output_b(state);
}

static const centronics_interface centronics_intf =
{
	DEVCB_NULL,
	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF, c64_geocable_device, busy_w),
	DEVCB_NULL
};


//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( c64_geocable )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( c64_geocable )
	MCFG_CENTRONICS_PRINTER_ADD(CENTRONICS_TAG, centronics_intf)
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor c64_geocable_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( c64_geocable );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64_geocable_device - constructor
//-------------------------------------------------

c64_geocable_device::c64_geocable_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, C64_GEOCABLE, "C64 geoCable", tag, owner, clock, "c64_geocable", __FILE__),
	device_pet_user_port_interface(mconfig, *this),
	m_centronics(*this, CENTRONICS_TAG)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64_geocable_device::device_start()
{
}


//-------------------------------------------------
//  update_output
//-------------------------------------------------

void c64_geocable_device::update_output()
{
	m_centronics->write(m_parallel_output);
}


//-------------------------------------------------
//  input_8 - CIA2 PC write
//-------------------------------------------------

WRITE_LINE_MEMBER(c64_geocable_device::input_8)
{
	m_centronics->strobe_w(state);
}
