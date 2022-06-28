// license:BSD-3-Clause
// copyright-holders:David Haywood
/*************************************************************************

    Super Slams

*************************************************************************/
#ifndef MAME_VSYSTEM_SUPRSLAM_H
#define MAME_VSYSTEM_SUPRSLAM_H

#pragma once

#include "vsystem_spr.h"

#include "machine/gen_latch.h"
#include "video/k053936.h"

#include "tilemap.h"


class suprslam_state : public driver_device
{
public:
	suprslam_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_screen_videoram(*this, "screen_videoram"),
		m_bg_videoram(*this, "bg_videoram"),
		m_sp_videoram(*this, "sp_videoram"),
		m_spriteram(*this, "spriteram"),
		m_screen_vregs(*this, "screen_vregs"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_k053936(*this, "k053936"),
		m_spr(*this, "vsystem_spr"),
		m_palette(*this, "palette"),
		m_gfxdecode(*this, "gfxdecode"),
		m_soundlatch(*this, "soundlatch")
	{ }

	void suprslam(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	/* memory pointers */
	required_shared_ptr<uint16_t> m_screen_videoram;
	required_shared_ptr<uint16_t> m_bg_videoram;
	required_shared_ptr<uint16_t> m_sp_videoram;
	required_shared_ptr<uint16_t> m_spriteram;
	required_shared_ptr<uint16_t> m_screen_vregs;

	/* video-related */
	tilemap_t     *m_screen_tilemap = nullptr;
	tilemap_t     *m_bg_tilemap = nullptr;
	uint16_t      m_screen_bank = 0;
	uint16_t      m_bg_bank = 0;
	uint32_t  suprslam_tile_callback( uint32_t code );
	uint8_t       m_spr_ctrl = 0;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<k053936_device> m_k053936;
	required_device<vsystem_spr_device> m_spr;
	required_device<palette_device> m_palette;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<generic_latch_8_device> m_soundlatch;

	void suprslam_sh_bankswitch_w(uint8_t data);
	void suprslam_screen_videoram_w(offs_t offset, uint16_t data);
	void suprslam_bg_videoram_w(offs_t offset, uint16_t data);
	void suprslam_bank_w(uint16_t data);
	void spr_ctrl_w(uint8_t data);
	TILE_GET_INFO_MEMBER(get_suprslam_tile_info);
	TILE_GET_INFO_MEMBER(get_suprslam_bg_tile_info);
	uint32_t screen_update_suprslam(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void sound_io_map(address_map &map);
	void sound_map(address_map &map);
	void suprslam_map(address_map &map);
};

#endif // MAME_VSYSTEM_SUPRSLAM_H
