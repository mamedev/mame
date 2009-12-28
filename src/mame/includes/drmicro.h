/*************************************************************************

    Dr. Micro

*************************************************************************/


typedef struct _drmicro_state drmicro_state;
struct _drmicro_state
{
	/* memory pointers */
	UINT8 *        videoram;

	/* video-related */
	tilemap_t        *bg1, *bg2;
	int            flipscreen;

	/* misc */
	int            nmi_enable;
	int            pcm_adr;

	/* devices */
	const device_config *msm;
};


/*----------- defined in video/drmicro.c -----------*/

PALETTE_INIT( drmicro );
VIDEO_START( drmicro );
VIDEO_UPDATE( drmicro );

WRITE8_HANDLER( drmicro_videoram_w );
