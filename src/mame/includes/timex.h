// license:GPL-2.0+
// copyright-holders:Kevin Thacker
/*****************************************************************************
 *
 * includes/timex.h
 *
 ****************************************************************************/

#ifndef MAME_INCLUDES_TIMEX_H
#define MAME_INCLUDES_TIMEX_H

#pragma once

#include "bus/generic/carts.h"
#include "bus/generic/slot.h"

/* Border sizes for TS2068. These are guesses based on the number of cycles
   available per frame. */
#define TS2068_TOP_BORDER    32
#define TS2068_BOTTOM_BORDER 32
#define TS2068_SCREEN_HEIGHT (TS2068_TOP_BORDER + SPEC_DISPLAY_YSIZE + TS2068_BOTTOM_BORDER)

/* Double the border sizes to maintain ratio of screen to border */
#define TS2068_LEFT_BORDER   96   /* Number of left hand border pixels */
#define TS2068_DISPLAY_XSIZE 512  /* Horizontal screen resolution */
#define TS2068_RIGHT_BORDER  96   /* Number of right hand border pixels */
#define TS2068_SCREEN_WIDTH (TS2068_LEFT_BORDER + TS2068_DISPLAY_XSIZE + TS2068_RIGHT_BORDER)

enum
{
	TIMEX_CART_NONE,
	TIMEX_CART_DOCK,
	TIMEX_CART_EXROM,
	TIMEX_CART_HOME
};


class timex_state : public spectrum_state
{
public:
	timex_state(const machine_config &mconfig, device_type type, const char *tag) :
		spectrum_state(mconfig, type, tag),
		m_dock(*this, "dockslot")
	{
	}

	void ts2068(machine_config &config);
	void uk2086(machine_config &config);
	void tc2048(machine_config &config);

private:
	DECLARE_READ8_MEMBER(ts2068_port_f4_r);
	DECLARE_WRITE8_MEMBER(ts2068_port_f4_w);
	DECLARE_READ8_MEMBER(ts2068_port_ff_r);
	DECLARE_WRITE8_MEMBER(ts2068_port_ff_w);
	DECLARE_WRITE8_MEMBER(tc2048_port_ff_w);

	DECLARE_MACHINE_RESET(tc2048);
	DECLARE_MACHINE_RESET(ts2068);
	DECLARE_VIDEO_START(ts2068);
	uint32_t screen_update_tc2048(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_ts2068(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(screen_vblank_timex);

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);
	int m_dock_cart_type, m_ram_chunks;
	memory_region *m_dock_crt;

	virtual void ts2068_update_memory() override;

	void tc2048_io(address_map &map);
	void tc2048_mem(address_map &map);
	void ts2068_io(address_map &map);
	void ts2068_mem(address_map &map);

	optional_device<generic_slot_device> m_dock;

	inline void spectrum_plot_pixel(bitmap_ind16 &bitmap, int x, int y, uint32_t color);
	void ts2068_hires_scanline(bitmap_ind16 &bitmap, int y, int borderlines);
	void ts2068_64col_scanline(bitmap_ind16 &bitmap, int y, int borderlines, unsigned short inkcolor);
	void ts2068_lores_scanline(bitmap_ind16 &bitmap, int y, int borderlines, int screen);
};


#endif // MAME_INCLUDES_TIMEX_H
