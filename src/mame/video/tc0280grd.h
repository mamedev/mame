// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#ifndef MAME_VIDEO_TC0280GRD_H
#define MAME_VIDEO_TC0280GRD_H

#pragma once

class tc0280grd_device : public device_t
{
public:
	tc0280grd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration
	void set_gfxdecode_tag(const char *tag) { m_gfxdecode.set_tag(tag); }
	void set_gfx_region(int gfxregion) { m_gfxnum = gfxregion; }

	DECLARE_READ16_MEMBER( tc0280grd_word_r );
	DECLARE_WRITE16_MEMBER( tc0280grd_word_w );
	DECLARE_WRITE16_MEMBER( tc0280grd_ctrl_word_w );
	void tc0280grd_tilemap_update(int base_color);
	void tc0280grd_zoom_draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int xoffset, int yoffset, uint32_t priority);

	DECLARE_READ16_MEMBER( tc0430grw_word_r );
	DECLARE_WRITE16_MEMBER( tc0430grw_word_w );
	DECLARE_WRITE16_MEMBER( tc0430grw_ctrl_word_w );
	void tc0430grw_tilemap_update(int base_color);
	void tc0430grw_zoom_draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int xoffset, int yoffset, uint32_t priority);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	// internal state
	std::unique_ptr<uint16_t[]>       m_ram;

	tilemap_t      *m_tilemap;

	uint16_t         m_ctrl[8];
	int            m_base_color;
	int            m_gfxnum;
	required_device<gfxdecode_device> m_gfxdecode;

	TILE_GET_INFO_MEMBER(tc0280grd_get_tile_info);
	void zoom_draw( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int xoffset, int yoffset, uint32_t priority, int xmultiply );
};

DECLARE_DEVICE_TYPE(TC0280GRD, tc0280grd_device)

#define TC0430GRW TC0280GRD

#define MCFG_TC0280GRD_GFX_REGION(_region) \
	downcast<tc0280grd_device &>(*device).set_gfx_region(_region);

#define MCFG_TC0430GRW_GFX_REGION(_region) \
	downcast<tc0280grd_device &>(*device).set_gfx_region(_region);

#define MCFG_TC0280GRD_GFXDECODE(_gfxtag) \
	downcast<tc0280grd_device &>(*device).set_gfxdecode_tag(_gfxtag);

#define MCFG_TC0430GRW_GFXDECODE(_gfxtag) \
	downcast<tc0280grd_device &>(*device).set_gfxdecode_tag(_gfxtag);

#endif // MAME_VIDEO_TC0280GRD_H
