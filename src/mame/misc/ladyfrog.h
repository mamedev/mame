// license:BSD-3-Clause
// copyright-holders:Tomasz Slanina
/*************************************************************************

    Lady Frog

*************************************************************************/
#ifndef MAME_MISC_LADYFROG_H
#define MAME_MISC_LADYFROG_H

#pragma once

#include "machine/gen_latch.h"
#include "sound/msm5232.h"
#include "emupal.h"
#include "tilemap.h"

class ladyfrog_state : public driver_device
{
public:
	ladyfrog_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_scrlram(*this, "scrlram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_msm(*this, "msm"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch")
	{ }

	void toucheme(machine_config &config);
	void ladyfrog(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	std::unique_ptr<uint8_t[]>    m_spriteram;
	required_shared_ptr<uint8_t> m_scrlram;
	std::vector<uint8_t> m_paletteram;
	std::vector<uint8_t> m_paletteram_ext;

	/* video-related */
	tilemap_t    *m_bg_tilemap = nullptr;
	int        m_tilebank = 0;
	int        m_palette_bank = 0;
	int        m_spritetilebase = 0;

	/* misc */
	int        m_sound_nmi_enable = 0;
	int        m_pending_nmi = 0;
	int        m_snd_flag = 0;
	uint8_t      m_snd_data = 0U;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<msm5232_device> m_msm;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	uint8_t from_snd_r();
	void to_main_w(uint8_t data);
	void sound_cpu_reset_w(uint8_t data);
	void sound_command_w(uint8_t data);
	void nmi_disable_w(uint8_t data);
	void nmi_enable_w(uint8_t data);
	uint8_t snd_flag_r();
	void ladyfrog_spriteram_w(offs_t offset, uint8_t data);
	uint8_t ladyfrog_spriteram_r(offs_t offset);
	void ladyfrog_videoram_w(offs_t offset, uint8_t data);
	uint8_t ladyfrog_videoram_r(offs_t offset);
	void ladyfrog_palette_w(offs_t offset, uint8_t data);
	uint8_t ladyfrog_palette_r(offs_t offset);
	void ladyfrog_gfxctrl_w(uint8_t data);
	void ladyfrog_gfxctrl2_w(uint8_t data);
	uint8_t ladyfrog_scrlram_r(offs_t offset);
	void ladyfrog_scrlram_w(offs_t offset, uint8_t data);
	void unk_w(uint8_t data);
	TILE_GET_INFO_MEMBER(get_tile_info);
	DECLARE_VIDEO_START(toucheme);
	DECLARE_VIDEO_START(ladyfrog_common);
	uint32_t screen_update_ladyfrog(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(nmi_callback);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void ladyfrog_map(address_map &map) ATTR_COLD;
	void ladyfrog_sound_map(address_map &map) ATTR_COLD;
};

#endif // MAME_MISC_LADYFROG_H
