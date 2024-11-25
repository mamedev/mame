// license:BSD-3-Clause
// copyright-holders:Takahiro Nogi
#ifndef MAME_NICHIBUTSU_NBMJ8900_H
#define MAME_NICHIBUTSU_NBMJ8900_H

#pragma once

#include "nb1413m3.h"
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
	required_device<cpu_device> m_maincpu;
	required_device<nb1413m3_device> m_nb1413m3;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	int m_scrolly = 0;
	int m_blitter_destx = 0;
	int m_blitter_desty = 0;
	int m_blitter_sizex = 0;
	int m_blitter_sizey = 0;
	int m_blitter_src_addr = 0;
	int m_blitter_direction_x = 0;
	int m_blitter_direction_y = 0;
	int m_vram = 0;
	int m_gfxrom = 0;
	int m_dispflag = 0;
	int m_flipscreen = 0;
	int m_clutsel = 0;
	int m_screen_refresh = 0;
	int m_gfxdraw_mode = 0;
	int m_screen_height = 0;
	int m_screen_width = 0;
	bitmap_ind16 m_tmpbitmap0;
	bitmap_ind16 m_tmpbitmap1;
	std::unique_ptr<uint8_t[]> m_videoram0;
	std::unique_ptr<uint8_t[]> m_videoram1;
	std::unique_ptr<uint8_t[]> m_palette_ptr;
	std::unique_ptr<uint8_t[]> m_clut;
	int m_flipscreen_old = 0;
	emu_timer *m_blitter_timer = nullptr;

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

	virtual void video_start() override ATTR_COLD;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void vramflip(int vram);
	void update_pixel0(int x, int y);
	void update_pixel1(int x, int y);
	void gfxdraw();
	void postload();

	void ohpaipee_io_map(address_map &map) ATTR_COLD;
	void ohpaipee_map(address_map &map) ATTR_COLD;
	void togenkyo_map(address_map &map) ATTR_COLD;

	TIMER_CALLBACK_MEMBER(clear_busy_flag);
};

#endif // MAME_NICHIBUTSU_NBMJ8900_H
