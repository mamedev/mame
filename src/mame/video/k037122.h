#pragma once
#ifndef __K037122_H__
#define __K037122_H__

struct k037122_interface
{
	const char     *m_screen_tag;
	int            m_gfx_index;
};

class k037122_device : public device_t,
										public k037122_interface
{
public:
	k037122_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~k037122_device() {}

	void tile_draw( bitmap_rgb32 &bitmap, const rectangle &cliprect );
	DECLARE_READ32_MEMBER( sram_r );
	DECLARE_WRITE32_MEMBER( sram_w );
	DECLARE_READ32_MEMBER( char_r );
	DECLARE_WRITE32_MEMBER( char_w );
	DECLARE_READ32_MEMBER( reg_r );
	DECLARE_WRITE32_MEMBER( reg_w );

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();

private:
	// internal state
	screen_device *m_screen;
	tilemap_t     *m_layer[2];
	
	UINT32 *       m_tile_ram;
	UINT32 *       m_char_ram;
	UINT32 *       m_reg;

	TILE_GET_INFO_MEMBER(tile_info_layer0);
	TILE_GET_INFO_MEMBER(tile_info_layer1);
	void update_palette_color( UINT32 palette_base, int color );
};

extern const device_type K037122;

#define MCFG_K037122_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, K037122, 0) \
	MCFG_DEVICE_CONFIG(_interface)

#endif
