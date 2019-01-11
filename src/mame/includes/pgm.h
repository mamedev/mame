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
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_soundlatch(*this, "soundlatch")
		, m_soundlatch3(*this, "soundlatch3")
		, m_ics(*this, "ics")
		, m_bdata(*this, "sprmask")
	{
		m_irq4_disabled = 0;
	}

	/* memory pointers */
	required_shared_ptr<uint16_t> m_videoregs;
	required_shared_ptr<uint16_t> m_videoram;
	required_shared_ptr<uint8_t> m_z80_mainram;
	required_shared_ptr<uint16_t> m_mainram;
	uint16_t *      m_bg_videoram;
	uint16_t *      m_tx_videoram;
	uint16_t *      m_rowscrollram;
	std::unique_ptr<uint8_t[]>      m_sprite_a_region;
	size_t        m_sprite_a_region_size;
	std::unique_ptr<uint16_t[]>     m_spritebufferram; // buffered spriteram

	/* video-related */
	tilemap_t     *m_bg_tilemap;
	tilemap_t     *m_tx_tilemap;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_soundcpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<generic_latch_8_device> m_soundlatch3;
	required_device<ics2115_device> m_ics;

	/* used by rendering */
	required_region_ptr<uint8_t> m_bdata;
	int m_aoffset;
	int m_boffset;

	/* hack */
	int m_irq4_disabled;

	/* calendar */
	uint8_t        m_cal_val;
	uint8_t        m_cal_mask;
	uint8_t        m_cal_com;
	uint8_t        m_cal_cnt;
	system_time  m_systime;

	DECLARE_READ16_MEMBER(pgm_videoram_r);
	DECLARE_WRITE16_MEMBER(pgm_videoram_w);
	DECLARE_WRITE16_MEMBER(pgm_coin_counter_w);
	DECLARE_READ16_MEMBER(z80_ram_r);
	DECLARE_WRITE16_MEMBER(z80_ram_w);
	DECLARE_WRITE16_MEMBER(z80_reset_w);
	DECLARE_WRITE16_MEMBER(z80_ctrl_w);
	DECLARE_WRITE16_MEMBER(m68k_l1_w);
	DECLARE_WRITE8_MEMBER(z80_l3_w);
	DECLARE_WRITE16_MEMBER(pgm_tx_videoram_w);
	DECLARE_WRITE16_MEMBER(pgm_bg_videoram_w);

	void init_pgm();

	TILE_GET_INFO_MEMBER(get_pgm_tx_tilemap_tile_info);
	TILE_GET_INFO_MEMBER(get_pgm_bg_tilemap_tile_info);
	DECLARE_VIDEO_START(pgm);
	DECLARE_MACHINE_START(pgm);
	DECLARE_MACHINE_RESET(pgm);
	uint32_t screen_update_pgm(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(screen_vblank_pgm);
	TIMER_DEVICE_CALLBACK_MEMBER(pgm_interrupt);

	inline void pgm_draw_pix( int xdrawpos, int pri, uint16_t* dest, uint8_t* destpri, uint16_t srcdat);
	inline void pgm_draw_pix_nopri( int xdrawpos, uint16_t* dest, uint8_t* destpri, uint16_t srcdat);
	inline void pgm_draw_pix_pri( int xdrawpos, uint16_t* dest, uint8_t* destpri, uint16_t srcdat);
	void draw_sprite_line( int wide, uint16_t* dest, uint8_t* destpri, int xzoom, int xgrow, int flip, int xpos, int pri, int realxsize, int palt, int draw );
	void draw_sprite_new_zoomed( int wide, int high, int xpos, int ypos, int palt, int flip, bitmap_ind16 &bitmap, bitmap_ind8 &priority_bitmap, uint32_t xzoom, int xgrow, uint32_t yzoom, int ygrow, int pri );
	void draw_sprite_line_basic( int wide, uint16_t* dest, uint8_t* destpri, int flip, int xpos, int pri, int realxsize, int palt, int draw );
	void draw_sprite_new_basic( int wide, int high, int xpos, int ypos, int palt, int flip, bitmap_ind16 &bitmap, bitmap_ind8 &priority_bitmap, int pri );
	void draw_sprites( bitmap_ind16& spritebitmap, uint16_t *sprite_source, bitmap_ind8& priority_bitmap );
	void expand_colourdata();
	void pgm_basic_init( bool set_bank = true);
	void pgm(machine_config &config);
	void pgmbase(machine_config &config);
	void pgm_base_mem(address_map &map);
	void pgm_basic_mem(address_map &map);
	void pgm_mem(address_map &map);
	void pgm_z80_io(address_map &map);
	void pgm_z80_mem(address_map &map);
};



/*----------- defined in drivers/pgm.c -----------*/

INPUT_PORTS_EXTERN( pgm );

extern gfx_decode_entry const gfx_pgm[];

#endif // MAME_INCLUDES_PGM_H
