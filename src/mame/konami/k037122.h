// license:BSD-3-Clause
// copyright-holders:Fabio Priuli, Acho A. Tang, R. Belmont
#ifndef MAME_KONAMI_K037122_H
#define MAME_KONAMI_K037122_H
#pragma once

#include "tilemap.h"

class k037122_device : public device_t,
						public device_video_interface,
						public device_gfx_interface,
						public device_palette_interface
{
public:
	static constexpr feature_type imperfect_features() { return feature::GRAPHICS; } // unimplemented tilemap ROZ, scroll registers

	k037122_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void tile_draw( screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect );
	uint32_t sram_r(offs_t offset);
	void sram_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t char_r(offs_t offset);
	void char_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t reg_r(offs_t offset);
	void reg_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_palette_interface impleemntations
	virtual uint32_t palette_entries() const noexcept override { return 8192; }

private:
	// internal state
	tilemap_t* m_tilemap_128 = nullptr;
	tilemap_t* m_tilemap_256 = nullptr;

	uint32_t m_palette_base = 0;
	uint32_t m_tilemap_base = 0;

	std::unique_ptr<uint32_t[]>       m_tile_ram;
	std::unique_ptr<uint32_t[]>       m_char_ram;
	std::unique_ptr<uint32_t[]>       m_reg;

	TILE_GET_INFO_MEMBER(tile_info);
};

DECLARE_DEVICE_TYPE(K037122, k037122_device)

#endif // MAME_KONAMI_K037122_H
