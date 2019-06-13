// license:BSD-3-Clause
// copyright-holders:hap
/******************************************************************************

  Generic screenless base class

  implementation is in machine/screenless.cpp

******************************************************************************/

#ifndef MAME_INCLUDES_SCREENLESS_H
#define MAME_INCLUDES_SCREENLESS_H

#pragma once

class screenless_state : public driver_device
{
public:
	screenless_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_out_x(*this, "%u.%u", 0U, 0U),
		m_out_a(*this, "%u.a", 0U),
		m_out_digit(*this, "digit%u", 0U),
		m_display_maxy(0),
		m_display_maxx(-1)
	{
		// set defaults (60hz frames, 0.5 interpolation, 1 brightness level)
		set_display_duration(attotime::from_hz(60));
		set_display_factor(0.5);
		set_display_levels(0.02);
	}

protected:
	output_finder<0x20, 0x40> m_out_x; // max 32, 63
	output_finder<0x20> m_out_a;
	output_finder<0x20> m_out_digit;

	int m_display_maxy;             // display matrix number of rows
	int m_display_maxx;             // display matrix number of columns
	u64 m_display_state[0x20];      // display matrix rows data (last bit is used for always-on)
	u16 m_display_segmask[0x20];    // if not 0, display matrix row is a digit, mask indicates connected segments

	void set_display_duration(attotime duration);
	void set_display_factor(double factor);

	void reset_display_levels();
	void set_display_level(u8 i, double level);
	void set_display_levels(double l0, double l1 = 1.0, double l2 = 1.0, double l3 = 1.0);

	void set_display_size(int maxx, int maxy);
	void set_display_segmask(u32 digits, u32 mask);
	void display_matrix(int maxx, int maxy, u64 setx, u32 sety, bool update = true);
	virtual void display_update();
	bool display_element_on(u32 x, u32 y);

	virtual void machine_start() override;

private:
	u64 m_ds_prev[0x20];
	double m_ds_bri[0x20][0x40];
	attotime m_ds_acc[0x20][0x40];
	attotime m_ds_update_time;
	attotime m_ds_frame_time;
	attotime m_ds_frame_time_set;
	double m_ds_frame_factor;
	double m_ds_level[0x100];

	emu_timer *m_display_frame_timer;
	TIMER_CALLBACK_MEMBER(display_frame);
	void display_schedule_frame();
};


#endif // MAME_INCLUDES_SCREENLESS_H
