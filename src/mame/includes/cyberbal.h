/*************************************************************************

    Atari Cyberball hardware

*************************************************************************/

#include "machine/atarigen.h"

class cyberbal_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, cyberbal_state(machine)); }

	cyberbal_state(running_machine &machine) { }

	atarigen_state	atarigen;

	UINT16 *		paletteram_0;
	UINT16 *		paletteram_1;
	UINT16			current_slip[2];
	UINT8			playfield_palette_bank[2];
	UINT16			playfield_xscroll[2];
	UINT16			playfield_yscroll[2];

	UINT8 *			bank_base;
	UINT8			fast_68k_int;
	UINT8			io_68k_int;
	UINT8			sound_data_from_68k;
	UINT8			sound_data_from_6502;
	UINT8			sound_data_from_68k_ready;
	UINT8			sound_data_from_6502_ready;
};



/*----------- defined in audio/cyberbal.c -----------*/

void cyberbal_sound_reset(running_machine *machine);

INTERRUPT_GEN( cyberbal_sound_68k_irq_gen );

READ8_HANDLER( cyberbal_special_port3_r );
READ8_HANDLER( cyberbal_sound_6502_stat_r );
READ8_HANDLER( cyberbal_sound_68k_6502_r );
WRITE8_HANDLER( cyberbal_sound_bank_select_w );
WRITE8_HANDLER( cyberbal_sound_68k_6502_w );

READ16_HANDLER( cyberbal_sound_68k_r );
WRITE16_HANDLER( cyberbal_io_68k_irq_ack_w );
WRITE16_HANDLER( cyberbal_sound_68k_w );
WRITE16_HANDLER( cyberbal_sound_68k_dac_w );


/*----------- defined in video/cyberbal.c -----------*/

READ16_HANDLER( cyberbal_paletteram_0_r );
READ16_HANDLER( cyberbal_paletteram_1_r );
WRITE16_HANDLER( cyberbal_paletteram_0_w );
WRITE16_HANDLER( cyberbal_paletteram_1_w );

VIDEO_START( cyberbal );
VIDEO_START( cyberbal2p );
VIDEO_UPDATE( cyberbal );

void cyberbal_scanline_update(running_device *screen, int scanline);
