/* C140.h */

#pragma once

#ifndef __C140_H__
#define __C140_H__

READ8_DEVICE_HANDLER( c140_r );
WRITE8_DEVICE_HANDLER( c140_w );

void c140_set_base(running_device *device, void *base);

enum
{
	C140_TYPE_SYSTEM2,
	C140_TYPE_SYSTEM21_A,
	C140_TYPE_SYSTEM21_B,
	C140_TYPE_ASIC219
};

typedef struct _c140_interface c140_interface;
struct _c140_interface {
    int banking_type;
};

DEVICE_GET_INFO( c140 );
#define SOUND_C140 DEVICE_GET_INFO_NAME( c140 )

#endif /* __C140_H__ */
