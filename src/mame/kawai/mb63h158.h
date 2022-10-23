// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Kawai MB63H158 Touch Sensor

***************************************************************************/

#ifndef MAME_MACHINE_MB63H158_H
#define MAME_MACHINE_MB63H158_H

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
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	// TODO
};


// device type declaration
DECLARE_DEVICE_TYPE(MB63H158, mb63h158_device)

#endif // MAME_MACHINE_MB63H158_H
