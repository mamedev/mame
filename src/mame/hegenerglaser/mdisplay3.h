// license:BSD-3-Clause
// copyright-holders:hap
/*******************************************************************************

    Mephisto Modular Display Module (3rd version)

*******************************************************************************/

#ifndef MAME_HEGENERGLASER_MDISPLAY3_H
#define MAME_HEGENERGLASER_MDISPLAY3_H

#pragma once

#include "video/pwm.h"


class mephisto_display3_device : public device_t
{
public:
	// construction/destruction
	mephisto_display3_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	void clk_w(int state);
	void common_w(u8 data);
	void data_w(u8 data) { m_data = data; }

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<pwm_display_device> m_lcd_pwm;
	output_finder<2, 24> m_out_lcd;

	int m_clk;
	u8 m_data;
	u32 m_lcd_segs;
	u8 m_lcd_com;

	void lcd_pwm_w(offs_t offset, u8 data);
	void update_lcd();
};


DECLARE_DEVICE_TYPE(MEPHISTO_DISPLAY_MODULE3, mephisto_display3_device)

#endif // MAME_HEGENERGLASER_MDISPLAY3_H
