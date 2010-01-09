/*************************************************************************

    Taito Dual Screen Games

*************************************************************************/

typedef struct _warriorb_state warriorb_state;
struct _warriorb_state
{
	/* memory pointers */
	UINT16 *   spriteram;
	size_t     spriteram_size;

	/* misc */
	INT32      banknum;
	int        pandata[4];

	/* devices */
	const device_config *maincpu;
	const device_config *audiocpu;
	const device_config *tc0140syt;
	const device_config *tc0100scn_1;
	const device_config *tc0100scn_2;
	const device_config *lscreen;
	const device_config *rscreen;
	const device_config *_2610_1l;
	const device_config *_2610_1r;
	const device_config *_2610_2l;
	const device_config *_2610_2r;
};


/*----------- defined in video/warriorb.c -----------*/

VIDEO_START( warriorb );
VIDEO_UPDATE( warriorb );
