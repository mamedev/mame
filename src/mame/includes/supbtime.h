/*************************************************************************

    Super Burger Time & China Town

*************************************************************************/

typedef struct _supbtime_state supbtime_state;
struct _supbtime_state
{
	/* memory pointers */
	UINT16 *  pf1_rowscroll;
	UINT16 *  pf2_rowscroll;
	UINT16 *  spriteram;
//  UINT16 *  paletteram;    // currently this uses generic palette handling (in decodev.c)
	size_t    spriteram_size;

	/* video-related */

	/* devices */
	running_device *maincpu;
	running_device *audiocpu;
	running_device *deco16ic;
};



/*----------- defined in video/supbtime.c -----------*/

VIDEO_UPDATE( supbtime );
