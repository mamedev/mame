// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// SH7042, sh2 variant

#ifndef MAME_CPU_SH_SH7042_H
#define MAME_CPU_SH_SH7042_H

#pragma once

#include "sh2.h"

class sh7042_device : public sh2_device
{
public:
	sh7042_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	void map(address_map &map);
};

DECLARE_DEVICE_TYPE(SH7042, sh7042_device)

#endif // MAME_CPU_SH_SH7042_H
