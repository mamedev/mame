#pragma once
#ifndef __K051316_H__
#define __K051316_H__

typedef void (*k051316_callback)(running_machine &machine, int *code, int *color, int *flags);

struct k051316_interface
{
	const char         *m_gfx_memory_region_tag;
	int                m_gfx_num;
	int                m_bpp, m_pen_is_mask, m_transparent_pen;
	int                m_wrap, m_xoffs, m_yoffs;
	k051316_callback   m_callback;
};

class k051316_device : public device_t,
										public k051316_interface
{
public:
	k051316_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~k051316_device() {}

	// static configuration
	static void static_set_gfxdecode_tag(device_t &device, const char *tag);
	static void static_set_palette_tag(device_t &device, const char *tag);

	/*
	The callback is passed:
	- code (range 00-FF, contents of the first tilemap RAM byte)
	- color (range 00-FF, contents of the first tilemap RAM byte). Note that bit 6
	  seems to be hardcoded as flip X.
	The callback must put:
	- in code the resulting tile number
	- in color the resulting color index
	- if necessary, put flags for the TileMap code in the tile_info
	  structure (e.g. TILE_FLIPX)
	*/

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );
	DECLARE_READ8_MEMBER( rom_r );
	DECLARE_WRITE8_MEMBER( ctrl_w );
	void zoom_draw(screen_device &screen, bitmap_ind16 &bitmap,const rectangle &cliprect,int flags,UINT32 priority);
	void wraparound_enable(int status);

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();
private:
	// internal state
	UINT8    *m_ram;
	tilemap_t  *m_tmap;
	UINT8    m_ctrlram[16];
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	TILE_GET_INFO_MEMBER(get_tile_info0);
	void get_tile_info( tile_data &tileinfo, int tile_index );
};

extern const device_type K051316;

#define MCFG_K051316_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, K051316, 0) \
	MCFG_DEVICE_CONFIG(_interface)

#define MCFG_K051316_GFXDECODE(_gfxtag) \
	k051316_device::static_set_gfxdecode_tag(*device, "^" _gfxtag);

#define MCFG_K051316_PALETTE(_palette_tag) \
	k051316_device::static_set_palette_tag(*device, "^" _palette_tag);
#endif
