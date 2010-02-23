/*************************************************************************

    Mosaic

*************************************************************************/

typedef struct _mosaic_state mosaic_state;
struct _mosaic_state
{
	/* memory pointers */
	UINT8 *        fgvideoram;
	UINT8 *        bgvideoram;
//  	UINT8 *        paletteram;    // currently this uses generic palette handling

	/* video-related */
	tilemap_t      *bg_tilemap,*fg_tilemap;

	/* misc */
	int            prot_val;
};


/*----------- defined in video/mosaic.c -----------*/

WRITE8_HANDLER( mosaic_fgvideoram_w );
WRITE8_HANDLER( mosaic_bgvideoram_w );

VIDEO_START( mosaic );
VIDEO_UPDATE( mosaic );
