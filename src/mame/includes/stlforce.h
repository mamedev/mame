class stlforce_state : public driver_device
{
public:
	stlforce_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	tilemap_t *m_bg_tilemap;
	tilemap_t *m_mlow_tilemap;
	tilemap_t *m_mhigh_tilemap;
	tilemap_t *m_tx_tilemap;

	UINT16 *m_bg_videoram;
	UINT16 *m_mlow_videoram;
	UINT16 *m_mhigh_videoram;
	UINT16 *m_tx_videoram;
	UINT16 *m_bg_scrollram;
	UINT16 *m_mlow_scrollram;
	UINT16 *m_mhigh_scrollram;
	UINT16 *m_vidattrram;

	UINT16 *m_spriteram;

	int m_sprxoffs;
	DECLARE_WRITE16_MEMBER(stlforce_bg_videoram_w);
	DECLARE_WRITE16_MEMBER(stlforce_mlow_videoram_w);
	DECLARE_WRITE16_MEMBER(stlforce_mhigh_videoram_w);
	DECLARE_WRITE16_MEMBER(stlforce_tx_videoram_w);
};


/*----------- defined in video/stlforce.c -----------*/

VIDEO_START( stlforce );
SCREEN_UPDATE_IND16( stlforce );
