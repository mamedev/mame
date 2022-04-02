// license:BSD-3-Clause
// copyright-holders:Tomasz Slanina

/*************************************************************************

    Kusayakyuu

*************************************************************************/
#ifndef MAME_INCLUDES_KSAYAKYU_H
#define MAME_INCLUDES_KSAYAKYU_H

#pragma once

#include "machine/gen_latch.h"
#include "emupal.h"
#include "tilemap.h"

class ksayakyu_state : public driver_device
{
public:
	ksayakyu_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch")
	{ }

	void ksayakyu(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;

	/* video-related */
	tilemap_t    *m_tilemap = nullptr;
	tilemap_t    *m_textmap = nullptr;
	int        m_video_ctrl = 0;
	int        m_flipscreen = 0;

	/* misc */
	int        m_sound_status = 0;
	void bank_select_w(uint8_t data);
	void latch_w(uint8_t data);
	uint8_t sound_status_r();
	void tomaincpu_w(uint8_t data);
	uint8_t int_ack_r();
	void ksayakyu_videoram_w(offs_t offset, uint8_t data);
	void ksayakyu_videoctrl_w(uint8_t data);
	void dummy1_w(uint8_t data);
	void dummy2_w(uint8_t data);
	void dummy3_w(uint8_t data);
	TILE_GET_INFO_MEMBER(get_ksayakyu_tile_info);
	TILE_GET_INFO_MEMBER(get_text_tile_info);
	void ksayakyu_palette(palette_device &palette) const;
	uint32_t screen_update_ksayakyu(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;
	void maincpu_map(address_map &map);
	void soundcpu_map(address_map &map);
};

#endif // MAME_INCLUDES_KSAYAKYU_H
