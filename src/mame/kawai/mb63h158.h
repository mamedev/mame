// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Kawai MB63H158 Touch Sensor

***************************************************************************/

#ifndef MAME_KAWAI_MB63H158_H
#define MAME_KAWAI_MB63H158_H

#pragma once

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> mb63h114_device

class mb63h158_device : public device_t
{
public:
	// device type constructor
	mb63h158_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// CPU read handler
	u8 read(offs_t offset);

protected:
	// device-specific overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	// TODO
};


// device type declaration
DECLARE_DEVICE_TYPE(MB63H158, mb63h158_device)

#endif // MAME_KAWAI_MB63H158_H
