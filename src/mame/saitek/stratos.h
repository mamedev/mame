// license:BSD-3-Clause
// copyright-holders:hap
/*

  Saitek Stratos family chess computers shared class
  Used in: stratos.cpp (main driver), corona.cpp

*/

#ifndef MAME_SAITEK_STRATOS_H
#define MAME_SAITEK_STRATOS_H

#pragma once

#include "video/pwm.h"

#include <algorithm>


class stratos_base_state : public driver_device
{
public:
	stratos_base_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_display(*this, "display"),
		m_out_digit(*this, "digit%u", 0U),
		m_out_lcd(*this, "lcd%u.%u.%u", 0U, 0U, 0U)
	{ }

	DECLARE_INPUT_CHANGED_MEMBER(change_cpu_freq);
	DECLARE_INPUT_CHANGED_MEMBER(go_button);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void device_post_load() override { update_lcd(); }

	bool m_power = false;
	bool m_lcd_ready = false;
	u8 m_lcd_count = 0;
	u8 m_lcd_command = 0;
	u8 m_lcd_data[0x40] = { };

	// devices/pointers
	required_device<cpu_device> m_maincpu;
	required_device<pwm_display_device> m_display;
	output_finder<8+1> m_out_digit;
	output_finder<4, 16, 4> m_out_lcd;

	// common handlers
	void clear_lcd() { std::fill(std::begin(m_lcd_data), std::end(m_lcd_data), 0); }
	void update_lcd();
	void power_off();
	void lcd_data_w(u8 data);
};

INPUT_PORTS_EXTERN( saitek_stratos );

#endif // MAME_SAITEK_STRATOS_H
