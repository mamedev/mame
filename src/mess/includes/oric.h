/*****************************************************************************
 *
 * includes/oric.h
 *
 ****************************************************************************/

#ifndef ORIC_H_
#define ORIC_H_

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "sound/ay8910.h"
#include "sound/wave.h"
#include "machine/6522via.h"
#include "machine/6551acia.h"
#include "machine/ctronics.h"
#include "machine/wd17xx.h"
//#include <stdio.h>
#include "machine/applefdc.h"
#include "imagedev/flopdrv.h"
#include "imagedev/cassette.h"
#include "formats/oric_dsk.h"
#include "formats/ap2_dsk.h"
#include "formats/oric_tap.h"

enum
{
	TELESTRAT_MEM_BLOCK_UNDEFINED,
	TELESTRAT_MEM_BLOCK_RAM,
	TELESTRAT_MEM_BLOCK_ROM
};

struct telestrat_mem_block 
{
	int		MemType;
	unsigned char *ptr;
};


/* current state of the display */
/* some attributes persist until they are turned off.
This structure holds this persistant information */
struct oric_vh_state 
{
	/* foreground and background colour used for rendering */
	/* if flash attribute is set, these two will both be equal to background colour */
	UINT8 active_foreground_colour;
	UINT8 active_background_colour;
	/* current foreground and background colour */
	UINT8 foreground_colour;
	UINT8 background_colour;
	UINT8 mode;
	/* text attributes */
	UINT8 text_attributes;

	offs_t read_addr;

	/* current addr to fetch data */
	UINT8 *char_data;
	/* base of char data */
	UINT8 *char_base;

	/* if (1<<3), display graphics, if 0, hide graphics */
	/* current count */
	UINT8 flash_count;
};


class oric_state : public driver_device
{
public:
	oric_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_ram(*this, "ram") { }

	optional_shared_ptr<UINT8> m_ram;
	bool m_is_telestrat;
	UINT8 m_irqs;
	UINT8 *m_ram_0x0c000;
	UINT8 m_keyboard_line;
	UINT8 m_key_sense_bit;
	UINT8 m_keyboard_mask;
	UINT8 m_via_port_a_data;
	UINT8 m_psg_control;
	UINT8 m_previous_portb_data;
	UINT8 m_port_3fa_w;
	UINT8 m_port_3fb_w;
	UINT8 m_wd179x_int_state;
	UINT8 m_port_314_r;
	UINT8 m_port_318_r;
	UINT8 m_port_314_w;
	UINT8 m_telestrat_bank_selection;
	UINT8 m_telestrat_via2_port_a_data;
	UINT8 m_telestrat_via2_port_b_data;
	telestrat_mem_block m_telestrat_blocks[8];
	oric_vh_state m_vh_state;
	DECLARE_WRITE8_MEMBER(oric_psg_porta_write);
	DECLARE_WRITE8_MEMBER(apple2_v2_interface_w);
	DECLARE_READ8_MEMBER(oric_jasmin_r);
	DECLARE_WRITE8_MEMBER(oric_jasmin_w);
	DECLARE_READ8_MEMBER(oric_microdisc_r);
	DECLARE_WRITE8_MEMBER(oric_microdisc_w);
	DECLARE_READ8_MEMBER(oric_IO_r);
	DECLARE_WRITE8_MEMBER(oric_IO_w);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	virtual void palette_init();
	DECLARE_MACHINE_START(telestrat);
};


/*----------- defined in machine/oric.c -----------*/

extern const via6522_interface oric_6522_interface;
extern const via6522_interface telestrat_via2_interface;
extern const wd17xx_interface oric_wd17xx_interface;





/* Telestrat specific */



/*----------- defined in video/oric.c -----------*/


SCREEN_UPDATE_IND16( oric );


#endif /* ORIC_H_ */
