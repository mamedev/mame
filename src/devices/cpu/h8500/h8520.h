// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_CPU_H8500_H8520_H
#define MAME_CPU_H8500_H8520_H

#pragma once

#include "h8500.h"

class h8520_device : public h8500_device
{
protected:
	h8520_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

private:
	void internal_map(address_map &map) ATTR_COLD;
};

class hd6435208_device : public h8520_device
{
public:
	// device type constructor
	hd6435208_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class hd6475208_device : public h8520_device
{
public:
	// device type constructor
	hd6475208_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

DECLARE_DEVICE_TYPE(HD6435208, hd6435208_device)
DECLARE_DEVICE_TYPE(HD6475208, hd6475208_device)

#endif // MAME_CPU_H8500_H8520_H
