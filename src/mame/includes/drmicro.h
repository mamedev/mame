/*************************************************************************

    Dr. Micro

*************************************************************************/


class drmicro_state : public driver_device
{
public:
	drmicro_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *        videoram;

	/* video-related */
	tilemap_t        *bg1, *bg2;
	int            flipscreen;

	/* misc */
	int            nmi_enable;
	int            pcm_adr;

	/* devices */
	device_t *msm;
};


/*----------- defined in video/drmicro.c -----------*/

PALETTE_INIT( drmicro );
VIDEO_START( drmicro );
VIDEO_UPDATE( drmicro );

WRITE8_HANDLER( drmicro_videoram_w );
