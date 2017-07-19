// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_MACHINE_GAELCODS5002FP_H
#define MAME_MACHINE_GAELCODS5002FP_H

#pragma once

#include "cpu/mcs51/mcs51.h"
#include "machine/nvram.h"

DECLARE_DEVICE_TYPE(GAELCO_DS5002FP,       gaelco_ds5002fp_device)
DECLARE_DEVICE_TYPE(GAELCO_DS5002FP_WRALLY, gaelco_ds5002fp_wrally_device)

class gaelco_ds5002fp_device_base : public device_t
{
public:
	required_shared_ptr<uint16_t> m_shareram;
	required_region_ptr<uint8_t> m_mcu_ram;

	DECLARE_READ8_MEMBER(dallas_ram_r);
	DECLARE_WRITE8_MEMBER(dallas_ram_w);
	DECLARE_READ8_MEMBER(dallas_share_r);
	DECLARE_WRITE8_MEMBER(dallas_share_w);

protected:
	gaelco_ds5002fp_device_base(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock);

	virtual void device_start() override;
	virtual void device_reset() override;

private:

};


class gaelco_ds5002fp_device : public gaelco_ds5002fp_device_base
{
public:
	gaelco_ds5002fp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual void device_add_mconfig(machine_config &config) override;

protected:
	gaelco_ds5002fp_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);
};


class gaelco_ds5002fp_wrally_device : public gaelco_ds5002fp_device
{
public:
	gaelco_ds5002fp_wrally_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	virtual void device_add_mconfig(machine_config &config) override;

protected:
};

#endif // MAME_MACHINE_GAELCODS5002FP_H
