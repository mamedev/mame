/*************************************************************************

    Taito Triple Screen Games

*************************************************************************/

typedef struct _ninjaw_state ninjaw_state;
struct _ninjaw_state
{
	/* memory pointers */
	UINT16 *   spriteram;
	size_t     spriteram_size;

	/* misc */
	UINT16     cpua_ctrl;
	INT32      banknum;
	int        pandata[4];

	/* devices */
	const device_config *maincpu;
	const device_config *audiocpu;
	const device_config *subcpu;
	const device_config *tc0140syt;
	const device_config *tc0100scn_1;
	const device_config *tc0100scn_2;
	const device_config *tc0100scn_3;
	const device_config *lscreen;
	const device_config *mscreen;
	const device_config *rscreen;
	const device_config *_2610_1l;
	const device_config *_2610_1r;
	const device_config *_2610_2l;
	const device_config *_2610_2r;
};


/*----------- defined in video/ninjaw.c -----------*/

VIDEO_START( ninjaw );
VIDEO_UPDATE( ninjaw );
