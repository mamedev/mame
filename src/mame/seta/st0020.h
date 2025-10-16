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
	void set_is_jclub2(int is_jclub2) { m_is_jclub2 = is_jclub2; }

	void update_screen(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, bool update_visible_area);

	virtual uint16_t gfxram_r(offs_t offset, uint16_t mem_mask = ~0);
	virtual void gfxram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	virtual uint16_t regs_r(offs_t offset);
	virtual void regs_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint16_t sprram_r(offs_t offset);
	void sprram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

protected:
	// sprites
	struct sprite_list_t
	{
		int num = 0;
		int sprite = 0;
		int xoffs = 0;
		int yoffs = 0;
	};

	st0020_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	void gfxram_bank_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	// blitter
	void do_blit_w(uint16_t data);

	// tilemaps
	template<int Layer> TILE_GET_INFO_MEMBER(get_tile_info);
	TILEMAP_MAPPER_MEMBER(scan_16x16);

	virtual int tmap_offset(int i);
	virtual int tmap_priority(int i);
	virtual int tmap_is_enabled(int i);
	virtual uint32_t get_tile_color(int i, uint32_t color);
	void tmap_st0020_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	virtual uint32_t get_sprite_color(uint32_t color);
	virtual void draw_zooming_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int priority);
	void draw_single_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int priority, sprite_list_t &list);

	// CRTC
	virtual int get_crtc_top();
	virtual int get_crtc_bottom();

	// per-game hack
	int m_is_jclub2;

	// RAM
	std::unique_ptr<uint16_t[]> m_gfxram;
	std::unique_ptr<uint16_t[]> m_spriteram;
	std::unique_ptr<uint16_t[]> m_regs;
	optional_region_ptr<uint8_t> m_rom_ptr;

	tilemap_t *m_tmap[4];
	uint32_t m_gfxram_bank;
};

class st0032_device : public st0020_device
{
public:
	st0032_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint16_t gfxram_r(offs_t offset, uint16_t mem_mask = ~0) override;
	virtual void gfxram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;

	virtual uint16_t regs_r(offs_t offset) override;
	virtual void regs_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;

protected:
	virtual int tmap_offset(int i) override;
	virtual int tmap_priority(int i) override;
	virtual int tmap_is_enabled(int i) override;
	virtual uint32_t get_tile_color(int i, uint32_t color) override;
	virtual uint32_t get_sprite_color(uint32_t color) override;
	virtual int get_crtc_top() override;
	virtual int get_crtc_bottom() override;

	// sprites
	virtual void draw_zooming_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int priority) override;

private:
	void tmap_st0032_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
};

DECLARE_DEVICE_TYPE(ST0020_SPRITES, st0020_device)
DECLARE_DEVICE_TYPE(ST0032_SPRITES, st0032_device)


#endif // MAME_SETA_ST0020_H
