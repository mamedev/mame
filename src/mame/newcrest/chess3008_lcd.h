// license:BSD-3-Clause
// copyright-holders:hap
/*******************************************************************************

    Newcrest CXG Chess 3008 LCD

*******************************************************************************/

#ifndef MAME_NEWCREST_CHESS3008_LCD_H
#define MAME_NEWCREST_CHESS3008_LCD_H

#pragma once

#include "video/lc7580.h"


class chess3008_lcd_device : public device_t
{
public:
	// construction/destruction
	chess3008_lcd_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	void inh_w(int state) { m_lcd->inh_w(state); }
	void lcd_w(u8 data);

protected:
	// device_t implementation
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

private:
	required_device<lc7580_device> m_lcd;
	output_finder<8> m_out_digit;
	output_finder<2, 52> m_out_lcd;

	void lcd_output_w(offs_t offset, u64 data);
};


DECLARE_DEVICE_TYPE(CHESS3008_LCD, chess3008_lcd_device)

#endif // MAME_NEWCREST_CHESS3008_LCD_H
