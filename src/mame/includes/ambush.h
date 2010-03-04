/*************************************************************************

    Ambush

*************************************************************************/

class ambush_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, ambush_state(machine)); }

	ambush_state(running_machine &machine) { }

	/* memory pointers */
	UINT8 *    videoram;
	UINT8 *    spriteram;
	UINT8 *    colorram;
	UINT8 *    scrollram;
	UINT8 *    colorbank;

	size_t     videoram_size;
	size_t     spriteram_size;
};


/*----------- defined in video/ambush.c -----------*/

PALETTE_INIT( ambush );
VIDEO_UPDATE( ambush );
