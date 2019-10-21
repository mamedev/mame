// license:BSD-3-Clause
// copyright-holders:Manuel Abadia
#ifndef MAME_INCLUDES_SKYKID_H
#define MAME_INCLUDES_SKYKID_H

#pragma once

#include "cpu/m6800/m6801.h"
#include "sound/namco.h"
#include "emupal.h"
#include "tilemap.h"

class skykid_state : public driver_device
{
public:
	skykid_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_videoram(*this, "videoram")
		, m_textram(*this, "textram")
		, m_spriteram(*this, "spriteram")
		, m_maincpu(*this, "maincpu")
		, m_mcu(*this, "mcu")
		, m_cus30(*this, "namco")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_leds(*this, "led%u", 0U)
	{ }

	void skykid(machine_config &config);

	void init_skykid();

private:
	DECLARE_WRITE8_MEMBER(inputport_select_w);
	DECLARE_READ8_MEMBER(inputport_r);
	DECLARE_WRITE8_MEMBER(skykid_led_w);
	DECLARE_WRITE8_MEMBER(skykid_subreset_w);
	DECLARE_WRITE8_MEMBER(skykid_bankswitch_w);
	DECLARE_WRITE8_MEMBER(skykid_irq_1_ctrl_w);
	DECLARE_WRITE8_MEMBER(skykid_irq_2_ctrl_w);
	DECLARE_READ8_MEMBER(skykid_videoram_r);
	DECLARE_WRITE8_MEMBER(skykid_videoram_w);
	DECLARE_READ8_MEMBER(skykid_textram_r);
	DECLARE_WRITE8_MEMBER(skykid_textram_w);
	DECLARE_WRITE8_MEMBER(skykid_scroll_x_w);
	DECLARE_WRITE8_MEMBER(skykid_scroll_y_w);
	DECLARE_WRITE8_MEMBER(skykid_flipscreen_priority_w);
	TILEMAP_MAPPER_MEMBER(tx_tilemap_scan);
	TILE_GET_INFO_MEMBER(tx_get_tile_info);
	TILE_GET_INFO_MEMBER(bg_get_tile_info);
	void skykid_palette(palette_device &palette) const;
	uint32_t screen_update_skykid(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(vblank_irq);
	void draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect);
	void mcu_map(address_map &map);
	void skykid_map(address_map &map);

	virtual void machine_start() override;
	virtual void video_start() override;

	uint8_t m_inputport_selected;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_textram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_device<cpu_device> m_maincpu;
	required_device<hd63701_cpu_device> m_mcu;
	required_device<namco_cus30_device> m_cus30;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	output_finder<2> m_leds;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_tx_tilemap;
	uint8_t m_priority;
	uint16_t m_scroll_x;
	uint16_t m_scroll_y;
	uint8_t m_main_irq_mask;
	uint8_t m_mcu_irq_mask;
};

#endif // MAME_INCLUDES_SKYKID_H
