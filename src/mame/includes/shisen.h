// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#ifndef MAME_INCLUDES_SHISEN_H
#define MAME_INCLUDES_SHISEN_H

#pragma once

#include "audio/m72.h"
#include "emupal.h"
#include "tilemap.h"

class shisen_state : public driver_device
{
public:
	shisen_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audio(*this, "m72"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_paletteram(*this, "paletteram"),
		m_videoram(*this, "videoram")
	{ }

	void shisen(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	required_device<m72_audio_device> m_audio;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_paletteram;
	required_shared_ptr<uint8_t> m_videoram;

	int m_gfxbank;
	tilemap_t *m_bg_tilemap;

	DECLARE_WRITE8_MEMBER(coin_w);
	DECLARE_WRITE8_MEMBER(videoram_w);
	DECLARE_WRITE8_MEMBER(bankswitch_w);
	DECLARE_WRITE8_MEMBER(paletteram_w);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);

	virtual void video_start() override;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void shisen_io_map(address_map &map);
	void shisen_map(address_map &map);
	void shisen_sound_io_map(address_map &map);
	void shisen_sound_map(address_map &map);
};

#endif // MAME_INCLUDES_SHISEN_H
