// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#ifndef MAME_BUS_FP1030_RAMPACK_H
#define MAME_BUS_FP1030_RAMPACK_H

#pragma once

#include "fp1060io_exp.h"
#include "machine/nvram.h"

class fp1030_rampack_device : public fp1060io_exp_device
{
public:
	fp1030_rampack_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void io_map(address_map &map) override ATTR_COLD;
	virtual u8 get_id() override { return 0x01; };

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	required_device<nvram_device> m_nvram;
	std::unique_ptr<uint8_t[]> m_nvram_ptr;
};

DECLARE_DEVICE_TYPE(FP1030_RAMPACK, fp1030_rampack_device)


#endif // MAME_BUS_FP1030_RAMPACK_H
