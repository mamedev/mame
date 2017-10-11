// license:BSD-3-Clause
// copyright-holders:Luca Elia, David Haywood
#ifndef MAME_VIDEO_KANEKO_TMAP_H
#define MAME_VIDEO_KANEKO_TMAP_H

#pragma once



class kaneko_view2_tilemap_device : public device_t
{
public:
	kaneko_view2_tilemap_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// static configuration
	static void static_set_gfxdecode_tag(device_t &device, const char *tag);
	static void set_gfx_region(device_t &device, int region);
	static void set_offset(device_t &device, int dx, int dy, int xdim, int ydim);
	static void set_invert_flip(device_t &device, int invert_flip); // for fantasia (bootleg)

	void get_tile_info(tile_data &tileinfo, tilemap_memory_index tile_index, int _N_);
	void kaneko16_vram_w(offs_t offset, uint16_t data, uint16_t mem_mask, int _N_);

	// call to do the rendering etc.
	template<class _BitmapClass>
	void kaneko16_prepare_common(_BitmapClass &bitmap, const rectangle &cliprect);
	template<class _BitmapClass>
	void render_tilemap_chip_common(screen_device &screen, _BitmapClass &bitmap, const rectangle &cliprect, int pri);
	template<class _BitmapClass>
	void render_tilemap_chip_alt_common(screen_device &screen, _BitmapClass &bitmap, const rectangle &cliprect, int pri, int v2pri);

	void kaneko16_prepare(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void kaneko16_prepare(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void render_tilemap_chip(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int pri);
	void render_tilemap_chip(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int pri);
	void render_tilemap_chip_alt(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int pri, int v2pri);
	void render_tilemap_chip_alt(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int pri, int v2pri);


	// access
	DECLARE_READ16_MEMBER( kaneko_tmap_vram_r );
	DECLARE_WRITE16_MEMBER( kaneko_tmap_vram_w );

	DECLARE_READ16_MEMBER( kaneko_tmap_regs_r );
	DECLARE_WRITE16_MEMBER( kaneko_tmap_regs_w );

	DECLARE_WRITE16_MEMBER(kaneko16_vram_0_w);
	DECLARE_WRITE16_MEMBER(kaneko16_vram_1_w);

	DECLARE_READ16_MEMBER(kaneko16_vram_0_r);
	DECLARE_READ16_MEMBER(kaneko16_vram_1_r);

	DECLARE_READ16_MEMBER(kaneko16_scroll_0_r);
	DECLARE_READ16_MEMBER(kaneko16_scroll_1_r);

	DECLARE_WRITE16_MEMBER( kaneko16_scroll_0_w );
	DECLARE_WRITE16_MEMBER( kaneko16_scroll_1_w );

	DECLARE_WRITE16_MEMBER(galsnew_vram_0_tilebank_w);
	DECLARE_WRITE16_MEMBER(galsnew_vram_1_tilebank_w);



protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	TILE_GET_INFO_MEMBER(get_tile_info_0);
	TILE_GET_INFO_MEMBER(get_tile_info_1);
	required_device<gfxdecode_device> m_gfxdecode;

	// set when creating device
	int m_tilebase;
	int m_dx, m_dy, m_xdim, m_ydim;
	int m_invert_flip;

	std::unique_ptr<uint16_t[]> m_vram[2];
	std::unique_ptr<uint16_t[]> m_vscroll[2];
	std::unique_ptr<uint16_t[]> m_regs;
	tilemap_t* m_tmap[2];
	uint16_t m_vram_tile_addition[2]; // galsnew
};


DECLARE_DEVICE_TYPE(KANEKO_TMAP, kaneko_view2_tilemap_device)

#define MCFG_KANEKO_TMAP_GFXDECODE(_gfxtag) \
	kaneko_view2_tilemap_device::static_set_gfxdecode_tag(*device, "^" _gfxtag);

#endif // MAME_VIDEO_KANEKO_TMAP_H
