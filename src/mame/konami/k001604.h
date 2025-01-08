// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_KONAMI_K001604_H
#define MAME_KONAMI_K001604_H

#pragma once

#include "tilemap.h"


class k001604_device : public device_t, public device_gfx_interface
{
public:
	k001604_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto irq_callback() { return m_irq.bind(); }

	void draw_tilemap(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, bool front, tilemap_t* tilemap);

	void draw_back_layer(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void draw_front_layer(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void tile_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t tile_r(offs_t offset);
	void char_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t char_r(offs_t offset);
	void reg_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t reg_r(offs_t offset);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
private:
	// internal state
	tilemap_t* m_fg_tilemap;
	tilemap_t* m_bg_tilemap8;
	tilemap_t* m_bg_tilemap16;

	std::unique_ptr<uint32_t[]> m_tile_ram;
	std::unique_ptr<uint8_t[]> m_fg_char_ram;
	std::unique_ptr<uint8_t[]> m_bg_char_ram;
	std::unique_ptr<uint32_t[]> m_reg;

	TILE_GET_INFO_MEMBER(tile_info_fg);
	TILE_GET_INFO_MEMBER(tile_info_bg8);
	TILE_GET_INFO_MEMBER(tile_info_bg16);

	devcb_write_line m_irq;
};

DECLARE_DEVICE_TYPE(K001604, k001604_device)

#endif // MAME_KONAMI_K001604_H
