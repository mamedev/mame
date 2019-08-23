// license:BSD-3-Clause
// copyright-holders:Uki
#ifndef MAME_INCLUDES_XXMISSIO_H
#define MAME_INCLUDES_XXMISSIO_H

#pragma once

#include "emupal.h"
#include "tilemap.h"

class xxmissio_state : public driver_device
{
public:
	xxmissio_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "sub"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_bgram(*this, "bgram"),
		m_fgram(*this, "fgram"),
		m_spriteram(*this, "spriteram")
	{ }

	void xxmissio(machine_config &config);

	DECLARE_CUSTOM_INPUT_MEMBER(status_r);

protected:
	virtual void machine_start() override;
	virtual void video_start() override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_bgram;
	required_shared_ptr<uint8_t> m_fgram;
	required_shared_ptr<uint8_t> m_spriteram;

	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	uint8_t m_status;
	uint8_t m_xscroll;
	uint8_t m_yscroll;
	uint8_t m_flipscreen;

	DECLARE_WRITE8_MEMBER(bank_sel_w);
	DECLARE_WRITE8_MEMBER(status_m_w);
	DECLARE_WRITE8_MEMBER(status_s_w);
	DECLARE_WRITE8_MEMBER(flipscreen_w);
	DECLARE_WRITE8_MEMBER(bgram_w);
	DECLARE_READ8_MEMBER(bgram_r);
	DECLARE_WRITE8_MEMBER(scroll_x_w);
	DECLARE_WRITE8_MEMBER(scroll_y_w);

	DECLARE_WRITE_LINE_MEMBER(interrupt_m);
	INTERRUPT_GEN_MEMBER(interrupt_s);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);

	static rgb_t BBGGRRII(uint32_t raw);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, gfx_element *gfx);

	void map1(address_map &map);
	void map2(address_map &map);
};

#endif // MAME_INCLUDES_XXMISSIO_H
