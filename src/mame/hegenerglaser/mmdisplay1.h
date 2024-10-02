// license:BSD-3-Clause
// copyright-holders:hap
/*******************************************************************************

    Mephisto Modular Display Module (1st version)

*******************************************************************************/

#ifndef MAME_HEGENERGLASER_MMDISPLAY1_H
#define MAME_HEGENERGLASER_MMDISPLAY1_H

#pragma once


class mephisto_display1_device : public device_t
{
public:
	mephisto_display1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// callback is optional, it will output to digitx when not used
	auto output_digit() { return m_output_digit.bind(); }

	void strobe_w(int state);
	int strobe_r() { return m_strobe; }
	void data_w(u8 data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	output_finder<4> m_digits;
	devcb_write8 m_output_digit;

	int m_strobe;
	u8 m_digit_idx;
	u8 m_digit_data[4];

	void update_lcd();
};


DECLARE_DEVICE_TYPE(MEPHISTO_DISPLAY_MODULE1, mephisto_display1_device)

#endif // MAME_HEGENERGLASER_MMDISPLAY1_H
