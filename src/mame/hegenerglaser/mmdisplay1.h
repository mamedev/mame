// license:BSD-3-Clause
// copyright-holders:hap
/*******************************************************************************

    Mephisto Modular Display Module (1st version)

*******************************************************************************/

#ifndef MAME_HEGENERGLASER_MMDISPLAY1_H
#define MAME_HEGENERGLASER_MMDISPLAY1_H

#pragma once

#include "video/pwm.h"


class mephisto_display1_device : public device_t
{
public:
	mephisto_display1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// callback is optional, it will output to digitx when not used
	auto output_digit() { return m_output_digit.bind(); }

	void common_w(int state);
	int common_r() { return m_common; }
	void data_w(u8 data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<pwm_display_device> m_lcd_pwm;
	output_finder<4> m_digits;
	devcb_write8 m_output_digit;

	int m_common;
	u32 m_digit_data;

	void update_lcd();
	void lcd_pwm_w(offs_t offset, u64 data);
};


DECLARE_DEVICE_TYPE(MEPHISTO_DISPLAY_MODULE1, mephisto_display1_device)

#endif // MAME_HEGENERGLASER_MMDISPLAY1_H
