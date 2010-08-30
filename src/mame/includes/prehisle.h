class prehisle_state : public driver_data_t
{
public:
	static driver_data_t *alloc(running_machine &machine) { return auto_alloc_clear(&machine, prehisle_state(machine)); }

	prehisle_state(running_machine &machine)
		: driver_data_t(machine) { }


	UINT16 *spriteram;
	UINT16 *videoram;
	UINT16 *bg_videoram16;
	UINT16 invert_controls;

	tilemap_t *bg2_tilemap;
	tilemap_t *bg_tilemap;
	tilemap_t *fg_tilemap;
};


/*----------- defined in video/prehisle.c -----------*/

WRITE16_HANDLER( prehisle_bg_videoram16_w );
WRITE16_HANDLER( prehisle_fg_videoram16_w );
WRITE16_HANDLER( prehisle_control16_w );
READ16_HANDLER( prehisle_control16_r );

VIDEO_START( prehisle );
VIDEO_UPDATE( prehisle );
