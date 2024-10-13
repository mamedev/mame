// license:BSD-3-Clause
// copyright-holders:Takahiro Nogi
#ifndef MAME_NICHIBUTSU_NBMJ8688_H
#define MAME_NICHIBUTSU_NBMJ8688_H

#pragma once

#include "video/hd61830.h"
#include "nb1413m3.h"
#include "emupal.h"

class nbmj8688_state : public driver_device
{
public:
	nbmj8688_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_nb1413m3(*this, "nb1413m3"),
		m_lcdc0(*this, "lcdc0"),
		m_lcdc1(*this, "lcdc1")
	{ }

	void NBMJDRV_4096(machine_config &config);
	void NBMJDRV_256(machine_config &config);
	void NBMJDRV_65536(machine_config &config);
	void mbmj_h12bit(machine_config &config);
	void mbmj_p12bit(machine_config &config);
	void mbmj_p16bit(machine_config &config);
	void mbmj_p16bit_LCD(machine_config &config);
	void swinggal(machine_config &config);
	void korinai(machine_config &config);
	void livegal(machine_config &config);
	void apparel(machine_config &config);
	void kyuhito(machine_config &config);
	void bijokkoy(machine_config &config);
	void barline(machine_config &config);
	void bijokkog(machine_config &config);
	void korinaim(machine_config &config);
	void ryuuha(machine_config &config);
	void seiham(machine_config &config);
	void orangeci(machine_config &config);
	void citylove(machine_config &config);
	void otonano(machine_config &config);
	void ojousanm(machine_config &config);
	void mcitylov(machine_config &config);
	void iemotom(machine_config &config);
	void crystalg(machine_config &config);
	void crystal2(machine_config &config);
	void secolove(machine_config &config);
	void orangec(machine_config &config);
	void mjsikaku(machine_config &config);
	void housemn2(machine_config &config);
	void kanatuen(machine_config &config);
	void nightlov(machine_config &config);
	void kaguya2(machine_config &config);
	void mjgaiden(machine_config &config);
	void mjcamera(machine_config &config);
	void mmsikaku(machine_config &config);
	void housemnq(machine_config &config);
	void idhimitu(machine_config &config);
	void iemoto(machine_config &config);
	void kaguya(machine_config &config);
	void vipclub(machine_config &config);
	void ojousan(machine_config &config);
	void seiha(machine_config &config);
	void bikkuri(machine_config &config);

	void init_kyuhito();
	void init_idhimitu();
	void init_kaguya2();
	void init_mjcamera();
	void init_kanatuen();

private:
	required_device<cpu_device> m_maincpu;
	required_device<nb1413m3_device> m_nb1413m3;
	optional_device<hd61830_device> m_lcdc0;
	optional_device<hd61830_device> m_lcdc1;

	// defined in video_start
	int m_gfxmode = 0;

	int m_scrolly = 0;
	int m_blitter_destx = 0;
	int m_blitter_desty = 0;
	int m_blitter_sizex = 0;
	int m_blitter_sizey = 0;
	int m_blitter_direction_x = 0;
	int m_blitter_direction_y = 0;
	int m_blitter_src_addr = 0;
	int m_gfxrom = 0;
	int m_dispflag = 0;
	int m_gfxflag2 = 0;
	int m_gfxflag3 = 0;
	int m_flipscreen = 0;
	int m_screen_refresh = 0;
	std::unique_ptr<bitmap_ind16> m_tmpbitmap;
	std::unique_ptr<uint16_t[]> m_videoram;
	std::unique_ptr<uint8_t[]> m_clut;
	int m_flipscreen_old = 0;
	emu_timer *m_blitter_timer = nullptr;

	// common
	uint8_t ff_r();
	void clut_w(offs_t offset, uint8_t data);
	void blitter_w(offs_t offset, uint8_t data);
	void scrolly_w(uint8_t data);

	void mjsikaku_gfxflag2_w(uint8_t data);
	void mjsikaku_gfxflag3_w(uint8_t data);
	void mjsikaku_romsel_w(uint8_t data);
	void secolove_romsel_w(uint8_t data);
	void crystalg_romsel_w(uint8_t data);
	void seiha_romsel_w(uint8_t data);
	void HD61830B_both_instr_w(uint8_t data);
	void HD61830B_both_data_w(uint8_t data);
	void barline_output_w(uint8_t data);

	DECLARE_VIDEO_START(mbmj8688_pure_12bit);
	void mbmj8688_12bit(palette_device &palette) const;
	DECLARE_VIDEO_START(mbmj8688_pure_16bit_LCD);
	void mbmj8688_16bit(palette_device &palette) const;
	void mbmj8688_lcd(palette_device &palette) const;
	DECLARE_VIDEO_START(mbmj8688_8bit);
	void mbmj8688_8bit(palette_device &palette) const;
	DECLARE_VIDEO_START(mbmj8688_hybrid_16bit);
	DECLARE_VIDEO_START(mbmj8688_hybrid_12bit);
	DECLARE_VIDEO_START(mbmj8688_pure_16bit);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void vramflip();
	void update_pixel(int x, int y);
	void writeram_low(int x, int y, int color);
	void writeram_high(int x, int y, int color);
	void gfxdraw(int gfxtype);
	void common_video_start();
	void postload();

	void barline_io_map(address_map &map) ATTR_COLD;
	void bikkuri_map(address_map &map) ATTR_COLD;
	void bikkuri_io_map(address_map &map) ATTR_COLD;
	void crystalg_io_map(address_map &map) ATTR_COLD;
	void iemoto_io_map(address_map &map) ATTR_COLD;
	void kaguya_io_map(address_map &map) ATTR_COLD;
	void mjgaiden_io_map(address_map &map) ATTR_COLD;
	void mjsikaku_io_map(address_map &map) ATTR_COLD;
	void mjsikaku_map(address_map &map) ATTR_COLD;
	void mmsikaku_io_map(address_map &map) ATTR_COLD;
	void ojousan_map(address_map &map) ATTR_COLD;
	void otonano_io_map(address_map &map) ATTR_COLD;
	void p16bit_LCD_io_map(address_map &map) ATTR_COLD;
	void secolove_io_map(address_map &map) ATTR_COLD;
	void secolove_map(address_map &map) ATTR_COLD;
	void seiha_io_map(address_map &map) ATTR_COLD;

	TIMER_CALLBACK_MEMBER(clear_busy_flag);
};

#endif // MAME_NICHIBUTSU_NBMJ8688_H
