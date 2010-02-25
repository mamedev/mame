/*************************************************************************

    Boogie Wings

*************************************************************************/

class boogwing_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, boogwing_state(machine)); }

	boogwing_state(running_machine &machine) { }

	/* memory pointers */
	UINT16 *  pf1_rowscroll;
	UINT16 *  pf2_rowscroll;
	UINT16 *  pf3_rowscroll;
	UINT16 *  pf4_rowscroll;

	/* devices */
	running_device *maincpu;
	running_device *audiocpu;
	running_device *deco16ic;
	running_device *oki1;
	running_device *oki2;
};


/*----------- defined in video/boogwing.c -----------*/

VIDEO_UPDATE( boogwing );

