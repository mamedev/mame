// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#ifndef __TC0280GRD_H__
#define __TC0280GRD_H__

class tc0280grd_device : public device_t
{
public:
	tc0280grd_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~tc0280grd_device() {}

	// static configuration
	static void static_set_gfxdecode_tag(device_t &device, const char *tag);
	static void set_gfx_region(device_t &device, int gfxregion) { downcast<tc0280grd_device &>(device).m_gfxnum = gfxregion; }

	DECLARE_READ16_MEMBER( tc0280grd_word_r );
	DECLARE_WRITE16_MEMBER( tc0280grd_word_w );
	DECLARE_WRITE16_MEMBER( tc0280grd_ctrl_word_w );
	void tc0280grd_tilemap_update(int base_color);
	void tc0280grd_zoom_draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int xoffset, int yoffset, UINT32 priority);

	DECLARE_READ16_MEMBER( tc0430grw_word_r );
	DECLARE_WRITE16_MEMBER( tc0430grw_word_w );
	DECLARE_WRITE16_MEMBER( tc0430grw_ctrl_word_w );
	void tc0430grw_tilemap_update(int base_color);
	void tc0430grw_zoom_draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int xoffset, int yoffset, UINT32 priority);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	// internal state
	std::unique_ptr<UINT16[]>       m_ram;

	tilemap_t      *m_tilemap;

	UINT16         m_ctrl[8];
	int            m_base_color;
	int            m_gfxnum;
	required_device<gfxdecode_device> m_gfxdecode;

	TILE_GET_INFO_MEMBER(tc0280grd_get_tile_info);
	void zoom_draw( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int xoffset, int yoffset, UINT32 priority, int xmultiply );
};

extern const device_type TC0280GRD;

#define TC0430GRW TC0280GRD

#define MCFG_TC0280GRD_GFX_REGION(_region) \
	tc0280grd_device::set_gfx_region(*device, _region);

#define MCFG_TC0430GRW_GFX_REGION(_region) \
	tc0280grd_device::set_gfx_region(*device, _region);

#define MCFG_TC0280GRD_GFXDECODE(_gfxtag) \
	tc0280grd_device::static_set_gfxdecode_tag(*device, "^" _gfxtag);

#define MCFG_TC0430GRW_GFXDECODE(_gfxtag) \
	tc0280grd_device::static_set_gfxdecode_tag(*device, "^" _gfxtag);
#endif
