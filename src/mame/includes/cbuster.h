/*************************************************************************

    Crude Buster

*************************************************************************/

class cbuster_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, cbuster_state(machine)); }

	cbuster_state(running_machine &machine) { }

	/* memory pointers */
	UINT16 *  pf1_rowscroll;
	UINT16 *  pf2_rowscroll;
	UINT16 *  pf3_rowscroll;
	UINT16 *  pf4_rowscroll;
	UINT16 *  ram;

	/* misc */
	UINT16    prot;
	int       pri;

	/* devices */
	running_device *maincpu;
	running_device *audiocpu;
	running_device *deco16ic;
};



/*----------- defined in video/cbuster.c -----------*/

WRITE16_HANDLER( twocrude_palette_24bit_rg_w );
WRITE16_HANDLER( twocrude_palette_24bit_b_w );

VIDEO_UPDATE( twocrude );
