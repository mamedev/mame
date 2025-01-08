// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino
#ifndef MAME_TAITO_TSAMURAI_H
#define MAME_TAITO_TSAMURAI_H

#pragma once

#include "machine/gen_latch.h"
#include "emupal.h"
#include "tilemap.h"

class tsamurai_state : public driver_device
{
public:
	tsamurai_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_audio2(*this, "audio2"),
		m_audio3(*this, "audio3"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_bg_videoram(*this, "bg_videoram"),
		m_spriteram(*this, "spriteram")
	{ }

	void tsamurai(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	// common
	void nmi_enable_w(int state);
	void coin1_counter_w(int state);
	void coin2_counter_w(int state);
	void textbank1_w(int state);
	void fg_videoram_w(offs_t offset, uint8_t data);

	// tsamurai and m660 specific
	void bg_videoram_w(offs_t offset, uint8_t data);
	void fg_colorram_w(offs_t offset, uint8_t data);
	void scrolly_w(uint8_t data);
	void scrollx_w(uint8_t data);
	void bgcolor_w(uint8_t data);
	uint8_t unknown_d806_r();
	uint8_t unknown_d900_r();
	uint8_t unknown_d938_r();
	void sound_command1_w(uint8_t data);
	void sound_command2_w(uint8_t data);
	uint8_t sound_command1_r();
	uint8_t sound_command2_r();

	// tsamurai/the26thz specific
	uint8_t tsamurai_unknown_d803_r();

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void vblank_irq(int state);

	void main_map(address_map &map) ATTR_COLD;
	void sound1_map(address_map &map) ATTR_COLD;
	void sound2_map(address_map &map) ATTR_COLD;
	void z80_io_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<cpu_device> m_audio2;
	optional_device<cpu_device> m_audio3;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_videoram;
	optional_shared_ptr<uint8_t> m_colorram;
	optional_shared_ptr<uint8_t> m_bg_videoram;
	required_shared_ptr<uint8_t> m_spriteram;

	tilemap_t *m_background = nullptr;
	tilemap_t *m_foreground = nullptr;

	//common
	int m_textbank1 = 0;
	int m_nmi_enabled = 0;

	// tsamurai and m660 specific
	int m_bgcolor = 0;
	int m_sound_command1 = 0;
	int m_sound_command2 = 0;

	virtual TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual TILE_GET_INFO_MEMBER(get_fg_tile_info);

	int m_numsprites = 0;
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect );
};

class vsgongf_state : public tsamurai_state
{
public:
	vsgongf_state(const machine_config &mconfig, device_type type, const char *tag) :
		tsamurai_state(mconfig, type, tag),
		m_soundlatch(*this, "soundlatch")
	{ }

	void vsgongf(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	optional_device<generic_latch_8_device> m_soundlatch; // vsgongf only

	void sound_vsgongf_map(address_map &map) ATTR_COLD;
	void vsgongf_audio_io_map(address_map &map) ATTR_COLD;
	void vsgongf_map(address_map &map) ATTR_COLD;

	// vsgongf specific
	void vsgongf_color_w(uint8_t data);
	void vsgongf_sound_nmi_enable_w(uint8_t data);
	uint8_t vsgongf_a006_r();
	uint8_t vsgongf_a100_r();
	void vsgongf_sound_command_w(uint8_t data);

	//vsgongf specific
	int m_vsgongf_sound_nmi_enabled = 0;
	int m_vsgongf_color = 0;
	int m_key_count = 0; //debug only

	INTERRUPT_GEN_MEMBER(vsgongf_sound_interrupt);

	TILE_GET_INFO_MEMBER(get_vsgongf_tile_info);

	uint32_t screen_update_vsgongf(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};

class m660_state : public tsamurai_state
{
public:
	m660_state(const machine_config &mconfig, device_type type, const char *tag) :
		tsamurai_state(mconfig, type, tag)
	{ }

	void m660(machine_config &config);

	void init_the26thz();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	virtual TILE_GET_INFO_MEMBER(get_bg_tile_info) override;
	virtual TILE_GET_INFO_MEMBER(get_fg_tile_info) override;

private:

	// m660 specific
	void textbank2_w(int state);
	uint8_t m660_unknown_d803_r();
	void m660_sound_command3_w(uint8_t data);
	uint8_t m660_sound_command3_r();

	void m660_map(address_map &map) ATTR_COLD;
	void sound1_m660_map(address_map &map) ATTR_COLD;
	void sound2_m660_map(address_map &map) ATTR_COLD;
	void sound3_m660_io_map(address_map &map) ATTR_COLD;
	void sound3_m660_map(address_map &map) ATTR_COLD;
	void z80_m660_io_map(address_map &map) ATTR_COLD;

	//m660 specific
	int m_textbank2 = 0;
	int m_sound_command3 = 0;
};

#endif // MAME_TAITO_TSAMURAI_H
