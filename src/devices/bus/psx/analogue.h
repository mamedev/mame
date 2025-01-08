// license:BSD-3-Clause
// copyright-holders:Carl
#ifndef MAME_BUS_PSX_ANALOGUE_H
#define MAME_BUS_PSX_ANALOGUE_H

#pragma once

#include "ctlrport.h"

DECLARE_DEVICE_TYPE(PSX_DUALSHOCK, psx_dualshock_device)
DECLARE_DEVICE_TYPE(PSX_ANALOG_JOYSTICK, psx_analog_joystick_device)

class psx_analog_controller_device :    public device_t,
										public device_psx_controller_interface
{
public:
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	DECLARE_INPUT_CHANGED_MEMBER(change_mode);
protected:
	enum class model { JOYSTICK, DUALSHOCK };

	psx_analog_controller_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, model mod);

	virtual void device_start() override { }
	virtual void device_reset() override ATTR_COLD;

private:
	virtual bool get_pad(int count, uint8_t *odata, uint8_t idata) override;
	uint8_t pad_data(int count, bool analog);

	const model m_model;

	bool m_confmode;
	bool m_analogmode;
	bool m_analoglock;

	uint8_t m_temp;
	uint8_t m_cmd;

	required_ioport m_pad0;
	required_ioport m_pad1;
	required_ioport m_rstickx;
	required_ioport m_rsticky;
	required_ioport m_lstickx;
	required_ioport m_lsticky;
};

class psx_dualshock_device : public psx_analog_controller_device
{
public:
	psx_dualshock_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class psx_analog_joystick_device : public psx_analog_controller_device
{
public:
	psx_analog_joystick_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

#endif // MAME_BUS_PSX_ANALOGUE_H
