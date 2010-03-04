/*************************************************************************

    Goindol

*************************************************************************/

class goindol_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, goindol_state(machine)); }

	goindol_state(running_machine &machine) { }

	/* memory pointers */
	UINT8 *    bg_videoram;
	UINT8 *    fg_videoram;
	UINT8 *    fg_scrollx;
	UINT8 *    fg_scrolly;
	UINT8 *    ram;
	UINT8 *    spriteram;
	UINT8 *    spriteram2;
	size_t     fg_videoram_size;
	size_t     bg_videoram_size;
	size_t     spriteram_size;

	/* video-related */
	tilemap_t     *bg_tilemap, *fg_tilemap;
	UINT16      char_bank;

	/* misc */
	int         prot_toggle;
};



/*----------- defined in video/goindol.c -----------*/

WRITE8_HANDLER( goindol_fg_videoram_w );
WRITE8_HANDLER( goindol_bg_videoram_w );

VIDEO_START( goindol );
VIDEO_UPDATE( goindol );
