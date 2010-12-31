/*********************************************************

    Konami 054539 PCM Sound Chip

*********************************************************/

#pragma once

#ifndef __K054539_H__
#define __K054539_H__

#include "devlegcy.h"

typedef struct _k054539_interface k054539_interface;
struct _k054539_interface
{
	const char *rgnoverride;
	void (*apan)(device_t *, double, double);	/* Callback for analog output mixing levels (0..1 for each channel) */
	void (*irq)(device_t *);
};


WRITE8_DEVICE_HANDLER( k054539_w );
READ8_DEVICE_HANDLER( k054539_r );

//* control flags, may be set at DRIVER_INIT().
#define K054539_RESET_FLAGS     0
#define K054539_REVERSE_STEREO  1
#define K054539_DISABLE_REVERB  2
#define K054539_UPDATE_AT_KEYON 4

void k054539_init_flags(device_t *device, int flags);

/*
    Note that the eight PCM channels of a K054539 do not have seperate
    volume controls. Considering the global attenuation equation may not
    be entirely accurate, k054539_set_gain() provides means to control
    channel gain. It can be called anywhere but preferrably from
    DRIVER_INIT().

    Parameters:
        chip    : 0 / 1
        channel : 0 - 7
        gain    : 0.0=silent, 1.0=no gain, 2.0=twice as loud, etc.
*/
void k054539_set_gain(device_t *device, int channel, double gain);

DECLARE_LEGACY_SOUND_DEVICE(K054539, k054539);

#endif /* __K054539_H__ */
