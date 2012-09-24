/*****************************************************************************
 *
 * includes/pc1401.h
 *
 * Pocket Computer 1401
 *
 ****************************************************************************/

#ifndef PC1401_H_
#define PC1401_H_

#include "machine/nvram.h"

#define CONTRAST (machine.root_device().ioport("DSW0")->read() & 0x07)


class pc1401_state : public driver_device
{
public:
	pc1401_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 m_portc;
	UINT8 m_outa;
	UINT8 m_outb;
	int m_power;
	UINT8 m_reg[0x100];
	DECLARE_DRIVER_INIT(pc1401);
	UINT32 screen_update_pc1401(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(pc1401_power_up);
};


/*----------- defined in machine/pc1401.c -----------*/

int pc1401_reset(device_t *device);
int pc1401_brk(device_t *device);
void pc1401_outa(device_t *device, int data);
void pc1401_outb(device_t *device, int data);
void pc1401_outc(device_t *device, int data);
int pc1401_ina(device_t *device);
int pc1401_inb(device_t *device);

MACHINE_START( pc1401 );


/*----------- defined in video/pc1401.c -----------*/

DECLARE_READ8_HANDLER(pc1401_lcd_read);
DECLARE_WRITE8_HANDLER(pc1401_lcd_write);



#endif /* PC1401_H_ */
