// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino
#ifndef MAME_INCLUDES_LKAGE_H
#define MAME_INCLUDES_LKAGE_H

#pragma once

#include "machine/gen_latch.h"
#include "machine/input_merger.h"
#include "machine/taito68705interface.h"
#include "emupal.h"
#include "tilemap.h"

class lkage_state : public driver_device
{
public:
	lkage_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_vreg(*this, "vreg"),
		m_scroll(*this, "scroll"),
		m_spriteram(*this, "spriteram"),
		m_videoram(*this, "videoram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_bmcu(*this, "bmcu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_soundnmi(*this, "soundnmi")
	{ }

	void lkageb(machine_config &config);
	void lkage(machine_config &config);

	void init_bygone();
	void init_lkage();

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	required_shared_ptr<uint8_t> m_vreg;
	required_shared_ptr<uint8_t> m_scroll;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_videoram;

	/* video-related */
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_tx_tilemap;
	uint8_t m_bg_tile_bank;
	uint8_t m_fg_tile_bank;
	uint8_t m_tx_tile_bank;

	int m_sprite_dx;

	/* lkageb fake mcu */
	uint8_t m_mcu_val;
	int m_mcu_ready;    /* cpu data/mcu ready status */

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<taito68705_mcu_device> m_bmcu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<input_merger_device> m_soundnmi;

	DECLARE_WRITE8_MEMBER(lkage_sh_nmi_disable_w);
	DECLARE_WRITE8_MEMBER(lkage_sh_nmi_enable_w);
	DECLARE_READ8_MEMBER(sound_status_r);
	DECLARE_READ8_MEMBER(port_fetch_r);
	DECLARE_READ8_MEMBER(mcu_status_r);
	DECLARE_READ8_MEMBER(fake_mcu_r);
	DECLARE_WRITE8_MEMBER(fake_mcu_w);
	DECLARE_READ8_MEMBER(fake_status_r);

	DECLARE_WRITE8_MEMBER(lkage_videoram_w);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_tx_tile_info);
	uint32_t screen_update_lkage(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect );
	void lkage_io_map(address_map &map);
	void lkage_map(address_map &map);
	void lkage_map_boot(address_map &map);
	void lkage_map_mcu(address_map &map);
	void lkage_sound_map(address_map &map);
};

#endif // MAME_INCLUDES_LKAGE_H
