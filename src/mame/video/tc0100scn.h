// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#ifndef __TC0100SCN_H__
#define __TC0100SCN_H__

class tc0100scn_device : public device_t
{
public:
	tc0100scn_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~tc0100scn_device() {}

	// static configuration
	static void static_set_gfxdecode_tag(device_t &device, const char *tag);
	static void static_set_palette_tag(device_t &device, const char *tag);
	static void set_gfx_region(device_t &device, int gfxregion) { downcast<tc0100scn_device &>(device).m_gfxnum = gfxregion; }
	static void set_tx_region(device_t &device, int txregion) { downcast<tc0100scn_device &>(device).m_txnum = txregion; }
	static void set_multiscr_xoffs(device_t &device, int xoffs) { downcast<tc0100scn_device &>(device).m_multiscrn_xoffs = xoffs; }
	static void set_multiscr_hack(device_t &device, int hack) { downcast<tc0100scn_device &>(device).m_multiscrn_hack = hack; }
	static void set_offsets(device_t &device, int x_offset, int y_offset)
	{
		tc0100scn_device &dev = downcast<tc0100scn_device &>(device);
		dev.m_x_offset = x_offset;
		dev.m_y_offset = y_offset;
	}
	static void set_offsets_flip(device_t &device, int x_offset, int y_offset)
	{
		tc0100scn_device &dev = downcast<tc0100scn_device &>(device);
		dev.m_flip_xoffs = x_offset;
		dev.m_flip_yoffs = y_offset;
	}
	static void set_offsets_fliptx(device_t &device, int x_offset, int y_offset)
	{
		tc0100scn_device &dev = downcast<tc0100scn_device &>(device);
		dev.m_flip_text_xoffs = x_offset;
		dev.m_flip_text_yoffs = y_offset;
	}

	#define TC0100SCN_SINGLE_VDU    1024

	/* Function to set separate color banks for the three tilemapped layers.
	To change from the default (0,0,0) use after calling TC0100SCN_vh_start */
	void set_colbanks(int bg0, int bg1, int tx);

	/* Function to set bg tilemask < 0xffff */
	void set_bg_tilemask(int mask);

	/* Function to for Mjnquest to select gfx bank */
	DECLARE_WRITE16_MEMBER(gfxbank_w);

	DECLARE_READ16_MEMBER(word_r);
	DECLARE_WRITE16_MEMBER(word_w);
	DECLARE_READ16_MEMBER(ctrl_word_r);
	DECLARE_WRITE16_MEMBER(ctrl_word_w);

	/* Functions for use with 68020 (Under Fire) */
	DECLARE_READ32_MEMBER(long_r);
	DECLARE_WRITE32_MEMBER(long_w);
	DECLARE_READ32_MEMBER(ctrl_long_r);
	DECLARE_WRITE32_MEMBER(ctrl_long_w);

	void tilemap_update();
	int tilemap_draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int layer, int flags, UINT32 priority);

	/* returns 0 or 1 depending on the lowest priority tilemap set in the internal
	register. Use this function to draw tilemaps in the correct order. */
	int bottomlayer();

	void postload();

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	// internal state
	UINT16       m_ctrl[8];

	std::unique_ptr<UINT16[]>    m_ram;
	UINT16 *     m_bg_ram;
	UINT16 *     m_fg_ram;
	UINT16 *     m_tx_ram;
	UINT16 *     m_char_ram;
	UINT16 *     m_bgscroll_ram;
	UINT16 *     m_fgscroll_ram;
	UINT16 *     m_colscroll_ram;

	int          m_bgscrollx, m_bgscrolly, m_fgscrollx, m_fgscrolly;

	/* We keep two tilemaps for each of the 3 actual tilemaps: one at standard width, one double */
	tilemap_t      *m_tilemap[3][2];

	int          m_bg_tilemask;
	INT32        m_gfxbank;
	INT32        m_bg0_colbank, m_bg1_colbank, m_tx_colbank;
	int          m_dblwidth;

	int          m_gfxnum;
	int          m_txnum;
	int          m_x_offset, m_y_offset;
	int          m_flip_xoffs, m_flip_yoffs;
	int          m_flip_text_xoffs, m_flip_text_yoffs;
	int          m_multiscrn_xoffs;
	int          m_multiscrn_hack;

	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_tx_tile_info);

	void common_get_tile_info(tile_data &tileinfo, int tile_index, UINT16 *ram, int colbank);

	void tilemap_draw_fg(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, tilemap_t* tmap, int flags, UINT32 priority);
	void set_layer_ptrs();
	void dirty_tilemaps();
	void restore_scroll();
};

extern const device_type TC0100SCN;


#define MCFG_TC0100SCN_GFX_REGION(_region) \
	tc0100scn_device::set_gfx_region(*device, _region);

#define MCFG_TC0100SCN_TX_REGION(_region) \
	tc0100scn_device::set_tx_region(*device, _region);

#define MCFG_TC0100SCN_OFFSETS(_xoffs, _yoffs) \
	tc0100scn_device::set_offsets(*device, _xoffs, _yoffs);

#define MCFG_TC0100SCN_OFFSETS_FLIP(_xoffs, _yoffs) \
	tc0100scn_device::set_offsets_flip(*device, _xoffs, _yoffs);

#define MCFG_TC0100SCN_OFFSETS_FLIPTX(_xoffs, _yoffs) \
	tc0100scn_device::set_offsets_fliptx(*device, _xoffs, _yoffs);

#define MCFG_TC0100SCN_MULTISCR_XOFFS(_xoffs) \
	tc0100scn_device::set_multiscr_xoffs(*device, _xoffs);

#define MCFG_TC0100SCN_MULTISCR_HACK(_hack) \
	tc0100scn_device::set_multiscr_hack(*device, _hack);

#define MCFG_TC0100SCN_GFXDECODE(_gfxtag) \
	tc0100scn_device::static_set_gfxdecode_tag(*device, "^" _gfxtag);

#define MCFG_TC0100SCN_PALETTE(_palette_tag) \
	tc0100scn_device::static_set_palette_tag(*device, "^" _palette_tag);

#endif
