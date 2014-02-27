#ifndef __PC080SN_H__
#define __PC080SN_H__

struct pc080sn_interface
{
	int                m_gfxnum;

	int                m_x_offset, m_y_offset;
	int                m_y_invert;
	int                m_dblwidth;
};

class pc080sn_device : public device_t,
										public pc080sn_interface
{
public:
	pc080sn_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~pc080sn_device() {}

	// static configuration
	static void static_set_gfxdecode_tag(device_t &device, const char *tag);
	static void static_set_palette_tag(device_t &device, const char *tag);

	DECLARE_READ16_MEMBER( word_r );
	DECLARE_WRITE16_MEMBER( word_w );
	DECLARE_WRITE16_MEMBER( xscroll_word_w );
	DECLARE_WRITE16_MEMBER( yscroll_word_w );
	DECLARE_WRITE16_MEMBER( ctrl_word_w );
	DECLARE_WRITE16_MEMBER( scrollram_w );

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);

	void common_get_pc080sn_bg_tile_info( tile_data &tileinfo, int tile_index, UINT16 *ram, int gfxnum );
	void common_get_pc080sn_fg_tile_info( tile_data &tileinfo, int tile_index, UINT16 *ram, int gfxnum );

	void set_scroll(int tilemap_num, int scrollx, int scrolly);
	void set_trans_pen(int tilemap_num, int pen);
	void tilemap_update();
	void tilemap_draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int layer, int flags, UINT32 priority);
	void tilemap_draw_offset(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int layer, int flags, UINT32 priority, int xoffs, int yoffs);
	void topspeed_custom_draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int layer, int flags, UINT32 priority, UINT16 *color_ctrl_ram);

	/* For Topspeed */
	void tilemap_draw_special(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int layer, int flags, UINT32 priority, UINT16 *ram);

	void restore_scroll();

	protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();

	private:
	// internal state
	UINT16         m_ctrl[8];

	UINT16         *m_ram;
	UINT16         *m_bg_ram[2];
	UINT16         *m_bgscroll_ram[2];

	int            m_bgscrollx[2], m_bgscrolly[2];

	tilemap_t      *m_tilemap[2];
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};

extern const device_type PC080SN;

#define MCFG_PC080SN_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, PC080SN, 0) \
	MCFG_DEVICE_CONFIG(_interface)

#define MCFG_PC080SN_GFXDECODE(_gfxtag) \
	pc080sn_device::static_set_gfxdecode_tag(*device, "^" _gfxtag);

#define MCFG_PC080SN_PALETTE(_palette_tag) \
	pc080sn_device::static_set_palette_tag(*device, "^" _palette_tag);

#endif
