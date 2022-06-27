// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#ifndef MAME_TAITO_FLSTORY_H
#define MAME_TAITO_FLSTORY_H

#pragma once

#include "taito68705.h"

#include "machine/gen_latch.h"
#include "machine/input_merger.h"
#include "sound/msm5232.h"
#include "sound/ta7630.h"
#include "sound/ay8910.h"

#include "emupal.h"
#include "tilemap.h"

class flstory_state : public driver_device
{
public:
	flstory_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_scrlram(*this, "scrlram"),
		m_workram(*this, "workram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_bmcu(*this, "bmcu"),
		m_msm(*this, "msm"),
		m_ay(*this, "aysnd"),
		m_ta7630(*this, "ta7630"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_soundlatch2(*this, "soundlatch2"),
		m_soundnmi(*this, "soundnmi"),
		m_extraio1(*this, "EXTRA_P1")
	{ }

	void common(machine_config &config);
	void flstory(machine_config &config);
	void rumba(machine_config &config);
	void onna34ro(machine_config &config);
	void victnine(machine_config &config);
	void onna34ro_mcu(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_scrlram;
	optional_shared_ptr<uint8_t> m_workram;

	/* video-related */
	tilemap_t  *m_bg_tilemap = nullptr;
	std::vector<uint8_t> m_paletteram;
	std::vector<uint8_t> m_paletteram_ext;
	uint8_t    m_gfxctrl = 0;
	uint8_t    m_char_bank = 0;
	uint8_t    m_palette_bank = 0;

	/* sound-related */
	uint8_t    m_snd_ctrl0 = 0;
	uint8_t    m_snd_ctrl1 = 0;
	uint8_t    m_snd_ctrl2 = 0;
	uint8_t    m_snd_ctrl3 = 0;

	/* protection sims */
	uint8_t m_from_mcu = 0;
	int      m_mcu_select = 0;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<taito68705_mcu_device> m_bmcu;
	required_device<msm5232_device> m_msm;
	required_device<ay8910_device> m_ay;
	required_device<ta7630_device> m_ta7630;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<generic_latch_8_device> m_soundlatch2;
	required_device<input_merger_device> m_soundnmi;
	optional_ioport m_extraio1;

	uint8_t snd_flag_r();
	void snd_reset_w(uint8_t data);
	uint8_t flstory_mcu_status_r();
	uint8_t victnine_mcu_status_r();
	void flstory_videoram_w(offs_t offset, uint8_t data);
	void flstory_palette_w(offs_t offset, uint8_t data);
	uint8_t flstory_palette_r(offs_t offset);
	void flstory_gfxctrl_w(uint8_t data);
	uint8_t victnine_gfxctrl_r();
	void victnine_gfxctrl_w(uint8_t data);
	void flstory_scrlram_w(offs_t offset, uint8_t data);
	void sound_control_0_w(uint8_t data);
	void sound_control_1_w(uint8_t data);
	void sound_control_2_w(uint8_t data);
	void sound_control_3_w(uint8_t data);
	TILE_GET_INFO_MEMBER(get_tile_info);
	TILE_GET_INFO_MEMBER(victnine_get_tile_info);
	TILE_GET_INFO_MEMBER(get_rumba_tile_info);
	DECLARE_MACHINE_RESET(flstory);
	DECLARE_VIDEO_START(flstory);
	DECLARE_VIDEO_START(victnine);
	DECLARE_MACHINE_RESET(rumba);
	DECLARE_VIDEO_START(rumba);
	DECLARE_MACHINE_RESET(ta7630);
	uint32_t screen_update_flstory(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_victnine(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_rumba(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void flstory_draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect, int pri );
	void victnine_draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void base_map(address_map &map);
	void flstory_map(address_map &map);
	void onna34ro_map(address_map &map);
	void onna34ro_mcu_map(address_map &map);
	void rumba_map(address_map &map);
	void sound_map(address_map &map);
	void victnine_map(address_map &map);
};

#endif // MAME_TAITO_FLSTORY_H
