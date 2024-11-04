// license:BSD-3-Clause
// copyright-holders:Sven Schnelle
#ifndef MAME_DEVICES_HP_HIL_HLEMOUSE_H
#define MAME_DEVICES_HP_HIL_HLEMOUSE_H

#pragma once

#include "hp_hil.h"
#include "hlebase.h"
#include "machine/keyboard.h"


namespace bus::hp_hil {

class hle_hp_46060b_device
		: public hle_device_base
{
public:
	hle_hp_46060b_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock);
	virtual void device_reset() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual int hil_poll() override;
	virtual void hil_idd() override;
	enum state_mask
	{
		MOUSE_YPOS    = 0x000000ff,
		MOUSE_XPOS    = 0x0000ff00,
		MOUSE_LBUTTON = 0x00010000,
		MOUSE_MBUTTON = 0x00020000,
		MOUSE_RBUTTON = 0x00040000,

		MOUSE_BUTTONS = 0x00070000
	};

	DECLARE_INPUT_CHANGED_MEMBER(mouse_button);
	DECLARE_INPUT_CHANGED_MEMBER(mouse_x);
	DECLARE_INPUT_CHANGED_MEMBER(mouse_y);

	int8_t mouse_x_delta;
	int8_t mouse_y_delta;
	uint32_t mouse_buttons;
	util::fifo<uint8_t, 16> m_fifo;

};

} // namespace bus::hp_hil

DECLARE_DEVICE_TYPE_NS(HP_46060B_MOUSE, bus::hp_hil, hle_hp_46060b_device);

#endif // MAME_DEVICES_HP_HIL_HLEMOUSE_H
