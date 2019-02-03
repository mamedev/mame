// license:BSD-3-Clause
// copyright-holders:Manuel Abadia
/*************************************************************************

    Rock'n Rage

*************************************************************************/
#ifndef MAME_INCLUDES_ROCKRAGE_H
#define MAME_INCLUDES_ROCKRAGE_H

#pragma once

#include "machine/gen_latch.h"
#include "sound/vlm5030.h"
#include "video/k007342.h"
#include "video/k007420.h"
#include "emupal.h"

class rockrage_state : public driver_device
{
public:
	rockrage_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_k007342(*this, "k007342"),
		m_k007420(*this, "k007420"),
		m_vlm(*this, "vlm"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_rombank(*this, "rombank")
	{ }

	void rockrage(machine_config &config);

private:
	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<k007342_device> m_k007342;
	required_device<k007420_device> m_k007420;
	required_device<vlm5030_device> m_vlm;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	/* memory pointers */
	required_memory_bank m_rombank;

	/* video-related */
	int        m_vreg;

	DECLARE_WRITE8_MEMBER(rockrage_bankswitch_w);
	DECLARE_WRITE8_MEMBER(rockrage_sh_irqtrigger_w);
	DECLARE_WRITE8_MEMBER(rockrage_vreg_w);
	DECLARE_READ8_MEMBER(rockrage_VLM5030_busy_r);
	DECLARE_WRITE8_MEMBER(rockrage_speech_w);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	void rockrage_palette(palette_device &palette) const;
	uint32_t screen_update_rockrage(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(vblank_irq);
	K007342_CALLBACK_MEMBER(rockrage_tile_callback);
	K007420_CALLBACK_MEMBER(rockrage_sprite_callback);

	void rockrage_map(address_map &map);
	void rockrage_sound_map(address_map &map);
	void rockrage_vlm_map(address_map &map);
};

#endif // MAME_INCLUDES_ROCKRAGE_H
