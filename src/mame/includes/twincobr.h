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
	virtual void video_start() override;

	optional_shared_ptr<uint8_t> m_sharedram;
	optional_device<buffered_spriteram8_device> m_spriteram8;
	optional_device<buffered_spriteram16_device> m_spriteram16;

	int32_t m_fg_rom_bank;
	int32_t m_bg_ram_bank;
	int m_intenable;
	int m_dsp_on;
	int m_dsp_bio;
	int m_fsharkbt_8741;
	int m_dsp_execute;
	uint32_t m_dsp_addr_w;
	uint32_t m_main_ram_seg;
	std::unique_ptr<uint16_t[]> m_bgvideoram16;
	std::unique_ptr<uint16_t[]> m_fgvideoram16;
	std::unique_ptr<uint16_t[]> m_txvideoram16;
	size_t m_bgvideoram_size;
	size_t m_fgvideoram_size;
	size_t m_txvideoram_size;
	int32_t m_txscrollx;
	int32_t m_txscrolly;
	int32_t m_fgscrollx;
	int32_t m_fgscrolly;
	int32_t m_bgscrollx;
	int32_t m_bgscrolly;
	int32_t m_txoffs;
	int32_t m_fgoffs;
	int32_t m_bgoffs;
	int32_t m_display_on;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_tx_tilemap;

	DECLARE_WRITE16_MEMBER(twincobr_dsp_addrsel_w);
	DECLARE_READ16_MEMBER(twincobr_dsp_r);
	DECLARE_WRITE16_MEMBER(twincobr_dsp_w);
	DECLARE_WRITE16_MEMBER(wardner_dsp_addrsel_w);
	DECLARE_READ16_MEMBER(wardner_dsp_r);
	DECLARE_WRITE16_MEMBER(wardner_dsp_w);
	DECLARE_WRITE16_MEMBER(twincobr_dsp_bio_w);
	DECLARE_READ16_MEMBER(fsharkbt_dsp_r);
	DECLARE_WRITE16_MEMBER(fsharkbt_dsp_w);
	DECLARE_READ_LINE_MEMBER(twincobr_bio_r);
	DECLARE_WRITE_LINE_MEMBER(int_enable_w);
	DECLARE_WRITE_LINE_MEMBER(dsp_int_w);
	DECLARE_WRITE_LINE_MEMBER(coin_counter_1_w);
	DECLARE_WRITE_LINE_MEMBER(coin_counter_2_w);
	DECLARE_WRITE_LINE_MEMBER(coin_lockout_1_w);
	DECLARE_WRITE_LINE_MEMBER(coin_lockout_2_w);
	DECLARE_READ16_MEMBER(twincobr_sharedram_r);
	DECLARE_WRITE16_MEMBER(twincobr_sharedram_w);
	DECLARE_WRITE16_MEMBER(twincobr_txoffs_w);
	DECLARE_READ16_MEMBER(twincobr_txram_r);
	DECLARE_WRITE16_MEMBER(twincobr_txram_w);
	DECLARE_WRITE16_MEMBER(twincobr_bgoffs_w);
	DECLARE_READ16_MEMBER(twincobr_bgram_r);
	DECLARE_WRITE16_MEMBER(twincobr_bgram_w);
	DECLARE_WRITE16_MEMBER(twincobr_fgoffs_w);
	DECLARE_READ16_MEMBER(twincobr_fgram_r);
	DECLARE_WRITE16_MEMBER(twincobr_fgram_w);
	DECLARE_WRITE16_MEMBER(twincobr_txscroll_w);
	DECLARE_WRITE16_MEMBER(twincobr_bgscroll_w);
	DECLARE_WRITE16_MEMBER(twincobr_fgscroll_w);
	DECLARE_WRITE16_MEMBER(twincobr_exscroll_w);
	DECLARE_WRITE8_MEMBER(wardner_txlayer_w);
	DECLARE_WRITE8_MEMBER(wardner_bglayer_w);
	DECLARE_WRITE8_MEMBER(wardner_fglayer_w);
	DECLARE_WRITE8_MEMBER(wardner_txscroll_w);
	DECLARE_WRITE8_MEMBER(wardner_bgscroll_w);
	DECLARE_WRITE8_MEMBER(wardner_fgscroll_w);
	DECLARE_WRITE8_MEMBER(wardner_exscroll_w);
	DECLARE_READ8_MEMBER(wardner_videoram_r);
	DECLARE_WRITE8_MEMBER(wardner_videoram_w);
	DECLARE_READ8_MEMBER(wardner_sprite_r);
	DECLARE_WRITE8_MEMBER(wardner_sprite_w);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_tx_tile_info);
	DECLARE_MACHINE_RESET(twincobr);
	uint32_t screen_update_toaplan0(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(twincobr_vblank_irq);
	DECLARE_WRITE_LINE_MEMBER(wardner_vblank_irq);
	void twincobr_restore_dsp();
	void twincobr_create_tilemaps();
	DECLARE_WRITE_LINE_MEMBER(display_on_w);
	DECLARE_WRITE_LINE_MEMBER(flipscreen_w);
	DECLARE_WRITE_LINE_MEMBER(bg_ram_bank_w);
	DECLARE_WRITE_LINE_MEMBER(fg_rom_bank_w);
	void twincobr_log_vram();
	void twincobr_driver_savestate();
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
