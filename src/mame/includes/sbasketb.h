class sbasketb_state : public driver_device
{
public:
	sbasketb_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *  m_videoram;
	UINT8 *  m_colorram;
	UINT8 *  m_scroll;
	UINT8 *  m_spriteram;
	UINT8 *  m_palettebank;
	UINT8 *  m_spriteram_select;

	/* video-related */
	tilemap_t  *m_bg_tilemap;
};

/*----------- defined in video/sbasketb.c -----------*/

WRITE8_HANDLER( sbasketb_videoram_w );
WRITE8_HANDLER( sbasketb_colorram_w );
WRITE8_HANDLER( sbasketb_flipscreen_w );

PALETTE_INIT( sbasketb );
VIDEO_START( sbasketb );
SCREEN_UPDATE( sbasketb );
