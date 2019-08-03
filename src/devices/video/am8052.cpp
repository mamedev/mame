// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    AMD Am8052 Alphanumeric CRT Controller (CRTC)

    Skeleton device.

**********************************************************************/

#include "emu.h"
#include "video/am8052.h"
//#include "screen.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(AM8052, am8052_device, "am8052", "Am8052 CRTC")

//**************************************************************************
//  DEVICE IMPLEMENTATION
//**************************************************************************

//-------------------------------------------------
//  am8052_device - constructor
//-------------------------------------------------

am8052_device::am8052_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, AM8052, tag, owner, clock)
	, m_pointer(0xffff)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void am8052_device::device_start()
{
	save_item(NAME(m_pointer));
}


//-------------------------------------------------
//  pointer_w - set pointer register
//-------------------------------------------------

void am8052_device::pointer_w(u16 data)
{
	m_pointer = data;
}


//-------------------------------------------------
//  data_w - write to data register
//-------------------------------------------------

void am8052_device::data_w(u16 data)
{
	logerror("%s: Register %04X = %04X\n", machine().describe_context(), m_pointer, data);
}
