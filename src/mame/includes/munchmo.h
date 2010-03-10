/*************************************************************************

    Munch Mobile

*************************************************************************/

class munchmo_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, munchmo_state(machine)); }

	munchmo_state(running_machine &machine) { }

	/* memory pointers */
	UINT8 *      vreg;
	UINT8 *      status_vram;
	UINT8 *      sprite_xpos;
	UINT8 *      sprite_attr;
	UINT8 *      sprite_tile;
	UINT8 *      videoram;

	/* video-related */
	bitmap_t     *tmpbitmap;
	int          palette_bank;
	int          flipscreen;

	/* misc */
	int          nmi_enable;
	int          which;
	UINT8        sound_nmi_enable;

	/* devices */
	running_device *maincpu;
	running_device *audiocpu;
};


/*----------- defined in video/munchmo.c -----------*/

WRITE8_HANDLER( mnchmobl_palette_bank_w );
WRITE8_HANDLER( mnchmobl_flipscreen_w );

PALETTE_INIT( mnchmobl );
VIDEO_START( mnchmobl );
VIDEO_UPDATE( mnchmobl );
