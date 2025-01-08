// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_CPU_H8500_H8534_H
#define MAME_CPU_H8500_H8534_H

#pragma once

#include "h8500.h"

class h8534_device : public h8500_device
{
protected:
	// delegating constructors
	h8534_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);
	h8534_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, address_map_constructor map);

	void register_field_map(address_map &map) ATTR_COLD;

private:
	void internal_map(address_map &map) ATTR_COLD;
};

class hd6435348_device : public h8534_device
{
public:
	// device type constructor
	hd6435348_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class hd6475348_device : public h8534_device
{
public:
	// device type constructor
	hd6475348_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class h8536_device : public h8534_device
{
protected:
	h8536_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

private:
	void internal_map(address_map &map) ATTR_COLD;
};

class hd6435368_device : public h8536_device
{
public:
	// device type constructor
	hd6435368_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class hd6475368_device : public h8536_device
{
public:
	// device type constructor
	hd6475368_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

DECLARE_DEVICE_TYPE(HD6435348, hd6435348_device)
DECLARE_DEVICE_TYPE(HD6475348, hd6475348_device)
DECLARE_DEVICE_TYPE(HD6435368, hd6435368_device)
DECLARE_DEVICE_TYPE(HD6475368, hd6475368_device)

#endif // MAME_CPU_H8500_H8534_H
