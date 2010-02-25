/*************************************************************************

    Tumble Pop

*************************************************************************/

typedef struct _tumblep_state tumblep_state;
struct _tumblep_state
{
	/* memory pointers */
	UINT16 *  pf1_rowscroll;
	UINT16 *  pf2_rowscroll;
	UINT16 *  spriteram;
//  UINT16 *  paletteram;    // currently this uses generic palette handling (in decodev.c)
	size_t    spriteram_size;

	/* devices */
	running_device *maincpu;
	running_device *audiocpu;
	running_device *deco16ic;
};



/*----------- defined in video/tumblep.c -----------*/

VIDEO_UPDATE( tumblep );
