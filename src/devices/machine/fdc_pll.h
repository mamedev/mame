// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef MAME_MACHINE_FDC_PLL_H
#define MAME_MACHINE_FDC_PLL_H

#pragma once

/*
 * Generic pll class for floppy controllers with analog plls
 */

class floppy_image_device;

class fdc_pll_t {
public:
	attotime ctime, period, min_period, max_period, period_adjust_base, phase_adjust;

	attotime write_start_time;
	attotime write_buffer[32];
	int write_position;
	int freq_hist;

	void set_clock(const attotime &period);
	void reset(const attotime &when);
	void read_reset(const attotime &when);
	int get_next_bit(attotime &tm, floppy_image_device *floppy, const attotime &limit);
	int feed_read_data(attotime &tm, const attotime& edge, const attotime &limit);
	bool write_next_bit(bool bit, attotime &tm, floppy_image_device *floppy, const attotime &limit);
	void start_writing(const attotime &tm);
	void commit(floppy_image_device *floppy, const attotime &tm);
	void stop_writing(floppy_image_device *floppy, const attotime &tm);

	std::string tts(attotime tm);
};

#endif // MAME_MACHINE_FDC_PLL_H
