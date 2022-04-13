// license:BSD-3-Clause
// copyright-holders:hap
/*

  Generic PWM display device

*/

#ifndef MAME_VIDEO_PWM_H
#define MAME_VIDEO_PWM_H

#pragma once

class pwm_display_device : public device_t
{
public:
	pwm_display_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// configuration helpers
	pwm_display_device &set_size(u8 numrows, u8 rowbits) { m_height = numrows; m_width = rowbits; return *this; } // max 64 * 64
	pwm_display_device &set_refresh(attotime duration) { m_framerate_set = duration; return *this; } // time between each outputs refresh
	pwm_display_device &set_interpolation(double factor) { m_interpolation = factor; return *this; } // frame interpolation (0.0 - 1.0)
	pwm_display_device &set_segmask(u64 digits, u64 mask); // mask for multi-state outputs, eg. 7seg led
	pwm_display_device &reset_segmask() { std::fill(std::begin(m_segmask), std::end(m_segmask), 0); return *this; }
	pwm_display_device &set_bri_levels(double l0, double l1 = 1.0, double l2 = 1.0, double l3 = 1.0); // brightness threshold per level (0.0 - 1.0)
	pwm_display_device &set_bri_minimum(u8 i) { m_level_min = i; return *this; } // minimum level index for element to be considered "on"
	pwm_display_device &set_bri_maximum(double b) { m_level_max = b; return *this; } // maximum brightness level, 0.0 for auto

	// output callbacks when not using the default output tags
	auto output_x() { return m_output_x_cb.bind(); } // x = offset >> 6, y = offset & 0x3f
	auto output_a() { return m_output_a_cb.bind(); }
	auto output_multi() { return m_output_multi_cb.bind(); } // b = offset >> 6, y = offset & 0x3f
	auto output_digit() { return m_output_digit_cb.bind(); }

	void reset_bri_levels() { std::fill(std::begin(m_levels), std::end(m_levels), 1.0); }
	void set_bri_one(u8 i, double level) { m_levels[i] = level; }
	void segmask_one(u8 y, u64 mask) { m_segmask[y] = mask; }

	// matrix accessors
	void matrix_partial(u8 start, u8 height, u64 rowsel, u64 rowdata);
	void matrix(u64 rowsel, u64 rowdata) { matrix_partial(0, m_height, rowsel, rowdata); }
	void clear() { matrix(0, 0); }

	void write_my(u64 y) { matrix(y, m_rowdata_last); }
	void write_mx(u64 x) { matrix(m_rowsel, x); }
	u64 read_my() { return m_rowsel; }
	u64 read_mx() { return m_rowdata_last; }

	// directly handle individual element (does not affect m_rowsel), y = row num, x = row bit
	int read_element(u8 y, u8 x) { return BIT(m_rowdata[y], x); }
	void write_element(u8 y, u8 x, int state) { sync(); m_rowdata[y] = (m_rowdata[y] & ~(u64(1) << x)) | (u64(state ? 1 : 0) << x); }

	// directly handle row data
	u64 read_row(offs_t offset) { return m_rowdata[offset]; }
	void write_row(offs_t offset, u64 data) { sync(); m_rowdata[offset] = data; m_rowsel |= u64(1) << offset; }
	void clear_row(offs_t offset, u64 data = 0) { sync(); m_rowdata[offset] = 0; m_rowsel &= ~(u64(1) << offset); }

	// directly handle element current brightness
	double read_element_bri(u8 y, u8 x) { return m_bri[y][x]; }
	void write_element_bri(u8 y, u8 x, double b) { m_bri[y][x] = b; }
	bool element_on(u8 y, u8 x) { return (read_element_bri(y, x) > m_levels[m_level_min]); }
	bool row_on(u8 y) { return element_on(y, m_width); }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	output_finder<0x40, 0x40> m_out_x;
	output_finder<0x40> m_out_a;
	output_finder<0x40, 0x40> m_out_multi;
	output_finder<0x40> m_out_digit;

	devcb_write8 m_output_x_cb;
	devcb_write8 m_output_a_cb;
	devcb_write64 m_output_multi_cb;
	devcb_write64 m_output_digit_cb;
	bool m_external_output;

	u8 m_width;
	u8 m_height;
	attotime m_framerate_set;
	attotime m_framerate;
	double m_interpolation;
	double m_levels[0x41];
	u8 m_level_min;
	double m_level_max;

	u64 m_segmask[0x40];
	u64 m_rowsel;
	u64 m_rowdata[0x40];
	u64 m_rowdata_last;

	double m_bri[0x40][0x41];
	attotime m_sync_time;
	attotime m_acc[0x40][0x41];

	emu_timer *m_frame_timer;
	TIMER_CALLBACK_MEMBER(frame_tick);
	void schedule_frame();
	void sync();
};


DECLARE_DEVICE_TYPE(PWM_DISPLAY, pwm_display_device)

#endif // MAME_VIDEO_PWM_H
