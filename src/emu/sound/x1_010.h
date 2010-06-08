#pragma once

#ifndef __X1_010_H__
#define __X1_010_H__

#include "devlegcy.h"


typedef struct _x1_010_interface x1_010_interface;
struct _x1_010_interface
{
	int adr;	/* address */
};


READ8_DEVICE_HANDLER ( seta_sound_r );
WRITE8_DEVICE_HANDLER( seta_sound_w );

READ16_DEVICE_HANDLER ( seta_sound_word_r );
WRITE16_DEVICE_HANDLER( seta_sound_word_w );

void seta_sound_enable_w(running_device *device, int data);

DECLARE_LEGACY_SOUND_DEVICE(X1_010, x1_010);

#endif /* __X1_010_H__ */
