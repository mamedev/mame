/*************************************************************************

    Super Burger Time & China Town

*************************************************************************/

class supbtime_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, supbtime_state(machine)); }

	supbtime_state(running_machine &machine) { }

	/* memory pointers */
	UINT16 *  pf1_rowscroll;
	UINT16 *  pf2_rowscroll;
	UINT16 *  spriteram;
//  UINT16 *  paletteram;    // currently this uses generic palette handling (in deco16ic.c)
	size_t    spriteram_size;

	/* video-related */

	/* devices */
	running_device *maincpu;
	running_device *audiocpu;
	running_device *deco16ic;
};



/*----------- defined in video/supbtime.c -----------*/

VIDEO_UPDATE( supbtime );
