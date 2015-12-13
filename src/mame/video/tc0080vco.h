// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#ifndef __TC0080VCO_H__
#define __TC0080VCO_H__

class tc0080vco_device : public device_t
{
public:
	tc0080vco_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~tc0080vco_device() {}

	// static configuration
	static void static_set_gfxdecode_tag(device_t &device, const char *tag);
	static void static_set_palette_tag(device_t &device, const char *tag);
	static void set_gfx_region(device_t &device, int gfxnum) { downcast<tc0080vco_device &>(device).m_gfxnum = gfxnum; }
	static void set_tx_region(device_t &device, int txnum) { downcast<tc0080vco_device &>(device).m_txnum = txnum; }
	static void set_offsets(device_t &device, int x_offset, int y_offset)
	{
		tc0080vco_device &dev = downcast<tc0080vco_device &>(device);
		dev.m_bg_xoffs = x_offset;
		dev.m_bg_yoffs = y_offset;
	}
	static void set_bgflip_yoffs(device_t &device, int offs) { downcast<tc0080vco_device &>(device).m_bg_flip_yoffs = offs; }

	DECLARE_READ16_MEMBER( word_r );
	DECLARE_WRITE16_MEMBER( word_w );

	void tilemap_update();
	void tilemap_draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int layer, int flags, UINT32 priority);
	void set_fg0_debug(bool debug) { m_has_fg0 = debug ? 0 : 1; }

	DECLARE_READ16_MEMBER( cram_0_r );
	DECLARE_READ16_MEMBER( cram_1_r );
	DECLARE_READ16_MEMBER( sprram_r );
	DECLARE_READ16_MEMBER( scrram_r );
	DECLARE_WRITE16_MEMBER( scrollram_w );
	READ_LINE_MEMBER( flipscreen_r );
	void postload();

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	// internal state
	UINT16 *       m_ram;
	UINT16 *       m_bg0_ram_0;
	UINT16 *       m_bg0_ram_1;
	UINT16 *       m_bg1_ram_0;
	UINT16 *       m_bg1_ram_1;
	UINT16 *       m_tx_ram_0;
	UINT16 *       m_tx_ram_1;
	UINT16 *       m_char_ram;
	UINT16 *       m_bgscroll_ram;

/* FIXME: This sprite related stuff still needs to be accessed in video/taito_h */
	UINT16 *       m_chain_ram_0;
	UINT16 *       m_chain_ram_1;
	UINT16 *       m_spriteram;
	UINT16 *       m_scroll_ram;

	UINT16         m_bg0_scrollx;
	UINT16         m_bg0_scrolly;
	UINT16         m_bg1_scrollx;
	UINT16         m_bg1_scrolly;

	tilemap_t      *m_tilemap[3];

	INT32          m_flipscreen;

	int            m_gfxnum;
	int            m_txnum;
	int            m_bg_xoffs, m_bg_yoffs;
	int            m_bg_flip_yoffs;
	int            m_has_fg0; // for debug, it can be enabled with set_fg0_debug(true)

	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	TILE_GET_INFO_MEMBER(get_bg0_tile_info);
	TILE_GET_INFO_MEMBER(get_bg1_tile_info);
	TILE_GET_INFO_MEMBER(get_tx_tile_info);
	void bg0_tilemap_draw( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int flags, UINT32 priority );
	void bg1_tilemap_draw( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int flags, UINT32 priority );
};

extern const device_type TC0080VCO;

#define MCFG_TC0080VCO_GFX_REGION(_region) \
	tc0080vco_device::set_gfx_region(*device, _region);

#define MCFG_TC0080VCO_TX_REGION(_region) \
	tc0080vco_device::set_tx_region(*device, _region);

#define MCFG_TC0080VCO_OFFSETS(_xoffs, _yoffs) \
	tc0080vco_device::set_offsets(*device, _xoffs, _yoffs);

#define MCFG_TC0080VCO_BGFLIP_OFFS(_offs) \
	tc0080vco_device::set_bgflip_yoffs(*device, _offs);

#define MCFG_TC0080VCO_GFXDECODE(_gfxtag) \
	tc0080vco_device::static_set_gfxdecode_tag(*device, "^" _gfxtag);

#define MCFG_TC0080VCO_PALETTE(_palette_tag) \
	tc0080vco_device::static_set_palette_tag(*device, "^" _palette_tag);

#endif
