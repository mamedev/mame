// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
#ifndef MAME_IREM_M90_H
#define MAME_IREM_M90_H

#pragma once

#include "m72_a.h"
#include "cpu/nec/v25.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

class m90_state : public driver_device
{
public:
	m90_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_video_control_data(*this, "video_control")
		, m_video_data(*this, "video_data")
		, m_spriteram(*this, "spriteram")
		, m_mainbank(*this, "mainbank")
		, m_maincpu(*this, "maincpu")
		, m_soundcpu(*this, "soundcpu")
		, m_audio(*this, "m72")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_screen(*this, "screen")
	{ }

	void m90(machine_config &config);
	void bbmanw(machine_config &config);
	void hasamu(machine_config &config);
	void bombrman(machine_config &config);
	void riskchal(machine_config &config);
	void bomblord(machine_config &config);
	void bbmanwj(machine_config &config);
	void dynablsb(machine_config &config);
	void matchit2(machine_config &config);
	void quizf1(machine_config &config);
	void dicegame(machine_config &config);

	void init_bomblord();
	void init_quizf1();

private:
	required_shared_ptr<uint16_t> m_video_control_data;
	required_shared_ptr<uint16_t> m_video_data;
	optional_shared_ptr<uint16_t> m_spriteram;

	optional_memory_bank m_mainbank;

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_soundcpu;
	optional_device<m72_audio_device> m_audio;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;

	tilemap_t *m_pf_layer[2][2]{};
	uint8_t m_last_pf[2]{};
	void coincounter_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void quizf1_bankswitch_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void m90_video_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void bootleg_video_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	TILE_GET_INFO_MEMBER(get_tile_info);
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
	void common_tilemap_init();
	DECLARE_VIDEO_START(bomblord);
	DECLARE_VIDEO_START(dynablsb);
	uint32_t screen_update_m90(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_bomblord(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_dynablsb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(fake_nmi);
	INTERRUPT_GEN_MEMBER(bomblord_fake_nmi);
	void dynablsb_vblank_int_w(int state);
	void bomblord_vblank_int_w(int state);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap,const rectangle &cliprect);
	void bomblord_draw_sprites(screen_device &screen, bitmap_ind16 &bitmap,const rectangle &cliprect);
	void dynablsb_draw_sprites(screen_device &screen, bitmap_ind16 &bitmap,const rectangle &cliprect);
	void bomblord_main_cpu_map(address_map &map) ATTR_COLD;
	void dynablsb_main_cpu_io_map(address_map &map) ATTR_COLD;
	void dynablsb_main_cpu_map(address_map &map) ATTR_COLD;
	void dynablsb_sound_cpu_io_map(address_map &map) ATTR_COLD;
	void m90_main_cpu_io_map(address_map &map) ATTR_COLD;
	void m90_main_cpu_map(address_map &map) ATTR_COLD;
	void m90_sound_cpu_io_map(address_map &map) ATTR_COLD;
	void m90_sound_cpu_map(address_map &map) ATTR_COLD;
	void m99_sound_cpu_io_map(address_map &map) ATTR_COLD;
	void quizf1_main_cpu_io_map(address_map &map) ATTR_COLD;
	void quizf1_main_cpu_map(address_map &map) ATTR_COLD;
};

#endif // MAME_IREM_M90_H
