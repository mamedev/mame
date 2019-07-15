// license:BSD-3-Clause
// copyright-holders:Darren Olafson, Quench
/***************************************************************************
                ToaPlan game hardware from 1988-1991
                ------------------------------------
****************************************************************************/
#ifndef MAME_INCLUDES_TOAPLAN1_H
#define MAME_INCLUDES_TOAPLAN1_H

#pragma once

#include "cpu/m68000/m68000.h"
#include "cpu/tms32010/tms32010.h"
#include "sound/3812intf.h"
#include "video/toaplan_scu.h"
#include "emupal.h"
#include "screen.h"

class toaplan1_state : public driver_device
{
public:
	toaplan1_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_bgpaletteram(*this, "bgpalette"),
		m_fgpaletteram(*this, "fgpalette"),
		m_sharedram(*this, "sharedram"),
		m_dswb_io(*this, "DSWB"),
		m_tjump_io(*this, "TJUMP"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_ymsnd(*this, "ymsnd"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette")
	{ }

	void truxton(machine_config &config);
	void outzone(machine_config &config);
	void vimana(machine_config &config);
	void outzonecv(machine_config &config);
	void hellfire(machine_config &config);
	void zerowing(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

	required_shared_ptr<u16> m_bgpaletteram;
	required_shared_ptr<u16> m_fgpaletteram;

	optional_shared_ptr<u8> m_sharedram;

	optional_ioport m_dswb_io;
	optional_ioport m_tjump_io;

	int m_intenable;

	std::unique_ptr<u16[]> m_tilevram[4];
	/*
	std::unique_ptr<u16[]> m_tilevram[3];   //  ||  Drawn in this order
	std::unique_ptr<u16[]> m_tilevram[2];   //  ||
	std::unique_ptr<u16[]> m_tilevram[1];   // \||/
	std::unique_ptr<u16[]> m_tilevram[0];   //  \/
	*/

	optional_shared_ptr<u16> m_spriteram;
	std::unique_ptr<u16[]> m_buffered_spriteram;
	std::unique_ptr<u16[]> m_spritesizeram;
	std::unique_ptr<u16[]> m_buffered_spritesizeram;

	s32 m_bcu_flipscreen;     /* Tile   controller flip flag */
	s32 m_fcu_flipscreen;     /* Sprite controller flip flag */

	s32 m_pf_voffs;
	s32 m_spriteram_offs;

	s32 m_scrollx[4];
	s32 m_scrolly[4];

#ifdef MAME_DEBUG
	int m_display_pf[4];
	int m_displog;
#endif

	s32 m_tiles_offsetx;
	s32 m_tiles_offsety;

	tilemap_t *m_tilemap[4];

	void intenable_w(u8 data);
	u8 shared_r(offs_t offset);
	void shared_w(offs_t offset, u8 data);
	void reset_sound_w(u8 data);
	void coin_w(u8 data);

	u16 frame_done_r();
	DECLARE_WRITE16_MEMBER(tile_offsets_w);
	void bcu_flipscreen_w(u8 data);
	void fcu_flipscreen_w(u8 data);
	u16 spriteram_offs_r();
	void spriteram_offs_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void bgpalette_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void fgpalette_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 spriteram_r();
	void spriteram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 spritesizeram_r();
	void spritesizeram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void bcu_control_w(offs_t offset, u16 data);
	u16 tileram_offs_r();
	void tileram_offs_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 tileram_r(offs_t offset);
	void tileram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 scroll_regs_r(offs_t offset);
	void scroll_regs_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	u8 vimana_dswb_invert_r();
	u8 vimana_tjump_invert_r();

	template<unsigned Layer> TILE_GET_INFO_MEMBER(get_tile_info);

	DECLARE_MACHINE_RESET(zerowing);
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	DECLARE_WRITE_LINE_MEMBER(screen_vblank);
	void interrupt();

	void create_tilemaps();
	void vram_alloc();
	void spritevram_alloc();
	void set_scrolls();
	void register_common();
	void log_vram();
	void draw_sprites(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect );
	void draw_sprite_custom(screen_device &screen, bitmap_rgb32 &dest_bmp, const rectangle &clip, gfx_element *gfx,
		u32 code, u32 color, int flipx, int flipy, int sx, int sy,
		int priority);
	void reset_sound();
	DECLARE_WRITE_LINE_MEMBER(reset_callback);
	required_device<m68000_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<ym3812_device> m_ymsnd;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	void hellfire_main_map(address_map &map);
	void hellfire_sound_io_map(address_map &map);
	void outzone_main_map(address_map &map);
	void outzone_sound_io_map(address_map &map);
	void outzonecv_main_map(address_map &map);
	void sound_map(address_map &map);
	void truxton_main_map(address_map &map);
	void truxton_sound_io_map(address_map &map);
	void vimana_hd647180_io_map(address_map &map);
	void vimana_hd647180_mem_map(address_map &map);
	void vimana_main_map(address_map &map);
	void zerowing_main_map(address_map &map);
	void zerowing_sound_io_map(address_map &map);
};

class toaplan1_rallybik_state : public toaplan1_state
{
public:
	toaplan1_rallybik_state(const machine_config &mconfig, device_type type, const char *tag) :
		toaplan1_state(mconfig, type, tag),
		m_spritegen(*this, "scu")
	{
	}

	void rallybik(machine_config &config);

protected:
	virtual void video_start() override;

private:
	DECLARE_WRITE_LINE_MEMBER(coin_counter_1_w);
	DECLARE_WRITE_LINE_MEMBER(coin_counter_2_w);
	DECLARE_WRITE_LINE_MEMBER(coin_lockout_1_w);
	DECLARE_WRITE_LINE_MEMBER(coin_lockout_2_w);
	u16 tileram_r(offs_t offset);
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(screen_vblank);

	required_device<toaplan_scu_device> m_spritegen;
	void rallybik_main_map(address_map &map);
	void rallybik_sound_io_map(address_map &map);
};

class toaplan1_demonwld_state : public toaplan1_state
{
public:
	toaplan1_demonwld_state(const machine_config &mconfig, device_type type, const char *tag) :
		toaplan1_state(mconfig, type, tag),
		m_dsp(*this, "dsp")
	{
	}

	void demonwld(machine_config &config);

protected:
	virtual void device_post_load() override;
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	/* Demon world */
	int m_dsp_on;
	int m_dsp_bio;
	int m_dsp_execute;
	u32 m_dsp_addr_w;
	u32 m_main_ram_seg;

	void dsp_addrsel_w(u16 data);
	u16 dsp_r();
	void dsp_w(u16 data);
	void dsp_bio_w(u16 data);
	DECLARE_READ_LINE_MEMBER(bio_r);
	void dsp_ctrl_w(u8 data);
	void dsp_int_w(int enable);

	required_device<tms32010_device> m_dsp;
	void dsp_io_map(address_map &map);
	void dsp_program_map(address_map &map);
	void main_map(address_map &map);
	void sound_io_map(address_map &map);
};

class toaplan1_samesame_state : public toaplan1_state
{
public:
	toaplan1_samesame_state(const machine_config &mconfig, device_type type, const char *tag) :
		toaplan1_state(mconfig, type, tag)
	{
	}

	void samesame(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	// Fire Shark sound
	u8 m_to_mcu;
	u8 m_cmdavailable;

	void mcu_w(u8 data);
	u8 soundlatch_r();
	void sound_done_w(u8 data);
	u8 cmdavailable_r();
	u8 port_6_word_r();

	DECLARE_WRITE_LINE_MEMBER(screen_vblank);

	void hd647180_io_map(address_map &map);
	void hd647180_mem_map(address_map &map);
	void main_map(address_map &map);
};

#endif // MAME_INCLUDES_TOAPLAN1_H
