// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
#ifndef MAME_INCLUDES_PCKTGAL_H
#define MAME_INCLUDES_PCKTGAL_H

#pragma once

#include "machine/gen_latch.h"
#include "sound/msm5205.h"
#include "video/decbac06.h"
#include "emupal.h"

class pcktgal_state : public driver_device
{
public:
	pcktgal_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_msm(*this, "msm"),
		m_tilegen(*this, "tilegen"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_spriteram(*this, "spriteram")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<msm5205_device> m_msm;
	required_device<deco_bac06_device> m_tilegen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	required_shared_ptr<uint8_t> m_spriteram;

	int m_msm5205next;
	int m_toggle;

	DECLARE_WRITE8_MEMBER(bank_w);
	DECLARE_WRITE8_MEMBER(sound_bank_w);
	DECLARE_WRITE8_MEMBER(sound_w);
	DECLARE_WRITE8_MEMBER(adpcm_data_w);
	DECLARE_READ8_MEMBER(adpcm_reset_r);
	DECLARE_WRITE_LINE_MEMBER(adpcm_int);

	void init_pcktgal();
	void pcktgal_palette(palette_device &palette) const;
	virtual void machine_start() override;

	uint32_t screen_update_pcktgal(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_pcktgalb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, bool flip_screen);
	void bootleg(machine_config &config);
	void pcktgal(machine_config &config);
	void pcktgal2(machine_config &config);
	void pcktgal_map(address_map &map);
	void pcktgal_sound_map(address_map &map);
};

#endif // MAME_INCLUDES_PCKTGAL_H
