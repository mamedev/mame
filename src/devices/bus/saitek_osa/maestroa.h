// license:BSD-3-Clause
// copyright-holders:hap
/***************************************************************************

    Saitek OSA Module: Kasparov Maestro A

***************************************************************************/

#ifndef MAME_BUS_SAITEKOSA_MAESTROA_H
#define MAME_BUS_SAITEKOSA_MAESTROA_H

#pragma once

#include "expansion.h"


class saitekosa_maestroa_device : public device_t, public device_saitekosa_expansion_interface
{
public:
	// construction/destruction
	saitekosa_maestroa_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// from host
	virtual u8 data_r() override;
	virtual void nmi_w(int state) override;

protected:
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	required_device<cpu_device> m_maincpu;

	void main_map(address_map &map);

	u8 p2200_r();
	u8 p2400_r();
	void p2400_w(u8 data);
	u8 p2600_r();
	void p2600_w(u8 data);

	u8 m_latch = 0xff;
	bool m_latch_enable = false;
};


DECLARE_DEVICE_TYPE(OSA_MAESTROA, saitekosa_maestroa_device)

#endif // MAME_BUS_SAITEKOSA_MAESTROA_H
