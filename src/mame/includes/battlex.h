/***************************************************************************

    Battle Cross

***************************************************************************/

class battlex_state : public driver_data_t
{
public:
	static driver_data_t *alloc(running_machine &machine) { return auto_alloc_clear(&machine, battlex_state(machine)); }

	battlex_state(running_machine &machine)
		: driver_data_t(machine) { }

	/* memory pointers */
	UINT8 * videoram;
	UINT8 * spriteram;

	/* video-related */
	tilemap_t *bg_tilemap;
	int     scroll_lsb, scroll_msb;
};


/*----------- defined in video/battlex.c -----------*/

extern WRITE8_HANDLER( battlex_palette_w );
extern WRITE8_HANDLER( battlex_videoram_w );
extern WRITE8_HANDLER( battlex_scroll_x_lsb_w );
extern WRITE8_HANDLER( battlex_scroll_x_msb_w );
extern WRITE8_HANDLER( battlex_flipscreen_w );

extern PALETTE_INIT( battlex );
extern VIDEO_START( battlex );
extern VIDEO_UPDATE( battlex );
