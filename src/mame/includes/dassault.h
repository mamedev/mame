/*************************************************************************

    Desert Assault

*************************************************************************/

class dassault_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, dassault_state(machine)); }

	dassault_state(running_machine &machine) { }

	/* memory pointers */
	UINT16 *  pf2_rowscroll;
	UINT16 *  pf4_rowscroll;
	UINT16 *  ram;
	UINT16 *  ram2;
	UINT16 *  shared_ram;

	/* devices */
	running_device *maincpu;
	running_device *audiocpu;
	running_device *subcpu;
	running_device *deco16ic;
	running_device *oki2;
};



/*----------- defined in video/dassault.c -----------*/

VIDEO_UPDATE( dassault );
