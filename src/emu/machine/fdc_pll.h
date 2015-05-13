// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef __FDC_PLL_H__
#define __FDC_PLL_H__

/*
 * Generic pll class for floppy controllers with analog plls
 */

#include "emu.h"
#include "imagedev/floppy.h"

class fdc_pll_t {
public:
	attotime ctime, period, min_period, max_period, period_adjust_base, phase_adjust;

	attotime write_start_time;
	attotime write_buffer[32];
	int write_position;
	int freq_hist;

	void set_clock(const attotime &period);
	void reset(const attotime &when);
	int get_next_bit(attotime &tm, floppy_image_device *floppy, const attotime &limit);
	bool write_next_bit(bool bit, attotime &tm, floppy_image_device *floppy, const attotime &limit);
	void start_writing(const attotime &tm);
	void commit(floppy_image_device *floppy, const attotime &tm);
	void stop_writing(floppy_image_device *floppy, const attotime &tm);

	std::string tts(attotime tm);
};

#endif
