// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#ifndef MAME_BUS_MZ80_MZ1E35_H
#define MAME_BUS_MZ80_MZ1E35_H

#pragma once

#include "mz80_exp.h"
#include "sound/ymopl.h"

class mz1e35_device : public mz80_exp_device
{
public:
	mz1e35_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static constexpr feature_type unemulated_features() { return feature::SOUND; }

	virtual void io_map(address_map &map) override ATTR_COLD;

private:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	required_device<y8950_device> m_opl;
};

DECLARE_DEVICE_TYPE(MZ1E35, mz1e35_device)


#endif // MAME_BUS_MZ80_MZ1E35_H
