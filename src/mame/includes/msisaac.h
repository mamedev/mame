// license:GPL-2.0+
// copyright-holders:Jarek Burczynski
#ifndef MAME_INCLUDES_MSISAAC_H
#define MAME_INCLUDES_MSISAAC_H

#pragma once

#include "machine/taito68705interface.h"
#include "machine/gen_latch.h"
#include "sound/msm5232.h"
#include "sound/ta7630.h"
#include "emupal.h"

/* Disabled because the mcu dump is currently unavailable. -AS */
//#define USE_MCU

class msisaac_state : public driver_device
{
public:
	msisaac_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_spriteram(*this, "spriteram"),
		m_videoram(*this, "videoram"),
		m_videoram3(*this, "videoram3"),
		m_videoram2(*this, "videoram2"),
		m_audiocpu(*this, "audiocpu"),
		m_maincpu(*this, "maincpu"),
		m_bmcu(*this, "bmcu"),
		m_msm(*this, "msm"),
		m_ta7630(*this, "ta7630"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch")
	{ }

	void msisaac(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	/* memory pointers */
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_videoram3;
	required_shared_ptr<uint8_t> m_videoram2;

	/* video-related */
	bitmap_ind16    *m_tmp_bitmap1;
	bitmap_ind16    *m_tmp_bitmap2;
	tilemap_t     *m_bg_tilemap;
	tilemap_t     *m_fg_tilemap;
	tilemap_t     *m_bg2_tilemap;
	int         m_bg2_textbank;

	/* sound-related */
	int         m_sound_nmi_enable;
	int         m_pending_nmi;

	/* fake mcu (in msisaac.c) */
#ifndef USE_MCU
	uint8_t       m_mcu_val;
	uint8_t       m_direction;
#endif

	uint8_t       m_snd_ctrl0;
	uint8_t       m_snd_ctrl1;
	uint8_t       m_snd_ctrl2;
	uint8_t       m_snd_ctrl3;

	/* devices */
	required_device<cpu_device> m_audiocpu;
	required_device<cpu_device> m_maincpu;
	optional_device<taito68705_mcu_device> m_bmcu;
	required_device<msm5232_device> m_msm;
	required_device<ta7630_device> m_ta7630;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	DECLARE_WRITE8_MEMBER(sound_command_w);
	DECLARE_WRITE8_MEMBER(nmi_disable_w);
	DECLARE_WRITE8_MEMBER(nmi_enable_w);
	DECLARE_WRITE8_MEMBER(ms_unknown_w);
	DECLARE_READ8_MEMBER(msisaac_mcu_r);
	DECLARE_READ8_MEMBER(msisaac_mcu_status_r);
	DECLARE_WRITE8_MEMBER(msisaac_mcu_w);
	DECLARE_WRITE8_MEMBER(sound_control_1_w);
	DECLARE_WRITE8_MEMBER(msisaac_fg_scrolly_w);
	DECLARE_WRITE8_MEMBER(msisaac_fg_scrollx_w);
	DECLARE_WRITE8_MEMBER(msisaac_bg2_scrolly_w);
	DECLARE_WRITE8_MEMBER(msisaac_bg2_scrollx_w);
	DECLARE_WRITE8_MEMBER(msisaac_bg_scrolly_w);
	DECLARE_WRITE8_MEMBER(msisaac_bg_scrollx_w);
	DECLARE_WRITE8_MEMBER(msisaac_bg2_textbank_w);
	DECLARE_WRITE8_MEMBER(msisaac_bg_videoram_w);
	DECLARE_WRITE8_MEMBER(msisaac_bg2_videoram_w);
	DECLARE_WRITE8_MEMBER(msisaac_fg_videoram_w);
	DECLARE_WRITE8_MEMBER(sound_control_0_w);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_bg2_tile_info);
	uint32_t screen_update_msisaac(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(nmi_callback);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void msisaac_map(address_map &map);
	void msisaac_sound_map(address_map &map);
};

#endif // MAME_INCLUDES_MSISAAC_H
