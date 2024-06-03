// license:BSD-3-Clause
// copyright-holders:

#ifndef MAME_SEGA_MEGALO50_DASS_H
#define MAME_SEGA_MEGALO50_DASS_H

#pragma once

#include "cpu/z80/z80.h"
#include "sound/ymopl.h"

DECLARE_DEVICE_TYPE(MEGALO50_DASS, m50dass_device)

class m50dass_device : public device_t
{
public:
	m50dass_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);
	void m50dass(machine_config &config);

protected:
	virtual void device_start() override;
	virtual void device_add_mconfig(machine_config &config) override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<ym2413_device> m_ym2413;
};

#endif // MAME_SEGA_MEGALO50_DASS_H
