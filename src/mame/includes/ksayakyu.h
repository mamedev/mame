/*************************************************************************

    Kusayakyuu

*************************************************************************/

class ksayakyu_state : public driver_device
{
public:
	ksayakyu_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *    videoram;
	UINT8 *    spriteram;
	size_t     spriteram_size;

	/* video-related */
	tilemap_t    *tilemap, *textmap;
	int        video_ctrl, flipscreen;

	/* misc */
	int        sound_status;
};


/*----------- defined in video/ksayakyu.c -----------*/

WRITE8_HANDLER( ksayakyu_videoram_w );
WRITE8_HANDLER( ksayakyu_videoctrl_w );
PALETTE_INIT( ksayakyu );
VIDEO_START( ksayakyu );
VIDEO_UPDATE( ksayakyu );
