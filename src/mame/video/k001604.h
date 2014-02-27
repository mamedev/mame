#pragma once
#ifndef __K001604_H__
#define __K001604_H__


struct k001604_interface
{
	int            m_gfx_index_1;
	int            m_gfx_index_2;
	int            m_layer_size;        // 0 -> width = 128 tiles, 1 -> width = 256 tiles
	int            m_roz_size;          // 0 -> 8x8, 1 -> 16x16
	int            m_txt_mem_offset;
	int            m_roz_mem_offset;
};


class k001604_device : public device_t,
										public k001604_interface
{
public:
	k001604_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~k001604_device() {}

	// static configuration
	static void static_set_gfxdecode_tag(device_t &device, const char *tag);
	static void static_set_palette_tag(device_t &device, const char *tag);
	
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
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();
private:
	// internal state
	tilemap_t        *m_layer_8x8[2];
	tilemap_t        *m_layer_roz;
	int            m_gfx_index[2];

	UINT32 *       m_tile_ram;
	UINT32 *       m_char_ram;
	UINT32 *       m_reg;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

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


#define MCFG_K001604_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, K001604, 0) \
	MCFG_DEVICE_CONFIG(_interface)

#define MCFG_K001604_MODIFY(_tag, _interface) \
	MCFG_DEVICE_MODIFY(_tag) \
	MCFG_DEVICE_CONFIG(_interface)

#define MCFG_K001604_GFXDECODE(_gfxtag) \
	k001604_device::static_set_gfxdecode_tag(*device, "^" _gfxtag);

#define MCFG_K001604_PALETTE(_palette_tag) \
	k001604_device::static_set_palette_tag(*device, "^" _palette_tag);

#endif
