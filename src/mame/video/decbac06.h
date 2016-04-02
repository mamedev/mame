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
	deco_bac06_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// static configuration
	static void static_set_gfxdecode_tag(device_t &device, const char *tag);
	static void set_gfx_region_wide(device_t &device, int region8x8, int region16x16, int wide);

	std::unique_ptr<UINT16[]> m_pf_data;
	std::unique_ptr<UINT16[]> m_pf_rowscroll;
	std::unique_ptr<UINT16[]> m_pf_colscroll;

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
	UINT16 m_pf_control_0[8];
	UINT16 m_pf_control_1[8];

	void deco_bac06_pf_draw(bitmap_ind16 &bitmap,const rectangle &cliprect,int flags,UINT16 penmask, UINT16 pencondition,UINT16 colprimask, UINT16 colpricondition);
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
	UINT8 get_flip_state(void) { return m_pf_control_0[0]&0x80; };


	void set_colmask(int data) { m_gfxcolmask = data; }
	void set_bppmultmask( int mult, int mask ) { m_bppmult = mult; m_bppmask = mask; } // stadium hero has 3bpp tiles
	UINT8 m_gfxcolmask;
	int m_rambank; // external connection?

	/* 16-bit accessors */

	DECLARE_WRITE16_MEMBER( pf_control_0_w );
	DECLARE_READ16_MEMBER( pf_control_1_r );
	DECLARE_WRITE16_MEMBER( pf_control_1_w );

	DECLARE_WRITE16_MEMBER( pf_data_w );
	DECLARE_READ16_MEMBER( pf_data_r );
	DECLARE_WRITE16_MEMBER( pf_rowscroll_w );
	DECLARE_READ16_MEMBER( pf_rowscroll_r );
	DECLARE_WRITE16_MEMBER( pf_colscroll_w );
	DECLARE_READ16_MEMBER( pf_colscroll_r );

	/* 8-bit accessors */

	/* for dec8.c, pcktgal.c */
	DECLARE_READ8_MEMBER( pf_data_8bit_r );
	DECLARE_WRITE8_MEMBER( pf_data_8bit_w );

	DECLARE_WRITE8_MEMBER( pf_control0_8bit_w );
	DECLARE_READ8_MEMBER( pf_control1_8bit_r );
	DECLARE_WRITE8_MEMBER( pf_control1_8bit_w );

	DECLARE_READ8_MEMBER( pf_rowscroll_8bit_r );
	DECLARE_WRITE8_MEMBER( pf_rowscroll_8bit_w );

	/* for hippodrm (dec0.c) and actfancr / triothep (H6280 based games)*/
	DECLARE_WRITE8_MEMBER( pf_control0_8bit_packed_w );
	DECLARE_WRITE8_MEMBER( pf_control1_8bit_swap_w );
	DECLARE_READ8_MEMBER( pf_data_8bit_swap_r );
	DECLARE_WRITE8_MEMBER( pf_data_8bit_swap_w );
	DECLARE_READ8_MEMBER( pf_rowscroll_8bit_swap_r );
	DECLARE_WRITE8_MEMBER( pf_rowscroll_8bit_swap_w );

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	UINT8 m_gfxregion8x8;
	UINT8 m_gfxregion16x16;
	int m_wide;

	UINT8 m_bppmult;
	UINT8 m_bppmask;

	void custom_tilemap_draw(bitmap_ind16 &bitmap,
							const rectangle &cliprect,
							tilemap_t *tilemap_ptr,
							const UINT16 *rowscroll_ptr,
							const UINT16 *colscroll_ptr,
							const UINT16 *control0,
							const UINT16 *control1,
							int flags,
							UINT16 penmask, UINT16 pencondition,UINT16 colprimask, UINT16 colpricondition);

private:
	TILEMAP_MAPPER_MEMBER(tile_shape0_scan);
	TILEMAP_MAPPER_MEMBER(tile_shape1_scan);
	TILEMAP_MAPPER_MEMBER(tile_shape2_scan);
	TILEMAP_MAPPER_MEMBER(tile_shape0_8x8_scan);
	TILEMAP_MAPPER_MEMBER(tile_shape1_8x8_scan);
	TILEMAP_MAPPER_MEMBER(tile_shape2_8x8_scan);
	TILE_GET_INFO_MEMBER(get_pf8x8_tile_info);
	TILE_GET_INFO_MEMBER(get_pf16x16_tile_info);
	required_device<gfxdecode_device> m_gfxdecode;
};

extern const device_type DECO_BAC06;

#define MCFG_DECO_BAC06_GFXDECODE(_gfxtag) \
	deco_bac06_device::static_set_gfxdecode_tag(*device, "^" _gfxtag);

#define MCFG_DECO_BAC06_GFX_REGION_WIDE(_8x8, _16x16, _wide) \
	deco_bac06_device::set_gfx_region_wide(*device, _8x8, _16x16, _wide);
