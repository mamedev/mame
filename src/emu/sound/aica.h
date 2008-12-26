/*

    Sega/Yamaha AICA emulation
*/

#ifndef __AICA_H__
#define __AICA_H__

#define MAX_AICA	(2)

typedef struct _aica_interface aica_interface;
struct _aica_interface
{
	int roffset;				/* offset in the region */
	void (*irq_callback)(running_machine *machine, int state);	/* irq callback */
};

void aica_set_ram_base(int which, void *base, int size);

// AICA register access
READ16_HANDLER( aica_0_r );
WRITE16_HANDLER( aica_0_w );

// MIDI I/O access
WRITE16_HANDLER( aica_midi_in );
READ16_HANDLER( aica_midi_out_r );

SND_GET_INFO( aica );

#endif /* __AICA_H__ */
