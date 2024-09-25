// license:BSD-3-Clause
// copyright-holders:David Haywood, ElSemi
#ifndef MAME_IGS_PGM_H
#define MAME_IGS_PGM_H

#pragma once

#include "igs025.h"
#include "igs022.h"
#include "igs028.h"
#include "pgmcrypt.h"

#include "cpu/arm7/arm7.h"
#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/nvram.h"
#include "machine/timer.h"
#include "machine/v3021.h"
#include "sound/ics2115.h"
#include "emupal.h"
#include "tilemap.h"


class pgm_state : public driver_device
{
public:
	pgm_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_mainram(*this, "sram")
		, m_region(*this, "Region")
		, m_regionhack(*this, "RegionHack")
		, m_maincpu(*this, "maincpu")
		, m_videoregs(*this, "videoregs")
		, m_videoram(*this, "videoram")
		, m_z80_mainram(*this, "z80_mainram")
		, m_soundcpu(*this, "soundcpu")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_soundlatch(*this, "soundlatch")
		, m_soundlatch3(*this, "soundlatch3")
		, m_ics(*this, "ics")
		, m_adata(*this, "sprcol")
		, m_bdata(*this, "sprmask")
	{
		m_irq4_disabled = 0;
	}

	void init_pgm();

	void pgm_basic_init(bool set_bank = true);
	void pgm(machine_config &config);
	void pgmbase(machine_config &config);

protected:
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	/* memory pointers */
	required_shared_ptr<u16> m_mainram;

	optional_ioport m_region;
	optional_ioport m_regionhack;

	/* devices */
	required_device<cpu_device> m_maincpu;

	/* hack */
	int m_irq4_disabled = 0;

	void pgm_base_mem(address_map &map) ATTR_COLD;
	void pgm_mem(address_map &map) ATTR_COLD;

private:
	/* memory pointers */
	required_shared_ptr<u16> m_videoregs;
	required_shared_ptr<u16> m_videoram;
	required_shared_ptr<u8>  m_z80_mainram;
	u16 *                    m_bg_videoram = nullptr;
	u16 *                    m_tx_videoram = nullptr;
	u16 *                    m_rowscrollram = nullptr;

	/* video-related */
	struct sprite_t
	{
		int x = 0, y = 0;
		bool xgrow = false, ygrow = false;
		u32 xzoom = 0, yzoom = 0;
		u32 color = 0, offs = 0;
		u32 width = 0, height = 0;
		u8 flip = 0, pri = 0;
	};

	std::unique_ptr<sprite_t[]> m_spritelist;
	struct sprite_t *m_sprite_ptr_pre = nullptr;
	tilemap_t     *m_bg_tilemap = nullptr;
	tilemap_t     *m_tx_tilemap = nullptr;

	/* devices */
	required_device<cpu_device>             m_soundcpu;
	required_device<gfxdecode_device>       m_gfxdecode;
	required_device<palette_device>         m_palette;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<generic_latch_8_device> m_soundlatch3;
	required_device<ics2115_device>         m_ics;

	/* used by rendering */
	required_region_ptr<u16> m_adata;
	required_region_ptr<u16> m_bdata;
	u32 m_aoffset = 0;
	u8 m_abit = 0;
	u32 m_boffset = 0;

	u16 videoram_r(offs_t offset);
	void videoram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void coin_counter_w(u16 data);
	u8 z80_ram_r(offs_t offset);
	void z80_ram_w(offs_t offset, u8 data);
	void z80_reset_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void z80_ctrl_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void m68k_l1_w(u8 data);
	void z80_l3_w(u8 data);
	void tx_videoram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void bg_videoram_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	TILE_GET_INFO_MEMBER(get_tx_tile_info);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_vblank(int state);
	TIMER_DEVICE_CALLBACK_MEMBER(interrupt);

	inline void pgm_draw_pix(int xdrawpos, int pri, u16* dest, u8* destpri, const rectangle &cliprect, u16 srcdat);
	inline void pgm_draw_pix_nopri(int xdrawpos, u16* dest, u8* destpri, const rectangle &cliprect, u16 srcdat);
	inline void pgm_draw_pix_pri(int xdrawpos, u16* dest, u8* destpri, const rectangle &cliprect, u16 srcdat);
	inline u8 get_sprite_pix();
	void draw_sprite_line(int wide, u16* dest, u8* destpri, const rectangle &cliprect, int xzoom, bool xgrow, int flip, int xpos, int pri, int realxsize, int palt, bool draw);
	void draw_sprite_new_zoomed(int wide, int high, int xpos, int ypos, int palt, int flip, bitmap_ind16 &bitmap, const rectangle &cliprect, bitmap_ind8 &priority_bitmap, u32 xzoom, bool xgrow, u32 yzoom, bool ygrow, int pri);
	void draw_sprite_line_basic(int wide, u16* dest, u8* destpri, const rectangle &cliprect, int flip, int xpos, int pri, int realxsize, int palt, bool draw);
	void draw_sprite_new_basic(int wide, int high, int xpos, int ypos, int palt, int flip, bitmap_ind16 &bitmap, const rectangle &cliprect, bitmap_ind8 &priority_bitmap, int pri);
	void draw_sprites(bitmap_ind16& spritebitmap, const rectangle &cliprect, bitmap_ind8& priority_bitmap);
	void get_sprites();
	void pgm_basic_mem(address_map &map) ATTR_COLD;
	void pgm_z80_io(address_map &map) ATTR_COLD;
	void pgm_z80_mem(address_map &map) ATTR_COLD;
};



/*----------- defined in drivers/pgm.cpp -----------*/

INPUT_PORTS_EXTERN(pgm);

extern gfx_decode_entry const gfx_pgm[];

#endif // MAME_IGS_PGM_H
