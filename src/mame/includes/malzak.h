/*************************************************************************

    Malzak

*************************************************************************/


class malzak_state : public driver_device
{
public:
	malzak_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

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
