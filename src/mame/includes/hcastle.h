/*************************************************************************

    Haunted Castle

*************************************************************************/

typedef struct _hcastle_state hcastle_state;
struct _hcastle_state
{
	/* memory pointers */
	UINT8 *    pf1_videoram;
	UINT8 *    pf2_videoram;
	UINT8 *    paletteram;
//  UINT8 *    spriteram;
//  UINT8 *    spriteram2;

	/* video-related */
	tilemap_t    *fg_tilemap, *bg_tilemap;
	int        pf2_bankbase, pf1_bankbase;
	int        old_pf1, old_pf2;
	int        gfx_bank;

	/* devices */
	const device_config *audiocpu;
	const device_config *k007121_1;
	const device_config *k007121_2;
};


/*----------- defined in video/hcastle.c -----------*/

WRITE8_HANDLER( hcastle_pf1_video_w );
WRITE8_HANDLER( hcastle_pf2_video_w );
READ8_HANDLER( hcastle_gfxbank_r );
WRITE8_HANDLER( hcastle_gfxbank_w );
WRITE8_HANDLER( hcastle_pf1_control_w );
WRITE8_HANDLER( hcastle_pf2_control_w );

PALETTE_INIT( hcastle );
VIDEO_UPDATE( hcastle );
VIDEO_START( hcastle );
