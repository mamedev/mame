// license:BSD-3-Clause
// copyright-holders:Stefan Jokisch
/*************************************************************************

    Atari Liberator hardware

*************************************************************************/
#ifndef MAME_INCLUDES_LIBERATR_H
#define MAME_INCLUDES_LIBERATR_H

#pragma once

#include "cpu/m6502/m6502.h"
#include "machine/74259.h"
#include "machine/er2055.h"
#include "machine/watchdog.h"
#include "sound/pokey.h"
#include "screen.h"

class liberatr_state : public driver_device
{
public:
	liberatr_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_earom(*this, "earom")
		, m_earom_data(0)
		, m_earom_control(0)
		, m_outlatch(*this, "outlatch")
		, m_screen(*this, "screen")
		, m_base_ram(*this, "base_ram")
		, m_planet_frame(*this, "planet_frame")
		, m_xcoord(*this, "xcoord")
		, m_ycoord(*this, "ycoord")
		, m_bitmapram(*this, "bitmapram")
		, m_colorram(*this, "colorram")
	{ }

	void liberat2(machine_config &config);
	void liberatr(machine_config &config);

private:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

	void output_latch_w(offs_t offset, uint8_t data);
	DECLARE_WRITE_LINE_MEMBER(coin_counter_left_w);
	DECLARE_WRITE_LINE_MEMBER(coin_counter_right_w);

	DECLARE_WRITE_LINE_MEMBER(trackball_reset_w);
	uint8_t port0_r();

	void bitmap_w(offs_t offset, uint8_t data);
	uint8_t bitmap_xy_r();
	void bitmap_xy_w(uint8_t data);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	// early raster EAROM interface
	uint8_t earom_r();
	void earom_w(offs_t offset, uint8_t data);
	void earom_control_w(uint8_t data);

	void liberat2_map(address_map &map);
	void liberatr_map(address_map &map);

	// The following structure describes the (up to 32) line segments
	// that make up one horizontal line (latitude) for one display frame of the planet.
	// Note: this and the following structure is only used to collect the
	// data before it is packed for actual use.
	struct planet_frame_line
	{
		uint8_t segment_count;    // the number of segments on this line
		uint8_t max_x;            // the maximum value of x_array for this line
		uint8_t color_array[32];  // the color values
		uint8_t x_array[32];      // and maximum x values for each segment
	};

	// The following structure describes the lines (latitudes)
	// that make up one complete display frame of the planet.
	// Note: this and the previous structure is only used to collect the
	// data before it is packed for actual use.
	struct planet_frame
	{
		planet_frame_line lines[0x80];
	};

	// The following structure collects the 256 frames of the
	// planet (one per value of longitude).
	// The data is packed segment_count,segment_start,color,length,color,length,...  then
	//                    segment_count,segment_start,color,length,color,length...  for the next line, etc
	// for the 128 lines.
	struct planet
	{
		std::unique_ptr<uint8_t []> frames[256];
	};

	void init_planet(planet &liberatr_planet, uint8_t *planet_rom);
	void get_pens(pen_t *pens);
	void draw_planet(bitmap_rgb32 &bitmap, pen_t *pens);
	void draw_bitmap(bitmap_rgb32 &bitmap, pen_t *pens);

	// vector and early raster EAROM interface
	required_device<er2055_device> m_earom;
	uint8_t               m_earom_data;
	uint8_t               m_earom_control;

	required_device<ls259_device> m_outlatch;

	required_device<screen_device> m_screen;
	required_shared_ptr<uint8_t> m_base_ram;
	required_shared_ptr<uint8_t> m_planet_frame;
	required_shared_ptr<uint8_t> m_xcoord;
	required_shared_ptr<uint8_t> m_ycoord;
	required_shared_ptr<uint8_t> m_bitmapram;
	required_shared_ptr<uint8_t> m_colorram;

	uint8_t       m_trackball_offset = 0U;
	uint8_t       m_ctrld = 0U;
	uint8_t       m_videoram[0x10000]{};

	bool m_planet_select = false;

	// The following array collects the 2 different planet
	// descriptions, which are selected by planetbit
	planet m_planets[2]{};
};

#endif // MAME_INCLUDES_LIBERATR_H
