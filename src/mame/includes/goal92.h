/*************************************************************************

    Goal! '92

*************************************************************************/

typedef struct _goal92_state goal92_state;
struct _goal92_state
{
	/* memory pointers */
	UINT16 *    bg_data;
	UINT16 *    fg_data;
	UINT16 *    tx_data;
	UINT16 *    scrollram;
//  UINT16 *    paletteram; // this currently use generic palette handling
//  UINT16 *    spriteram;  // this currently use generic buffered spriteram

	/* video-related */
	tilemap_t     *bg_layer, *fg_layer, *tx_layer;
	UINT16      fg_bank;

	/* misc */
	int         msm5205next;
	int         adpcm_toggle;

	/* devices */
	const device_config *audiocpu;
};





/*----------- defined in video/goal92.c -----------*/

WRITE16_HANDLER( goal92_background_w );
WRITE16_HANDLER( goal92_foreground_w );
WRITE16_HANDLER( goal92_text_w );
WRITE16_HANDLER( goal92_fg_bank_w );
READ16_HANDLER( goal92_fg_bank_r );

VIDEO_START( goal92 );
VIDEO_UPDATE( goal92 );
VIDEO_EOF( goal92 );
