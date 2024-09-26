// license:GPL-2.0+
// copyright-holders:Kevin Thacker
/*****************************************************************************
 *
 * includes/timex.h
 *
 ****************************************************************************/

#ifndef MAME_SINCLAIR_TIMEX_H
#define MAME_SINCLAIR_TIMEX_H

#pragma once

#include "spec128.h"

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

class tc2048_state : public spectrum_128_state
{
public:
	tc2048_state(const machine_config &mconfig, device_type type, const char *tag) :
		spectrum_128_state(mconfig, type, tag)
	{
	}

	void tc2048(machine_config &config);

protected:
	virtual void machine_reset() override ATTR_COLD;

	u8 port_ff_r();

	void spectrum_update_screen(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect) override;
	rectangle get_screen_area() override;
	void hires_scanline(bitmap_ind16 &bitmap, int y, int borderlines);
	void _64col_scanline(bitmap_ind16 &bitmap, int y, int borderlines, unsigned short inkcolor);
	void lores_scanline(bitmap_ind16 &bitmap, int y, int borderlines, int screen);

private:
	void port_ff_w(offs_t offset, u8 data);

	void tc2048_io(address_map &map) ATTR_COLD;
	void tc2048_mem(address_map &map) ATTR_COLD;

	inline void spectrum_plot_pixel(bitmap_ind16 &bitmap, int x, int y, uint32_t color);
};

class ts2068_state : public tc2048_state
{
public:
	ts2068_state(const machine_config &mconfig, device_type type, const char *tag) :
		tc2048_state(mconfig, type, tag),
		m_dock(*this, "dockslot")
	{
	}

	void ts2068(machine_config &config);
	void uk2086(machine_config &config);

protected:
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	virtual void ts2068_update_memory() override;

private:
	enum
	{
		TIMEX_CART_NONE,
		TIMEX_CART_DOCK,
		TIMEX_CART_EXROM,
		TIMEX_CART_HOME
	};

	u8 port_f4_r();
	void port_f4_w(u8 data);
	void port_ff_w(offs_t offset, u8 data);

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);
	int m_dock_cart_type = 0, m_ram_chunks = 0;
	memory_region *m_dock_crt = nullptr;

	void ts2068_io(address_map &map) ATTR_COLD;
	void ts2068_mem(address_map &map) ATTR_COLD;

	required_device<generic_slot_device> m_dock;
};


#endif // MAME_SINCLAIR_TIMEX_H
