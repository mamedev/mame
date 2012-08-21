/*****************************************************************************
 *
 * includes/pc1403.h
 *
 * Pocket Computer 1403
 *
 ****************************************************************************/

#ifndef PC1403_H_
#define PC1403_H_

#include "machine/nvram.h"

#define CONTRAST (machine.root_device().ioport("DSW0")->read() & 0x07)


class pc1403_state : public driver_device
{
public:
	pc1403_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 m_portc;
	UINT8 m_outa;
	int m_power;
	UINT8 m_asic[4];
	int m_DOWN;
	int m_RIGHT;
	UINT8 m_reg[0x100];

	DECLARE_DRIVER_INIT(pc1403);
};


/*----------- defined in machine/pc1403.c -----------*/

int pc1403_reset(device_t *device);
int pc1403_brk(device_t *device);
void pc1403_outa(device_t *device, int data);
//void pc1403_outb(device_t *device, int data);
void pc1403_outc(device_t *device, int data);
int pc1403_ina(device_t *device);
//int pc1403_inb(device_t *device);

MACHINE_START( pc1403 );

READ8_HANDLER(pc1403_asic_read);
WRITE8_HANDLER(pc1403_asic_write);


/*----------- defined in video/pc1403.c -----------*/

VIDEO_START( pc1403 );
SCREEN_UPDATE_IND16( pc1403 );

READ8_HANDLER(pc1403_lcd_read);
WRITE8_HANDLER(pc1403_lcd_write);


#endif /* PC1403_H_ */
