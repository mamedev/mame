// license:BSD-3-Clause
// copyright-holders:Manuel Abadia
#ifndef MAME_INCLUDES_BARADUKE_H
#define MAME_INCLUDES_BARADUKE_H

#pragma once

#include "cpu/m6800/m6801.h"
#include "sound/namco.h"
#include "emupal.h"
#include "tilemap.h"

class baraduke_state : public driver_device
{
public:
	baraduke_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_spriteram(*this, "spriteram"),
		m_videoram(*this, "videoram"),
		m_textram(*this, "textram"),
		m_maincpu(*this, "maincpu"),
		m_mcu(*this, "mcu"),
		m_cus30(*this, "namco"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_lamps(*this, "lamp%u", 0U)
	{ }

	void init_baraduke();
	void baraduke(machine_config &config);

private:
	void inputport_select_w(uint8_t data);
	uint8_t inputport_r();
	void baraduke_lamps_w(uint8_t data);
	void baraduke_irq_ack_w(uint8_t data);
	uint8_t soundkludge_r();
	uint8_t baraduke_videoram_r(offs_t offset);
	void baraduke_videoram_w(offs_t offset, uint8_t data);
	uint8_t baraduke_textram_r(offs_t offset);
	void baraduke_textram_w(offs_t offset, uint8_t data);
	void baraduke_scroll0_w(offs_t offset, uint8_t data);
	void baraduke_scroll1_w(offs_t offset, uint8_t data);
	uint8_t baraduke_spriteram_r(offs_t offset);
	void baraduke_spriteram_w(offs_t offset, uint8_t data);
	TILEMAP_MAPPER_MEMBER(tx_tilemap_scan);
	TILE_GET_INFO_MEMBER(tx_get_tile_info);
	TILE_GET_INFO_MEMBER(get_tile_info0);
	TILE_GET_INFO_MEMBER(get_tile_info1);
	virtual void video_start() override;
	void baraduke_palette(palette_device &palette) const;
	uint32_t screen_update_baraduke(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(screen_vblank_baraduke);
	void scroll_w(int layer, int offset, int data);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int sprite_priority);
	void set_scroll(int layer);
	void baraduke_map(address_map &map);
	void mcu_map(address_map &map);

protected:
	virtual void machine_start() override;

	int m_inputport_selected = 0;
	int m_counter = 0;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_textram;
	required_device<cpu_device> m_maincpu;
	required_device<hd63701v0_cpu_device> m_mcu;
	required_device<namco_cus30_device> m_cus30;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	tilemap_t *m_tx_tilemap;
	tilemap_t *m_bg_tilemap[2];
	int m_xscroll[2]{};
	int m_yscroll[2]{};
	int m_copy_sprites = 0;
	output_finder<2> m_lamps;
};

#endif // MAME_INCLUDES_BARADUKE_H
