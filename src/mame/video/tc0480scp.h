#ifndef __TC0480SCP_H__
#define __TC0480SCP_H__

struct tc0480scp_interface
{
	int                m_gfxnum;
	int                m_txnum;

	int                m_pixels;

	int                m_x_offset, m_y_offset;
	int                m_text_xoffs, m_text_yoffs;
	int                m_flip_xoffs, m_flip_yoffs;

	int                m_col_base;
};

class tc0480scp_device : public device_t,
											public tc0480scp_interface
{
public:
	tc0480scp_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~tc0480scp_device() {}

	// static configuration
	static void static_set_gfxdecode_tag(device_t &device, const char *tag);
	static void static_set_palette_tag(device_t &device, const char *tag);

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
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();

private:
	// internal state
	UINT16           m_ctrl[0x18];

	UINT16 *         m_ram;
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
	tilemap_t         *m_tilemap[5][2];
	INT32           m_dblwidth;
	int             m_x_offs;

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

#define MCFG_TC0480SCP_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, TC0480SCP, 0) \
	MCFG_DEVICE_CONFIG(_interface)

#define MCFG_TC0480SCP_GFXDECODE(_gfxtag) \
	tc0480scp_device::static_set_gfxdecode_tag(*device, "^" _gfxtag);

#define MCFG_TC0480SCP_PALETTE(_palette_tag) \
	tc0480scp_device::static_set_palette_tag(*device, "^" _palette_tag);

#endif
