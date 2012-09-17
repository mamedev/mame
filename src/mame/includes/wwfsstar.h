class wwfsstar_state : public driver_device
{
public:
	wwfsstar_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_spriteram(*this, "spriteram"),
		m_fg0_videoram(*this, "fg0_videoram"),
		m_bg0_videoram(*this, "bg0_videoram"){ }

	int m_vblank;
	int m_scrollx;
	int m_scrolly;
	required_shared_ptr<UINT16> m_spriteram;
	required_shared_ptr<UINT16> m_fg0_videoram;
	required_shared_ptr<UINT16> m_bg0_videoram;
	tilemap_t *m_fg0_tilemap;
	tilemap_t *m_bg0_tilemap;
	DECLARE_WRITE16_MEMBER(wwfsstar_scrollwrite);
	DECLARE_WRITE16_MEMBER(wwfsstar_soundwrite);
	DECLARE_WRITE16_MEMBER(wwfsstar_flipscreen_w);
	DECLARE_WRITE16_MEMBER(wwfsstar_irqack_w);
	DECLARE_WRITE16_MEMBER(wwfsstar_fg0_videoram_w);
	DECLARE_WRITE16_MEMBER(wwfsstar_bg0_videoram_w);
	DECLARE_CUSTOM_INPUT_MEMBER(wwfsstar_vblank_r);
	TILE_GET_INFO_MEMBER(get_fg0_tile_info);
	TILEMAP_MAPPER_MEMBER(bg0_scan);
	TILE_GET_INFO_MEMBER(get_bg0_tile_info);
	virtual void video_start();
	UINT32 screen_update_wwfsstar(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};


/*----------- defined in video/wwfsstar.c -----------*/



