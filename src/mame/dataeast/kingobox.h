// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
/*************************************************************************

    King of Boxer - Ring King

*************************************************************************/
#ifndef MAME_DATAEAST_KINGOBOX_H
#define MAME_DATAEAST_KINGOBOX_H

#pragma once

#include "machine/gen_latch.h"
#include "machine/input_merger.h"
#include "emupal.h"
#include "tilemap.h"

class kingofb_state : public driver_device
{
public:
	kingofb_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_scroll_y(*this, "scroll_y"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_videoram2(*this, "videoram2"),
		m_colorram2(*this, "colorram2"),
		m_spriteram(*this, "spriteram"),
		m_video_cpu(*this, "video"),
		m_sprite_cpu(*this, "sprite"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_nmigate(*this, "nmigate"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch")
	{ }

	void kingofb(machine_config &config);
	void ringking(machine_config &config);

	void init_ringkingw();
	void init_ringking3();

private:
	/* memory pointers */
	required_shared_ptr<uint8_t> m_scroll_y;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_videoram2;
	required_shared_ptr<uint8_t> m_colorram2;
	required_shared_ptr<uint8_t> m_spriteram;

	/* video-related */
	tilemap_t    *m_bg_tilemap = nullptr;
	tilemap_t    *m_fg_tilemap = nullptr;
	int        m_palette_bank = 0;

	/* devices */
	required_device<cpu_device> m_video_cpu;
	required_device<cpu_device> m_sprite_cpu;
	void video_interrupt_w(uint8_t data);
	void sprite_interrupt_w(uint8_t data);
	void scroll_interrupt_w(uint8_t data);
	void sound_command_w(uint8_t data);
	void kingofb_videoram_w(offs_t offset, uint8_t data);
	void kingofb_colorram_w(offs_t offset, uint8_t data);
	void kingofb_videoram2_w(offs_t offset, uint8_t data);
	void kingofb_colorram2_w(offs_t offset, uint8_t data);
	void kingofb_f800_w(uint8_t data);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(ringking_get_bg_tile_info);
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	DECLARE_VIDEO_START(kingofb);
	void kingofb_palette(palette_device &palette);
	DECLARE_VIDEO_START(ringking);
	void ringking_palette(palette_device &palette);
	uint32_t screen_update_kingofb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_ringking(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(kingofb_interrupt);
	void palette_init_common(palette_device &palette, const uint8_t *color_prom, void (kingofb_state::*get_rgb_data)(const uint8_t *, int, int *, int *, int *));
	void kingofb_get_rgb_data( const uint8_t *color_prom, int i, int *r_data, int *g_data, int *b_data );
	void ringking_get_rgb_data( const uint8_t *color_prom, int i, int *r_data, int *g_data, int *b_data );
	void kingofb_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void ringking_draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<input_merger_device> m_nmigate;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;
	void kingobox_map(address_map &map) ATTR_COLD;
	void kingobox_sound_io_map(address_map &map) ATTR_COLD;
	void kingobox_sound_map(address_map &map) ATTR_COLD;
	void kingobox_sprite_map(address_map &map) ATTR_COLD;
	void kingobox_video_map(address_map &map) ATTR_COLD;
	void ringking_map(address_map &map) ATTR_COLD;
	void ringking_sound_io_map(address_map &map) ATTR_COLD;
	void ringking_sprite_map(address_map &map) ATTR_COLD;
	void ringking_video_map(address_map &map) ATTR_COLD;
};

#endif // MAME_DATAEAST_KINGOBOX_H
