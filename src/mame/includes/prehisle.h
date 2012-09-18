class prehisle_state : public driver_device
{
public:
	prehisle_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_bg_videoram16(*this, "bg_videoram16"){ }


	required_shared_ptr<UINT16> m_videoram;
	required_shared_ptr<UINT16> m_spriteram;
	required_shared_ptr<UINT16> m_bg_videoram16;
	UINT16 m_invert_controls;

	tilemap_t *m_bg2_tilemap;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	DECLARE_WRITE16_MEMBER(prehisle_sound16_w);
	DECLARE_WRITE16_MEMBER(prehisle_bg_videoram16_w);
	DECLARE_WRITE16_MEMBER(prehisle_fg_videoram16_w);
	DECLARE_READ16_MEMBER(prehisle_control16_r);
	DECLARE_WRITE16_MEMBER(prehisle_control16_w);
	DECLARE_WRITE8_MEMBER(D7759_write_port_0_w);
	DECLARE_WRITE8_MEMBER(D7759_upd_reset_w);
	TILE_GET_INFO_MEMBER(get_bg2_tile_info);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	virtual void video_start();
	UINT32 screen_update_prehisle(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};
