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
		, m_mainbank(*this, "mainbank")
		, m_bdata(*this, "sprmask")
	{
		m_irq4_disabled = 0;
	}

	void init_pgm();

	void pgm(machine_config &config);
protected:
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

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_soundcpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<generic_latch_8_device> m_soundlatch3;
	required_device<ics2115_device> m_ics;

	optional_memory_bank m_mainbank;

	/* hack */
	int m_irq4_disabled;

	virtual void video_start() override;
	//virtual void machine_start() override;
	virtual void machine_reset() override;

	void base_mem(address_map &map);
	void pgm_mem(address_map &map);
private:
	/* video-related */
	tilemap_t     *m_bg_tilemap;
	tilemap_t     *m_tx_tilemap;

	/* used by rendering */
	required_region_ptr<uint8_t> m_bdata;
	int m_aoffset;
	int m_boffset;

	/* calendar */
	uint8_t        m_cal_val;
	uint8_t        m_cal_mask;
	uint8_t        m_cal_com;
	uint8_t        m_cal_cnt;
	system_time  m_systime;

	DECLARE_READ16_MEMBER(videoram_r);
	DECLARE_WRITE16_MEMBER(videoram_w);
	DECLARE_WRITE16_MEMBER(coin_counter_w);
	DECLARE_READ8_MEMBER(z80_ram_r);
	DECLARE_WRITE8_MEMBER(z80_ram_w);
	DECLARE_WRITE16_MEMBER(z80_reset_w);
	DECLARE_WRITE16_MEMBER(z80_ctrl_w);
	DECLARE_WRITE8_MEMBER(m68k_l1_w);
	DECLARE_WRITE8_MEMBER(z80_l3_w);
	void pgm_tx_videoram_w(offs_t offset, u16 data, u16 mem_mask);
	void pgm_bg_videoram_w(offs_t offset, u16 data, u16 mem_mask);

	TILE_GET_INFO_MEMBER(get_tx_tilemap_tile_info);
	TILE_GET_INFO_MEMBER(get_bg_tilemap_tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(screen_vblank);
	TIMER_DEVICE_CALLBACK_MEMBER(interrupt);

	inline void pgm_draw_pix(uint16_t const xdrawpos, uint8_t const pri, uint16_t* dest, uint8_t* destpri, const rectangle &cliprect, uint16_t const srcdat);
	inline void pgm_draw_pix_nopri(uint16_t const xdrawpos, uint16_t* dest, uint8_t* destpri, const rectangle &cliprect, uint16_t const srcdat);
	inline void pgm_draw_pix_pri(uint16_t const xdrawpos, uint16_t* dest, uint8_t* destpri, const rectangle &cliprect, uint16_t const srcdat);

	void draw_sprite_line(
	uint16_t const wide,
	uint16_t* dest, uint8_t* destpri, const rectangle &cliprect,
	uint32_t const xzoom, bool const xgrow,
	uint8_t const flip, int16_t const xpos,
	uint8_t const pri,
	uint16_t const realxsize,
	uint8_t const palt,
	bool const draw);

	void draw_sprite_new_zoomed(
	uint16_t const wide, uint16_t const high,
	int16_t const xpos, int16_t const ypos,
	uint8_t const palt, uint8_t const flip,
	bitmap_ind16 &bitmap, bitmap_ind8 &priority_bitmap, const rectangle &cliprect,
	uint32_t const xzoom, bool const xgrow, uint32_t const yzoom, bool const ygrow,
	uint8_t const pri);

	void draw_sprite_line_basic(
	uint16_t const wide,
	uint16_t* dest, uint8_t* destpri, const rectangle &cliprect,
	uint8_t const flip,
	int16_t const xpos,
	uint8_t const pri,
	uint16_t const realxsize,
	uint8_t const palt,
	bool const draw);

	void draw_sprite_new_basic(
	uint16_t const wide, uint16_t const high,
	int16_t const xpos, int16_t const ypos,
	uint8_t const palt, uint8_t const flip,
	bitmap_ind16 &bitmap, bitmap_ind8 &priority_bitmap, const rectangle &cliprect,
	uint8_t const pri);

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
