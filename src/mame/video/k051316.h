// license:BSD-3-Clause
// copyright-holders:Fabio Priuli,Acho A. Tang, R. Belmont
#pragma once
#ifndef __K051316_H__
#define __K051316_H__

typedef device_delegate<void (int *code, int *color, int *flags)> k051316_cb_delegate;
#define K051316_CB_MEMBER(_name)   void _name(int *code, int *color, int *flags)


#define MCFG_K051316_CB(_class, _method) \
	k051316_device::set_k051316_callback(*device, k051316_cb_delegate(&_class::_method, #_class "::" #_method, downcast<_class *>(owner)));

#define MCFG_K051316_OFFSETS(_xoffs, _yoffs) \
	k051316_device::set_offsets(*device, _xoffs, _yoffs);

#define MCFG_K051316_BPP(_bpp) \
	k051316_device::set_bpp(*device, _bpp);

#define MCFG_K051316_LAYER_MASK(_mask) \
	k051316_device::set_layermask(*device, _mask);

#define MCFG_K051316_WRAP(_wrap) \
	k051316_device::set_wrap(*device, _wrap);


class k051316_device : public device_t,
						public device_gfx_interface
{
public:
	k051316_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~k051316_device() {}

	static const gfx_layout charlayout4;
	static const gfx_layout charlayout7;
	static const gfx_layout charlayout8;
	DECLARE_GFXDECODE_MEMBER(gfxinfo);
	DECLARE_GFXDECODE_MEMBER(gfxinfo7);
	DECLARE_GFXDECODE_MEMBER(gfxinfo8);
	DECLARE_GFXDECODE_MEMBER(gfxinfo4_ram);

	// static configuration
	static void set_k051316_callback(device_t &device, k051316_cb_delegate callback) { downcast<k051316_device &>(device).m_k051316_cb = callback; }
	static void set_wrap(device_t &device, int wrap) { downcast<k051316_device &>(device).m_wrap = wrap; }
	static void set_bpp(device_t &device, int bpp);
	static void set_layermask(device_t &device, int mask) { downcast<k051316_device &>(device).m_layermask = mask; }
	static void set_offsets(device_t &device, int x_offset, int y_offset)
	{
		k051316_device &dev = downcast<k051316_device &>(device);
		dev.m_dx = x_offset;
		dev.m_dy = y_offset;
	}

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

	void mark_gfx_dirty(offs_t byteoffset) { gfx(0)->mark_dirty(byteoffset * m_pixels_per_byte / (16 * 16)); }
	void mark_tmap_dirty() { m_tmap->mark_all_dirty(); }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	// internal state
	std::vector<UINT8> m_ram;
	UINT8 m_ctrlram[16];
	tilemap_t *m_tmap;

	optional_region_ptr<UINT8> m_zoom_rom;

	int m_dx, m_dy;
	int m_wrap;
	int m_pixels_per_byte;
	int m_layermask;
	k051316_cb_delegate m_k051316_cb;

	TILE_GET_INFO_MEMBER(get_tile_info);
};

extern const device_type K051316;

#endif
