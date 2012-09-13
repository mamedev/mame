/*****************************************************************************
 *
 * includes/ac1.h
 *
 ****************************************************************************/

#ifndef AC1_H_
#define AC1_H_

#include "machine/z80pio.h"
#include "imagedev/cassette.h"

class ac1_state : public driver_device
{
public:
	ac1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	cassette_image_device *m_cassette;
	DECLARE_DRIVER_INIT(ac1);
	virtual void machine_reset();
	virtual void video_start();
};


/*----------- defined in machine/ac1.c -----------*/



extern const z80pio_interface ac1_z80pio_intf;

/*----------- defined in video/ac1.c -----------*/

extern const gfx_layout ac1_charlayout;


SCREEN_UPDATE_IND16( ac1 );
SCREEN_UPDATE_IND16( ac1_32 );


#endif /* AC1_h_ */
