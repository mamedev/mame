class yiear_state : public driver_device
{
public:
	yiear_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *  m_videoram;
	UINT8 *  m_spriteram;
	UINT8 *  m_spriteram2;
	size_t   m_spriteram_size;

	/* video-related */
	tilemap_t  *m_bg_tilemap;

	int      m_yiear_nmi_enable;
};


/*----------- defined in video/yiear.c -----------*/

WRITE8_HANDLER( yiear_videoram_w );
WRITE8_HANDLER( yiear_control_w );

PALETTE_INIT( yiear );
VIDEO_START( yiear );
SCREEN_UPDATE( yiear );
