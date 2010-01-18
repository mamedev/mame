/*************************************************************************

    Act Fancer

*************************************************************************/

typedef struct _actfancr_state actfancr_state;
struct _actfancr_state
{
	/* memory pointers */
	UINT8 *        pf1_data;
	UINT8 *        pf2_data;
	UINT8 *        pf1_rowscroll_data;
	UINT8 *        main_ram;
//  UINT8 *        spriteram;   // currently this uses buffered_spriteram
//  UINT8 *        paletteram;  // currently this uses generic palette handling

	/* video-related */
	tilemap_t        *pf1_tilemap, *pf1_alt_tilemap, *pf2_tilemap;
	UINT8          control_1[0x20], control_2[0x20];
	int            flipscreen;

	/* misc */
	int            trio_control_select;

	/* devices */
	running_device *maincpu;
	running_device *audiocpu;
};


/*----------- defined in video/actfancr.c -----------*/

WRITE8_HANDLER( actfancr_pf1_data_w );
READ8_HANDLER( actfancr_pf1_data_r );
WRITE8_HANDLER( actfancr_pf1_control_w );
WRITE8_HANDLER( actfancr_pf2_data_w );
READ8_HANDLER( actfancr_pf2_data_r );
WRITE8_HANDLER( actfancr_pf2_control_w );

VIDEO_START( actfancr );
VIDEO_START( triothep );

VIDEO_UPDATE( actfancr );
VIDEO_UPDATE( triothep );
