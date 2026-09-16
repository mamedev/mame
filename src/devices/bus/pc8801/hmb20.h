// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#ifndef MAME_BUS_PC8801_HMB20_H
#define MAME_BUS_PC8801_HMB20_H

#pragma once

#include "pc8801_exp.h"
#include "sound/ymopm.h"

class hmb20_device : public pc8801_exp_device
{
public:
	hmb20_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void io_map(address_map &map) override ATTR_COLD;

private:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	required_device<ym2151_device> m_opm;
};

DECLARE_DEVICE_TYPE(HMB20, hmb20_device)


#endif // MAME_BUS_PC8801_HMB20_H
