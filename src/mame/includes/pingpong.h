// license:BSD-3-Clause
// copyright-holders:Jarek Parchanski
#ifndef MAME_INCLUDES_PINGPONG_H
#define MAME_INCLUDES_PINGPONG_H

#pragma once

#include "machine/timer.h"
#include "emupal.h"
#include "tilemap.h"

class pingpong_state : public driver_device
{
public:
	pingpong_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_colorram(*this, "colorram"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_banks(*this, "bank%d", 1U)
	{ }

	int m_intenable = 0;
	int m_question_addr_high = 0;
	required_device<cpu_device> m_maincpu;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	optional_memory_bank_array<8> m_banks;
	tilemap_t *m_bg_tilemap = nullptr;

	void cashquiz_question_bank_high_w(uint8_t data);
	void cashquiz_question_bank_low_w(uint8_t data);
	void coin_w(uint8_t data);
	void pingpong_videoram_w(offs_t offset, uint8_t data);
	void pingpong_colorram_w(offs_t offset, uint8_t data);
	void init_cashquiz();
	void init_merlinmm();
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void video_start() override;
	void pingpong_palette(palette_device &palette) const;
	uint32_t screen_update_pingpong(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(pingpong_interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(merlinmm_interrupt);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect );
	void merlinmm(machine_config &config);
	void pingpong(machine_config &config);
	void cashquiz(machine_config &config);
	void merlinmm_map(address_map &map);
	void pingpong_map(address_map &map);
	void cashquiz_map(address_map &map);
};

#endif // MAME_INCLUDES_PINGPONG_H
