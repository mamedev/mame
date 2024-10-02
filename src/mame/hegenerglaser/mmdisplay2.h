// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/*******************************************************************************

    Mephisto Modular Display Module (2nd version)

*******************************************************************************/

#ifndef MAME_HEGENERGLASER_MMDISPLAY2_H
#define MAME_HEGENERGLASER_MMDISPLAY2_H

#pragma once

#include "sound/dac.h"
#include "video/hd44780.h"

#include "emupal.h"


class mephisto_display2_device : public device_t
{
public:
	// construction/destruction
	mephisto_display2_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	hd44780_device *get() { return m_lcd; }

	void latch_w(u8 data);
	void io_w(u8 data);
	u8 io_r() { return m_ctrl; }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<hd44780_device> m_lcd;
	required_device<dac_2bit_ones_complement_device> m_dac;

	u8 m_latch;
	u8 m_ctrl;

	void lcd_palette(palette_device &palette) const;
	HD44780_PIXEL_UPDATE(lcd_pixel_update);
};


DECLARE_DEVICE_TYPE(MEPHISTO_DISPLAY_MODULE2, mephisto_display2_device)

#endif // MAME_HEGENERGLASER_MMDISPLAY2_H
