// license:BSD-3-Clause
// copyright-holders:David Haywood


class vs920a_text_tilemap_device : public device_t
{
public:
	vs920a_text_tilemap_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	static void static_set_gfxdecode_tag(device_t &device, const char *tag);
	static void set_gfx_region(device_t &device, int gfxregion);

	tilemap_t* m_tmap;
	std::unique_ptr<uint16_t[]> m_vram;
	uint16_t m_pal_base;

	void get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	tilemap_t* get_tilemap();
	void set_pal_base(int m_pal_base);
	void draw(screen_device &screen, bitmap_ind16& bitmap, const rectangle &cliprect, int priority);

	void vram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t vram_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);


protected:
	virtual void device_start() override;
	virtual void device_reset() override;


private:
	uint8_t m_gfx_region;

	required_device<gfxdecode_device> m_gfxdecode;
};

extern const device_type VS920A;


#define MCFG_VS920A_GFX_REGION(_region) \
	vs920a_text_tilemap_device::set_gfx_region(*device, _region);

#define MCFG_VS920A_GFXDECODE(_gfxtag) \
	vs920a_text_tilemap_device::static_set_gfxdecode_tag(*device, "^" _gfxtag);
