// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/*************************************************************************

    Last Duel

*************************************************************************/
#ifndef MAME_INCLUDES_LASTDUEL_H
#define MAME_INCLUDES_LASTDUEL_H

#pragma once

#include "machine/gen_latch.h"
#include "machine/timer.h"
#include "video/bufsprite.h"
#include "emupal.h"
#include "tilemap.h"

class lastduel_state : public driver_device
{
public:
	lastduel_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_spriteram(*this, "spriteram"),
		m_txram(*this, "txram"),
		m_vram(*this, "vram_%u", 0U),
		m_audiobank(*this, "audiobank")
	{ }

	void lastduel(machine_config &config);
	void madgear(machine_config &config);

private:
	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	/* memory pointers */
	required_device<buffered_spriteram16_device> m_spriteram;
	required_shared_ptr<uint16_t> m_txram;
	required_shared_ptr_array<uint16_t, 2> m_vram;

	optional_memory_bank m_audiobank;

	/* video-related */
	tilemap_t     *m_tilemap[2];
	tilemap_t     *m_tx_tilemap;
	uint16_t      m_vctrl[8];
	int         m_sprite_flipy_mask;
	int         m_sprite_pri_mask;
	int         m_tilemap_priority;

	DECLARE_WRITE8_MEMBER(mg_bankswitch_w);
	DECLARE_WRITE8_MEMBER(flip_w);
	DECLARE_WRITE16_MEMBER(vctrl_w);
	template<int Layer> DECLARE_WRITE16_MEMBER(lastduel_vram_w);
	DECLARE_WRITE16_MEMBER(txram_w);
	template<int Layer> DECLARE_WRITE16_MEMBER(madgear_vram_w);
	static rgb_t lastduel_RRRRGGGGBBBBIIII(uint32_t raw);
	TILE_GET_INFO_MEMBER(ld_get_bg_tile_info);
	TILE_GET_INFO_MEMBER(ld_get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_fix_info);
	virtual void machine_reset() override;
	DECLARE_MACHINE_START(lastduel);
	DECLARE_VIDEO_START(lastduel);
	DECLARE_MACHINE_START(madgear);
	DECLARE_VIDEO_START(madgear);
	uint32_t screen_update_lastduel(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_madgear(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(lastduel_timer_cb);
	TIMER_DEVICE_CALLBACK_MEMBER(madgear_timer_cb);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int pri);
	void lastduel_map(address_map &map);
	void madgear_map(address_map &map);
	void madgear_sound_map(address_map &map);
	void sound_map(address_map &map);
};

#endif // MAME_INCLUDES_LASTDUEL_H
