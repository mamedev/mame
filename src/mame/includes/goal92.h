/*************************************************************************

    Goal! '92

*************************************************************************/

class goal92_state : public driver_data_t
{
public:
	static driver_data_t *alloc(running_machine &machine) { return auto_alloc_clear(&machine, goal92_state(machine)); }

	goal92_state(running_machine &machine)
		: driver_data_t(machine) { }

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
	running_device *audiocpu;
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
