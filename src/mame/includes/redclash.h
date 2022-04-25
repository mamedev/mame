// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/*************************************************************************

    Zero Hour / Red Clash

*************************************************************************/
#ifndef MAME_INCLUDES_REDCLASH_H
#define MAME_INCLUDES_REDCLASH_H

#pragma once

#include "video/ladybug.h"
#include "emupal.h"
#include "tilemap.h"


// redclash/zerohour
class redclash_state : public driver_device
{
public:
	redclash_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_videoram(*this, "videoram")
		, m_spriteram(*this, "spriteram")
		, m_maincpu(*this, "maincpu")
		, m_palette(*this, "palette")
		, m_gfxdecode(*this, "gfxdecode")
		, m_stars(*this, "stars")
	{ }

	void redclash(machine_config &config);
	void zerohour(machine_config &config);

	void init_redclash();

	DECLARE_INPUT_CHANGED_MEMBER(left_coin_inserted);
	DECLARE_INPUT_CHANGED_MEMBER(right_coin_inserted);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	DECLARE_WRITE_LINE_MEMBER(screen_vblank);
	void videoram_w(offs_t offset, uint8_t data);
	void gfxbank_w(uint8_t data);
	void flipscreen_w(uint8_t data);
	void irqack_w(uint8_t data);
	void star_reset_w(uint8_t data);
	template <unsigned B> void star_w(uint8_t data);
	void palette(palette_device &palette) const;
	TILE_GET_INFO_MEMBER(get_fg_tile_info);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_bullets(bitmap_ind16 &bitmap, const rectangle &cliprect);

	void redclash_map(address_map &map);
	void zerohour_map(address_map &map);

	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_device<cpu_device> m_maincpu;
	required_device<palette_device> m_palette;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<zerohour_stars_device> m_stars;

	tilemap_t   *m_fg_tilemap = nullptr;
	int         m_gfxbank = 0;   // redclash only
};

#endif // MAME_INCLUDES_REDCLASH_H
