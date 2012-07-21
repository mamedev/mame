/* C140.h */

#pragma once

#ifndef __C140_H__
#define __C140_H__

#include "devlegcy.h"

READ8_DEVICE_HANDLER( c140_r );
WRITE8_DEVICE_HANDLER( c140_w );

void c140_set_base(device_t *device, void *base);

enum
{
	C140_TYPE_SYSTEM2,
	C140_TYPE_SYSTEM21,
	C140_TYPE_ASIC219
};

typedef struct _c140_interface c140_interface;
struct _c140_interface {
    int banking_type;
};

DECLARE_LEGACY_SOUND_DEVICE(C140, c140);

#endif /* __C140_H__ */
