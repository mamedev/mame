/*****************************************************************************
 *
 * includes/kramermc.h
 *
 ****************************************************************************/

#ifndef KRAMERMC_H_
#define KRAMERMC_H_

#include "machine/z80pio.h"

class kramermc_state : public driver_device
{
public:
	kramermc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 m_key_row;
	DECLARE_DRIVER_INIT(kramermc);
	virtual void machine_reset();
	virtual void video_start();
	UINT32 screen_update_kramermc(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);	
	DECLARE_READ8_MEMBER(kramermc_port_a_r);
	DECLARE_READ8_MEMBER(kramermc_port_b_r);
	DECLARE_WRITE8_MEMBER(kramermc_port_a_w);
};


/*----------- defined in machine/kramermc.c -----------*/

extern const z80pio_interface kramermc_z80pio_intf;

/*----------- defined in video/kramermc.c -----------*/

extern const gfx_layout kramermc_charlayout;


#endif /* KRAMERMC_h_ */
