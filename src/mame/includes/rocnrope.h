// license:BSD-3-Clause
// copyright-holders:Chris Hardy
#ifndef MAME_INCLUDES_ROCNROPE_H
#define MAME_INCLUDES_ROCNROPE_H

#pragma once

#include "emupal.h"
#include "tilemap.h"

class rocnrope_state : public driver_device
{
public:
	rocnrope_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_spriteram2(*this, "spriteram2"),
		m_spriteram(*this, "spriteram"),
		m_colorram(*this, "colorram"),
		m_videoram(*this, "videoram")
	{ }

	void rocnrope(machine_config &config);

	void init_rocnrope();

private:
	// devices
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	// memory pointers
	required_shared_ptr<uint8_t> m_spriteram2;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_videoram;

	tilemap_t *m_bg_tilemap;
	uint8_t m_irq_mask;

	DECLARE_WRITE8_MEMBER(rocnrope_interrupt_vector_w);
	DECLARE_WRITE_LINE_MEMBER(irq_mask_w);
	DECLARE_WRITE_LINE_MEMBER(coin_counter_1_w);
	DECLARE_WRITE_LINE_MEMBER(coin_counter_2_w);
	DECLARE_WRITE8_MEMBER(rocnrope_videoram_w);
	DECLARE_WRITE8_MEMBER(rocnrope_colorram_w);
	DECLARE_WRITE_LINE_MEMBER(flip_screen_w);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void video_start() override;
	void rocnrope_palette(palette_device &palette) const;
	uint32_t screen_update_rocnrope(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(vblank_irq);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect );
	void rocnrope_map(address_map &map);
};

#endif // MAME_INCLUDES_ROCNROPE_H
