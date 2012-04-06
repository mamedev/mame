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
	DECLARE_READ8_MEMBER(fake_VRLE_r);
	DECLARE_READ8_MEMBER(s2636_portA_r);
	DECLARE_READ8_MEMBER(s2650_data_r);
	DECLARE_WRITE8_MEMBER(port40_w);
	DECLARE_WRITE8_MEMBER(port60_w);
	DECLARE_WRITE8_MEMBER(portc0_w);
	DECLARE_READ8_MEMBER(collision_r);
	DECLARE_WRITE8_MEMBER(malzak_playfield_w);
};


/*----------- defined in video/malzak.c -----------*/


SCREEN_UPDATE_IND16( malzak );
