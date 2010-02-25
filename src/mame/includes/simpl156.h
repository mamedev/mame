/*************************************************************************

    Simple 156 based board

*************************************************************************/

class simpl156_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, simpl156_state(machine)); }

	simpl156_state(running_machine &machine) { }

	/* memory pointers */
	UINT16 *  pf1_rowscroll;
	UINT16 *  pf2_rowscroll;
	UINT32 *  mainram;
	UINT32 *  systemram;

	/* devices */
	running_device *maincpu;
	running_device *deco16ic;
	running_device *eeprom;
	running_device *okimusic;
};



/*----------- defined in video/simpl156.c -----------*/

VIDEO_START( simpl156 );
VIDEO_UPDATE( simpl156 );
