/*****************************************************************************
 *
 * includes/radio86.h
 *
 ****************************************************************************/

#ifndef radio86_H_
#define radio86_H_

#include "machine/i8255.h"
#include "machine/8257dma.h"
#include "video/i8275.h"

class radio86_state : public driver_device
{
public:
	radio86_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 m_tape_value;
	UINT8 m_mikrosha_font_page;
	int m_keyboard_mask;
	UINT8* m_radio_ram_disk;
	UINT8 m_romdisk_lsb;
	UINT8 m_romdisk_msb;
	UINT8 m_disk_sel;
	DECLARE_READ8_MEMBER(radio_cpu_state_r);
	DECLARE_READ8_MEMBER(radio_io_r);
	DECLARE_WRITE8_MEMBER(radio_io_w);
	DECLARE_WRITE8_MEMBER(radio86_pagesel);
	DECLARE_DRIVER_INIT(radioram);
	DECLARE_DRIVER_INIT(radio86);
	DECLARE_MACHINE_RESET(radio86);
	DECLARE_PALETTE_INIT(radio86);
	TIMER_CALLBACK_MEMBER(radio86_reset);
	DECLARE_READ8_MEMBER(radio86_8255_portb_r2);
	DECLARE_READ8_MEMBER(radio86_8255_portc_r2);
	DECLARE_WRITE8_MEMBER(radio86_8255_porta_w2);
	DECLARE_WRITE8_MEMBER(radio86_8255_portc_w2);
	DECLARE_READ8_MEMBER(rk7007_8255_portc_r);
	DECLARE_WRITE_LINE_MEMBER(hrq_w);
	DECLARE_READ8_MEMBER(radio86_romdisk_porta_r);
	DECLARE_WRITE8_MEMBER(radio86_romdisk_portb_w);
	DECLARE_WRITE8_MEMBER(radio86_romdisk_portc_w);
	DECLARE_WRITE8_MEMBER(mikrosha_8255_font_page_w);
};


/*----------- defined in drivers/radio86.c -----------*/

INPUT_PORTS_EXTERN( radio86 );
INPUT_PORTS_EXTERN( ms7007 );


/*----------- defined in machine/radio86.c -----------*/

extern const i8255_interface radio86_ppi8255_interface_1;
extern const i8255_interface radio86_ppi8255_interface_2;
extern const i8255_interface rk7007_ppi8255_interface;

extern const i8255_interface mikrosha_ppi8255_interface_1;
extern const i8255_interface mikrosha_ppi8255_interface_2;

extern const i8275_interface radio86_i8275_interface;
extern const i8275_interface partner_i8275_interface;
extern const i8275_interface mikrosha_i8275_interface;
extern const i8275_interface apogee_i8275_interface;

extern const i8257_interface radio86_dma;


extern void radio86_init_keyboard(running_machine &machine);


/*----------- defined in video/radio86.c -----------*/

extern I8275_DISPLAY_PIXELS(radio86_display_pixels);
extern I8275_DISPLAY_PIXELS(partner_display_pixels);
extern I8275_DISPLAY_PIXELS(mikrosha_display_pixels);
extern I8275_DISPLAY_PIXELS(apogee_display_pixels);

#endif /* radio86_H_ */
