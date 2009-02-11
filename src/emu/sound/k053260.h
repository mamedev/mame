/*********************************************************

    Konami 053260 PCM/ADPCM Sound Chip

*********************************************************/

#pragma once

#ifndef __K053260_H__
#define __K053260_H__

typedef struct _k053260_interface k053260_interface;
struct _k053260_interface {
	const char *rgnoverride;
	timer_fired_func irq;			/* called on SH1 complete cycle ( clock / 32 ) */
};


WRITE8_DEVICE_HANDLER( k053260_w );
READ8_DEVICE_HANDLER( k053260_r );

DEVICE_GET_INFO( k053260 );
#define SOUND_K053260 DEVICE_GET_INFO_NAME( k053260 )

#endif /* __K053260_H__ */
