/*************************************************************************

    Dr. Micro

*************************************************************************/


class drmicro_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, drmicro_state(machine)); }

	drmicro_state(running_machine &machine) { }

	/* memory pointers */
	UINT8 *        videoram;

	/* video-related */
	tilemap_t        *bg1, *bg2;
	int            flipscreen;

	/* misc */
	int            nmi_enable;
	int            pcm_adr;

	/* devices */
	running_device *msm;
};


/*----------- defined in video/drmicro.c -----------*/

PALETTE_INIT( drmicro );
VIDEO_START( drmicro );
VIDEO_UPDATE( drmicro );

WRITE8_HANDLER( drmicro_videoram_w );
