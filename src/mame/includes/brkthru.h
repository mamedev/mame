/***************************************************************************

    Break Thru

***************************************************************************/

class brkthru_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, brkthru_state(machine)); }

	brkthru_state(running_machine &machine) { }

	/* memory pointers */
	UINT8 * videoram;
	UINT8 * spriteram;
	UINT8 * fg_videoram;
	size_t  videoram_size;
	size_t  spriteram_size;
	size_t  fg_videoram_size;

	/* video-related */
	tilemap_t *fg_tilemap, *bg_tilemap;
	int     bgscroll;
	int     bgbasecolor;
	int     flipscreen;
	//UINT8 *brkthru_nmi_enable; /* needs to be tracked down */

	/* devices */
	running_device *maincpu;
	running_device *audiocpu;
};


/*----------- defined in video/brkthru.c -----------*/

WRITE8_HANDLER( brkthru_1800_w );
WRITE8_HANDLER( brkthru_bgram_w );
WRITE8_HANDLER( brkthru_fgram_w );
VIDEO_START( brkthru );
PALETTE_INIT( brkthru );
VIDEO_UPDATE( brkthru );
