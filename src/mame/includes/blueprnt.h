/***************************************************************************

    Blue Print

***************************************************************************/

class blueprnt_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, blueprnt_state(machine)); }

	blueprnt_state(running_machine &machine) { }

	/* memory pointers */
	UINT8 * videoram;
	UINT8 * colorram;
	UINT8 * spriteram;
	UINT8 * scrollram;
	size_t  spriteram_size;

	/* video-related */
	tilemap_t *bg_tilemap;
	int     gfx_bank;

	/* misc */
	int     dipsw;

	/* devices */
	running_device *audiocpu;
};


/*----------- defined in video/blueprnt.c -----------*/

WRITE8_HANDLER( blueprnt_videoram_w );
WRITE8_HANDLER( blueprnt_colorram_w );
WRITE8_HANDLER( blueprnt_flipscreen_w );

PALETTE_INIT( blueprnt );
VIDEO_START( blueprnt );
VIDEO_UPDATE( blueprnt );
