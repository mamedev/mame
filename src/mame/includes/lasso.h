/***************************************************************************

 Lasso and similar hardware

***************************************************************************/

typedef struct _lasso_state lasso_state;
struct _lasso_state
{
	/* memory pointers */
	UINT8 *  videoram;
	UINT8 *  colorram;
	UINT8 *  spriteram;
	UINT8 *  bitmap_ram;	/* 0x2000 bytes for a 256 x 256 x 1 bitmap */
	UINT8 *  back_color;
	UINT8 *  chip_data;
	UINT8 *  track_scroll;
	UINT8 *  last_colors;
	size_t   spriteram_size;

	/* video-related */
	tilemap_t  *bg_tilemap, *track_tilemap;
	UINT8    gfxbank;		/* used by lasso, chameleo, wwjgtin and pinbo */
	UINT8    track_enable;	/* used by wwjgtin */

	/* devices */
	running_device *maincpu;
	running_device *audiocpu;
	running_device *sn_1;
	running_device *sn_2;
};


/*----------- defined in video/lasso.c -----------*/

WRITE8_HANDLER( lasso_videoram_w );
WRITE8_HANDLER( lasso_colorram_w );
WRITE8_HANDLER( lasso_video_control_w );
WRITE8_HANDLER( wwjgtin_video_control_w );
WRITE8_HANDLER( pinbo_video_control_w );

PALETTE_INIT( lasso );
PALETTE_INIT( wwjgtin );

VIDEO_START( lasso );
VIDEO_START( wwjgtin );
VIDEO_START( pinbo );

VIDEO_UPDATE( lasso );
VIDEO_UPDATE( chameleo );
VIDEO_UPDATE( wwjgtin );
VIDEO_UPDATE( pinbo );
