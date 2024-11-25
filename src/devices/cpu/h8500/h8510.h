// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_CPU_H8500_H8510_H
#define MAME_CPU_H8500_H8510_H

#pragma once

#include "h8500.h"

class h8510_device : public h8500_device
{
protected:
	h8510_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

private:
	void internal_map(address_map &map) ATTR_COLD;
};

class hd6415108_device : public h8510_device
{
public:
	// device type constructor
	hd6415108_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

DECLARE_DEVICE_TYPE(HD6415108, hd6415108_device)

#endif // MAME_CPU_H8500_H8510_H
