// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#ifndef MAME_VIDEO_TC0280GRD_H
#define MAME_VIDEO_TC0280GRD_H

#pragma once

class tc0280grd_device : public device_t
{
public:
	tc0280grd_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// configuration
	template <typename T> void set_gfxdecode_tag(T &&tag) { m_gfxdecode.set_tag(std::forward<T>(tag)); }
	void set_gfx_region(int gfxregion) { m_gfxnum = gfxregion; }

	u16 tc0280grd_word_r(offs_t offset);
	void tc0280grd_word_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void tc0280grd_ctrl_word_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void tc0280grd_tilemap_update(int base_color);
	void tc0280grd_zoom_draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int xoffset, int yoffset, u32 priority);

	u16 tc0430grw_word_r(offs_t offset);
	void tc0430grw_word_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void tc0430grw_ctrl_word_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void tc0430grw_tilemap_update(int base_color);
	void tc0430grw_zoom_draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int xoffset, int yoffset, u32 priority);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	// internal state
	std::unique_ptr<u16[]> m_ram;

	tilemap_t   *m_tilemap;

	u16         m_ctrl[8];
	int         m_base_color;
	int         m_gfxnum;
	required_device<gfxdecode_device> m_gfxdecode;

	TILE_GET_INFO_MEMBER(get_tile_info);
	void zoom_draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int xoffset, int yoffset, u32 priority, int xmultiply);
};

DECLARE_DEVICE_TYPE(TC0280GRD, tc0280grd_device)

#define TC0430GRW TC0280GRD

#endif // MAME_VIDEO_TC0280GRD_H
