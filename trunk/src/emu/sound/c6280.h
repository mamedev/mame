#pragma once

#ifndef __C6280_H__
#define __C6280_H__

#include "devlegcy.h"

typedef struct _c6280_interface c6280_interface;
struct _c6280_interface
{
	const char *	cpu;
};

/* Function prototypes */
WRITE8_DEVICE_HANDLER( c6280_w );
READ8_DEVICE_HANDLER( c6280_r );

DECLARE_LEGACY_SOUND_DEVICE(C6280, c6280);

#endif /* __C6280_H__ */
