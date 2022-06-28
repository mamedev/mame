// license:BSD-3-Clause
// copyright-holders:Mirko Buffoni
/*************************************************************************

    Bomb Jack

*************************************************************************/
#ifndef MAME_INCLUDES_BOMBJACK_H
#define MAME_INCLUDES_BOMBJACK_H

#pragma once

#include "machine/gen_latch.h"
#include "emupal.h"
#include "tilemap.h"

class bombjack_state : public driver_device
{
public:
	bombjack_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch")
	{ }

	void bombjack(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	uint8_t soundlatch_read_and_clear();
	void irq_mask_w(uint8_t data);
	void bombjack_videoram_w(offs_t offset, uint8_t data);
	void bombjack_colorram_w(offs_t offset, uint8_t data);
	void bombjack_background_w(uint8_t data);
	void bombjack_flipscreen_w(uint8_t data);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	uint32_t screen_update_bombjack(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(vblank_irq);
	TIMER_CALLBACK_MEMBER(soundlatch_callback);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);

	void audio_io_map(address_map &map);
	void audio_map(address_map &map);
	void main_map(address_map &map);

	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_spriteram;

	/* video-related */
	tilemap_t   *m_fg_tilemap = nullptr;
	tilemap_t   *m_bg_tilemap = nullptr;
	uint8_t       m_background_image = 0U;

	bool          m_nmi_mask = false;

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;
};

#endif // MAME_INCLUDES_BOMBJACK_H
