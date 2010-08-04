#pragma once

#ifndef __NAMCO_H__
#define __NAMCO_H__

#include "devlegcy.h"

typedef struct _namco_interface namco_interface;
struct _namco_interface
{
	int voices;		/* number of voices */
	int stereo;		/* set to 1 to indicate stereo (e.g., System 1) */
};

WRITE8_DEVICE_HANDLER( pacman_sound_enable_w );
WRITE8_DEVICE_HANDLER( pacman_sound_w );

void polepos_sound_enable(running_device *device, int enable);
WRITE8_DEVICE_HANDLER( polepos_sound_w );

void mappy_sound_enable(running_device *device, int enable);

WRITE8_DEVICE_HANDLER( namcos1_cus30_w );	/* wavedata + sound registers + RAM */
READ8_DEVICE_HANDLER( namcos1_cus30_r );

READ8_DEVICE_HANDLER( namco_snd_sharedram_r );
WRITE8_DEVICE_HANDLER( namco_snd_sharedram_w );

DECLARE_LEGACY_SOUND_DEVICE(NAMCO, namco);
DECLARE_LEGACY_SOUND_DEVICE(NAMCO_15XX, namco_15xx);
DECLARE_LEGACY_SOUND_DEVICE(NAMCO_CUS30, namco_cus30);


#endif /* __NAMCO_H__ */

