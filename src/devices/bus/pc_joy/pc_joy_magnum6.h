// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#ifndef MAME_BUS_PC_JOY_MAGNUM6_H
#define MAME_BUS_PC_JOY_MAGNUM6_H

#include "pc_joy.h"

class pc_joy_magnum6_device : public pc_basic_joy_device
{
public:
	pc_joy_magnum6_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	static constexpr feature_type imperfect_features() { return feature::CONTROLS; }

	virtual uint8_t x2(int delta) override { return BIT(m_btn->read(), 4); }
	virtual uint8_t y2(int delta) override { return BIT(m_btn->read(), 5); }

	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
};

DECLARE_DEVICE_TYPE(PC_MAGNUM6_PAD, pc_joy_magnum6_device)

#endif // MAME_BUS_PC_JOY_MAGNUM6_H
