// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_INCLUDES_PGM2_H
#define MAME_INCLUDES_PGM2_H

#pragma once

#include "emu.h"
#include "cpu/arm7/arm7.h"
#include "cpu/arm7/arm7core.h"
#include "sound/ymz770.h"
#include "machine/igs036crypt.h"
#include "screen.h"
#include "speaker.h"
#include "machine/nvram.h"
#include "machine/timer.h"
#include "machine/atmel_arm_aic.h"

class pgm2_state : public driver_device
{
public:
	pgm2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_lineram(*this, "lineram"),
		m_sp_zoom(*this, "sp_zoom"),
		m_mainram(*this, "mainram"),
		m_fg_videoram(*this, "fg_videoram"),
		m_bg_videoram(*this, "bg_videoram"),
		m_sp_videoram(*this, "sp_videoram"),
		m_bgscroll(*this, "bgscroll"),
		m_gfxdecode2(*this, "gfxdecode2"),
		m_gfxdecode3(*this, "gfxdecode3"),
		m_arm_aic(*this, "arm_aic"),
		m_sprites_mask(*this, "sprites_mask"),
		m_sprites_colour(*this, "sprites_colour"),
		m_sp_palette(*this, "sp_palette"),
		m_bg_palette(*this, "bg_palette"),
		m_tx_palette(*this, "tx_palette")
	{ }

	DECLARE_READ32_MEMBER(unk_startup_r);
	DECLARE_WRITE32_MEMBER(fg_videoram_w);
	DECLARE_WRITE32_MEMBER(bg_videoram_w);

	DECLARE_READ32_MEMBER(orleg2_speedup_r);
	DECLARE_READ32_MEMBER(kov2nl_speedup_r);

	DECLARE_DRIVER_INIT(kov2nl);
	DECLARE_DRIVER_INIT(orleg2);
	DECLARE_DRIVER_INIT(ddpdojh);
	DECLARE_DRIVER_INIT(kov3);
	DECLARE_DRIVER_INIT(kov3_104);
	DECLARE_DRIVER_INIT(kov3_102);
	DECLARE_DRIVER_INIT(kov3_100);

	uint32_t screen_update_pgm2(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(screen_vblank_pgm2);
	DECLARE_WRITE_LINE_MEMBER(irq);

	INTERRUPT_GEN_MEMBER(igs_interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(igs_interrupt2);

private:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);

	void pgm_create_dummy_internal_arm_region();
	void decrypt_kov3_module(uint32_t addrxor, uint16_t dataxor);

	tilemap_t    *m_fg_tilemap;
	tilemap_t    *m_bg_tilemap;

	std::unique_ptr<uint32_t[]>     m_spritebufferram; // buffered spriteram

	bitmap_ind16 m_sprite_bitmap;

	void skip_sprite_chunk(int &palette_offset, uint32_t maskdata, int reverse);
	void draw_sprite_pixel(const rectangle &cliprect, int palette_offset, int realx, int realy, int pal);
	void draw_sprite_chunk(const rectangle &cliprect, int &palette_offset, int x, int realy, int sizex, int xdraw, int pal, uint32_t maskdata, uint32_t zoomx_bits, int growx, int &realxdraw, int realdraw_inc, int palette_inc);
	void draw_sprite_line(const rectangle &cliprect, int &mask_offset, int &palette_offset, int x, int realy, int flipx, int reverse, int sizex, int pal, int zoomybit, int zoomx_bits, int growx);
	void draw_sprites(screen_device &screen, const rectangle &cliprect, uint32_t* spriteram);
	void copy_sprites_from_bitmap(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int pri);

	uint32_t m_sprites_mask_mask;
	uint32_t m_sprites_colour_mask;

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_shared_ptr<uint32_t> m_lineram;
	required_shared_ptr<uint32_t> m_sp_zoom;
	required_shared_ptr<uint32_t> m_mainram;
	required_shared_ptr<uint32_t> m_fg_videoram;
	required_shared_ptr<uint32_t> m_bg_videoram;
	required_shared_ptr<uint32_t> m_sp_videoram;
	required_shared_ptr<uint32_t> m_bgscroll;
	required_device<gfxdecode_device> m_gfxdecode2;
	required_device<gfxdecode_device> m_gfxdecode3;
	required_device<arm_aic_device> m_arm_aic;
	required_region_ptr<uint8_t> m_sprites_mask;
	required_region_ptr<uint8_t> m_sprites_colour;
	required_device<palette_device> m_sp_palette;
	required_device<palette_device> m_bg_palette;
	required_device<palette_device> m_tx_palette;
};

#endif
