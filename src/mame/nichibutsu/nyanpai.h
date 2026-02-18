// license:BSD-3-Clause
// copyright-holders:Takahiro Nogi
#ifndef MAME_NICHIBUTSU_NYANPAI_H
#define MAME_NICHIBUTSU_NYANPAI_H

#pragma once

#include "nb1413m3.h"
#include "nichisnd.h"

#include "cpu/m68000/tmp68301.h"
#include "machine/ticket.h"

#include "emupal.h"
#include "screen.h"


class nyanpai_state : public driver_device
{
public:
	nyanpai_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag) ,
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_paletteram(*this, "paletteram"),
		m_gfx(*this, "gfx"),
		m_io_dipsw(*this, "DSW%c", 'A'),
		m_layer{*this, *this, *this}
	{ }

	void nyanpai(machine_config &config) ATTR_COLD;

	void init_nyanpai() ATTR_COLD;

protected:
	virtual void video_start() override ATTR_COLD;

	u16 dipsw_r();
	void palette_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void tmp68301_parallel_port_w(u16 data);

	template <unsigned Layer> void layer_map(address_map &map) ATTR_COLD;
	void video_sound_map(address_map &map) ATTR_COLD;

	struct nyanpai_layer
	{
		nyanpai_layer(nyanpai_state &host)
			: m_host(host)
			, m_screen_width(1)
			, m_screen_height(1)
			, m_colbase(0)
			, m_scrollx(0)
			, m_scrolly(0)
			, m_blitter_destx(0)
			, m_blitter_desty(0)
			, m_blitter_sizex(0)
			, m_blitter_sizey(0)
			, m_blitter_src_addr(0)
			, m_blitter_direction_x(0)
			, m_blitter_direction_y(0)
			, m_dispflag(0)
			, m_flipscreen(0)
			, m_clutmode(0)
			, m_transparency(0)
			, m_clutsel(0)
			, m_flipscreen_old(-1)
		{
		}

		u8 layer_r(offs_t offset);
		void layer_w(offs_t offset, u8 data);
		void clut_w(offs_t offset, u8 data)
		{
			m_clut[((m_clutsel & 0xff) * 0x10) + (offset & 0x0f)] = data;
		}
		void clutsel_w(u8 data) { m_clutsel = data; }
		void update_pixel(int x, int y);
		void vramflip();
		void gfxdraw();

		// configurations
		nyanpai_state &m_host;
		s32 m_screen_width;
		s32 m_screen_height;
		u16 m_colbase;

		// internal states
		s32 m_scrollx;
		s32 m_scrolly;
		s32 m_blitter_destx;
		s32 m_blitter_desty;
		s32 m_blitter_sizex;
		s32 m_blitter_sizey;
		s32 m_blitter_src_addr;
		u8 m_blitter_direction_x;
		u8 m_blitter_direction_y;
		u8 m_dispflag;
		u8 m_flipscreen;
		u8 m_clutmode;
		u8 m_transparency;
		s32 m_clutsel;
		bitmap_ind16 m_tmpbitmap;
		std::unique_ptr<u16[]> m_videoram;
		std::unique_ptr<u16[]> m_videoworkram;
		std::unique_ptr<u8[]> m_clut;
		s32 m_flipscreen_old;
	};

	required_device<tmp68301_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_shared_ptr<u16> m_paletteram;
	required_region_ptr<u8> m_gfx;
	required_ioport_array<2> m_io_dipsw;

private:
	static constexpr unsigned VRAM_MAX = 3;

	template <unsigned Layer> void blitter_w(offs_t offset, u8 data);
	template <unsigned Layer> u8 blitter_r(offs_t offset);
	template <unsigned Layer> void clut_w(offs_t offset, u8 data);
	template <unsigned Layer> void clutsel_w(u8 data);
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void vramflip(nyanpai_layer &layer);
	void gfxdraw(nyanpai_layer &layer);

	void vblank_irq(int state);

	void nyanpai_map(address_map &map) ATTR_COLD;

	TIMER_CALLBACK_MEMBER(clear_busy_flag);

	// common
	nyanpai_layer m_layer[VRAM_MAX];

	s32 m_screen_refresh = 0;
	s32 m_nb19010_busyctr = 0;
	s32 m_nb19010_busyflag = 0;
	emu_timer *m_blitter_timer = nullptr;
};


class musobana_state : public nyanpai_state
{
public:
	musobana_state(const machine_config &mconfig, device_type type, const char *tag) :
		nyanpai_state(mconfig, type, tag) ,
		m_hopper(*this, "hopper"),
		m_io_key(*this, "KEY%u", 0U)
	{ }

	void musobana(machine_config &config) ATTR_COLD;
	void mhhonban(machine_config &config) ATTR_COLD;
	void zokumahj(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void musobana_tmp68301_parallel_port_w(u16 data);
	u16 input_matrix_r();
	void input_matrix_w(u16 data);

	void musobana_common_map(address_map &map) ATTR_COLD;
	void musobana_map(address_map &map) ATTR_COLD;
	void mhhonban_map(address_map &map) ATTR_COLD;
	void zokumahj_map(address_map &map) ATTR_COLD;

	required_device<hopper_device> m_hopper;
	required_ioport_array<5> m_io_key;

	u16 m_input_matrix = 0;
};

#endif // MAME_NICHIBUTSU_NYANPAI_H
