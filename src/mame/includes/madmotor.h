/*************************************************************************

    Mad Motor

*************************************************************************/

class madmotor_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, madmotor_state(machine)); }

	madmotor_state(running_machine &machine) { }

	/* memory pointers */
	UINT16 *        pf1_rowscroll;
	UINT16 *        pf1_data;
	UINT16 *        pf2_data;
	UINT16 *        pf3_data;
	UINT16 *        pf1_control;
	UINT16 *        pf2_control;
	UINT16 *        pf3_control;
	UINT16 *        spriteram;
//  UINT16 *        paletteram;     // this currently uses generic palette handlers
	size_t          spriteram_size;

	/* video-related */
	tilemap_t       *pf1_tilemap, *pf2_tilemap, *pf3_tilemap, *pf3a_tilemap;
	int             flipscreen;

	/* devices */
	running_device *maincpu;
	running_device *audiocpu;
};


/*----------- defined in video/madmotor.c -----------*/

WRITE16_HANDLER( madmotor_pf1_data_w );
WRITE16_HANDLER( madmotor_pf2_data_w );
WRITE16_HANDLER( madmotor_pf3_data_w );

VIDEO_START( madmotor );
VIDEO_UPDATE( madmotor );
