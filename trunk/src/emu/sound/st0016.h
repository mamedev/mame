#pragma once

#ifndef __ST0016_H__
#define __ST0016_H__

#include "devlegcy.h"

typedef struct _st0016_interface st0016_interface;
struct _st0016_interface
{
	UINT8 **p_soundram;
};

READ8_DEVICE_HANDLER( st0016_snd_r );
WRITE8_DEVICE_HANDLER( st0016_snd_w );

DECLARE_LEGACY_SOUND_DEVICE(ST0016, st0016);

#endif /* __ST0016_H__ */
