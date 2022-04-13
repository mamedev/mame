// license:BSD-3-Clause
// copyright-holders:Mirko Buffoni
/***************************************************************************

  Capcom Vulgus hardware

***************************************************************************/
#ifndef MAME_INCLUDES_VULGUS_H
#define MAME_INCLUDES_VULGUS_H

#pragma once

#include "emupal.h"
#include "tilemap.h"

class vulgus_state : public driver_device
{
public:
	vulgus_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_scroll_low(*this, "scroll_low"),
		m_scroll_high(*this, "scroll_high"),
		m_spriteram(*this, "spriteram"),
		m_fgvideoram(*this, "fgvideoram"),
		m_bgvideoram(*this, "bgvideoram")
	{ }

	void vulgus(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_scroll_low;
	required_shared_ptr<uint8_t> m_scroll_high;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_fgvideoram;
	required_shared_ptr<uint8_t> m_bgvideoram;

	int m_palette_bank = 0;
	tilemap_t *m_fg_tilemap = nullptr;
	tilemap_t *m_bg_tilemap = nullptr;

	void fgvideoram_w(offs_t offset, uint8_t data);
	void bgvideoram_w(offs_t offset, uint8_t data);
	void c804_w(uint8_t data);
	void palette_bank_w(uint8_t data);

	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);

	virtual void video_start() override;
	void vulgus_palette(palette_device &palette) const;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect);

	INTERRUPT_GEN_MEMBER(vblank_irq);

	void main_map(address_map &map);
	void sound_map(address_map &map);
};

#endif // MAME_INCLUDES_VULGUS_H
