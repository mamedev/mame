/*
    SCSP (YMF292-F) header
*/

#pragma once

#ifndef __SCSP_H__
#define __SCSP_H__

typedef struct _scsp_interface scsp_interface;
struct _scsp_interface
{
	int roffset;				/* offset in the region */
	void (*irq_callback)(running_machine *machine, int state);	/* irq callback */
};

void SCSP_set_ram_base(int which, void *base);

// SCSP register access
READ16_HANDLER( scsp_0_r );
WRITE16_HANDLER( scsp_0_w );
READ16_HANDLER( scsp_1_r );
WRITE16_HANDLER( scsp_1_w );

// MIDI I/O access (used for comms on Model 2/3)
WRITE16_HANDLER( scsp_midi_in );
READ16_HANDLER( scsp_midi_out_r );

extern UINT32* stv_scu;

#endif /* __SCSP_H__ */
