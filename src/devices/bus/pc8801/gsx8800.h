// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#ifndef MAME_BUS_PC8801_GSX8800_H
#define MAME_BUS_PC8801_GSX8800_H

#pragma once

#include "pc8801_exp.h"
#include "sound/ay8910.h"

class gsx8800_device : public pc8801_exp_device
{
public:
	gsx8800_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static constexpr feature_type unemulated_features() { return feature::SOUND; }

	virtual void io_map(address_map &map) override ATTR_COLD;

private:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	required_device_array<ym2149_device, 2> m_psg;
};

DECLARE_DEVICE_TYPE(GSX8800, gsx8800_device)


#endif // MAME_BUS_PC8801_GSX8800_H
