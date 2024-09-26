// license:BSD-3-Clause
// copyright-holders:Chris Hardy
/***************************************************************************

    Track'n'Field

***************************************************************************/
#ifndef MAME_KONAMI_TRACKFLD_H
#define MAME_KONAMI_TRACKFLD_H

#pragma once

#include "trackfld_a.h"
#include "machine/74259.h"
#include "sound/dac.h"
#include "sound/sn76496.h"
#include "sound/vlm5030.h"

#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

class trackfld_state : public driver_device
{
public:
	trackfld_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_spriteram2(*this, "spriteram2"),
		m_scroll(*this, "scroll"),
		m_spriteram(*this, "spriteram"),
		m_scroll2(*this, "scroll2"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_maincpu(*this, "maincpu"),
		m_mainlatch(*this, "mainlatch"),
		m_audiocpu(*this, "audiocpu"),
		m_soundbrd(*this, "trackfld_audio"),
		m_sn(*this, "snsnd"),
		m_vlm(*this, "vlm"),
		m_dac(*this, "dac"),
		m_screen(*this, "screen"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	void reaktor(machine_config &config);
	void atlantol(machine_config &config);
	void yieartf(machine_config &config);
	void wizzquiz(machine_config &config);
	void trackfld(machine_config &config);
	void trackfldu(machine_config &config);
	void hyprolyb(machine_config &config);
	void mastkin(machine_config &config);

	void init_trackfld();
	void init_atlantol();
	void init_wizzquiz();
	void init_mastkin();
	void init_trackfldnz();

private:
	void questions_bank_w(uint8_t data);
	void trackfld_videoram_w(offs_t offset, uint8_t data);
	void trackfld_colorram_w(offs_t offset, uint8_t data);
	void atlantol_gfxbank_w(uint8_t data);
	uint8_t trackfld_SN76496_r();
	uint8_t trackfld_speech_r();
	void trackfld_VLM5030_control_w(uint8_t data);
	void konami_SN76496_latch_w(uint8_t data) { m_SN76496_latch = data; }
	void konami_SN76496_w(uint8_t data) { m_sn->write(m_SN76496_latch); }

	void hyprolyb_sound_map(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;
	void mastkin_map(address_map &map) ATTR_COLD;
	void reaktor_io_map(address_map &map) ATTR_COLD;
	void reaktor_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
	void vlm_map(address_map &map) ATTR_COLD;
	void wizzquiz_map(address_map &map) ATTR_COLD;
	void yieartf_map(address_map &map) ATTR_COLD;
	void hyprolyb_adpcm_map(address_map &map) ATTR_COLD;

	/* memory pointers */
	required_shared_ptr<uint8_t> m_spriteram2;
	required_shared_ptr<uint8_t> m_scroll;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_scroll2;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;

	/* devices */
	required_device<cpu_device> m_maincpu;
	optional_device<ls259_device> m_mainlatch;
	optional_device<cpu_device> m_audiocpu;
	optional_device<trackfld_audio_device> m_soundbrd;
	optional_device<sn76496_device> m_sn;
	optional_device<vlm5030_device> m_vlm;
	required_device<dac_8bit_r2r_device> m_dac;
	required_device<screen_device> m_screen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	/* video-related */
	tilemap_t  *m_bg_tilemap = nullptr;
	int      m_bg_bank = 0;
	int      m_sprite_bank1 = 0;
	int      m_sprite_bank2 = 0;
	int      m_old_gfx_bank = 0;                    // needed by atlantol
	int      m_sprites_gfx_banked = 0;

	bool     m_irq_mask = false;
	bool     m_nmi_mask = false;

	uint8_t m_SN76496_latch = 0;

	void coin_counter_1_w(int state);
	void coin_counter_2_w(int state);
	void irq_mask_w(int state);
	void nmi_mask_w(int state);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	DECLARE_MACHINE_START(trackfld);
	DECLARE_MACHINE_RESET(trackfld);
	DECLARE_VIDEO_START(trackfld);
	void trackfld_palette(palette_device &palette) const;
	DECLARE_VIDEO_START(atlantol);
	uint32_t screen_update_trackfld(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void vblank_irq(int state);
	void vblank_nmi(int state);
	INTERRUPT_GEN_MEMBER(yieartf_timer_irq);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
};

#endif // MAME_KONAMI_TRACKFLD_H
