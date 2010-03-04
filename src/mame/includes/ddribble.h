/***************************************************************************

    Double Dribble

***************************************************************************/

class ddribble_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, ddribble_state(machine)); }

	ddribble_state(running_machine &machine) { }

	/* memory pointers */
	UINT8 *     sharedram;
	UINT8 *     snd_sharedram;
	UINT8 *     spriteram_1;
	UINT8 *     spriteram_2;
	UINT8 *     bg_videoram;
	UINT8 *     fg_videoram;
	UINT8 *     paletteram;

	/* video-related */
	tilemap_t     *fg_tilemap,*bg_tilemap;
	int         vregs[2][5];
	int         charbank[2];

	/* misc */
	int         int_enable_0, int_enable_1;

	/* devices */
	running_device *filter1;
	running_device *filter2;
	running_device *filter3;
};

/*----------- defined in video/ddribble.c -----------*/

WRITE8_HANDLER( ddribble_fg_videoram_w );
WRITE8_HANDLER( ddribble_bg_videoram_w );
WRITE8_HANDLER( K005885_0_w );
WRITE8_HANDLER( K005885_1_w );

PALETTE_INIT( ddribble );
VIDEO_START( ddribble );
VIDEO_UPDATE( ddribble );
