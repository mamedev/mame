/*********************************************************

    Konami 053260 PCM/ADPCM Sound Chip

*********************************************************/

#pragma once

#ifndef __K053260_H__
#define __K053260_H__

#include "devlegcy.h"

typedef struct _k053260_interface k053260_interface;
struct _k053260_interface {
	const char *rgnoverride;
	timer_fired_func irq;			/* called on SH1 complete cycle ( clock / 32 ) */
};


WRITE8_DEVICE_HANDLER( k053260_w );
READ8_DEVICE_HANDLER( k053260_r );

DECLARE_LEGACY_SOUND_DEVICE(K053260, k053260);

#endif /* __K053260_H__ */
