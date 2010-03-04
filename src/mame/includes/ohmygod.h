/*************************************************************************

    Oh My God!

*************************************************************************/

class ohmygod_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, ohmygod_state(machine)); }

	ohmygod_state(running_machine &machine) { }


	/* memory pointers */
	UINT16 *    videoram;
	UINT16 *    spriteram;
	size_t      spriteram_size;

	/* video-related */
	tilemap_t    *bg_tilemap;
	int spritebank;
	UINT16 scrollx, scrolly;

	/* misc */
	int adpcm_bank_shift;
	int sndbank;
};


/*----------- defined in video/ohmygod.c -----------*/

WRITE16_HANDLER( ohmygod_videoram_w );
WRITE16_HANDLER( ohmygod_spritebank_w );
WRITE16_HANDLER( ohmygod_scrollx_w );
WRITE16_HANDLER( ohmygod_scrolly_w );

VIDEO_START( ohmygod );
VIDEO_UPDATE( ohmygod );
