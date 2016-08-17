// license:BSD-3-Clause
// copyright-holders:David Haywood
#pragma once
#ifndef __K001604_H__
#define __K001604_H__


class k001604_device : public device_t, public device_gfx_interface
{
public:
	k001604_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~k001604_device() {}

	// static configuration
	static void set_layer_size(device_t &device, int size) { downcast<k001604_device &>(device).m_layer_size = size; }
	static void set_roz_size(device_t &device, int size) { downcast<k001604_device &>(device).m_roz_size = size; }
	static void set_txt_mem_offset(device_t &device, int offs) { downcast<k001604_device &>(device).m_txt_mem_offset = offs; }
	static void set_roz_mem_offset(device_t &device, int offs) { downcast<k001604_device &>(device).m_roz_mem_offset = offs; }

	void draw_back_layer( bitmap_rgb32 &bitmap, const rectangle &cliprect );
	void draw_front_layer( screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect );
	DECLARE_WRITE32_MEMBER( tile_w );
	DECLARE_READ32_MEMBER( tile_r );
	DECLARE_WRITE32_MEMBER( char_w );
	DECLARE_READ32_MEMBER( char_r );
	DECLARE_WRITE32_MEMBER( reg_w );
	DECLARE_READ32_MEMBER( reg_r );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
private:
	// internal state
	int            m_layer_size;        // 0 -> width = 128 tiles, 1 -> width = 256 tiles
	int            m_roz_size;          // 0 -> 8x8, 1 -> 16x16
	int            m_txt_mem_offset;
	int            m_roz_mem_offset;

	tilemap_t      *m_layer_8x8[2];
	tilemap_t      *m_layer_roz;

	std::unique_ptr<UINT32[]>       m_tile_ram;
	std::unique_ptr<UINT32[]>       m_char_ram;
	std::unique_ptr<UINT32[]>       m_reg;

	TILEMAP_MAPPER_MEMBER(scan_layer_8x8_0_size0);
	TILEMAP_MAPPER_MEMBER(scan_layer_8x8_0_size1);
	TILEMAP_MAPPER_MEMBER(scan_layer_8x8_1_size0);
	TILEMAP_MAPPER_MEMBER(scan_layer_8x8_1_size1);
	TILEMAP_MAPPER_MEMBER(scan_layer_roz_256);
	TILEMAP_MAPPER_MEMBER(scan_layer_roz_128);
	TILE_GET_INFO_MEMBER(tile_info_layer_8x8);
	TILE_GET_INFO_MEMBER(tile_info_layer_roz);
};

extern const device_type K001604;


#define MCFG_K001604_LAYER_SIZE(_size) \
	k001604_device::set_layer_size(*device, _size);

#define MCFG_K001604_ROZ_SIZE(_size) \
	k001604_device::set_roz_size(*device, _size);

#define MCFG_K001604_TXT_OFFSET(_offs) \
	k001604_device::set_txt_mem_offset(*device, _offs);

#define MCFG_K001604_ROZ_OFFSET(_offs) \
	k001604_device::set_roz_mem_offset(*device, _offs);

#define MCFG_K001604_PALETTE(_palette_tag) \
	MCFG_GFX_PALETTE(_palette_tag)

#endif
