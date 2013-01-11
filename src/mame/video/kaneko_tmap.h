

class kaneko_view2_tilemap_device : public device_t
{
public:
	kaneko_view2_tilemap_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// used to set values when creating device
	static void set_gfx_region(device_t &device, int region);
	static void set_offset(device_t &device, int dx, int dy, int xdim, int ydim);
	static void set_invert_flip(device_t &device, int invert_flip); // for fantasia (bootleg)

	// set when creating device
	int m_tilebase;
	int m_dx, m_dy, m_xdim, m_ydim;
	int m_invert_flip;

	UINT16* m_vram[2];
	UINT16* m_vscroll[2];
	UINT16* m_regs;
	tilemap_t* m_tmap[2];
	UINT16 m_vram_tile_addition[2]; // galsnew

	void get_tile_info(tile_data &tileinfo, tilemap_memory_index tile_index, int _N_);
	void kaneko16_vram_w(offs_t offset, UINT16 data, UINT16 mem_mask, int _N_);

	// call to do the rendering etc.
	void kaneko16_prepare(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void render_tilemap_chip(bitmap_ind16 &bitmap, const rectangle &cliprect, int pri);
	void render_tilemap_chip_alt(bitmap_ind16 &bitmap, const rectangle &cliprect, int pri, int v2pri);

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
	virtual void device_start();
	virtual void device_reset();

private:
	TILE_GET_INFO_MEMBER(get_tile_info_0);
	TILE_GET_INFO_MEMBER(get_tile_info_1);
};


extern const device_type KANEKO_TMAP;
