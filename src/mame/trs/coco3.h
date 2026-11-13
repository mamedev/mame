// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    coco3.h

    TRS-80 Radio Shack Color Computer 3 Family

***************************************************************************/

#ifndef MAME_TRS_COCO3_H
#define MAME_TRS_COCO3_H

#pragma once

#include "coco.h"
#include "gime.h"
#include "machine/quadmouse.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class coco3_state : public coco_state
{
public:
	coco3_state(const machine_config &mconfig, device_type type, const char *tag)
		: coco_state(mconfig, type, tag)
		, m_gime(*this, "gime")
		, m_screen_config(*this, "screen_config")
		, m_ratmouse_r(*this, "ratmouse_r")
		, m_ratmouse_l(*this, "ratmouse_l")
	{
	}

	virtual void ff20_write(offs_t offset, uint8_t data) override;
	virtual uint8_t ff40_read(offs_t offset) override;
	virtual void ff40_write(offs_t offset, uint8_t data) override;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void coco3p(machine_config &config);
	void coco3h(machine_config &config);
	void coco3dw1(machine_config &config);
	void coco3(machine_config &config);
	void coco3_mem(address_map &map) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

	virtual void update_cart_base(uint8_t *cart_base) override;
	void pia0_pa_w(uint8_t value);
	void pia0_pb_w(uint8_t value);

	// miscellaneous
	virtual void cart_w(bool line) override;

	bool m_prev_keyboard_pressed = false;
	uint8_t m_pia1b_control_register = 0U;

	virtual void on_keyboard_state_changed(bool any_pressed) override;
	virtual std::unique_ptr<coco_joy_handler> make_joy_handler(uint8_t selection, int port) override;
	virtual const std::type_info& get_type_info_for_selection(uint8_t selection) override;
	void bind_rat_mouse(quadmouse_device &quad, int port_index);

private:
	required_device<gime_device> m_gime;
	required_ioport m_screen_config;
	required_device<quadmouse_device> m_ratmouse_r;
	required_device<quadmouse_device> m_ratmouse_l;
};

//**************************************************************************
//  coco_joy_handler - classes for things that plug into the joystick port
//  and sometimes casette / serial
//**************************************************************************

class coco_rat_mouse : public coco_joy_handler
{
public:
	coco_rat_mouse(coco_state &host, int base_slot, ioport_port *buttons);

	void update_axis(int axis);
	void up_w(int state);
	void down_w(int state);
	void left_w(int state);
	void right_w(int state);

private:
	const int joy_rat_table[4];
	int m_left, m_up, m_right, m_down;
};



#endif // MAME_TRS_COCO3_H
