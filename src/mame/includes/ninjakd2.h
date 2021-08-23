// license:BSD-3-Clause
// copyright-holders:Roberto Ventura, Leandro Dardini, Yochizo, Nicola Salmoria
/******************************************************************************

    UPL "sprite framebuffer" hardware

******************************************************************************/
#ifndef MAME_INCLUDES_NINJAKD2_H
#define MAME_INCLUDES_NINJAKD2_H

#pragma once

#include "sound/samples.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

class ninjakd2_state : public driver_device
{
public:
	ninjakd2_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_palette(*this, "palette"),
		m_bg_videoram(*this, "bg_videoram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_soundcpu(*this, "soundcpu"),
		m_pcm(*this, "pcm"),
		m_pcm_region(*this, "pcm"),
		m_fg_videoram(*this, "fg_videoram"),
		m_spriteram(*this, "spriteram"),
		m_decrypted_opcodes(*this, "decrypted_opcodes"),
		m_mainbank(*this, "mainbank")
	{ }

	void ninjakd2b(machine_config &config);
	void ninjakd2(machine_config &config);

	void init_ninjakd2();
	void init_bootleg();

protected:
	void ninjakd2_bgvideoram_w(offs_t offset, uint8_t data);
	void ninjakd2_fgvideoram_w(offs_t offset, uint8_t data);
	void ninjakd2_bg_ctrl_w(offs_t offset, uint8_t data);
	void ninjakd2_sprite_overdraw_w(uint8_t data);

	void ninjakd2_bankselect_w(uint8_t data);
	void ninjakd2_soundreset_w(uint8_t data);

	required_device<cpu_device> m_maincpu;
	required_device<palette_device> m_palette;
	optional_shared_ptr<uint8_t> m_bg_videoram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<cpu_device> m_soundcpu;

	uint8_t m_vram_bank_mask;
	int m_robokid_sprites;
	bool (*m_stencil_compare_function) (uint16_t pal);
	int m_sprites_updated;
	tilemap_t* m_fg_tilemap;
	tilemap_t* m_bg_tilemap;
	bitmap_ind16 m_sprites_bitmap;

	void video_init_common();

	void ninjakd2_pcm_play_w(uint8_t data);
	SAMPLES_START_CB_MEMBER(ninjakd2_init_samples);

	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(ninjakd2_get_bg_tile_info);
	uint32_t screen_update_ninjakd2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(screen_vblank_ninjakd2);

	void bg_ctrl(int offset, int data, tilemap_t* tilemap);
	void gfx_unscramble();
	void update_sprites();

	void ninjakid_nopcm_sound_cpu(address_map &map);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

	void ninjakd2_core(machine_config &config);

private:
	optional_device<samples_device> m_pcm;
	optional_memory_region m_pcm_region;
	required_shared_ptr<uint8_t> m_fg_videoram;
	required_shared_ptr<uint8_t> m_spriteram;
	optional_shared_ptr<uint8_t> m_decrypted_opcodes;

	required_memory_bank m_mainbank;

	std::unique_ptr<int16_t[]> m_sampledata;
	int m_next_sprite_overdraw_enabled;
	uint8_t m_rom_bank_mask;

	void draw_sprites( bitmap_ind16 &bitmap);
	void erase_sprites( bitmap_ind16 &bitmap);
	void lineswap_gfx_roms(const char *region, const int bit);
	void decrypted_opcodes_map(address_map &map);
	void ninjakd2_main_cpu(address_map &map);
	void ninjakd2_sound_cpu(address_map &map);
	void ninjakd2_sound_io(address_map &map);
};

class mnight_state : public ninjakd2_state
{
public:
	mnight_state(const machine_config &mconfig, device_type type, const char *tag) :
		ninjakd2_state(mconfig, type, tag)
	{ }

	void arkarea(machine_config &config);
	void mnight(machine_config &config);

	void init_mnight();

private:
	void mnight_main_cpu(address_map &map);

	TILE_GET_INFO_MEMBER(mnight_get_bg_tile_info);
	DECLARE_VIDEO_START(mnight);
	DECLARE_VIDEO_START(arkarea);

};

class robokid_state : public mnight_state
{
public:
	robokid_state(const machine_config &mconfig, device_type type, const char *tag) :
		mnight_state(mconfig, type, tag)
	{ }

	void robokid(machine_config &config);

	void init_robokid();
	void init_robokidj();

protected:
	template<int Layer> uint8_t robokid_bg_videoram_r(offs_t offset);
	template<int Layer> void robokid_bg_videoram_w(offs_t offset, uint8_t data);
	template<int Layer> void robokid_bg_ctrl_w(offs_t offset, uint8_t data);
	template<int Layer> void robokid_bg_bank_w(uint8_t data);
	tilemap_t* m_robokid_tilemap[3];

	void video_init_banked(uint32_t vram_alloc_size);
	TILEMAP_MAPPER_MEMBER(robokid_bg_scan);
	template<int Layer> TILE_GET_INFO_MEMBER(robokid_get_bg_tile_info);

	void robokid_main_cpu(address_map &map);

private:
	uint8_t motion_error_verbose_r();

	DECLARE_VIDEO_START(robokid);
	uint32_t screen_update_robokid(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void motion_error_kludge(uint16_t offset);
	uint8_t m_robokid_bg_bank[3];
	std::unique_ptr<uint8_t[]> m_robokid_bg_videoram[3];
};

class omegaf_state : public robokid_state
{
public:
	omegaf_state(const machine_config &mconfig, device_type type, const char *tag) :
		robokid_state(mconfig, type, tag),
		m_dsw_io(*this, "DIPSW%u", 1U),
		m_pad_io(*this, "PAD%u", 1U)
	{ }

	void omegaf(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	uint8_t unk_r();
	uint8_t io_protection_r(offs_t offset);
	void io_protection_w(offs_t offset, uint8_t data);

	void omegaf_main_cpu(address_map &map);

	DECLARE_VIDEO_START(omegaf);
	TILEMAP_MAPPER_MEMBER(omegaf_bg_scan);
	uint32_t screen_update_omegaf(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	required_ioport_array<2> m_dsw_io;
	required_ioport_array<2> m_pad_io;

	void io_protection_start();
	void io_protection_reset();

	uint8_t m_io_protection[3];
	uint8_t m_io_protection_input;
	int m_io_protection_tick;
};

#endif // MAME_INCLUDES_NINJAKD2_H
