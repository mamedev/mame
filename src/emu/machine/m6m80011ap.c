/***************************************************************************

Template for skeleton device

***************************************************************************/

#include "emu.h"
#include "machine/m6m80011ap.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
const device_type M6M80011AP = &device_creator<m6m80011ap_device>;


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  m6m80011ap_device - constructor
//-------------------------------------------------

m6m80011ap_device::m6m80011ap_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, M6M80011AP, "m6m80011ap", tag, owner, clock)
{

}


//-------------------------------------------------
//  device_validity_check - perform validity checks
//  on this device
//-------------------------------------------------

void m6m80011ap_device::device_validity_check(validity_checker &valid) const
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void m6m80011ap_device::device_start()
{

}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void m6m80011ap_device::device_reset()
{
}


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************


READ_LINE_MEMBER( m6m80011ap_device::read_bit )
{
	return 0;
}

READ_LINE_MEMBER( m6m80011ap_device::ready_line )
{
	return 1; // TODO
}

WRITE_LINE_MEMBER( m6m80011ap_device::set_cs_line )
{

}

WRITE_LINE_MEMBER( m6m80011ap_device::set_clock_line )
{

}

WRITE_LINE_MEMBER( m6m80011ap_device::write_bit )
{

}
