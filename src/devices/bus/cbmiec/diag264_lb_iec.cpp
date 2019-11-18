// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Diag264 Serial Loop Back Connector emulation

**********************************************************************/

#include "emu.h"
#include "diag264_lb_iec.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(DIAG264_SERIAL_LOOPBACK, diag264_serial_loopback_device, "diag264_serial_loopback", "Diag264 Serial Loopback")



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  diag264_serial_loopback_device - constructor
//-------------------------------------------------

diag264_serial_loopback_device::diag264_serial_loopback_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, DIAG264_SERIAL_LOOPBACK, tag, owner, clock)
	, device_cbm_iec_interface(mconfig, *this)
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
	m_bus->host_clk_w(state);
}
