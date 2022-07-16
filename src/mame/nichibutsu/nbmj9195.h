// license:BSD-3-Clause
// copyright-holders:Takahiro Nogi
/******************************************************************************

    nbmj9195 - Nichibutsu Mahjong games for years 1991-1995

******************************************************************************/
#ifndef MAME_INCLUDES_NBMJ9195_H
#define MAME_INCLUDES_NBMJ9195_H

#pragma once

#include "cpu/z80/tmpz84c011.h"
#include "nb1413m3.h"      // needed for mahjong input controller
#include "machine/gen_latch.h"
#include "emupal.h"
#include "screen.h"

#define VRAM_MAX    2

#define SCANLINE_MIN    0
#define SCANLINE_MAX    512


class nbmj9195_state : public driver_device
{
public:
	nbmj9195_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_palette_ptr(*this, "paletteram"),
		m_blit_region(*this, "blitter")
	{ }

	void NBMJDRV1_base(machine_config &config);
	void NBMJDRV1(machine_config &config);
	void NBMJDRV2(machine_config &config);
	void NBMJDRV3(machine_config &config);
	void patimono(machine_config &config);
	void mjuraden(machine_config &config);
	void psailor1(machine_config &config);
	void ngpgal(machine_config &config);
	void mjgottsu(machine_config &config);
	void mkeibaou(machine_config &config);
	void gal10ren(machine_config &config);
	void mscoutm(machine_config &config);
	void imekura(machine_config &config);
	void mkoiuraa(machine_config &config);
	void mjkoiura(machine_config &config);
	void janbari(machine_config &config);
	void mjlaman(machine_config &config);
	void yosimotm(machine_config &config);
	void cmehyou(machine_config &config);
	void sailorwr(machine_config &config);
	void koinomp(machine_config &config);
	void sailorws(machine_config &config);
	void mjegolf(machine_config &config);
	void renaiclb(machine_config &config);
	void psailor2(machine_config &config);
	void yosimoto(machine_config &config);
	void pachiten(machine_config &config);
	void jituroku(machine_config &config);
	void mmehyou(machine_config &config);
	void bakuhatu(machine_config &config);
	void ultramhm(machine_config &config);
	void otatidai(machine_config &config);

	void init_nbmj9195();

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

	TIMER_CALLBACK_MEMBER(clear_busy_flag);

private:
	required_device<tmpz84c011_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	optional_shared_ptr<uint8_t> m_palette_ptr; //shabdama doesn't use it at least for now

	required_region_ptr<uint8_t> m_blit_region;

	int m_inputport = 0;
	int m_dipswbitsel = 0;
	int m_outcoin_flag = 0;
	int m_mscoutm_inputport = 0;
	int m_scrollx[VRAM_MAX];
	int m_scrolly[VRAM_MAX];
	int m_scrollx_raster[VRAM_MAX][SCANLINE_MAX];
	int m_scanline[VRAM_MAX];
	int m_blitter_destx[VRAM_MAX];
	int m_blitter_desty[VRAM_MAX];
	int m_blitter_sizex[VRAM_MAX];
	int m_blitter_sizey[VRAM_MAX];
	int m_blitter_src_addr[VRAM_MAX];
	int m_blitter_direction_x[VRAM_MAX];
	int m_blitter_direction_y[VRAM_MAX];
	int m_dispflag[VRAM_MAX];
	int m_flipscreen[VRAM_MAX];
	int m_clutmode[VRAM_MAX];
	int m_transparency[VRAM_MAX];
	int m_clutsel = 0;
	int m_screen_refresh = 0;
	int m_gfxflag2 = 0;
	int m_gfxdraw_mode = 0;
	int m_nb19010_busyctr = 0;
	int m_nb19010_busyflag = 0;
	bitmap_ind16 m_tmpbitmap[VRAM_MAX];
	std::unique_ptr<uint16_t[]> m_videoram[VRAM_MAX];
	std::unique_ptr<uint16_t[]> m_videoworkram[VRAM_MAX];
	std::unique_ptr<uint8_t[]> m_clut[VRAM_MAX];
	int m_flipscreen_old[VRAM_MAX];
	emu_timer *m_blitter_timer = nullptr;

	void soundbank_w(uint8_t data);
	void inputportsel_w(uint8_t data);
	uint8_t mscoutm_dipsw_0_r();
	uint8_t mscoutm_dipsw_1_r();
	uint8_t mscoutm_cpu_portb_r();
	uint8_t mscoutm_cpu_portc_r();
	uint8_t others_cpu_porta_r();
	uint8_t others_cpu_portb_r();
	uint8_t others_cpu_portc_r();
	void soundcpu_porte_w(uint8_t data);
	void palette_w(offs_t offset, uint8_t data);
	void nb22090_palette_w(offs_t offset, uint8_t data);
	void blitter_0_w(offs_t offset, uint8_t data);
	void blitter_1_w(offs_t offset, uint8_t data);
	uint8_t blitter_0_r(offs_t offset);
	uint8_t blitter_1_r(offs_t offset);
	void clut_0_w(offs_t offset, uint8_t data);
	void clut_1_w(offs_t offset, uint8_t data);
	void clutsel_w(uint8_t data);
	void gfxflag2_w(uint8_t data);
	void outcoin_flag_w(uint8_t data);
	void dipswbitsel_w(uint8_t data);
	void mscoutm_inputportsel_w(uint8_t data);

	DECLARE_VIDEO_START(_1layer);
	DECLARE_VIDEO_START(nb22090);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	int blitter_r(int offset, int vram);
	void blitter_w(int offset, int data, int vram);
	void clut_w(int offset, int data, int vram);
	void vramflip(int vram);
	void update_pixel(int vram, int x, int y);
	void gfxdraw(int vram);
	int dipsw_r();
	void postload();

	void cmehyou_io_map(address_map &map);
	void gal10ren_io_map(address_map &map);
	void imekura_io_map(address_map &map);
	void jituroku_io_map(address_map &map);
	void koinomp_io_map(address_map &map);
	void koinomp_map(address_map &map);
	void mjegolf_io_map(address_map &map);
	void mjegolf_map(address_map &map);
	void mjgottsu_io_map(address_map &map);
	void mjkoiura_io_map(address_map &map);
	void mjlaman_io_map(address_map &map);
	void mjuraden_io_map(address_map &map);
	void mjuraden_map(address_map &map);
	void mkeibaou_io_map(address_map &map);
	void mkoiuraa_io_map(address_map &map);
	void mmehyou_io_map(address_map &map);
	void mscoutm_io_map(address_map &map);
	void mscoutm_map(address_map &map);
	void ngpgal_io_map(address_map &map);
	void ngpgal_map(address_map &map);
	void otatidai_io_map(address_map &map);
	void pachiten_io_map(address_map &map);
	void patimono_io_map(address_map &map);
	void psailor1_io_map(address_map &map);
	void psailor2_io_map(address_map &map);
	void renaiclb_io_map(address_map &map);
	void sailorwr_io_map(address_map &map);
	void sailorws_io_map(address_map &map);
	void sailorws_map(address_map &map);
	void sailorws_sound_io_map(address_map &map);
	void sailorws_sound_map(address_map &map);
	void yosimotm_io_map(address_map &map);
	void yosimoto_io_map(address_map &map);
};

#endif // MAME_INCLUDES_NBMJ9195_H
