// license:BSD-3-Clause
// copyright-holders:Felipe Sanches

#ifndef MAME_ROLAND_PG1000_H
#define MAME_ROLAND_PG1000_H

#include "cpu/upd7810/upd7810.h"


class pg1000_device : public device_t
{
public:
	pg1000_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	required_device<upd7810_device> m_pgcpu;
};

DECLARE_DEVICE_TYPE(PG1000, pg1000_device)

#endif // MAME_ROLAND_PG1000_H
