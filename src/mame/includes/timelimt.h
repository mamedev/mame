// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
#ifndef MAME_INCLUDES_TIMELIMT_H
#define MAME_INCLUDES_TIMELIMT_H

#pragma once

#include "emupal.h"
#include "tilemap.h"

class timelimt_state : public driver_device
{
public:
	timelimt_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_videoram(*this, "videoram"),
		m_bg_videoram(*this, "bg_videoram"),
		m_spriteram(*this, "spriteram")
	{ }

	void timelimt(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void video_start() override;

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_bg_videoram;
	required_shared_ptr<uint8_t> m_spriteram;

	int m_nmi_enabled;
	int m_scrollx;
	int m_scrolly;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;

	DECLARE_WRITE_LINE_MEMBER(nmi_enable_w);
	DECLARE_WRITE_LINE_MEMBER(coin_lockout_w);

	DECLARE_WRITE8_MEMBER(videoram_w);
	DECLARE_WRITE8_MEMBER(bg_videoram_w);
	DECLARE_WRITE8_MEMBER(scroll_x_lsb_w);
	DECLARE_WRITE8_MEMBER(scroll_x_msb_w);
	DECLARE_WRITE8_MEMBER(scroll_y_w);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);

	void timelimt_palette(palette_device &palette) const;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);

	INTERRUPT_GEN_MEMBER(irq);

	void main_io_map(address_map &map);
	void main_map(address_map &map);
	void sound_io_map(address_map &map);
	void sound_map(address_map &map);
};

#endif // MAME_INCLUDES_TIMELIMT_H
