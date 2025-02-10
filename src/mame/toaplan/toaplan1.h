// license:BSD-3-Clause
// copyright-holders:Darren Olafson, Quench
/***************************************************************************
                ToaPlan game hardware from 1988-1991
                ------------------------------------
****************************************************************************/
#ifndef MAME_TOAPLAN_TOAPLAN1_H
#define MAME_TOAPLAN_TOAPLAN1_H

#pragma once

#include "cpu/m68000/m68000.h"
#include "cpu/tms32010/tms32010.h"
#include "machine/gen_latch.h"
#include "sound/ymopl.h"
#include "toaplan_scu.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

class toaplan1_state : public driver_device
{
public:
	toaplan1_state(const machine_config &mconfig, device_type type, const char *tag, bool large = false) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_ymsnd(*this, "ymsnd"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_bgpaletteram(*this, "bgpalette"),
		m_fgpaletteram(*this, "fgpalette"),
		m_sharedram(*this, "sharedram"),
		m_spriteram(*this, "spriteram", large ? 0x1000 : 0x800, ENDIANNESS_BIG),
		m_dswb_io(*this, "DSWB"),
		m_tjump_io(*this, "TJUMP")
	{ }

	void truxton(machine_config &config);
	void outzone(machine_config &config);
	void vimana(machine_config &config);
	void outzonecv(machine_config &config);
	void hellfire(machine_config &config);
	void zerowing(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	required_device<m68000_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<ym3812_device> m_ymsnd;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	required_shared_ptr<u16> m_bgpaletteram;
	required_shared_ptr<u16> m_fgpaletteram;

	optional_shared_ptr<u8> m_sharedram;
	memory_share_creator<u16> m_spriteram;

	optional_ioport m_dswb_io;
	optional_ioport m_tjump_io;

	std::unique_ptr<u16[]> m_tilevram[4];
	/*
	std::unique_ptr<u16[]> m_tilevram[3];   //  ||  Drawn in this order
	std::unique_ptr<u16[]> m_tilevram[2];   //  ||
	std::unique_ptr<u16[]> m_tilevram[1];   // \||/
	std::unique_ptr<u16[]> m_tilevram[0];   //  \/
	*/

	std::unique_ptr<u16[]> m_buffered_spriteram;
	std::unique_ptr<u16[]> m_spritesizeram;
	std::unique_ptr<u16[]> m_buffered_spritesizeram;

	u8 m_intenable = 0;

	s32 m_bcu_flipscreen;     /* Tile   controller flip flag */
	bool m_fcu_flipscreen;    /* Sprite controller flip flag */

	s32 m_pf_voffs = 0;
	s32 m_spriteram_offs = 0;

	s32 m_scrollx[4]{};
	s32 m_scrolly[4]{};

#ifdef MAME_DEBUG
	bool m_display_pf[4]{};
	bool m_displog = false;
#endif

	s32 m_tiles_offsetx = 0;
	s32 m_tiles_offsety = 0;

	tilemap_t *m_tilemap[4]{};

	void intenable_w(u8 data);
	u8 shared_r(offs_t offset);
	void shared_w(offs_t offset, u8 data);
	void reset_sound_w(u8 data);
	void coin_w(u8 data);

	u16 frame_done_r();
	void tile_offsets_w(offs_t offset, u16 data, u16 mem_mask = ~0);
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

	void screen_vblank(int state);
	void interrupt();

	void create_tilemaps();
	void vram_alloc();
	void spritevram_alloc();
	void set_scrolls();
	void register_common();
	void log_vram();
	void draw_sprites(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	virtual void reset_sound();
	void reset_callback(int state);

	void hellfire_main_map(address_map &map) ATTR_COLD;
	void hellfire_sound_io_map(address_map &map) ATTR_COLD;
	void outzone_main_map(address_map &map) ATTR_COLD;
	void outzone_sound_io_map(address_map &map) ATTR_COLD;
	void outzonecv_main_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
	void truxton_main_map(address_map &map) ATTR_COLD;
	void truxton_sound_io_map(address_map &map) ATTR_COLD;
	void vimana_hd647180_io_map(address_map &map) ATTR_COLD;
	void vimana_hd647180_mem_map(address_map &map) ATTR_COLD;
	void vimana_main_map(address_map &map) ATTR_COLD;
	void zerowing_main_map(address_map &map) ATTR_COLD;
	void zerowing_sound_io_map(address_map &map) ATTR_COLD;
};

class toaplan1_rallybik_state : public toaplan1_state
{
public:
	toaplan1_rallybik_state(const machine_config &mconfig, device_type type, const char *tag) :
		toaplan1_state(mconfig, type, tag, true),
		m_spritegen(*this, "scu")
	{
	}

	void rallybik(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_device<toaplan_scu_device> m_spritegen;

	void coin_counter_1_w(int state);
	void coin_counter_2_w(int state);
	void coin_lockout_1_w(int state);
	void coin_lockout_2_w(int state);
	u16 tileram_r(offs_t offset);
	void pri_cb(u8 priority, u32 &pri_mask);
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void screen_vblank(int state);

	void rallybik_main_map(address_map &map) ATTR_COLD;
	void rallybik_sound_io_map(address_map &map) ATTR_COLD;
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
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<tms32010_device> m_dsp;

	/* Demon world */
	s32 m_dsp_on = 0;
	s32 m_dsp_bio = 0;
	bool m_dsp_execute = false;
	u32 m_dsp_addr_w = 0;
	u32 m_main_ram_seg = 0;

	void dsp_addrsel_w(u16 data);
	u16 dsp_r();
	void dsp_w(u16 data);
	void dsp_bio_w(u16 data);
	int bio_r();
	void dsp_ctrl_w(u8 data);
	void dsp_int_w(int enable);

	void dsp_io_map(address_map &map) ATTR_COLD;
	void dsp_program_map(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;
	void sound_io_map(address_map &map) ATTR_COLD;
};

class toaplan1_samesame_state : public toaplan1_state
{
public:
	toaplan1_samesame_state(const machine_config &mconfig, device_type type, const char *tag) :
		toaplan1_state(mconfig, type, tag),
		m_soundlatch(*this, "soundlatch")
	{
	}

	void samesame(machine_config &config);

protected:
	virtual void reset_sound() override;

private:
	// Fire Shark sound
	required_device<generic_latch_8_device> m_soundlatch;

	u8 cmdavailable_r();
	u8 port_6_word_r();

	void screen_vblank(int state);

	void hd647180_io_map(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;
};

#endif // MAME_TOAPLAN_TOAPLAN1_H
