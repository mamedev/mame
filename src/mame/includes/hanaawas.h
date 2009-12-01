/*************************************************************************

    Hana Awase

*************************************************************************/

typedef struct _hanaawas_state hanaawas_state;
struct _hanaawas_state
{
	/* memory pointers */
	UINT8 *    videoram;
	UINT8 *    colorram;

	/* video-related */
	tilemap    *bg_tilemap;

	/* misc */
	int        mux;
};


/*----------- defined in video/hanaawas.c -----------*/

WRITE8_HANDLER( hanaawas_videoram_w );
WRITE8_HANDLER( hanaawas_colorram_w );
WRITE8_DEVICE_HANDLER( hanaawas_portB_w );

PALETTE_INIT( hanaawas );
VIDEO_START( hanaawas );
VIDEO_UPDATE( hanaawas );
