class spbactn_state : public driver_device
{
public:
	spbactn_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_bgvideoram(*this, "bgvideoram"),
		m_fgvideoram(*this, "fgvideoram"),
		m_spvideoram(*this, "spvideoram"){ }

	required_shared_ptr<UINT16> m_bgvideoram;
	required_shared_ptr<UINT16> m_fgvideoram;
	required_shared_ptr<UINT16> m_spvideoram;

	tilemap_t    *m_bg_tilemap;
	tilemap_t    *m_fg_tilemap;
	DECLARE_WRITE16_MEMBER(bg_videoram_w);
	DECLARE_WRITE16_MEMBER(fg_videoram_w);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);


	bitmap_ind16 m_tile_bitmap_bg;
	bitmap_ind16 m_tile_bitmap_fg;
	DECLARE_WRITE16_MEMBER(soundcommand_w);
	virtual void video_start();
	UINT32 screen_update_spbactn(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_spbactnp(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	int draw_video(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, bool alt_sprites);

	// temp hack
	DECLARE_READ16_HANDLER(temp_read_handler_r)
	{
		return 0xffff;
	}

};
