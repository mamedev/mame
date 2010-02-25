/*************************************************************************

    Caveman Ninja (and other DECO 16bit titles)

*************************************************************************/

class cninja_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, cninja_state(machine)); }

	cninja_state(running_machine &machine) { }

	/* memory pointers */
	UINT16 *   ram;
	UINT16 *   pf1_rowscroll;
	UINT16 *   pf2_rowscroll;
	UINT16 *   pf3_rowscroll;
	UINT16 *   pf4_rowscroll;

	/* misc */
	int        scanline, irq_mask;

	/* devices */
	running_device *maincpu;
	running_device *audiocpu;
	running_device *deco16ic;
	running_device *raster_irq_timer;
	running_device *oki2;
};

/*----------- defined in video/cninja.c -----------*/

VIDEO_START( stoneage );

VIDEO_UPDATE( cninja );
VIDEO_UPDATE( cninjabl );
VIDEO_UPDATE( edrandy );
VIDEO_UPDATE( robocop2 );
VIDEO_UPDATE( mutantf );
