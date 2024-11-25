// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    MSM6222B

    A somewhat hd44780-compatible LCD controller.

    The -01 variant has a fixed cgrom, the other variants are mask-programmed.

***************************************************************************/

#ifndef MAME_VIDEO_MSM6222B_H
#define MAME_VIDEO_MSM6222B_H

#pragma once


class msm6222b_device : public device_t {
public:
	msm6222b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void control_w(uint8_t data);
	uint8_t control_r();
	void data_w(uint8_t data);

	// Character n bits are at bytes n*16..n*16+7 when 8-high, +10 when 11-high.  Only the low 5 bits are used.
	// In one line mode n = 0..79.  In two line mode first line is 0..39 and second is 40..79.
	const uint8_t *render();

protected:
	msm6222b_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;

	optional_region_ptr<uint8_t> m_cgrom;

	void cursor_step(bool direction);
	void shift_step(bool direction);
	bool blink_on() const;

private:
	uint8_t cgram[8*8];
	uint8_t ddram[80];
	uint8_t render_buf[80*16];
	bool cursor_direction, cursor_blinking, two_line, shift_on_write, double_height, cursor_on, display_on;
	uint8_t adc, shift;
};

class msm6222b_01_device : public msm6222b_device {
public:
	msm6222b_01_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};

DECLARE_DEVICE_TYPE(MSM6222B,    msm6222b_device)
DECLARE_DEVICE_TYPE(MSM6222B_01, msm6222b_01_device)

#endif // MAME_VIDEO_MSM6222B_H
