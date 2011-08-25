/*************************************************************************

    Malzak

*************************************************************************/


class malzak_state : public driver_device
{
public:
	malzak_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* misc */
//  int playfield_x[256];
//  int playfield_y[256];
	int m_playfield_code[256];
	int m_malzak_x;
	int m_malzak_y;
	int m_collision_counter;

	/* devices */
	device_t *m_s2636_0;
	device_t *m_s2636_1;
	device_t *m_saa5050;
};


/*----------- defined in video/malzak.c -----------*/

WRITE8_HANDLER( malzak_playfield_w );

SCREEN_UPDATE( malzak );
