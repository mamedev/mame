// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, David Haywood
/* BAC06 */

#define MCFG_BAC06_BOOTLEG_DISABLE_8x8 \
	deco_bac06_device::disable_8x8(*device);

#define MCFG_BAC06_BOOTLEG_DISABLE_16x16 \
	deco_bac06_device::disable_16x16(*device);

#define MCFG_BAC06_BOOTLEG_DISABLE_RC_SCROLL \
	deco_bac06_device::disable_rc_scroll(*device);


class deco_bac06_device : public device_t
{
public:
	deco_bac06_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// static configuration
	static void static_set_gfxdecode_tag(device_t &device, const char *tag);
	static void set_gfx_region_wide(device_t &device, int region8x8, int region16x16, int wide);

	std::unique_ptr<uint16_t[]> m_pf_data;
	std::unique_ptr<uint16_t[]> m_pf_rowscroll;
	std::unique_ptr<uint16_t[]> m_pf_colscroll;

	tilemap_t* m_pf8x8_tilemap[3];
	tilemap_t* m_pf16x16_tilemap[3];
	int    m_tile_region_8;
	int    m_tile_region_16;

	// some bootlegs (eg midresb / midresbj) don't appear to actually support the alt modes, they set them and end up with broken gfx on later levels.
	bool    m_supports_8x8;
	bool    m_supports_16x16;
	bool    m_supports_rc_scroll;

	static void disable_8x8(device_t &device)
	{
		deco_bac06_device &dev = downcast<deco_bac06_device &>(device);
		dev.m_supports_8x8 = false;
	}

	static void disable_16x16(device_t &device)
	{
		deco_bac06_device &dev = downcast<deco_bac06_device &>(device);
		dev.m_supports_16x16 = false;
	}

	static void disable_rc_scroll(device_t &device)
	{
		deco_bac06_device &dev = downcast<deco_bac06_device &>(device);
		dev.m_supports_rc_scroll = false;
	}

	void create_tilemaps(int region8x8,int region16x16);
	uint16_t m_pf_control_0[8];
	uint16_t m_pf_control_1[8];

	void deco_bac06_pf_draw(bitmap_ind16 &bitmap,const rectangle &cliprect,int flags,uint16_t penmask, uint16_t pencondition,uint16_t colprimask, uint16_t colpricondition);
	void deco_bac06_pf_draw_bootleg(bitmap_ind16 &bitmap,const rectangle &cliprect,int flags, int mode, int type);


	/* I wonder if pf_control_0 is really registers, or a selection of pins.

	  For games with multiple chips typically the flip bit only gets set on one of the chips, but
	  is expected to apply to both (and often the sprites as well?)

	  Furthermore we have the m_rambank thing used by Stadium Hero which appears to be used to
	  control the upper address line on some external RAM even if it gets written to the control_0
	  area

	  For now we have this get_flip_state function so that drivers can query the bit and set other
	  flip flags accordingly
	*/
	uint8_t get_flip_state(void) { return m_pf_control_0[0]&0x80; };


	void set_colmask(int data) { m_gfxcolmask = data; }
	void set_bppmultmask( int mult, int mask ) { m_bppmult = mult; m_bppmask = mask; } // stadium hero has 3bpp tiles
	uint8_t m_gfxcolmask;
	int m_rambank; // external connection?

	/* 16-bit accessors */

	void pf_control_0_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t pf_control_1_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void pf_control_1_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	void pf_data_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t pf_data_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void pf_rowscroll_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t pf_rowscroll_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void pf_colscroll_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t pf_colscroll_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);

	/* 8-bit accessors */

	/* for dec8.c, pcktgal.c */
	uint8_t pf_data_8bit_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void pf_data_8bit_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void pf_control0_8bit_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t pf_control1_8bit_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void pf_control1_8bit_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	uint8_t pf_rowscroll_8bit_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void pf_rowscroll_8bit_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	/* for hippodrm (dec0.c) and actfancr / triothep (H6280 based games)*/
	void pf_control0_8bit_packed_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pf_control1_8bit_swap_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t pf_data_8bit_swap_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void pf_data_8bit_swap_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t pf_rowscroll_8bit_swap_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void pf_rowscroll_8bit_swap_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	uint8_t m_gfxregion8x8;
	uint8_t m_gfxregion16x16;
	int m_wide;

	uint8_t m_bppmult;
	uint8_t m_bppmask;

	void custom_tilemap_draw(bitmap_ind16 &bitmap,
							const rectangle &cliprect,
							tilemap_t *tilemap_ptr,
							const uint16_t *rowscroll_ptr,
							const uint16_t *colscroll_ptr,
							const uint16_t *control0,
							const uint16_t *control1,
							int flags,
							uint16_t penmask, uint16_t pencondition,uint16_t colprimask, uint16_t colpricondition);

private:
	tilemap_memory_index tile_shape0_scan(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows);
	tilemap_memory_index tile_shape1_scan(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows);
	tilemap_memory_index tile_shape2_scan(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows);
	tilemap_memory_index tile_shape0_8x8_scan(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows);
	tilemap_memory_index tile_shape1_8x8_scan(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows);
	tilemap_memory_index tile_shape2_8x8_scan(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows);
	void get_pf8x8_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_pf16x16_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	required_device<gfxdecode_device> m_gfxdecode;
};

extern const device_type DECO_BAC06;

#define MCFG_DECO_BAC06_GFXDECODE(_gfxtag) \
	deco_bac06_device::static_set_gfxdecode_tag(*device, "^" _gfxtag);

#define MCFG_DECO_BAC06_GFX_REGION_WIDE(_8x8, _16x16, _wide) \
	deco_bac06_device::set_gfx_region_wide(*device, _8x8, _16x16, _wide);
