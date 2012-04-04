class prehisle_state : public driver_device
{
public:
	prehisle_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }


	UINT16 *m_spriteram;
	UINT16 *m_videoram;
	UINT16 *m_bg_videoram16;
	UINT16 m_invert_controls;

	tilemap_t *m_bg2_tilemap;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	DECLARE_WRITE16_MEMBER(prehisle_sound16_w);
};


/*----------- defined in video/prehisle.c -----------*/

WRITE16_HANDLER( prehisle_bg_videoram16_w );
WRITE16_HANDLER( prehisle_fg_videoram16_w );
WRITE16_HANDLER( prehisle_control16_w );
READ16_HANDLER( prehisle_control16_r );

VIDEO_START( prehisle );
SCREEN_UPDATE_IND16( prehisle );
