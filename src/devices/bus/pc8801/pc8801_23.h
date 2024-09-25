// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#ifndef MAME_BUS_PC8801_23_H
#define MAME_BUS_PC8801_23_H

#pragma once

#include "pc8801_exp.h"
#include "sound/ymopn.h"

class pc8801_23_device : public pc8801_exp_device
{
public:
	pc8801_23_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static constexpr feature_type unemulated_features() { return feature::SOUND; }

	virtual void io_map(address_map &map) override ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void int4_w(int state) override;

private:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	required_device<ym2608_device> m_opna;

	u8 irq_status_r();
	void irq_mask_w(u8 data);

	bool m_irq_mask = false;
	bool m_irq_pending = false;

	void opna_map(address_map &map) ATTR_COLD;
};

DECLARE_DEVICE_TYPE(PC8801_23, pc8801_23_device)


#endif // MAME_BUS_PC8801_23_H
