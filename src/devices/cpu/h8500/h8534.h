// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_CPU_H8500_H8534_H
#define MAME_CPU_H8500_H8534_H

#pragma once

#include "h8500.h"

class h8534_device : public h8500_device
{
protected:
	h8534_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

private:
	void internal_map(address_map &map);
};

class hd6475348_device : public h8534_device
{
public:
	// device type constructor
	hd6475348_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

DECLARE_DEVICE_TYPE(HD6475348, hd6475348_device)

#endif // MAME_CPU_H8500_H8532_H
