// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    AMD Am8052 Alphanumeric CRT Controller (CRTC)

**********************************************************************/

#ifndef MAME_VIDEO_AM8052_H
#define MAME_VIDEO_AM8052_H

#pragma once

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> am8052_device

class am8052_device : public device_t
{
public:
	// device constructor
	am8052_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// read/write handlers
	void pointer_w(u16 data);
	void data_w(u16 data);

protected:
	// device-specific overrides
	virtual void device_start() override ATTR_COLD;

private:
	// internal state
	u8 m_pointer;
};

// device type declarations
DECLARE_DEVICE_TYPE(AM8052, am8052_device)

#endif // MAME_VIDEO_AM8052_H
