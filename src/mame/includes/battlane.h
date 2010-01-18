/***************************************************************************

    Battle Lane Vol. 5

***************************************************************************/

typedef struct _battlane_state battlane_state;
struct _battlane_state
{
	/* memory pointers */
	UINT8 *     tileram;
	UINT8 *     spriteram;

	/* video-related */
	tilemap_t     *bg_tilemap;
	bitmap_t    *screen_bitmap;
	int         video_ctrl;
	int         cpu_control;	/* CPU interrupt control register */

	/* devices */
	running_device *maincpu;
	running_device *subcpu;
};


/*----------- defined in video/battlane.c -----------*/

WRITE8_HANDLER( battlane_palette_w );
WRITE8_HANDLER( battlane_scrollx_w );
WRITE8_HANDLER( battlane_scrolly_w );
WRITE8_HANDLER( battlane_tileram_w );
WRITE8_HANDLER( battlane_spriteram_w );
WRITE8_HANDLER( battlane_bitmap_w );
WRITE8_HANDLER( battlane_video_ctrl_w );

VIDEO_START( battlane );
VIDEO_UPDATE( battlane );
