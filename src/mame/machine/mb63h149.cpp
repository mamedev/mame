// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Roland MB63H149 gate array

    This ASIC was used to scan piano keyboards on several synthesizers.
    Two mode pins allow it to interface with various microcontroller types.
    It may generate the microcontroller clock, share one with it or be
    independently clocked. It also can write to and read from a 2048 x
    8-bit SRAM over a private bus.

    MB63H130 was an older version of MB63H149 with a slightly different
    pinout and no interrupt output.

***************************************************************************/

#include "emu.h"
#include "mb63h149.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definitions
DEFINE_DEVICE_TYPE(MB63H149, mb63h149_device, "mb63h149", "Roland MB63H149 Key Assigner")
DEFINE_DEVICE_TYPE(MB63H130, mb63h130_device, "mb63h130", "Roland MB63H130 Key Assigner")


//**************************************************************************
//  DEVICE IMPLEMENTATION
//**************************************************************************

//-------------------------------------------------
//  mb63h149_device - constructor
//-------------------------------------------------

mb63h149_device::mb63h149_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_int_callback(*this)
{
}

mb63h149_device::mb63h149_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: mb63h149_device(mconfig, MB63H149, tag, owner, clock)
{
}


//-------------------------------------------------
//  mb63h130_device - constructor
//-------------------------------------------------

mb63h130_device::mb63h130_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: mb63h149_device(mconfig, MB63H130, tag, owner, clock)
{
}


//-------------------------------------------------
//  device_resolve_objects - resolve objects that
//  may be needed for other devices to set
//  initial conditions at start time
//-------------------------------------------------

void mb63h149_device::device_resolve_objects()
{
	m_int_callback.resolve_safe();
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mb63h149_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mb63h149_device::device_reset()
{
}


//-------------------------------------------------
//  read - microcontroller read from gate array
//-------------------------------------------------

u8 mb63h149_device::read(offs_t offset)
{
	// TODO
	return 0;
}


//-------------------------------------------------
//  write - microcontroller write to gate array
//-------------------------------------------------

void mb63h149_device::write(offs_t offset, u8 data)
{
	// TODO
}
