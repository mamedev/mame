// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

    Fire Trap

***************************************************************************/
#ifndef MAME_INCLUDES_FIRETRAP_H
#define MAME_INCLUDES_FIRETRAP_H

#pragma once

#include "machine/74157.h"
#include "machine/gen_latch.h"
#include "sound/msm5205.h"
#include "emupal.h"
#include "tilemap.h"

class firetrap_state : public driver_device
{
public:
	firetrap_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_bg1videoram(*this, "bg1videoram"),
		m_bg2videoram(*this, "bg2videoram"),
		m_fgvideoram(*this, "fgvideoram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_msm(*this, "msm"),
		m_adpcm_select(*this, "adpcm_select"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch")
	{ }

	void firetrapbl(machine_config &config);
	void firetrap(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);

private:
	/* memory pointers */
	required_shared_ptr<uint8_t> m_bg1videoram;
	required_shared_ptr<uint8_t> m_bg2videoram;
	required_shared_ptr<uint8_t> m_fgvideoram;
	required_shared_ptr<uint8_t> m_spriteram;

	/* video-related */
	tilemap_t       *m_fg_tilemap;
	tilemap_t       *m_bg1_tilemap;
	tilemap_t       *m_bg2_tilemap;
	uint8_t         m_scroll1_x[2];
	uint8_t         m_scroll1_y[2];
	uint8_t         m_scroll2_x[2];
	uint8_t         m_scroll2_y[2];

	/* misc */
	int           m_sound_irq_enable;
	int           m_nmi_enable;
	int           m_i8751_return;
	int           m_i8751_current_command;
	int           m_i8751_init_ptr;
	int           m_adpcm_toggle;
	int           m_coin_command_pending;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<msm5205_device> m_msm;
	required_device<ls157_device> m_adpcm_select;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	DECLARE_WRITE8_MEMBER(firetrap_nmi_disable_w);
	DECLARE_WRITE8_MEMBER(firetrap_bankselect_w);
	DECLARE_READ8_MEMBER(firetrap_8751_bootleg_r);
	DECLARE_READ8_MEMBER(firetrap_8751_r);
	DECLARE_WRITE8_MEMBER(firetrap_8751_w);
	DECLARE_WRITE8_MEMBER(sound_command_w);
	DECLARE_WRITE8_MEMBER(sound_flip_flop_w);
	DECLARE_WRITE8_MEMBER(sound_bankselect_w);
	DECLARE_WRITE8_MEMBER(adpcm_data_w);
	DECLARE_WRITE8_MEMBER(flip_screen_w);
	DECLARE_WRITE8_MEMBER(firetrap_fgvideoram_w);
	DECLARE_WRITE8_MEMBER(firetrap_bg1videoram_w);
	DECLARE_WRITE8_MEMBER(firetrap_bg2videoram_w);
	DECLARE_WRITE8_MEMBER(firetrap_bg1_scrollx_w);
	DECLARE_WRITE8_MEMBER(firetrap_bg1_scrolly_w);
	DECLARE_WRITE8_MEMBER(firetrap_bg2_scrollx_w);
	DECLARE_WRITE8_MEMBER(firetrap_bg2_scrolly_w);
	TILEMAP_MAPPER_MEMBER(get_fg_memory_offset);
	TILEMAP_MAPPER_MEMBER(get_bg_memory_offset);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_bg1_tile_info);
	TILE_GET_INFO_MEMBER(get_bg2_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void firetrap_palette(palette_device &palette) const;
	uint32_t screen_update_firetrap(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(firetrap_irq);
	inline void get_bg_tile_info(tile_data &tileinfo, int tile_index, uint8_t *bgvideoram, int gfx_region);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	DECLARE_WRITE_LINE_MEMBER(firetrap_adpcm_int);
	void firetrap_base_map(address_map &map);
	void firetrap_bootleg_map(address_map &map);
	void firetrap_map(address_map &map);
	void sound_map(address_map &map);
};

#endif // MAME_INCLUDES_FIRETRAP_H
