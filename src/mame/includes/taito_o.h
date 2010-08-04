/*************************************************************************

    Taito O system

*************************************************************************/

class taitoo_state : public driver_data_t
{
public:
	static driver_data_t *alloc(running_machine &machine) { return auto_alloc_clear(&machine, taitoo_state(machine)); }

	taitoo_state(running_machine &machine)
		: driver_data_t(machine) { }

	/* memory pointers */
//  UINT16 *    paletteram;    // currently this uses generic palette handling

	/* devices */
	running_device *maincpu;
	running_device *tc0080vco;
};

/*----------- defined in video/taito_o.c -----------*/

VIDEO_UPDATE( parentj );
