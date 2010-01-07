/*************************************************************************

    Top Speed / Full Throttle

*************************************************************************/

typedef struct _topspeed_state topspeed_state;
struct _topspeed_state
{
	/* memory pointers */
	UINT16 *   spritemap;
	UINT16 *   raster_ctrl;
	UINT16 *   spriteram;
	UINT16 *   sharedram;
//  UINT16 *    paletteram;    // currently this uses generic palette handling
	size_t     spriteram_size;
	size_t     sharedram_size;

	/* misc */
	UINT16     cpua_ctrl;
	INT32      ioc220_port;
	INT32      banknum;
	int        adpcm_pos;
	int        adpcm_data;

	/* devices */
	const device_config *maincpu;
	const device_config *audiocpu;
	const device_config *subcpu;
	const device_config *pc080sn_1;
	const device_config *pc080sn_2;
	const device_config *tc0220ioc;
};


/*----------- defined in video/topspeed.c -----------*/

VIDEO_START( topspeed );
VIDEO_UPDATE( topspeed );
