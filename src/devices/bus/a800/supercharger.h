// license: BSD-3-Clause
// copyright-holders: Angelo Salese

#ifndef MAME_BUS_A800_SUPERCHARGER_H
#define MAME_BUS_A800_SUPERCHARGER_H

#pragma once

#include "a800_slot.h"


// ======================> a800_rtime8_device

class a800_supercharger_device : public device_t,
								 public device_a800_cart_interface
{
public:
	a800_supercharger_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void cctl_map(address_map &map) override ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	u8 m_data[3]{};
	u8 m_status;

	u8 status_r(offs_t offset);
	void command_w(offs_t offset, u8 data);
};

DECLARE_DEVICE_TYPE(A800_SUPER_CHARGER, a800_supercharger_device)

#endif // MAME_BUS_A800_SUPERCHARGER_H
