/**********************************************************************

    geoCable Centronics Cable emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "c64_geocable.h"



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
	m_slot->flag2_w(state);
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
	device_t(mconfig, C64_GEOCABLE, "C64 geoCable", tag, owner, clock),
	device_c64_user_port_interface(mconfig, *this),
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
//  c64_pb_w - port B write
//-------------------------------------------------

void c64_geocable_device::c64_pb_w(address_space &space, offs_t offset, UINT8 data)
{
	m_centronics->write(space, 0, data);
}


//-------------------------------------------------
//  c64_pb_w - port B write
//-------------------------------------------------

void c64_geocable_device::c64_pa2_w(int level)
{
	m_centronics->strobe_w(level);
}
