// license:BSD-3-Clause
// copyright-holders:Quench, Yochizo, David Haywood

#ifndef MAME_TOAPLAN_TOAPLAN_COINCOUNTER_H
#define MAME_TOAPLAN_TOAPLAN_COINCOUNTER_H

#pragma once

class toaplan_coincounter_device : public device_t
{
public:
	toaplan_coincounter_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	void coin_w(u8 data);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
};

DECLARE_DEVICE_TYPE(TOAPLAN_COINCOUNTER, toaplan_coincounter_device)

#endif // MAME_TOAPLAN_TOAPLAN_COINCOUNTER_H
