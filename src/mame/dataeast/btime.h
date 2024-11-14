// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari, Couriersud
/***************************************************************************

    Burger Time hardware

***************************************************************************/
#ifndef MAME_DATAEAST_BTIME_H
#define MAME_DATAEAST_BTIME_H

#pragma once

#include "machine/gen_latch.h"
#include "machine/input_merger.h"
#include "machine/timer.h"
#include "emupal.h"
#include "screen.h"

class btime_state : public driver_device
{
public:
	btime_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_rambase(*this, "rambase")
		, m_videoram(*this, "videoram")
		, m_colorram(*this, "colorram")
		, m_bnj_backgroundram(*this, "bnj_bgram")
		, m_zoar_scrollram(*this, "zoar_scrollram")
		, m_lnc_charbank(*this, "lnc_charbank")
		, m_deco_charram(*this, "deco_charram")
		, m_spriteram(*this, "spriteram")
		, m_audio_rambase(*this, "audio_rambase")
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_audionmi(*this, "audionmi")
		, m_gfxdecode(*this, "gfxdecode")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_soundlatch(*this, "soundlatch")
		, m_prom_region(*this, "proms")
	{
	}

	/* memory pointers */
	optional_shared_ptr<uint8_t> m_rambase;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	optional_shared_ptr<uint8_t> m_bnj_backgroundram;
	optional_shared_ptr<uint8_t> m_zoar_scrollram;
	optional_shared_ptr<uint8_t> m_lnc_charbank;
	optional_shared_ptr<uint8_t> m_deco_charram;
	optional_shared_ptr<uint8_t> m_spriteram;     // used by disco
//  uint8_t *  m_decrypted;
	optional_shared_ptr<uint8_t> m_audio_rambase;

	/* video-related */
	std::unique_ptr<bitmap_ind16> m_background_bitmap;
	uint8_t    m_btime_palette = 0;
	uint8_t    m_bnj_scroll1 = 0;
	uint8_t    m_bnj_scroll2 = 0;
	uint8_t    m_btime_tilemap[4]{};

	/* audio-related */
	uint8_t    m_audio_nmi_enable_type = 0;

	/* protection-related (for mmonkey) */
	int      m_protection_command = 0;
	int      m_protection_status = 0;
	int      m_protection_value = 0;
	int      m_protection_ret = 0;

	/* devices */
	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	optional_device<input_merger_device> m_audionmi;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	optional_device<generic_latch_8_device> m_soundlatch;
	optional_memory_region m_prom_region;

	void audio_nmi_enable_w(uint8_t data);
	uint8_t zoar_dsw1_read();
	uint8_t wtennis_reset_hack_r();
	uint8_t mmonkey_protection_r(offs_t offset);
	void mmonkey_protection_w(offs_t offset, uint8_t data);
	void lnc_videoram_w(offs_t offset, uint8_t data);
	uint8_t btime_mirrorvideoram_r(offs_t offset);
	uint8_t btime_mirrorcolorram_r(offs_t offset);
	void btime_mirrorvideoram_w(offs_t offset, uint8_t data);
	void lnc_mirrorvideoram_w(offs_t offset, uint8_t data);
	void btime_mirrorcolorram_w(offs_t offset, uint8_t data);
	void deco_charram_w(offs_t offset, uint8_t data);
	void bnj_background_w(offs_t offset, uint8_t data);
	void bnj_scroll1_w(uint8_t data);
	void bnj_scroll2_w(uint8_t data);
	void btime_video_control_w(uint8_t data);
	void bnj_video_control_w(uint8_t data);
	void zoar_video_control_w(uint8_t data);
	void disco_video_control_w(uint8_t data);
	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted_irq_hi);
	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted_irq_lo);
	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted_nmi_lo);
	void ay_audio_nmi_enable_w(uint8_t data);

	void init_btime();
	void init_tisland();
	void init_cookrace();
	void init_zoar();
	void init_sdtennis();
	void init_wtennis();
	void init_bnj();
	void init_protennb();
	void init_disco();
	void init_lnc();
	DECLARE_MACHINE_START(btime);
	DECLARE_MACHINE_RESET(btime);
	void btime_palette(palette_device &palette) const;
	DECLARE_MACHINE_RESET(lnc);
	void lnc_palette(palette_device &palette) const;
	DECLARE_MACHINE_START(mmonkey);
	DECLARE_MACHINE_RESET(mmonkey);
	DECLARE_VIDEO_START(bnj);
	DECLARE_VIDEO_START(disco);
	uint32_t screen_update_btime(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_cookrace(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_lnc(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_eggs(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_bnj(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_zoar(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_disco(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(audio_nmi_gen);
	void draw_chars( bitmap_ind16 &bitmap, const rectangle &cliprect, uint8_t transparency, uint8_t color, int priority );
	void draw_background( bitmap_ind16 &bitmap, const rectangle &cliprect, uint8_t* tmap, uint8_t color );
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, uint8_t color,
							uint8_t sprite_y_adjust, uint8_t sprite_y_adjust_flip_screen,
							uint8_t *sprite_ram, offs_t interleave );

	void lnc(machine_config &config);
	void disco(machine_config &config);
	void mmonkey(machine_config &config);
	void bnj(machine_config &config);
	void cookrace(machine_config &config);
	void protenn(machine_config &config);
	void wtennis(machine_config &config);
	void sdtennis(machine_config &config);
	void tisland(machine_config &config);
	void zoar(machine_config &config);
	void btime(machine_config &config);
	void audio_map(address_map &map) ATTR_COLD;
	void bnj_map(address_map &map) ATTR_COLD;
	void btime_map(address_map &map) ATTR_COLD;
	void cookrace_map(address_map &map) ATTR_COLD;
	void disco_audio_map(address_map &map) ATTR_COLD;
	void disco_map(address_map &map) ATTR_COLD;
	void lnc_map(address_map &map) ATTR_COLD;
	void mmonkey_map(address_map &map) ATTR_COLD;
	void protenn_map(address_map &map) ATTR_COLD;
	void tisland_map(address_map &map) ATTR_COLD;
	void zoar_map(address_map &map) ATTR_COLD;
};

#endif // MAME_DATAEAST_BTIME_H
