#ifndef _GAMEPOCK_H_
#define _GAMEPOCK_H_

struct HD44102CH {
	UINT8	enabled;
	UINT8	start_page;
	UINT8	address;
	UINT8	y_inc;
	UINT8	ram[256];	/* There are actually 50 x 4 x 8 bits. This just makes addressing easier. */
};

class gamepock_state : public driver_device
{
public:
	gamepock_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	virtual void machine_reset();

	UINT8 m_port_a;
	UINT8 m_port_b;
	HD44102CH m_hd44102ch[3];

	void hd44102ch_w( int which, int c_d, UINT8 data );
	void hd44102ch_init( int which );
	void lcd_update();

	DECLARE_WRITE8_MEMBER( port_a_w );
	DECLARE_READ8_MEMBER( port_b_r );
	DECLARE_WRITE8_MEMBER( port_b_w );
	DECLARE_READ8_MEMBER( port_c_r );
};


/*----------- defined in machine/gamepock.c -----------*/

SCREEN_UPDATE_IND16( gamepock );

int gamepock_io_callback( device_t *device, int ioline, int state );

#endif
