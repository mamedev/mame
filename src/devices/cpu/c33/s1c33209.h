// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/*
    Epson S1C33209/221/222 CMOS 32-bit single chip microcomputer
*/

#ifndef MAME_CPU_C33_S2C33209_H
#define MAME_CPU_C33_S2C33209_H

#pragma once

#include "c33std.h"


class s1c33209_device : public c33std_cpu_device_base
{
public:
	s1c33209_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

protected:
	s1c33209_device(
			machine_config const &mconfig,
			device_type type,
			char const *tag,
			device_t *owner,
			u32 clock,
			address_map_constructor internal_map);

	template <offs_t RomBytes> void memory_map(address_map &map) ATTR_COLD;

protected:
	void device_reset() override ATTR_COLD;

private:
	void peripheral_map(address_map &map) ATTR_COLD;
};


class s1c33221_device : public s1c33209_device
{
public:
	s1c33221_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);
};


class s1c33222_device : public s1c33209_device
{
public:
	s1c33222_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);
};


DECLARE_DEVICE_TYPE(S1C33209, s1c33209_device)
DECLARE_DEVICE_TYPE(S1C33221, s1c33221_device)
DECLARE_DEVICE_TYPE(S1C33222, s1c33222_device)

#endif // MAME_CPU_C33_S2C33209_H
