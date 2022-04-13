// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_MACHINE_PG200_H
#define MAME_MACHINE_PG200_H

#include "cpu/mcs48/mcs48.h"


class pg200_device : public device_t
{
public:
	pg200_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

protected:
	virtual void device_start() override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

private:
	required_device<mcs48_cpu_device> m_pgcpu;
};

DECLARE_DEVICE_TYPE(PG200, pg200_device)

#endif // MAME_MACHINE_PG200_H
