// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Roland RF5C36 (15229840) & SA-16 (15229874) Sampler Custom ICs

    Skeleton devices.

    Waveform data is 12 bits, and is normally stored in DRAM banks, though
    at least one Roland product also uses ROMs. 16-bit output can be
    connected directly to a PCM54 or MD6209 DAC or conditioned through a
    MB654419 TVF interface.

    Sampling rate is either 30kHz or 15kHz.

****************************************************************************/

#include "emu.h"
#include "sa16.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definitions
DEFINE_DEVICE_TYPE(RF5C36, rf5c36_device, "rf5c36", "Roland RF5C36 Sampler")
DEFINE_DEVICE_TYPE(SA16, sa16_device, "sa16", "Roland SA-16 Sampler")


//**************************************************************************
//  DEVICE IMPLEMENTATION
//**************************************************************************

//-------------------------------------------------
//  sa16_base_device - constructor
//-------------------------------------------------

sa16_base_device::sa16_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_int_callback(*this)
	, m_sh_callback(*this)
{
}


//-------------------------------------------------
//  rf5c36_device - constructor
//-------------------------------------------------

rf5c36_device::rf5c36_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: sa16_base_device(mconfig, RF5C36, tag, owner, clock)
{
}


//-------------------------------------------------
//  sa16_device - constructor
//-------------------------------------------------

sa16_device::sa16_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: sa16_base_device(mconfig, SA16, tag, owner, clock)
{
}


//-------------------------------------------------
//  device_resolve_objects - resolve objects that
//  may be needed for other devices to set
//  initial conditions at start time
//-------------------------------------------------

void sa16_base_device::device_resolve_objects()
{
	m_int_callback.resolve_safe();
	m_sh_callback.resolve_safe();
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sa16_base_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void sa16_base_device::device_reset()
{
}


//-------------------------------------------------
//  read - read data to CPU bus
//-------------------------------------------------

u8 sa16_base_device::read(offs_t offset)
{
	return 0;
}


//-------------------------------------------------
//  write - write data from CPU bus
//-------------------------------------------------

void sa16_base_device::write(offs_t offset, u8 data)
{
}
