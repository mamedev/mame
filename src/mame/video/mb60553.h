// license:BSD-3-Clause
// copyright-holders:David Haywood

/*** MB60553 **********************************************/


class mb60553_zooming_tilemap_device : public device_t
{
public:
	mb60553_zooming_tilemap_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	static void static_set_gfxdecode_tag(device_t &device, std::string tag);
	static void set_gfx_region(device_t &device, int gfxregion);

	tilemap_t* m_tmap;
	std::unique_ptr<UINT16[]> m_vram;
	UINT16 m_regs[8];
	UINT8 m_bank[8];
	UINT16 m_pal_base;
	void reg_written( int num_reg);
	TILE_GET_INFO_MEMBER(get_tile_info);
	void set_pal_base( int m_pal_base);
	void draw( screen_device &screen, bitmap_ind16& bitmap, const rectangle &cliprect, int priority);
	tilemap_t* get_tilemap();

	std::unique_ptr<UINT16[]> m_lineram;

	TILEMAP_MAPPER_MEMBER(twc94_scan);

	DECLARE_WRITE16_MEMBER(regs_w);
	DECLARE_WRITE16_MEMBER(vram_w);
	DECLARE_WRITE16_MEMBER(line_w);

	DECLARE_READ16_MEMBER(regs_r);
	DECLARE_READ16_MEMBER(vram_r);
	DECLARE_READ16_MEMBER(line_r);

	void draw_roz_core(screen_device &screen, bitmap_ind16 &destbitmap, const rectangle &cliprect,
		UINT32 startx, UINT32 starty, int incxx, int incxy, int incyx, int incyy, bool wraparound);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;


private:
	UINT8 m_gfx_region;

	required_device<gfxdecode_device> m_gfxdecode;

};

extern const device_type MB60553;


#define MCFG_MB60553_GFX_REGION(_region) \
	mb60553_zooming_tilemap_device::set_gfx_region(*device, _region);

#define MCFG_MB60553_GFXDECODE(_gfxtag) \
	mb60553_zooming_tilemap_device::static_set_gfxdecode_tag(*device, "^" _gfxtag);
