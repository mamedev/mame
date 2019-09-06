// license:BSD-3-Clause
// copyright-holders:hap
/*

  Saitek Stratos family chess computers shared class
  Used in: saitek_stratos.cpp (main driver), saitek_corona.cpp

*/

#ifndef MAME_INCLUDES_SAITEK_STRATOS_H
#define MAME_INCLUDES_SAITEK_STRATOS_H

#pragma once

#include "machine/timer.h"
#include "video/pwm.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

#include <algorithm>


class saitek_stratos_state : public driver_device
{
public:
	saitek_stratos_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_display(*this, "display"),
		m_extrom(*this, "extrom"),
		m_out_digit(*this, "digit%u", 0U),
		m_out_lcd(*this, "lcd%u.%u.%u", 0U, 0U, 0U)
	{ }

	DECLARE_INPUT_CHANGED_MEMBER(switch_cpu_freq) { set_cpu_freq(); }
	DECLARE_INPUT_CHANGED_MEMBER(acl_button) { if (newval) power_off(); }
	DECLARE_INPUT_CHANGED_MEMBER(go_button);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void device_post_load() override { update_lcd(); }

	// devices/pointers
	required_device<cpu_device> m_maincpu;
	required_device<pwm_display_device> m_display;
	required_device<generic_slot_device> m_extrom;
	output_finder<8+1> m_out_digit;
	output_finder<4, 16, 4> m_out_lcd;

	// common handlers
	void clear_lcd() { std::fill_n(m_lcd_data, ARRAY_LENGTH(m_lcd_data), 0); }
	void update_lcd();
	void power_off();
	void set_cpu_freq();

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(extrom_load);
	void lcd_data_w(u8 data);

	bool m_power;
	bool m_lcd_ready;
	u8 m_lcd_count;
	u8 m_lcd_command;
	u8 m_lcd_data[0x40];
};

INPUT_PORTS_EXTERN( saitek_stratos );

#endif // MAME_INCLUDES_SAITEK_STRATOS_H
