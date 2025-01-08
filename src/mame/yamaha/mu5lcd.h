// license:BSD-3-Clause
// copyright-holders:R. Belmont, Olivier Galibert

// LC7985/LCD image combo used in the yamaha mu5 and mu15

#ifndef MAME_YAMAHA_MU5LCD_H
#define MAME_YAMAHA_MU5LCD_H

#pragma once

#include "video/lc7985.h"
#include "screen.h"

DECLARE_DEVICE_TYPE(MU5LCD, mu5lcd_device)

class mu5lcd_device : public device_t
{
public:
	mu5lcd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	u8 dr_r() { return m_lcd->dr_r(); }
	u8 status_r() { return m_lcd->status_r(); }
	void dr_w(u8 data) { m_lcd->dr_w(data); }
	void ir_w(u8 data) { m_lcd->ir_w(data); }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<lc7985_device> m_lcd;
	output_finder<2, 8, 8, 5> m_outputs;
	void render_w(int state);
};

#endif // MAME_YAMAHA_MU5LCD_H
