// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    AMD Am8052 Alphanumeric CRT Controller (CRTC)

    Skeleton device.

**********************************************************************/

#include "emu.h"
#include "am8052.h"
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
	, m_pointer(0x1f)
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
	m_pointer = data & 0x1f;
}


//-------------------------------------------------
//  data_w - write to data register
//-------------------------------------------------

void am8052_device::data_w(u16 data)
{
	switch (m_pointer)
	{
	case 0x00:
		logerror("%s: Mode Register 1 = %04X\n", machine().describe_context(), data);
		break;

	case 0x01:
		logerror("%s: Mode Register 2 = %04X\n", machine().describe_context(), data & 0xb7ff);
		break;

	case 0x02:
		logerror("%s: Attribute Port Enable Register = %04X\n", machine().describe_context(), data & 0x67ff);
		break;

	case 0x03:
		logerror("%s: Attribute Redefinition Register = %02X\n", machine().describe_context(), data & 0x001f);
		break;

	case 0x04:
		logerror("%s: Top Of Page Soft Register (HI) = %02X:%02X\n", machine().describe_context(), (data & 0x7f00) >> 8, data & 0x00ff);
		break;

	case 0x05:
		logerror("%s: Top Of Page Soft Register (LO) = %04X\n", machine().describe_context(), data);
		break;

	case 0x06:
		logerror("%s: Top Of Window Soft Register (HI) = %02X:%02X\n", machine().describe_context(), (data & 0x7f00) >> 8, data & 0x00ff);
		break;

	case 0x07:
		logerror("%s: Top Of Window Soft Register (LO) = %04X\n", machine().describe_context(), data);
		break;

	case 0x08:
		logerror("%s: Attribute Flag Register = %02X (value) & %02X (mask)\n", machine().describe_context(), data & 0x00ff, (data & 0xff00) >> 8);
		break;

	case 0x09:
		logerror("%s: Top Of Page Hard Register (HI) = %02X:%02X\n", machine().describe_context(), (data & 0x7f00) >> 8, data & 0x00ff);
		break;

	case 0x0a:
		logerror("%s: Top Of Page Hard Register (LO) = %04X\n", machine().describe_context(), data);
		break;

	case 0x0b:
		logerror("%s: Top Of Window Hard Register (HI) = %02X:%02X\n", machine().describe_context(), (data & 0x7f00) >> 8, data & 0x00ff);
		break;

	case 0x0c:
		logerror("%s: Top Of Window Hard Register (LO) = %04X\n", machine().describe_context(), data);
		break;

	case 0x10:
		logerror("%s: Burst Space Register = %02X\n", machine().describe_context(), (data & 0xff00) >> 8);
		logerror("%s: Burst Count Register = %02X\n", machine().describe_context(), data & 0x00ff);
		break;

	case 0x11:
		logerror("%s: Vertical Sync Width Register = %02X\n", machine().describe_context(), data & 0x003f);
		logerror("%s: Vertical Scan Delay Register = %02X\n", machine().describe_context(), (data & 0x0fc0) >> 6);
		break;

	case 0x12:
		logerror("%s: Vertical Active Lines Register = %03X\n", machine().describe_context(), data & 0x0fff);
		break;

	case 0x13:
		logerror("%s: Vertical Total Lines Register = %03X\n", machine().describe_context(), data & 0x0fff);
		break;

	case 0x14:
		logerror("%s: Horizontal Sync Width Register = %02X\n", machine().describe_context(), data & 0x00ff);
		logerror("%s: Vertical Interrupt Register = %02X\n", machine().describe_context(), (data & 0xff00) >> 8);
		break;

	case 0x15:
		logerror("%s: Horizontal Drive Register = %03X\n", machine().describe_context(), data & 0x01ff);
		break;

	case 0x16:
		logerror("%s: Horizontal Scan Delay Register = %03X\n", machine().describe_context(), data & 0x01ff);
		break;

	case 0x17:
		logerror("%s: Horizontal Total Count Register = %03X\n", machine().describe_context(), data & 0x03ff);
		break;

	case 0x18:
		logerror("%s: Horizontal Total Display Register = %03X\n", machine().describe_context(), data & 0x03ff);
		break;

	default:
		logerror("%s: Unknown Register (%02X) = %04X\n", machine().describe_context(), m_pointer, data);
		break;
	}
}
