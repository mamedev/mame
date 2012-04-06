class wwfsstar_state : public driver_device
{
public:
	wwfsstar_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	int m_vblank;
	int m_scrollx;
	int m_scrolly;
	UINT16 *m_spriteram;
	UINT16 *m_fg0_videoram;
	UINT16 *m_bg0_videoram;
	tilemap_t *m_fg0_tilemap;
	tilemap_t *m_bg0_tilemap;
	DECLARE_WRITE16_MEMBER(wwfsstar_scrollwrite);
	DECLARE_WRITE16_MEMBER(wwfsstar_soundwrite);
	DECLARE_WRITE16_MEMBER(wwfsstar_flipscreen_w);
	DECLARE_WRITE16_MEMBER(wwfsstar_irqack_w);
	DECLARE_WRITE16_MEMBER(wwfsstar_fg0_videoram_w);
	DECLARE_WRITE16_MEMBER(wwfsstar_bg0_videoram_w);
};


/*----------- defined in video/wwfsstar.c -----------*/

VIDEO_START( wwfsstar );
SCREEN_UPDATE_IND16( wwfsstar );
