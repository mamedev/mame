// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_CPU_C33_S1C33L17_H
#define MAME_CPU_C33_S1C33L17_H

#pragma once

#include "c33std.h"

class s1c33l17_device : public c33std_cpu_device_base
{
public:
	s1c33l17_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	s1c33l17_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, address_map_constructor map);

	// device_t implementation
	virtual void device_reset() override ATTR_COLD;

	void base_internal_map(address_map &map);

private:
	void internal_map(address_map &map);
};

class s1c33e07_device : public s1c33l17_device
{
public:
	s1c33e07_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

private:
	void internal_map(address_map &map);
};

// device type declarations
DECLARE_DEVICE_TYPE(S1C33L17, s1c33l17_device)
DECLARE_DEVICE_TYPE(S1C33E07, s1c33e07_device)

#endif // MAME_CPU_C33_S1C33L17_H
