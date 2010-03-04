/*************************************************************************

    Taito Dual Screen Games

*************************************************************************/

class warriorb_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, warriorb_state(machine)); }

	warriorb_state(running_machine &machine) { }

	/* memory pointers */
	UINT16 *   spriteram;
	size_t     spriteram_size;

	/* misc */
	INT32      banknum;
	int        pandata[4];

	/* devices */
	running_device *maincpu;
	running_device *audiocpu;
	running_device *tc0140syt;
	running_device *tc0100scn_1;
	running_device *tc0100scn_2;
	running_device *lscreen;
	running_device *rscreen;
	running_device *_2610_1l;
	running_device *_2610_1r;
	running_device *_2610_2l;
	running_device *_2610_2r;
};


/*----------- defined in video/warriorb.c -----------*/

VIDEO_START( warriorb );
VIDEO_UPDATE( warriorb );
