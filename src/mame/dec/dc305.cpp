// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    DEC DC305 Printer Controller

    This device normally operates at 2 MHz with one wait state. It
    generates two vectored interrupts.

***************************************************************************/

#include "emu.h"
#include "dc305.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(DC305, dc305_device, "dc305", "DC305 Printer Controller")


//**************************************************************************
//  DEVICE IMPLEMENTATION
//**************************************************************************

//-------------------------------------------------
//  dc305_device - constructor
//-------------------------------------------------

dc305_device::dc305_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, DC305, tag, owner, clock)
	, m_int_callback(*this)
	, m_rxc_callback(*this)
	, m_txc_callback(*this)
{
}


//-------------------------------------------------
//  device_resolve_objects - resolve objects that
//  may be needed for other devices to set
//  initial conditions at start time
//-------------------------------------------------

void dc305_device::device_resolve_objects()
{
	m_int_callback.resolve_safe();
	m_rxc_callback.resolve_safe();
	m_txc_callback.resolve_safe();
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void dc305_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void dc305_device::device_reset()
{
}


//-------------------------------------------------
//  read - read from internal register
//-------------------------------------------------

u8 dc305_device::read(offs_t offset)
{
	// TODO
	return 0;
}


//-------------------------------------------------
//  write - write to internal register
//-------------------------------------------------

void dc305_device::write(offs_t offset, u8 data)
{
	// TODO
}


//-------------------------------------------------
//  inta - acknowledge and vector interrupt
//-------------------------------------------------

u8 dc305_device::inta()
{
	// TODO: generate RST 5, RST 3 or RST 1 if both requested
	return 0xff;
}
