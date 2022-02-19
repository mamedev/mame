// license:BSD-3-Clause
// copyright-holders:Takahiro Nogi
#ifndef MAME_INCLUDES_NBMJ8900_H
#define MAME_INCLUDES_NBMJ8900_H

#pragma once

#include "machine/nb1413m3.h"
#include "emupal.h"
#include "screen.h"

class nbmj8900_state : public driver_device
{
public:
	nbmj8900_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag) ,
		m_maincpu(*this, "maincpu"),
		m_nb1413m3(*this, "nb1413m3"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette")
	{ }

	void ohpaipee(machine_config &config);
	void togenkyo(machine_config &config);

	void init_togenkyo();
	void init_ohpaipee();

private:
	enum
	{
		TIMER_BLITTER
	};

	required_device<cpu_device> m_maincpu;
	required_device<nb1413m3_device> m_nb1413m3;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	int m_scrolly;
	int m_blitter_destx;
	int m_blitter_desty;
	int m_blitter_sizex;
	int m_blitter_sizey;
	int m_blitter_src_addr;
	int m_blitter_direction_x;
	int m_blitter_direction_y;
	int m_vram;
	int m_gfxrom;
	int m_dispflag;
	int m_flipscreen;
	int m_clutsel;
	int m_screen_refresh;
	int m_gfxdraw_mode;
	int m_screen_height;
	int m_screen_width;
	bitmap_ind16 m_tmpbitmap0;
	bitmap_ind16 m_tmpbitmap1;
	std::unique_ptr<uint8_t[]> m_videoram0;
	std::unique_ptr<uint8_t[]> m_videoram1;
	std::unique_ptr<uint8_t[]> m_palette_ptr;
	std::unique_ptr<uint8_t[]> m_clut;
	int m_flipscreen_old;
	emu_timer *m_blitter_timer;

	uint8_t palette_type1_r(offs_t offset);
	void palette_type1_w(offs_t offset, uint8_t data);
	[[maybe_unused]] uint8_t palette_type2_r(offs_t offset);
	[[maybe_unused]] void palette_type2_w(offs_t offset, uint8_t data);
	[[maybe_unused]] uint8_t palette_type3_r(offs_t offset);
	[[maybe_unused]] void palette_type3_w(offs_t offset, uint8_t data);
	void clutsel_w(uint8_t data);
	uint8_t clut_r(offs_t offset);
	void clut_w(offs_t offset, uint8_t data);
	void blitter_w(offs_t offset, uint8_t data);
	void scrolly_w(uint8_t data);
	void vramsel_w(uint8_t data);
	void romsel_w(uint8_t data);

	virtual void video_start() override;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void vramflip(int vram);
	void update_pixel0(int x, int y);
	void update_pixel1(int x, int y);
	void gfxdraw();
	void postload();

	void ohpaipee_io_map(address_map &map);
	void ohpaipee_map(address_map &map);
	void togenkyo_map(address_map &map);

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param) override;
};

#endif // MAME_INCLUDES_NBMJ8900_H
