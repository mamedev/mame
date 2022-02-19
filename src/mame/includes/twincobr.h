// license:BSD-3-Clause
// copyright-holders:Quench
/***************************************************************************
        Twincobr/Flying Shark/Wardner  game hardware from 1986-1987
        -----------------------------------------------------------
****************************************************************************/
#ifndef MAME_INCLUDES_TWINCOBR_H
#define MAME_INCLUDES_TWINCOBR_H

#pragma once

#include "cpu/tms32010/tms32010.h"
#include "machine/74259.h"
#include "video/mc6845.h"
#include "video/bufsprite.h"
#include "video/toaplan_scu.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

class twincobr_state : public driver_device
{
public:
	twincobr_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_sharedram(*this, "sharedram"),
		m_spriteram8(*this, "spriteram8"),
		m_spriteram16(*this, "spriteram16"),
		m_maincpu(*this, "maincpu"),
		m_dsp(*this, "dsp"),
		m_spritegen(*this, "scu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_mainlatch(*this, "mainlatch"),
		m_coinlatch(*this, "coinlatch")
	{ }

	void twincobr(machine_config &config);
	void twincobrw(machine_config &config);
	void fsharkbt(machine_config &config);
	void fshark(machine_config &config);

	void init_twincobr();

protected:
	virtual void machine_reset() override;
	virtual void video_start() override;

	optional_shared_ptr<u8> m_sharedram;
	optional_device<buffered_spriteram8_device> m_spriteram8;
	optional_device<buffered_spriteram16_device> m_spriteram16;

	u32 m_fg_rom_bank;
	u32 m_bg_ram_bank;
	int m_intenable;
	int m_dsp_bio;
	int m_fsharkbt_8741;
	int m_dsp_execute;
	u32 m_dsp_addr_w;
	u32 m_main_ram_seg;
	std::unique_ptr<u16[]> m_bgvideoram16;
	std::unique_ptr<u16[]> m_fgvideoram16;
	std::unique_ptr<u16[]> m_txvideoram16;
	size_t m_bgvideoram_size;
	size_t m_fgvideoram_size;
	size_t m_txvideoram_size;
	s32 m_txscrollx;
	s32 m_txscrolly;
	s32 m_fgscrollx;
	s32 m_fgscrolly;
	s32 m_bgscrollx;
	s32 m_bgscrolly;
	s32 m_txoffs;
	s32 m_fgoffs;
	s32 m_bgoffs;
	s32 m_display_on;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_tx_tilemap;

	void twincobr_dsp_addrsel_w(u16 data);
	u16 twincobr_dsp_r();
	void twincobr_dsp_w(u16 data);
	void wardner_dsp_addrsel_w(u16 data);
	u16 wardner_dsp_r();
	void wardner_dsp_w(u16 data);
	void twincobr_dsp_bio_w(u16 data);
	u16 fsharkbt_dsp_r();
	void fsharkbt_dsp_w(u16 data);
	DECLARE_READ_LINE_MEMBER(twincobr_bio_r);
	DECLARE_WRITE_LINE_MEMBER(int_enable_w);
	DECLARE_WRITE_LINE_MEMBER(dsp_int_w);
	DECLARE_WRITE_LINE_MEMBER(coin_counter_1_w);
	DECLARE_WRITE_LINE_MEMBER(coin_counter_2_w);
	DECLARE_WRITE_LINE_MEMBER(coin_lockout_1_w);
	DECLARE_WRITE_LINE_MEMBER(coin_lockout_2_w);
	u8 twincobr_sharedram_r(offs_t offset);
	void twincobr_sharedram_w(offs_t offset, u8 data);
	void twincobr_txoffs_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 twincobr_txram_r();
	void twincobr_txram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void twincobr_bgoffs_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 twincobr_bgram_r();
	void twincobr_bgram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void twincobr_fgoffs_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 twincobr_fgram_r();
	void twincobr_fgram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void twincobr_txscroll_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void twincobr_bgscroll_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void twincobr_fgscroll_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void twincobr_exscroll_w(offs_t offset, u16 data);
	void wardner_txlayer_w(offs_t offset, u8 data);
	void wardner_bglayer_w(offs_t offset, u8 data);
	void wardner_fglayer_w(offs_t offset, u8 data);
	void wardner_txscroll_w(offs_t offset, u8 data);
	void wardner_bgscroll_w(offs_t offset, u8 data);
	void wardner_fgscroll_w(offs_t offset, u8 data);
	void wardner_exscroll_w(offs_t offset, u8 data);
	u8 wardner_videoram_r(offs_t offset);
	void wardner_videoram_w(offs_t offset, u8 data);
	u8 wardner_sprite_r(offs_t offset);
	void wardner_sprite_w(offs_t offset, u8 data);
	void pri_cb(u8 priority, u32 &pri_mask);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_tx_tile_info);
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(twincobr_vblank_irq);
	void twincobr_create_tilemaps();
	DECLARE_WRITE_LINE_MEMBER(display_on_w);
	DECLARE_WRITE_LINE_MEMBER(flipscreen_w);
	DECLARE_WRITE_LINE_MEMBER(bg_ram_bank_w);
	DECLARE_WRITE_LINE_MEMBER(fg_rom_bank_w);
	void log_vram();
	void driver_savestate();
	required_device<cpu_device> m_maincpu;
	required_device<tms32010_device> m_dsp;
	required_device<toaplan_scu_device> m_spritegen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<ls259_device> m_mainlatch;
	required_device<ls259_device> m_coinlatch;

	void dsp_io_map(address_map &map);
	void dsp_program_map(address_map &map);
	void fsharkbt_i8741_io_map(address_map &map);
	void main_program_map(address_map &map);
	void sound_io_map(address_map &map);
	void sound_program_map(address_map &map);
};

#endif // MAME_INCLUDES_TWINCOBR_H
