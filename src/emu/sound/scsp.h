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
	void (*irq_callback)(running_device *device, int state);	/* irq callback */
};

void scsp_set_ram_base(running_device *device, void *base);

// SCSP register access
READ16_DEVICE_HANDLER( scsp_r );
WRITE16_DEVICE_HANDLER( scsp_w );

// MIDI I/O access (used for comms on Model 2/3)
WRITE16_DEVICE_HANDLER( scsp_midi_in );
READ16_DEVICE_HANDLER( scsp_midi_out_r );

extern UINT32* stv_scu;

DEVICE_GET_INFO( scsp );
#define SOUND_SCSP DEVICE_GET_INFO_NAME( scsp )

#endif /* __SCSP_H__ */
