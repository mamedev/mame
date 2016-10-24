// license:BSD-3-Clause
// copyright-holders:Fabio Priuli,Acho A. Tang, R. Belmont
#pragma once
#ifndef __K037122_H__
#define __K037122_H__

class k037122_device : public device_t,
						public device_video_interface,
						public device_gfx_interface
{
public:
	k037122_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~k037122_device() {}

	// static configuration
	static void static_set_gfx_index(device_t &device, int index) { downcast<k037122_device &>(device).m_gfx_index = index; }

	void tile_draw( screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect );
	uint32_t sram_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void sram_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t char_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void char_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t reg_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void reg_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	// internal state
	tilemap_t     *m_layer[2];

	std::unique_ptr<uint32_t[]>       m_tile_ram;
	std::unique_ptr<uint32_t[]>       m_char_ram;
	std::unique_ptr<uint32_t[]>       m_reg;

	int            m_gfx_index;

	void tile_info_layer0(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void tile_info_layer1(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void update_palette_color( uint32_t palette_base, int color );
};

extern const device_type K037122;

#define MCFG_K037122_ADD(_tag, _screen) \
	MCFG_DEVICE_ADD(_tag, K037122, 0) \
	MCFG_VIDEO_SET_SCREEN(_screen)

#define MCFG_K037122_PALETTE(_palette_tag) \
	MCFG_GFX_PALETTE(_palette_tag)

#endif
