// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli, Quench
#ifndef MAME_PLAYMARK_SSLAM_H
#define MAME_PLAYMARK_SSLAM_H

#pragma once

#include "cpu/mcs51/mcs51.h"
#include "machine/gen_latch.h"
#include "sound/okim6295.h"
#include "emupal.h"
#include "tilemap.h"

class sslam_state : public driver_device
{
public:
	sslam_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_oki(*this, "oki"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_bg_tileram(*this, "bg_tileram"),
		m_md_tileram(*this, "md_tileram"),
		m_tx_tileram(*this, "tx_tileram"),
		m_regs(*this, "regs"),
		m_spriteram(*this, "spriteram")
	{ }

	void sslam(machine_config &config);

	void init_sslam();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<mcs51_cpu_device> m_audiocpu;
	required_device<okim6295_device> m_oki;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	optional_device<generic_latch_8_device> m_soundlatch;

	required_shared_ptr<uint16_t> m_bg_tileram;
	optional_shared_ptr<uint16_t> m_md_tileram;
	optional_shared_ptr<uint16_t> m_tx_tileram;
	required_shared_ptr<uint16_t> m_regs;
	required_shared_ptr<uint16_t> m_spriteram;

	emu_timer *m_music_timer = nullptr;

	int m_sound = 0;
	int m_melody = 0;
	int m_bar = 0;
	int m_track = 0;
	int m_snd_bank = 0;

	uint8_t m_oki_control = 0;
	uint8_t m_oki_command = 0;
	uint8_t m_oki_bank = 0;

	uint32_t m_unk_458 = 0;

	tilemap_t *m_bg_tilemap = nullptr;
	tilemap_t *m_tx_tilemap = nullptr;
	tilemap_t *m_md_tilemap = nullptr;

	int m_sprites_x_offset = 0;
	uint8_t playmark_snd_command_r();
	void playmark_oki_w(uint8_t data);
	void playmark_snd_control_w(uint8_t data);
	void sslam_tx_tileram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void sslam_md_tileram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void sslam_bg_tileram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void sslam_snd_w(uint8_t data);

	TILE_GET_INFO_MEMBER(get_sslam_tx_tile_info);
	TILE_GET_INFO_MEMBER(get_sslam_md_tile_info);
	TILE_GET_INFO_MEMBER(get_sslam_bg_tile_info);
	DECLARE_VIDEO_START(sslam);
	DECLARE_VIDEO_START(powerbls);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(music_playback);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void sslam_play(int track, int data);

	void sslam_program_map(address_map &map) ATTR_COLD;
};

class powerbls_state : public sslam_state
{
public:
	powerbls_state(const machine_config &mconfig, device_type type, const char *tag) :
		sslam_state(mconfig, type, tag)
	{ }

	void powerbls(machine_config &config);

	void init_powerbls();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	void powerbls_map(address_map &map) ATTR_COLD;

	TILE_GET_INFO_MEMBER(get_powerbls_bg_tile_info);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void powerbls_sound_w(uint16_t data);
	void powerbls_bg_tileram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
};

#endif // MAME_PLAYMARK_SSLAM_H
