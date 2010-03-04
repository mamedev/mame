/*************************************************************************

    Taito O system

*************************************************************************/

class taitoo_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, taitoo_state(machine)); }

	taitoo_state(running_machine &machine) { }

	/* memory pointers */
//  UINT16 *    paletteram;    // currently this uses generic palette handling

	/* devices */
	running_device *maincpu;
	running_device *tc0080vco;
};

/*----------- defined in video/taito_o.c -----------*/

VIDEO_UPDATE( parentj );
