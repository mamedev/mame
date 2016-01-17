// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Diag264 Serial Loop Back Connector emulation

**********************************************************************/

#include "diag264_lb_iec.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type DIAG264_SERIAL_LOOPBACK = &device_creator<diag264_serial_loopback_device>;



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  diag264_serial_loopback_device - constructor
//-------------------------------------------------

diag264_serial_loopback_device::diag264_serial_loopback_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, DIAG264_SERIAL_LOOPBACK, "Diag264 Serial Loopback", tag, owner, clock, "diag264_serial_loopback", __FILE__),
		device_cbm_iec_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void diag264_serial_loopback_device::device_start()
{
}


//-------------------------------------------------
//  cbm_iec_atn -
//-------------------------------------------------

void diag264_serial_loopback_device::cbm_iec_atn(int state)
{
	m_bus->clk_w(state);
}
