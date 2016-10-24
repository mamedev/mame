// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#ifndef __TC0280GRD_H__
#define __TC0280GRD_H__

class tc0280grd_device : public device_t
{
public:
	tc0280grd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~tc0280grd_device() {}

	// static configuration
	static void static_set_gfxdecode_tag(device_t &device, const char *tag);
	static void set_gfx_region(device_t &device, int gfxregion) { downcast<tc0280grd_device &>(device).m_gfxnum = gfxregion; }

	uint16_t tc0280grd_word_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void tc0280grd_word_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void tc0280grd_ctrl_word_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void tc0280grd_tilemap_update(int base_color);
	void tc0280grd_zoom_draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int xoffset, int yoffset, uint32_t priority);

	uint16_t tc0430grw_word_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void tc0430grw_word_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void tc0430grw_ctrl_word_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
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

	void tc0280grd_get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void zoom_draw( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int xoffset, int yoffset, uint32_t priority, int xmultiply );
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
