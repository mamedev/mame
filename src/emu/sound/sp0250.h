#pragma once

#ifndef __SP0250_H__
#define __SP0250_H__

#include "devlegcy.h"

struct sp0250_interface {
	void (*drq_callback)(device_t *device, int state);
};

WRITE8_DEVICE_HANDLER( sp0250_w );
UINT8 sp0250_drq_r(device_t *device);

DECLARE_LEGACY_SOUND_DEVICE(SP0250, sp0250);

#endif /* __SP0250_H__ */
