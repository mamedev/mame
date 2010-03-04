/*************************************************************************

    Top Speed / Full Throttle

*************************************************************************/

class topspeed_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, topspeed_state(machine)); }

	topspeed_state(running_machine &machine) { }

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
	running_device *maincpu;
	running_device *audiocpu;
	running_device *subcpu;
	running_device *pc080sn_1;
	running_device *pc080sn_2;
	running_device *tc0220ioc;
};


/*----------- defined in video/topspeed.c -----------*/

VIDEO_START( topspeed );
VIDEO_UPDATE( topspeed );
