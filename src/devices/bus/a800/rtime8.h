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
	a800_rtime8_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void cctl_map(address_map &map) override ATTR_COLD;

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

private:
	required_device<m3002_device> m_rtc;
};

DECLARE_DEVICE_TYPE(A800_RTIME8, a800_rtime8_device)

#endif // MAME_BUS_A800_RTIME8_H
