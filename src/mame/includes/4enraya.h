/*************************************************************************

    4enraya

*************************************************************************/

class _4enraya_state : public driver_device
{
public:
	_4enraya_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *    videoram;
	size_t     videoram_size;

	/* video-related */
	tilemap_t    *bg_tilemap;

	/* sound-related */
	int        soundlatch;
	int        last_snd_ctrl;
};


/*----------- defined in video/4enraya.c -----------*/

WRITE8_HANDLER( fenraya_videoram_w );

VIDEO_START( 4enraya );
VIDEO_UPDATE( 4enraya );
