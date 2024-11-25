// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_MACHINE_LC89510_H
#define MAME_MACHINE_LC89510_H

#pragma once

class lc89510_device : public device_t
{
public:
	lc89510_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
};


DECLARE_DEVICE_TYPE(LC89510, lc89510_device)

#endif // MAME_MACHINE_LC89510_H
