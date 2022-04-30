// license:BSD-3-Clause
// copyright-holders:Carlos A. Lozano
#ifndef MAME_INCLUDES_CABAL_H
#define MAME_INCLUDES_CABAL_H

#pragma once

#include "audio/seibu.h"
#include "sound/msm5205.h"
#include "emupal.h"
#include "tilemap.h"

class cabal_base_state : public driver_device
{
public:
	cabal_base_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_msm(*this, "msm%u", 1U),
		m_spriteram(*this, "spriteram"),
		m_colorram(*this, "colorram"),
		m_videoram(*this, "videoram")
	{ }

protected:
	virtual void video_start() override;

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device_array<msm5205_device, 2> m_msm;

	void flipscreen_w(uint8_t data);
	void background_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void text_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

private:
	required_shared_ptr<uint16_t> m_spriteram;
	required_shared_ptr<uint16_t> m_colorram;
	required_shared_ptr<uint16_t> m_videoram;

	tilemap_t *m_background_layer = nullptr;
	tilemap_t *m_text_layer = nullptr;

	TILE_GET_INFO_MEMBER(get_back_tile_info);
	TILE_GET_INFO_MEMBER(get_text_tile_info);

	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
};

class cabal_state : public cabal_base_state
{
public:
	cabal_state(const machine_config &mconfig, device_type type, const char *tag) :
		cabal_base_state(mconfig, type, tag),
		m_seibu_sound(*this, "seibu_sound"),
		m_adpcm(*this, "adpcm%u", 1U)
	{ }

	void cabal(machine_config &config);
	void cabalbl2(machine_config &config);
	void cabalt(machine_config &config);

	void init_cabal();

private:
	required_device<seibu_sound_device> m_seibu_sound;
	required_device_array<seibu_adpcm_device, 2> m_adpcm;

	void sound_irq_trigger_word_w(offs_t, u16 data, u16 mem_mask);

	void main_map(address_map &map);
	void sound_decrypted_opcodes_map(address_map &map);
	void sound_map(address_map &map);
	void trackball_main_map(address_map &map);
	void cabalbl2_predecrypted_opcodes_map(address_map &map);
	void cabalbl2_sound_map(address_map &map);
};

class cabalbl_state : public cabal_base_state
{
public:
	cabalbl_state(const machine_config &mconfig, device_type type, const char *tag) :
		cabal_base_state(mconfig, type, tag)
	{ }

	void cabalbl(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	int m_sound_command[2]{};

	void sndcmd_w(offs_t offset, uint16_t data);
	void sound_irq_trigger_word_w(uint16_t data);
	template<uint8_t Which> uint8_t snd_r();
	void coin_w(uint8_t data);
	template<uint8_t Which> void adpcm_w(uint8_t data);

	void main_map(address_map &map);
	void sound_map(address_map &map);
	void talk1_map(address_map &map);
	void talk1_portmap(address_map &map);
	void talk2_map(address_map &map);
	void talk2_portmap(address_map &map);
};

#endif // MAME_INCLUDES_CABAL_H
