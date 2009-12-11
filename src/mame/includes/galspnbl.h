/*************************************************************************

    Hot Pinball
    Gals Pinball

*************************************************************************/

typedef struct _galspnbl_state galspnbl_state;
struct _galspnbl_state
{
	/* memory pointers */
	UINT16 *    videoram;
	UINT16 *    bgvideoram;
	UINT16 *    colorram;
	UINT16 *    scroll;
	UINT16 *    spriteram;
//  UINT16 *    paletteram; // currently this uses generic palette handling
	size_t      spriteram_size;

	/* devices */
	const device_config *audiocpu;
};


/*----------- defined in video/galspnbl.c -----------*/


PALETTE_INIT( galspnbl );
VIDEO_UPDATE( galspnbl );
