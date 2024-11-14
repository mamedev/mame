// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#ifndef MAME_NAMCO_DIGDUG_H
#define MAME_NAMCO_DIGDUG_H

#pragma once

#include "machine/er2055.h"
#include "tilemap.h"

class digdug_state : public galaga_state
{
public:
	digdug_state(const machine_config &mconfig, device_type type, const char *tag) :
		galaga_state(mconfig, type, tag),
		m_earom(*this, "earom"),
		m_digdug_objram(*this, "digdug_objram"),
		m_digdug_posram(*this, "digdug_posram"),
		m_digdug_flpram(*this, "digdug_flpram")
	{ }

	void dzigzag(machine_config &config);
	void digdug(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<er2055_device> m_earom;
	required_shared_ptr<uint8_t> m_digdug_objram;
	required_shared_ptr<uint8_t> m_digdug_posram;
	required_shared_ptr<uint8_t> m_digdug_flpram;

	uint8_t m_bg_select = 0U;
	uint8_t m_tx_color_mode = 0U;
	uint8_t m_bg_disable = 0U;
	uint8_t m_bg_color_bank = 0U;

	TILEMAP_MAPPER_MEMBER(tilemap_scan);
	TILE_GET_INFO_MEMBER(bg_get_tile_info);
	TILE_GET_INFO_MEMBER(tx_get_tile_info);
	void digdug_palette(palette_device &palette) const;
	uint32_t screen_update_digdug(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void digdug_videoram_w(offs_t offset, uint8_t data);
	void bg_select_w(uint8_t data);
	void tx_color_mode_w(int state);
	void bg_disable_w(int state);

	uint8_t earom_read();
	void earom_write(offs_t offset, uint8_t data);
	void earom_control_w(uint8_t data);

	void digdug_map(address_map &map) ATTR_COLD;
};

#endif // MAME_NAMCO_DIGDUG_H
