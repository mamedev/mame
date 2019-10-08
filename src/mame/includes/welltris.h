// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_INCLUDES_WELLTRIS_H
#define MAME_INCLUDES_WELLTRIS_H

#pragma once

#include "machine/gen_latch.h"
#include "video/vsystem_spr2.h"
#include "screen.h"
#include "tilemap.h"

class welltris_state : public driver_device
{
public:
	welltris_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_spr_old(*this, "vsystem_spr_old"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_soundlatch(*this, "soundlatch"),
		m_spriteram(*this, "spriteram"),
		m_pixelram(*this, "pixelram"),
		m_charvideoram(*this, "charvideoram")
	{ }

	void quiz18k(machine_config &config);
	void welltris(machine_config &config);

	void init_quiz18k();
	void init_welltris();

protected:
	virtual void machine_start() override;
	virtual void video_start() override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<vsystem_spr2_device> m_spr_old;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<generic_latch_8_device> m_soundlatch;

	required_shared_ptr<uint16_t> m_spriteram;
	required_shared_ptr<uint16_t> m_pixelram;
	required_shared_ptr<uint16_t> m_charvideoram;

	tilemap_t *m_char_tilemap;
	uint8_t m_gfxbank[2];
	uint16_t m_charpalettebank;
	uint16_t m_spritepalettebank;
	uint16_t m_pixelpalettebank;
	int m_scrollx;
	int m_scrolly;

	DECLARE_WRITE8_MEMBER(sound_bankswitch_w);
	DECLARE_WRITE16_MEMBER(palette_bank_w);
	DECLARE_WRITE16_MEMBER(gfxbank_w);
	DECLARE_WRITE16_MEMBER(scrollreg_w);
	DECLARE_WRITE16_MEMBER(charvideoram_w);

	TILE_GET_INFO_MEMBER(get_tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_background(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void setbank(int num, int bank);
	void main_map(address_map &map);
	void sound_map(address_map &map);
	void sound_port_map(address_map &map);
};

#endif // MAME_INCLUDES_WELLTRIS_H
