// license:BSD-3-Clause
// copyright-holders:R. Belmont, Olivier Galibert

// HD44780/LCD image combo used in the yamaha mu, vl70m, fs1r and
// probably others

#ifndef MAME_MACHINE_MULCD_H
#define MAME_MACHINE_MULCD_H

#pragma once

#include "video/hd44780.h"
#include "screen.h"

class mulcd_device : public device_t
{
public:
	mulcd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	void set_contrast(float contrast) { m_contrast = contrast; }
	void set_leds(u8 leds) { m_leds = leds; }
	u8 data_read() { return m_lcd->data_read(); }
	u8 control_read() { return m_lcd->control_read(); }
	void data_write(u8 data) { m_lcd->data_write(data); }
	void control_write(u8 data) { m_lcd->control_write(data); }


protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;

private:
	required_device<hd44780_device> m_lcd;

	float m_contrast;
	u8 m_leds;

	float lightlevel(const u8 *src, const u8 *render);
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};

DECLARE_DEVICE_TYPE(MULCD, mulcd_device)

#endif
