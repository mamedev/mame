// license:BSD-3-Clause
// copyright-holders:David Haywood, ElSemi
#ifndef MAME_INCLUDES_PGM_H
#define MAME_INCLUDES_PGM_H

#pragma once

#include "machine/igs025.h"
#include "machine/igs022.h"
#include "machine/igs028.h"
#include "machine/pgmcrypt.h"

#include "cpu/arm7/arm7.h"
#include "cpu/arm7/arm7core.h"
#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/nvram.h"
#include "machine/timer.h"
#include "machine/v3021.h"
#include "sound/ics2115.h"
#include "screen.h"
#include "emupal.h"

#define PGMARM7LOGERROR 0

class pgm_state : public driver_device
{
public:
	pgm_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_videoregs(*this, "videoregs")
		, m_videoram(*this, "videoram")
		, m_z80_mainram(*this, "z80_mainram")
		, m_mainram(*this, "sram")
		, m_maincpu(*this, "maincpu")
		, m_soundcpu(*this, "soundcpu")
		, m_screen(*this, "screen")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_soundlatch(*this, "soundlatch")
		, m_soundlatch3(*this, "soundlatch3")
		, m_ics(*this, "ics")
		, m_mainbank(*this, "mainbank")
		, m_bdata(*this, "sprmask")
	{
		m_irq4_disabled = 0;
	}

	void init_pgm();

	void pgm(machine_config &config);
protected:
	/* memory pointers */
	required_shared_ptr<u16> m_videoregs;
	required_shared_ptr<u16> m_videoram;
	required_shared_ptr<u8> m_z80_mainram;
	required_shared_ptr<u16> m_mainram;
	u16 *      m_bg_videoram;
	u16 *      m_tx_videoram;
	u16 *      m_rowscrollram;
	std::unique_ptr<u8[]>      m_sprite_a_region;
	size_t        m_sprite_a_region_size;
	std::unique_ptr<u16[]>     m_spritebufferram; // buffered spriteram

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_soundcpu;
	required_device<screen_device> m_screen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<generic_latch_8_device> m_soundlatch3;
	required_device<ics2115_device> m_ics;

	optional_memory_bank m_mainbank;

	/* hack */
	int m_irq4_disabled;

	virtual void video_start() override;
	virtual void machine_reset() override;

	void base_mem(address_map &map);
	void pgm_mem(address_map &map);
private:
	/* video-related */
	tilemap_t     *m_bg_tilemap;
	tilemap_t     *m_tx_tilemap;

	/* used by rendering */
	required_region_ptr<u8> m_bdata;
	u32 m_aoffset;
	u32 m_boffset;

	DECLARE_READ16_MEMBER(videoram_r);
	DECLARE_WRITE16_MEMBER(videoram_w);
	DECLARE_WRITE16_MEMBER(coin_counter_w);
	DECLARE_READ8_MEMBER(z80_ram_r);
	DECLARE_WRITE8_MEMBER(z80_ram_w);
	DECLARE_WRITE16_MEMBER(z80_reset_w);
	DECLARE_WRITE16_MEMBER(z80_ctrl_w);
	DECLARE_WRITE8_MEMBER(m68k_l1_w);
	DECLARE_WRITE8_MEMBER(z80_l3_w);
	void tx_videoram_w(offs_t offset, u16 data, u16 mem_mask);
	void bg_videoram_w(offs_t offset, u16 data, u16 mem_mask);

	TILE_GET_INFO_MEMBER(get_tx_tilemap_tile_info);
	TILE_GET_INFO_MEMBER(get_bg_tilemap_tile_info);
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(screen_vblank);
	TIMER_DEVICE_CALLBACK_MEMBER(interrupt);

	inline void pgm_draw_pix(u16 xdrawpos, u8 pri, u16* dest, u8* destpri, const rectangle &cliprect, u16 srcdat);
	inline void pgm_draw_pix_nopri(u16 xdrawpos, u16* dest, u8* destpri, const rectangle &cliprect, u16 srcdat);
	inline void pgm_draw_pix_pri(u16 xdrawpos, u16* dest, u8* destpri, const rectangle &cliprect, u16 srcdat);

	void draw_sprite_line(u16 wide, u16* dest, u8* destpri, const rectangle &cliprect,
	        u32 xzoom, bool xgrow, u8 flip, s16 xpos, u8 pri, u16 realxsize, u8 palt, bool draw);

	void draw_sprite_new_zoomed(u16 wide, u16 high, s16 xpos, s16 ypos, u8 palt, u8 flip,
			bitmap_ind16 &bitmap, bitmap_ind8 &priority_bitmap, const rectangle &cliprect, u32 xzoom, bool xgrow, u32 yzoom, bool ygrow, u8 pri);

	void draw_sprite_line_basic(u16 wide, u16* dest, u8* destpri, const rectangle &cliprect, u8 flip, s16 xpos, u8 pri, u16 realxsize, u8 palt, bool draw);

	void draw_sprite_new_basic(u16 wide, u16 high, s16 xpos, s16 ypos, u8 palt, u8 flip,
			bitmap_ind16 &bitmap, bitmap_ind8 &priority_bitmap, const rectangle &cliprect, u8 pri);

	void draw_sprites(bitmap_ind16& spritebitmap, const rectangle &cliprect, bitmap_ind8& priority_bitmap);
	void expand_colourdata();
	void basic_mem(address_map &map);
	void z80_io(address_map &map);
	void z80_mem(address_map &map);
};



/*----------- defined in drivers/pgm.c -----------*/

INPUT_PORTS_EXTERN( pgm );

extern gfx_decode_entry const gfx_pgm[];

#endif // MAME_INCLUDES_PGM_H
