// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
/******************************************************************************

    Data East Side Pocket hardware

******************************************************************************/
#ifndef MAME_INCLUDES_SIDEPKT_H
#define MAME_INCLUDES_SIDEPKT_H

#pragma once

#include "cpu/mcs51/mcs51.h"
#include "machine/gen_latch.h"
#include "emupal.h"
#include "tilemap.h"

class sidepckt_state : public driver_device
{
public:
	sidepckt_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_mcu(*this, "mcu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_spriteram(*this, "spriteram")
	{ }

	void sidepcktb(machine_config &config);
	void sidepckt(machine_config &config);

	void init_sidepckt();

protected:
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	required_device<cpu_device> m_maincpu;
	optional_device<i8751_device> m_mcu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_spriteram;

	tilemap_t *m_bg_tilemap = nullptr;

	uint8_t m_mcu_p1 = 0;
	uint8_t m_mcu_p2 = 0;
	uint8_t m_mcu_p3 = 0;

	uint8_t m_scroll_y = 0;

	uint8_t mcu_r();
	void mcu_w(uint8_t data);

	void mcu_p1_w(uint8_t data);
	uint8_t mcu_p2_r();
	void mcu_p3_w(uint8_t data);

	void videoram_w(offs_t offset, uint8_t data);
	void colorram_w(offs_t offset, uint8_t data);
	uint8_t scroll_y_r();
	void scroll_y_w(uint8_t data);

	TILE_GET_INFO_MEMBER(get_tile_info);

	void sidepckt_palette(palette_device &palette) const;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect);

	void sidepckt_map(address_map &map);
	void sidepcktb_map(address_map &map);
	void sound_map(address_map &map);
};

#endif // MAME_INCLUDES_SIDEPKT_H
