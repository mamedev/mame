/*************************************************************************

    Yun Sung 8 Bit Games

*************************************************************************/

class yunsung8_state : public driver_device
{
public:
	yunsung8_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* video-related */
	tilemap_t     *m_tilemap_0;
	tilemap_t     *m_tilemap_1;
	UINT8       *m_videoram_0;
	UINT8       *m_videoram_1;
	int         m_layers_ctrl;
	int         m_videobank;

	/* misc */
	int         m_adpcm;
	int         m_toggle;

	/* devices */
	device_t *m_audiocpu;

	/* memory */
	UINT8      m_videoram[0x4000];
};


/*----------- defined in video/yunsung8.c -----------*/

WRITE8_HANDLER( yunsung8_videobank_w );
READ8_HANDLER ( yunsung8_videoram_r );
WRITE8_HANDLER( yunsung8_videoram_w );
WRITE8_HANDLER( yunsung8_flipscreen_w );

VIDEO_START( yunsung8 );
SCREEN_UPDATE( yunsung8 );
