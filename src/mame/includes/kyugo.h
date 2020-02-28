// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
/***************************************************************************

    Kyugo hardware games

***************************************************************************/
#ifndef MAME_INCLUDES_KYUGO_H
#define MAME_INCLUDES_KYUGO_H

#pragma once

#include "emupal.h"
#include "tilemap.h"

class kyugo_state : public driver_device
{
public:
	kyugo_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_fgvideoram(*this, "fgvideoram"),
		m_bgvideoram(*this, "bgvideoram"),
		m_bgattribram(*this, "bgattribram"),
		m_spriteram_1(*this, "spriteram_1"),
		m_spriteram_2(*this, "spriteram_2"),
		m_shared_ram(*this, "shared_ram"),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "sub"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	void kyugo_base(machine_config &config);
	void repulse(machine_config &config);
	void flashgala(machine_config &config);
	void srdmissn(machine_config &config);
	void legend(machine_config &config);
	void gyrodine(machine_config &config);

	void init_srdmissn();

private:
	DECLARE_WRITE_LINE_MEMBER(nmi_mask_w);
	DECLARE_WRITE8_MEMBER(coin_counter_w);
	DECLARE_WRITE8_MEMBER(fgvideoram_w);
	DECLARE_WRITE8_MEMBER(bgvideoram_w);
	DECLARE_WRITE8_MEMBER(bgattribram_w);
	DECLARE_READ8_MEMBER(spriteram_2_r);
	DECLARE_WRITE8_MEMBER(scroll_x_lo_w);
	DECLARE_WRITE8_MEMBER(gfxctrl_w);
	DECLARE_WRITE8_MEMBER(scroll_y_w);
	DECLARE_WRITE_LINE_MEMBER(flipscreen_w);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(vblank_irq);

	void flashgala_sub_map(address_map &map);
	void flashgala_sub_portmap(address_map &map);
	void gyrodine_main_map(address_map &map);
	void gyrodine_sub_map(address_map &map);
	void gyrodine_sub_portmap(address_map &map);
	void kyugo_main_map(address_map &map);
	void kyugo_main_portmap(address_map &map);
	void legend_sub_map(address_map &map);
	void repulse_sub_map(address_map &map);
	void repulse_sub_portmap(address_map &map);
	void srdmissn_sub_map(address_map &map);
	void srdmissn_sub_portmap(address_map &map);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

	/* memory pointers */
	required_shared_ptr<uint8_t> m_fgvideoram;
	required_shared_ptr<uint8_t> m_bgvideoram;
	required_shared_ptr<uint8_t> m_bgattribram;
	required_shared_ptr<uint8_t> m_spriteram_1;
	required_shared_ptr<uint8_t> m_spriteram_2;
	required_shared_ptr<uint8_t> m_shared_ram;

	uint8_t m_nmi_mask;

	/* video-related */
	tilemap_t     *m_bg_tilemap;
	tilemap_t     *m_fg_tilemap;
	uint8_t       m_scroll_x_lo;
	uint8_t       m_scroll_x_hi;
	uint8_t       m_scroll_y;
	int         m_bgpalbank;
	int         m_fgcolor;
	const uint8_t *m_color_codes;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
};

#endif // MAME_INCLUDES_KYUGO_H
