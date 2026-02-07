// license:BSD-3-Clause
// copyright-holders:Takahiro Nogi
#ifndef MAME_NICHIBUTSU_NIYANPAI_H
#define MAME_NICHIBUTSU_NIYANPAI_H

#pragma once

#include "nb1413m3.h"
#include "nichisnd.h"

#include "cpu/m68000/tmp68301.h"
#include "machine/ticket.h"

#include "emupal.h"
#include "screen.h"


class niyanpai_state : public driver_device
{
public:
	niyanpai_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag) ,
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_io_dipsw(*this, "DSW%c", 'A')
	{ }

	void niyanpai(machine_config &config) ATTR_COLD;

	void init_niyanpai() ATTR_COLD;

protected:
	virtual void video_start() override ATTR_COLD;

	uint16_t dipsw_r();
	uint16_t palette_r(offs_t offset);
	void palette_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void tmp68301_parallel_port_w(uint16_t data);

	void video_sound_map(address_map &map) ATTR_COLD;

	required_device<tmp68301_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_ioport_array<2> m_io_dipsw;

private:
	static constexpr unsigned VRAM_MAX = 3;

	void blitter_0_w(offs_t offset, uint8_t data);
	void blitter_1_w(offs_t offset, uint8_t data);
	void blitter_2_w(offs_t offset, uint8_t data);
	uint8_t blitter_0_r(offs_t offset);
	uint8_t blitter_1_r(offs_t offset);
	uint8_t blitter_2_r(offs_t offset);
	void clut_0_w(offs_t offset, uint8_t data);
	void clut_1_w(offs_t offset, uint8_t data);
	void clut_2_w(offs_t offset, uint8_t data);
	void clutsel_0_w(uint8_t data);
	void clutsel_1_w(uint8_t data);
	void clutsel_2_w(uint8_t data);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	int blitter_r(int vram, int offset);
	void blitter_w(int vram, int offset, uint8_t data);
	void clutsel_w(int vram, uint8_t data);
	void clut_w(int vram, int offset, uint8_t data);
	void vramflip(int vram);
	void update_pixel(int vram, int x, int y);
	void gfxdraw(int vram);

	void vblank_irq(int state);

	void niyanpai_map(address_map &map) ATTR_COLD;

	TIMER_CALLBACK_MEMBER(clear_busy_flag);

	// common
	int m_scrollx[VRAM_MAX];
	int m_scrolly[VRAM_MAX];
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
	int m_clutsel[VRAM_MAX];
	int m_screen_refresh = 0;
	int m_nb19010_busyctr = 0;
	int m_nb19010_busyflag = 0;
	bitmap_ind16 m_tmpbitmap[VRAM_MAX];
	std::unique_ptr<uint16_t[]> m_videoram[VRAM_MAX];
	std::unique_ptr<uint16_t[]> m_videoworkram[VRAM_MAX];
	std::unique_ptr<uint16_t[]> m_palette_ptr;
	std::unique_ptr<uint8_t[]> m_clut[VRAM_MAX];
	int m_flipscreen_old[VRAM_MAX];
	emu_timer *m_blitter_timer = nullptr;
};


class musobana_state : public niyanpai_state
{
public:
	musobana_state(const machine_config &mconfig, device_type type, const char *tag) :
		niyanpai_state(mconfig, type, tag) ,
		m_hopper(*this, "hopper"),
		m_io_key(*this, "KEY%u", 0U)
	{ }

	void musobana(machine_config &config) ATTR_COLD;
	void mhhonban(machine_config &config) ATTR_COLD;
	void zokumahj(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void musobana_tmp68301_parallel_port_w(uint16_t data);
	uint16_t musobana_inputport_0_r();
	void musobana_inputport_w(uint16_t data);

	void musobana_common_map(address_map &map) ATTR_COLD;
	void musobana_map(address_map &map) ATTR_COLD;
	void mhhonban_map(address_map &map) ATTR_COLD;
	void zokumahj_map(address_map &map) ATTR_COLD;

	required_device<hopper_device> m_hopper;
	required_ioport_array<5> m_io_key;

	uint16_t m_musobana_inputport = 0;
};

#endif // MAME_NICHIBUTSU_NIYANPAI_H
