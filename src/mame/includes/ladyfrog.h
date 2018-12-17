// license:BSD-3-Clause
// copyright-holders:Tomasz Slanina
/*************************************************************************

    Lady Frog

*************************************************************************/
#ifndef MAME_INCLUDES_LADYFROG_H
#define MAME_INCLUDES_LADYFROG_H

#pragma once

#include "machine/gen_latch.h"
#include "sound/msm5232.h"
#include "emupal.h"

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
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	std::unique_ptr<uint8_t[]>    m_spriteram;
	required_shared_ptr<uint8_t> m_scrlram;
	std::vector<uint8_t> m_paletteram;
	std::vector<uint8_t> m_paletteram_ext;

	/* video-related */
	tilemap_t    *m_bg_tilemap;
	int        m_tilebank;
	int        m_palette_bank;
	int        m_spritetilebase;

	/* misc */
	int        m_sound_nmi_enable;
	int        m_pending_nmi;
	int        m_snd_flag;
	uint8_t      m_snd_data;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<msm5232_device> m_msm;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	DECLARE_READ8_MEMBER(from_snd_r);
	DECLARE_WRITE8_MEMBER(to_main_w);
	DECLARE_WRITE8_MEMBER(sound_cpu_reset_w);
	DECLARE_WRITE8_MEMBER(sound_command_w);
	DECLARE_WRITE8_MEMBER(nmi_disable_w);
	DECLARE_WRITE8_MEMBER(nmi_enable_w);
	DECLARE_READ8_MEMBER(snd_flag_r);
	DECLARE_WRITE8_MEMBER(ladyfrog_spriteram_w);
	DECLARE_READ8_MEMBER(ladyfrog_spriteram_r);
	DECLARE_WRITE8_MEMBER(ladyfrog_videoram_w);
	DECLARE_READ8_MEMBER(ladyfrog_videoram_r);
	DECLARE_WRITE8_MEMBER(ladyfrog_palette_w);
	DECLARE_READ8_MEMBER(ladyfrog_palette_r);
	DECLARE_WRITE8_MEMBER(ladyfrog_gfxctrl_w);
	DECLARE_WRITE8_MEMBER(ladyfrog_gfxctrl2_w);
	DECLARE_READ8_MEMBER(ladyfrog_scrlram_r);
	DECLARE_WRITE8_MEMBER(ladyfrog_scrlram_w);
	DECLARE_WRITE8_MEMBER(unk_w);
	TILE_GET_INFO_MEMBER(get_tile_info);
	DECLARE_VIDEO_START(toucheme);
	DECLARE_VIDEO_START(ladyfrog_common);
	uint32_t screen_update_ladyfrog(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(nmi_callback);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void ladyfrog_map(address_map &map);
	void ladyfrog_sound_map(address_map &map);
};

#endif // MAME_INCLUDES_LADYFROG_H
