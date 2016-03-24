// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#ifndef __TC0480SCP_H__
#define __TC0480SCP_H__

class tc0480scp_device : public device_t
{
public:
	tc0480scp_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~tc0480scp_device() {}

	// static configuration
	static void static_set_gfxdecode_tag(device_t &device, const char *tag);
	static void static_set_palette_tag(device_t &device, const char *tag);
	static void set_gfx_region(device_t &device, int gfxregion) { downcast<tc0480scp_device &>(device).m_gfxnum = gfxregion; }
	static void set_tx_region(device_t &device, int txregion) { downcast<tc0480scp_device &>(device).m_txnum = txregion; }
	static void set_col_base(device_t &device, int col) { downcast<tc0480scp_device &>(device).m_col_base = col; }
	static void set_offsets(device_t &device, int x_offset, int y_offset)
	{
		tc0480scp_device &dev = downcast<tc0480scp_device &>(device);
		dev.m_x_offset = x_offset;
		dev.m_y_offset = y_offset;
	}
	static void set_offsets_tx(device_t &device, int x_offset, int y_offset)
	{
		tc0480scp_device &dev = downcast<tc0480scp_device &>(device);
		dev.m_text_xoffs = x_offset;
		dev.m_text_yoffs = y_offset;
	}
	static void set_offsets_flip(device_t &device, int x_offset, int y_offset)
	{
		tc0480scp_device &dev = downcast<tc0480scp_device &>(device);
		dev.m_flip_xoffs = x_offset;
		dev.m_flip_yoffs = y_offset;
	}

	/* When writing a driver, pass zero for the text and flip offsets initially:
	then tweak them once you have the 4 bg layer positions correct. Col_base
	may be needed when tilemaps use a palette area from sprites. */

	DECLARE_READ16_MEMBER( word_r );
	DECLARE_WRITE16_MEMBER( word_w );
	DECLARE_READ16_MEMBER( ctrl_word_r );
	DECLARE_WRITE16_MEMBER( ctrl_word_w );

	/* Functions for use with 68020 (Super-Z system) */
	DECLARE_READ32_MEMBER( long_r );
	DECLARE_WRITE32_MEMBER( long_w );
	DECLARE_READ32_MEMBER( ctrl_long_r );
	DECLARE_WRITE32_MEMBER( ctrl_long_w );

	void tilemap_update();
	void tilemap_draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int layer, int flags, UINT32 priority);

	/* Returns the priority order of the bg tilemaps set in the internal
	register. The order in which the four layers should be drawn is
	returned in the lowest four nibbles  (msn = bottom layer; lsn = top) */
	int get_bg_priority();

	/* Undrfire needs to read this for a sprite/tile priority hack */
	DECLARE_READ8_MEMBER( pri_reg_r );

	void postload();

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	// internal state
	UINT16           m_ctrl[0x18];

	std::vector<UINT16>   m_ram;
	UINT16 *         m_bg_ram[4];
	UINT16 *         m_tx_ram;
	UINT16 *         m_char_ram;
	UINT16 *         m_bgscroll_ram[4];
	UINT16 *         m_rowzoom_ram[4];
	UINT16 *         m_bgcolumn_ram[4];
	int              m_bgscrollx[4];
	int              m_bgscrolly[4];
	int              m_pri_reg;

	/* We keep two tilemaps for each of the 5 actual tilemaps: one at standard width, one double */
	tilemap_t        *m_tilemap[5][2];
	INT32            m_dblwidth;

	int              m_gfxnum;
	int              m_txnum;
	int              m_x_offset, m_y_offset;
	int              m_text_xoffs, m_text_yoffs;
	int              m_flip_xoffs, m_flip_yoffs;

	int              m_col_base;

	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	void common_get_tc0480bg_tile_info( tile_data &tileinfo, int tile_index, UINT16 *ram, int gfxnum );
	void common_get_tc0480tx_tile_info( tile_data &tileinfo, int tile_index, UINT16 *ram, int gfxnum );

	TILE_GET_INFO_MEMBER(get_bg0_tile_info);
	TILE_GET_INFO_MEMBER(get_bg1_tile_info);
	TILE_GET_INFO_MEMBER(get_bg2_tile_info);
	TILE_GET_INFO_MEMBER(get_bg3_tile_info);
	TILE_GET_INFO_MEMBER(get_tx_tile_info);

	void dirty_tilemaps();
	void set_layer_ptrs();
	void bg01_draw( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int layer, int flags, UINT32 priority );
	void bg23_draw( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int layer, int flags, UINT32 priority );
};

extern const device_type TC0480SCP;


#define MCFG_TC0480SCP_GFX_REGION(_region) \
	tc0480scp_device::set_gfx_region(*device, _region);

#define MCFG_TC0480SCP_TX_REGION(_region) \
	tc0480scp_device::set_tx_region(*device, _region);

#define MCFG_TC0480SCP_OFFSETS(_xoffs, _yoffs) \
	tc0480scp_device::set_offsets(*device, _xoffs, _yoffs);

#define MCFG_TC0480SCP_OFFSETS_TX(_xoffs, _yoffs) \
	tc0480scp_device::set_offsets_tx(*device, _xoffs, _yoffs);

#define MCFG_TC0480SCP_OFFSETS_FLIP(_xoffs, _yoffs) \
	tc0480scp_device::set_offsets_flip(*device, _xoffs, _yoffs);

#define MCFG_TC0480SCP_COL_BASE(_col) \
	tc0480scp_device::set_col_base(*device, _col);

#define MCFG_TC0480SCP_GFXDECODE(_gfxtag) \
	tc0480scp_device::static_set_gfxdecode_tag(*device, "^" _gfxtag);

#define MCFG_TC0480SCP_PALETTE(_palette_tag) \
	tc0480scp_device::static_set_palette_tag(*device, "^" _palette_tag);

#endif
