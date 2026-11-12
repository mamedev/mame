// license:BSD-3-Clause
// copyright-holders:Carl, Angelo Salese

#ifndef MAME_BUS_PC_JOY_BASIC_JOY_H
#define MAME_BUS_PC_JOY_BASIC_JOY_H

#pragma once

#include "pc_joy.h"

class pc_basic_joy_device : public device_t,
							public device_pc_joy_interface
{
public:
	pc_basic_joy_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual uint8_t x1(int delta) override { return (m_x1->read() > delta); }
	virtual uint8_t x2(int delta) override { return (m_x2->read() > delta); }
	virtual uint8_t y1(int delta) override { return (m_y1->read() > delta); }
	virtual uint8_t y2(int delta) override { return (m_y2->read() > delta); }
	virtual uint8_t btn() override { return m_btn->read(); }

protected:
	pc_basic_joy_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock = 0);

	virtual void device_start() override { }

	required_ioport m_btn;

private:
	required_ioport m_x1;
	required_ioport m_y1;
	required_ioport m_x2;
	required_ioport m_y2;
};

class pc_joy_gravis_gamepad_device : public pc_basic_joy_device
{
public:
	pc_joy_gravis_gamepad_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// autofire mode
	static constexpr feature_type imperfect_features() { return feature::CONTROLS; }

	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
};

class pc_joy_magnum6_device : public pc_basic_joy_device
{
public:
	pc_joy_magnum6_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// other modes
	static constexpr feature_type imperfect_features() { return feature::CONTROLS; }

	virtual uint8_t x2(int delta) override { return BIT(m_btn->read(), 4); }
	virtual uint8_t y2(int delta) override { return BIT(m_btn->read(), 5); }

	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
};

DECLARE_DEVICE_TYPE(PC_BASIC_JOY,      pc_basic_joy_device)
DECLARE_DEVICE_TYPE(PC_GRAVIS_GAMEPAD, pc_joy_gravis_gamepad_device)
DECLARE_DEVICE_TYPE(PC_MAGNUM6_STICK,  pc_joy_magnum6_device)

#endif // MAME_BUS_PC_JOY_BASIC_JOY_H
