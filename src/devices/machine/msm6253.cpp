// license:BSD-3-Clause
// copyright-holders: AJR
/**********************************************************************

    OKI MSM6253 8-Bit 4-Channel A/D Converter

**********************************************************************/

#include "emu.h"
#include "machine/msm6253.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(MSM6253, msm6253_device, "msm6253", "OKI MSM6253 A/D Converter")

//**************************************************************************
//  DEVICE DEFINITION
//**************************************************************************

//-------------------------------------------------
//  msm6253_device - constructor
//-------------------------------------------------

msm6253_device::msm6253_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, MSM6253, tag, owner, clock)
	, m_analog_input_cb{{*this}, {*this}, {*this}, {*this}}
	, m_shift_register(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void msm6253_device::device_start()
{
	for (int port = 0; port < 4; port++)
		m_analog_input_cb[port].resolve_safe(0);

	// save our state
	save_item(NAME(m_shift_register));
}

//-------------------------------------------------
//  address_w - write from address bus to select
//  one of four internal latches
//-------------------------------------------------

WRITE8_MEMBER(msm6253_device::address_w)
{
	// fill the shift register from the internal A/D latch
	m_shift_register = m_analog_input_cb[offset & 3]();
}

//-------------------------------------------------
//  select_w - write D0/D1 to address latch
//-------------------------------------------------

WRITE8_MEMBER(msm6253_device::select_w)
{
	// fill the shift register from the internal A/D latch
	m_shift_register = m_analog_input_cb[data & 3]();
}

//-------------------------------------------------
//  shift_out - MSB-first serial data output
//-------------------------------------------------

bool msm6253_device::shift_out()
{
	// capture the shifted bit
	bool msb = BIT(m_shift_register, 7);

	// shift the bit out, with zero coming in on the other end
	if (!machine().side_effects_disabled())
		m_shift_register <<= 1;

	// return the bit
	return msb;
}

//-------------------------------------------------
//  d0_r - shift data bit out to D0
//-------------------------------------------------

READ8_MEMBER(msm6253_device::d0_r)
{
	// offset is ignored
	return shift_out() | (space.unmap() & 0xfe);
}

//-------------------------------------------------
//  d7_r - shift data bit out to D7
//-------------------------------------------------

READ8_MEMBER(msm6253_device::d7_r)
{
	// offset is ignored
	return (shift_out() << 7) | (space.unmap() & 0x7f);
}
