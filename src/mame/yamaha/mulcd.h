// license:BSD-3-Clause
// copyright-holders:R. Belmont, Olivier Galibert

// HD44780/LCD image combo used in the yamaha mu, vl70m, fs1r and
// probably others

#ifndef MAME_YAMAHA_MULCD_H
#define MAME_YAMAHA_MULCD_H

#pragma once

#include "video/hd44780.h"
#include "screen.h"

DECLARE_DEVICE_TYPE(MULCD, mulcd_device)

class mulcd_device : public hd44780_base_device
{
public:
	mulcd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 270000);

	void set_contrast(u8 contrast);
	void set_leds(u16 leds);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	output_finder<64, 8, 5> m_outputs;
	output_finder<> m_contrast;
	output_finder<10> m_led_outputs;

	u32 mu_screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};

#endif // MAME_YAMAHA_MULCD_H
