/*************************************************************************

    Markham

*************************************************************************/

class markham_state : public driver_device
{
public:
	markham_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_spriteram(*this, "spriteram"),
		m_videoram(*this, "videoram"),
		m_xscroll(*this, "xscroll"){ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_xscroll;

	/* video-related */
	tilemap_t  *m_bg_tilemap;
	DECLARE_READ8_MEMBER(markham_e004_r);
	DECLARE_WRITE8_MEMBER(markham_videoram_w);
	DECLARE_WRITE8_MEMBER(markham_flipscreen_w);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void video_start();
	virtual void palette_init();
	UINT32 screen_update_markham(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};
