// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_BUS_A800_RTIME8_H
#define MAME_BUS_A800_RTIME8_H

#pragma once

#include "a800_slot.h"
#include "machine/m3002.h"


// ======================> a800_rtime8_device

class a800_rtime8_device : public device_t, public device_a800_cart_interface
{
public:
	// construction/destruction
	a800_rtime8_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;

	// device_a800_cart_interface overrides
	virtual u8 read_d5xx(offs_t offset) override;
	virtual void write_d5xx(offs_t offset, u8 data) override;

private:
	required_device<m3002_device> m_rtc;
};

// device type declaration
DECLARE_DEVICE_TYPE(A800_RTIME8, a800_rtime8_device)

#endif // MAME_BUS_A800_RTIME8_H
