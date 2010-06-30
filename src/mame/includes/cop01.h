/*************************************************************************

    Cops 01

*************************************************************************/

class cop01_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, cop01_state(machine)); }

	cop01_state(running_machine &machine) { }

	/* memory pointers */
	UINT8 *        bgvideoram;
	UINT8 *        fgvideoram;
	UINT8 *        spriteram;
	size_t         spriteram_size;

	/* video-related */
	tilemap_t        *bg_tilemap, *fg_tilemap;
	UINT8          vreg[4];

	/* sound-related */
	int            pulse;
	int            timer;	// kludge for ym3526 in mightguy

	/* devices */
	cpu_device *audiocpu;
};



/*----------- defined in video/cop01.c -----------*/


PALETTE_INIT( cop01 );
VIDEO_START( cop01 );
VIDEO_UPDATE( cop01 );

WRITE8_HANDLER( cop01_background_w );
WRITE8_HANDLER( cop01_foreground_w );
WRITE8_HANDLER( cop01_vreg_w );
