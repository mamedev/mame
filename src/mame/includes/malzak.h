/*************************************************************************

    Malzak

*************************************************************************/


class malzak_state : public driver_data_t
{
public:
	static driver_data_t *alloc(running_machine &machine) { return auto_alloc_clear(&machine, malzak_state(machine)); }

	malzak_state(running_machine &machine)
		: driver_data_t(machine) { }

	/* misc */
//  int playfield_x[256];
//  int playfield_y[256];
	int playfield_code[256];
	int malzak_x;
	int malzak_y;
	int collision_counter;

	/* devices */
	running_device *s2636_0;
	running_device *s2636_1;
	running_device *saa5050;
};


/*----------- defined in video/malzak.c -----------*/

WRITE8_HANDLER( malzak_playfield_w );

VIDEO_UPDATE( malzak );
