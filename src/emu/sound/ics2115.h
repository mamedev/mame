#pragma once

#ifndef __ICS2115_H__
#define __ICS2115_H__

#include "devlegcy.h"

typedef struct _ics2115_interface ics2115_interface;
struct _ics2115_interface {
	void (*irq_cb)(running_device *, int);
};

READ8_DEVICE_HANDLER( ics2115_r );
WRITE8_DEVICE_HANDLER( ics2115_w );

DECLARE_LEGACY_SOUND_DEVICE(ICS2115, ics2115);

#endif /* __ICS2115_H__ */
