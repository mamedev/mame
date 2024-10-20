// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Roland MB63H114 Multiple Address Counter

***************************************************************************/

#ifndef MAME_ROLAND_MB63H114_H
#define MAME_ROLAND_MB63H114_H

#pragma once

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> mb63h114_device

class mb63h114_device : public device_t
{
public:
	// device type constructor
	mb63h114_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// CPU write handler
	void xst_w(u8 data);

protected:
	// device-specific overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	// TODO
};


// device type declaration
DECLARE_DEVICE_TYPE(MB63H114, mb63h114_device)

#endif // MAME_ROLAND_MB63H114_H
