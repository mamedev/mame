// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_CPU_H8500_H8532_H
#define MAME_CPU_H8500_H8532_H

#pragma once

#include "h8500.h"

class h8532_device : public h8500_device
{
protected:
	h8532_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

private:
	void internal_map(address_map &map) ATTR_COLD;
};

class hd6435328_device : public h8532_device
{
public:
	// device type constructor
	hd6435328_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class hd6475328_device : public h8532_device
{
public:
	// device type constructor
	hd6475328_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

DECLARE_DEVICE_TYPE(HD6435328, hd6435328_device)
DECLARE_DEVICE_TYPE(HD6475328, hd6475328_device)

#endif // MAME_CPU_H8500_H8532_H
