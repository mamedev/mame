/*************************************************************************

    Data East 'Rohga' era hardware

*************************************************************************/

class rohga_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, rohga_state(machine)); }

	rohga_state(running_machine &machine) { }

	/* memory pointers */
	UINT16 *  pf1_rowscroll;
	UINT16 *  pf2_rowscroll;
	UINT16 *  pf3_rowscroll;
	UINT16 *  pf4_rowscroll;
	UINT16 *  spriteram;

	/* devices */
	running_device *maincpu;
	running_device *audiocpu;
	running_device *deco16ic;
	running_device *oki1;
	running_device *oki2;
};



/*----------- defined in video/rohga.c -----------*/

WRITE16_HANDLER( rohga_buffer_spriteram16_w );

VIDEO_START( rohga );

VIDEO_UPDATE( rohga );
VIDEO_UPDATE( schmeisr );
VIDEO_UPDATE( wizdfire );
VIDEO_UPDATE( nitrobal );
