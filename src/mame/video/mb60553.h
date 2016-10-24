// license:BSD-3-Clause
// copyright-holders:David Haywood

/*** MB60553 **********************************************/


class mb60553_zooming_tilemap_device : public device_t
{
public:
	mb60553_zooming_tilemap_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	static void static_set_gfxdecode_tag(device_t &device, const char *tag);
	static void set_gfx_region(device_t &device, int gfxregion);

	tilemap_t* m_tmap;
	std::unique_ptr<uint16_t[]> m_vram;
	uint16_t m_regs[8];
	uint8_t m_bank[8];
	uint16_t m_pal_base;
	void reg_written( int num_reg);
	void get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void set_pal_base( int m_pal_base);
	void draw( screen_device &screen, bitmap_ind16& bitmap, const rectangle &cliprect, int priority);
	tilemap_t* get_tilemap();

	std::unique_ptr<uint16_t[]> m_lineram;

	tilemap_memory_index twc94_scan(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows);

	void regs_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void vram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void line_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	uint16_t regs_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t vram_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t line_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);

	void draw_roz_core(screen_device &screen, bitmap_ind16 &destbitmap, const rectangle &cliprect,
		uint32_t startx, uint32_t starty, int incxx, int incxy, int incyx, int incyy, bool wraparound);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;


private:
	uint8_t m_gfx_region;

	required_device<gfxdecode_device> m_gfxdecode;

};

extern const device_type MB60553;


#define MCFG_MB60553_GFX_REGION(_region) \
	mb60553_zooming_tilemap_device::set_gfx_region(*device, _region);

#define MCFG_MB60553_GFXDECODE(_gfxtag) \
	mb60553_zooming_tilemap_device::static_set_gfxdecode_tag(*device, "^" _gfxtag);
