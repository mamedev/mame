

class funkybee_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, funkybee_state(machine)); }

	funkybee_state(running_machine &machine) { }

	/* memory pointers */
	UINT8 *    videoram;
	UINT8 *    colorram;

	/* video-related */
	tilemap_t    *bg_tilemap;
	int        gfx_bank;
};


/*----------- defined in video/funkybee.c -----------*/

WRITE8_HANDLER( funkybee_videoram_w );
WRITE8_HANDLER( funkybee_colorram_w );
WRITE8_HANDLER( funkybee_gfx_bank_w );
WRITE8_HANDLER( funkybee_scroll_w );
WRITE8_HANDLER( funkybee_flipscreen_w );

PALETTE_INIT( funkybee );
VIDEO_START( funkybee );
VIDEO_UPDATE( funkybee );
