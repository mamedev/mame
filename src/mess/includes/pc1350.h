/*****************************************************************************
 *
 * includes/pc1350.h
 *
 * Pocket Computer 1350
 *
 ****************************************************************************/

#ifndef PC1350_H_
#define PC1350_H_

#include "machine/nvram.h"

#define PC1350_CONTRAST (machine.root_device().ioport("DSW0")->read() & 0x07)


class pc1350_state : public driver_device
{
public:
	pc1350_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 m_outa;
	UINT8 m_outb;
	int m_power;
	UINT8 m_reg[0x1000];
};


/*----------- defined in machine/pc1350.c -----------*/

void pc1350_outa(device_t *device, int data);
void pc1350_outb(device_t *device, int data);
void pc1350_outc(device_t *device, int data);

int pc1350_brk(device_t *device);
int pc1350_ina(device_t *device);
int pc1350_inb(device_t *device);

MACHINE_START( pc1350 );


/*----------- defined in video/pc1350.c -----------*/

DECLARE_READ8_HANDLER(pc1350_lcd_read);
DECLARE_WRITE8_HANDLER(pc1350_lcd_write);
SCREEN_UPDATE_IND16( pc1350 );

int pc1350_keyboard_line_r(running_machine &machine);


#endif /* PC1350_H_ */
