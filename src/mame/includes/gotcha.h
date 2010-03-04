/*************************************************************************

    Gotcha

*************************************************************************/

class gotcha_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, gotcha_state(machine)); }

	gotcha_state(running_machine &machine) { }

	/* memory pointers */
	UINT16 *    fgvideoram;
	UINT16 *    bgvideoram;
	UINT16 *    spriteram;
//  UINT16 *    paletteram; // currently this uses generic palette handling
	size_t      spriteram_size;

	/* video-related */
	tilemap_t     *bg_tilemap, *fg_tilemap;
	int         banksel, gfxbank[4];
	UINT16      scroll[4];

	/* devices */
	running_device *audiocpu;
};


/*----------- defined in video/gotcha.c -----------*/


VIDEO_START( gotcha );
VIDEO_UPDATE( gotcha );

WRITE16_HANDLER( gotcha_fgvideoram_w );
WRITE16_HANDLER( gotcha_bgvideoram_w );
WRITE16_HANDLER( gotcha_gfxbank_select_w );
WRITE16_HANDLER( gotcha_gfxbank_w );
WRITE16_HANDLER( gotcha_scroll_w );
