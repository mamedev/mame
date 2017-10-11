// license:BSD-3-Clause
// copyright-holders:AJR
/******************************************************************************

    Video System C7-01 GGA (Graphics Gate Array?)

    Skeleton device.

******************************************************************************/

#include "emu.h"
#include "vsystem_gga.h"


//**************************************************************************
//  VIDEO SYSTEM GGA DEVICE
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(VSYSTEM_GGA, vsystem_gga_device, "vsystem_gga", "Video System C7-01 GGA")

//-------------------------------------------------
//  vsystem_gga_device - constructor
//-------------------------------------------------

vsystem_gga_device::vsystem_gga_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, VSYSTEM_GGA, tag, owner, clock),
		device_video_interface(mconfig, *this, false),
		m_write_cb(*this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vsystem_gga_device::device_start()
{
	m_write_cb.resolve();

	m_address_latch = 0;
	for (u8 &reg : m_regs)
		reg = 0;

	save_item(NAME(m_address_latch));
	save_item(NAME(m_regs));
}

//-------------------------------------------------
//  write - register write handler
//-------------------------------------------------

WRITE8_MEMBER(vsystem_gga_device::write)
{
	if (offset & 1)
	{
		// address write
		m_address_latch = data & 0x0f;
	}
	else
	{
		// data write
		m_regs[m_address_latch] = data;
		if (m_write_cb.isnull())
			logerror("Setting register $%02x = %02x\n", m_address_latch, data);
		else
			m_write_cb(m_address_latch, data, 0xff);
	}
}
