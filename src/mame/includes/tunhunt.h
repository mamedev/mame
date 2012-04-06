class tunhunt_state : public driver_device
{
public:
	tunhunt_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 m_control;
	UINT8 *m_workram;
	UINT8 *m_spriteram;
	UINT8 *m_videoram;
	tilemap_t *m_fg_tilemap;
	bitmap_ind16 m_tmpbitmap;
	DECLARE_WRITE8_MEMBER(tunhunt_control_w);
	DECLARE_READ8_MEMBER(tunhunt_button_r);
	DECLARE_WRITE8_MEMBER(tunhunt_videoram_w);
};


/*----------- defined in video/tunhunt.c -----------*/


PALETTE_INIT( tunhunt );
VIDEO_START( tunhunt );
SCREEN_UPDATE_IND16( tunhunt );
