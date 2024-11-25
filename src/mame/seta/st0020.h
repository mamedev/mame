// license:BSD-3-Clause
// copyright-holders:Luca Elia,David Haywood
#ifndef MAME_SETA_ST0020_H
#define MAME_SETA_ST0020_H

#pragma once

#include "tilemap.h"


class st0020_device : public device_t, public device_gfx_interface
{
public:
	st0020_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration
	void set_is_st0032(int is_st0032) { m_is_st0032 = is_st0032; }
	void set_is_jclub2(int is_jclub2) { m_is_jclub2 = is_jclub2; }

	void update_screen(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, bool update_visible_area);

	uint16_t gfxram_r(offs_t offset, uint16_t mem_mask = ~0);
	void gfxram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint16_t regs_r(offs_t offset);
	void regs_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint16_t sprram_r(offs_t offset);
	void sprram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	// see if we can handle the difference between this and the st0032 in here, or if we need another device
	int m_is_st0032;

	// per-game hack
	int m_is_jclub2;

	// RAM
	std::unique_ptr<uint16_t[]> m_gfxram;
	std::unique_ptr<uint16_t[]> m_spriteram;
	std::unique_ptr<uint16_t[]> m_regs;

	void regs_st0020_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void regs_st0032_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	int m_gfxram_bank = 0;
	void gfxram_bank_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	// blitter
	optional_region_ptr<uint8_t> m_rom_ptr;
	void do_blit_w(uint16_t data);

	// tilemaps
	tilemap_t *m_tmap[4]{};

	template<int Layer> TILE_GET_INFO_MEMBER(get_tile_info);
	TILEMAP_MAPPER_MEMBER(scan_16x16);

	int tmap_offset(int i);
	int tmap_priority(int i);
	int tmap_is_enabled(int i);
	void tmap_st0020_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void tmap_st0032_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	// sprites
	void draw_zooming_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int priority);
};

DECLARE_DEVICE_TYPE(ST0020_SPRITES, st0020_device)


#endif // MAME_SETA_ST0020_H
