/*************************************************************************

    Gotcha

*************************************************************************/

class gotcha_state : public driver_device
{
public:
	gotcha_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_fgvideoram(*this, "fgvideoram"),
		m_bgvideoram(*this, "bgvideoram"),
		m_spriteram(*this, "spriteram"){ }

	/* memory pointers */
	required_shared_ptr<UINT16> m_fgvideoram;
	required_shared_ptr<UINT16> m_bgvideoram;
	required_shared_ptr<UINT16> m_spriteram;
//  UINT16 *    m_paletteram; // currently this uses generic palette handling

	/* video-related */
	tilemap_t     *m_bg_tilemap;
	tilemap_t     *m_fg_tilemap;
	int         m_banksel;
	int         m_gfxbank[4];
	UINT16      m_scroll[4];

	/* devices */
	cpu_device *m_audiocpu;
	DECLARE_WRITE16_MEMBER(gotcha_lamps_w);
	DECLARE_WRITE16_MEMBER(gotcha_fgvideoram_w);
	DECLARE_WRITE16_MEMBER(gotcha_bgvideoram_w);
	DECLARE_WRITE16_MEMBER(gotcha_gfxbank_select_w);
	DECLARE_WRITE16_MEMBER(gotcha_gfxbank_w);
	DECLARE_WRITE16_MEMBER(gotcha_scroll_w);
	DECLARE_WRITE16_MEMBER(gotcha_oki_bank_w);
	TILEMAP_MAPPER_MEMBER(gotcha_tilemap_scan);
	TILE_GET_INFO_MEMBER(fg_get_tile_info);
	TILE_GET_INFO_MEMBER(bg_get_tile_info);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	UINT32 screen_update_gotcha(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};


/*----------- defined in video/gotcha.c -----------*/





