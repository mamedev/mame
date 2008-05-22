/*

    Sega/Yamaha AICA emulation
*/

#ifndef _AICA_H_
#define _AICA_H_

#define MAX_AICA	(2)

struct AICAinterface
{
	int region;				/* region of 2M/8M RAM */
	int roffset;				/* offset in the region */
	void (*irq_callback)(running_machine *machine, int state);	/* irq callback */
};

void AICA_set_ram_base(int which, void *base, int size);

// AICA register access
READ16_HANDLER( AICA_0_r );
WRITE16_HANDLER( AICA_0_w );

// MIDI I/O access
WRITE16_HANDLER( AICA_MidiIn );
READ16_HANDLER( AICA_MidiOutR );

#endif
