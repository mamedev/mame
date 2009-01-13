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


WRITE8_HANDLER( k053260_0_w );
WRITE8_HANDLER( k053260_1_w );
READ8_HANDLER( k053260_0_r );
READ8_HANDLER( k053260_1_r );
WRITE16_HANDLER( k053260_0_lsb_w );
READ16_HANDLER( k053260_0_lsb_r );
WRITE16_HANDLER( k053260_1_lsb_w );
READ16_HANDLER( k053260_1_lsb_r );

SND_GET_INFO( k053260 );
#define SOUND_K053260 SND_GET_INFO_NAME( k053260 )

#endif /* __K053260_H__ */
