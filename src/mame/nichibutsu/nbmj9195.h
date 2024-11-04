// license:BSD-3-Clause
// copyright-holders:Takahiro Nogi
/******************************************************************************

    nbmj9195 - Nichibutsu Mahjong games for years 1991-1995

******************************************************************************/

#ifndef MAME_NICHIBUTSU_NBMJ9195_H
#define MAME_NICHIBUTSU_NBMJ9195_H

#pragma once

#include "nbmjctrl.h"

#include "cpu/z80/tmpz84c011.h"
#include "machine/74166.h"

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
		m_keys{ { *this, "P1_KEY%u", 0U }, { *this, "P2_KEY%u", 0U } },
		m_coin(*this, "P%u_COIN", 1U),
		m_system(*this, "SYSTEM"),
		m_dsw(*this, "DSW%c", 'A'),
		m_dsw_shifter(*this, "ttl166_%u", 1U),
		m_palette_ptr(*this, "paletteram"),
		m_blit_region(*this, "blitter")
	{ }

	int hopper_r();

	void nbmjtype1(machine_config &config);
	void nbmjtype2(machine_config &config);

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

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(clear_busy_flag);

private:
	required_device<tmpz84c011_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_ioport_array<5> m_keys[2];
	optional_ioport_array<2> m_coin;
	optional_ioport m_system;
	required_ioport_array<2> m_dsw;
	optional_device_array<ttl166_device, 2> m_dsw_shifter;

	optional_shared_ptr<uint8_t> m_palette_ptr; //shabdama doesn't use it at least for now

	required_region_ptr<uint8_t> m_blit_region;

	uint8_t m_key_select = 0;
	int m_dsw_data = 0;
	int m_outcoin_flag = 0;

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

	void key_select_w(uint8_t data);
	uint8_t mscoutm_cpu_portb_r();
	uint8_t mscoutm_cpu_portc_r();
	uint8_t others_cpu_portb_r();
	uint8_t others_cpu_portc_r();
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

	DECLARE_VIDEO_START(_1layer);
	DECLARE_VIDEO_START(nb22090);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	int blitter_r(int offset, int vram);
	void blitter_w(int offset, int data, int vram);
	void clut_w(int offset, int data, int vram);
	void vramflip(int vram);
	void update_pixel(int vram, int x, int y);
	void gfxdraw(int vram);
	void postload();

	void cmehyou_io_map(address_map &map) ATTR_COLD;
	void gal10ren_io_map(address_map &map) ATTR_COLD;
	void imekura_io_map(address_map &map) ATTR_COLD;
	void jituroku_io_map(address_map &map) ATTR_COLD;
	void koinomp_io_map(address_map &map) ATTR_COLD;
	void koinomp_map(address_map &map) ATTR_COLD;
	void mjegolf_io_map(address_map &map) ATTR_COLD;
	void mjegolf_map(address_map &map) ATTR_COLD;
	void mjgottsu_io_map(address_map &map) ATTR_COLD;
	void mjkoiura_io_map(address_map &map) ATTR_COLD;
	void mjlaman_io_map(address_map &map) ATTR_COLD;
	void mjuraden_io_map(address_map &map) ATTR_COLD;
	void mjuraden_map(address_map &map) ATTR_COLD;
	void mkeibaou_io_map(address_map &map) ATTR_COLD;
	void mkoiuraa_io_map(address_map &map) ATTR_COLD;
	void mmehyou_io_map(address_map &map) ATTR_COLD;
	void mscoutm_io_map(address_map &map) ATTR_COLD;
	void mscoutm_map(address_map &map) ATTR_COLD;
	void ngpgal_io_map(address_map &map) ATTR_COLD;
	void ngpgal_map(address_map &map) ATTR_COLD;
	void otatidai_io_map(address_map &map) ATTR_COLD;
	void pachiten_io_map(address_map &map) ATTR_COLD;
	void patimono_io_map(address_map &map) ATTR_COLD;
	void psailor1_io_map(address_map &map) ATTR_COLD;
	void psailor2_io_map(address_map &map) ATTR_COLD;
	void renaiclb_io_map(address_map &map) ATTR_COLD;
	void sailorwr_io_map(address_map &map) ATTR_COLD;
	void sailorws_io_map(address_map &map) ATTR_COLD;
	void sailorws_map(address_map &map) ATTR_COLD;
	void yosimotm_io_map(address_map &map) ATTR_COLD;
	void yosimoto_io_map(address_map &map) ATTR_COLD;
};

#endif // MAME_NICHIBUTSU_NBMJ9195_H
