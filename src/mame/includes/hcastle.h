// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/*************************************************************************

    Haunted Castle

*************************************************************************/
#ifndef MAME_INCLUDES_HCASTLE_H
#define MAME_INCLUDES_HCASTLE_H

#pragma once

#include "video/bufsprite.h"
#include "sound/k007232.h"
#include "video/k007121.h"
#include "emupal.h"
#include "tilemap.h"

class hcastle_state : public driver_device
{
public:
	hcastle_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_spriteram(*this, "spriteram"),
		m_spriteram2(*this, "spriteram2") ,
		m_pf1_videoram(*this, "pf1_videoram"),
		m_pf2_videoram(*this, "pf2_videoram"),
		m_audiocpu(*this, "audiocpu"),
		m_k007121_1(*this, "k007121_1"),
		m_k007121_2(*this, "k007121_2"),
		m_k007232(*this, "k007232"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	void hcastle(machine_config &config);

private:
	required_device<buffered_spriteram8_device> m_spriteram;
	required_device<buffered_spriteram8_device> m_spriteram2;
	/* memory pointers */
	required_shared_ptr<uint8_t> m_pf1_videoram;
	required_shared_ptr<uint8_t> m_pf2_videoram;

	/* video-related */
	tilemap_t    *m_fg_tilemap = nullptr;
	tilemap_t    *m_bg_tilemap = nullptr;
	int        m_pf2_bankbase = 0;
	int        m_pf1_bankbase = 0;
	int        m_old_pf1 = 0;
	int        m_old_pf2 = 0;
	int        m_gfx_bank = 0;

	/* devices */
	required_device<cpu_device> m_audiocpu;
	required_device<k007121_device> m_k007121_1;
	required_device<k007121_device> m_k007121_2;
	required_device<k007232_device> m_k007232;

	void hcastle_bankswitch_w(uint8_t data);
	void hcastle_soundirq_w(uint8_t data);
	void hcastle_coin_w(uint8_t data);
	void hcastle_pf1_video_w(offs_t offset, uint8_t data);
	void hcastle_pf2_video_w(offs_t offset, uint8_t data);
	void hcastle_gfxbank_w(uint8_t data);
	uint8_t hcastle_gfxbank_r();
	void hcastle_pf1_control_w(offs_t offset, uint8_t data);
	void hcastle_pf2_control_w(offs_t offset, uint8_t data);
	void sound_bank_w(uint8_t data);
	TILEMAP_MAPPER_MEMBER(tilemap_scan);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void hcastle_palette(palette_device &palette) const;
	uint32_t screen_update_hcastle(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, bitmap_ind8 &priority_bitmap, uint8_t *sbank, int bank );
	void volume_callback(uint8_t data);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	void hcastle_map(address_map &map);
	void sound_map(address_map &map);
};

#endif // MAME_INCLUDES_HCASTLE_H
