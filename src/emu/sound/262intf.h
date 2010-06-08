#pragma once

#ifndef __262INTF_H__
#define __262INTF_H__

#include "devlegcy.h"


typedef struct _ymf262_interface ymf262_interface;
struct _ymf262_interface
{
	void (*handler)(running_device *device, int irq);
};


READ8_DEVICE_HANDLER( ymf262_r );
WRITE8_DEVICE_HANDLER( ymf262_w );

READ8_DEVICE_HANDLER ( ymf262_status_r );
WRITE8_DEVICE_HANDLER( ymf262_register_a_w );
WRITE8_DEVICE_HANDLER( ymf262_register_b_w );
WRITE8_DEVICE_HANDLER( ymf262_data_a_w );
WRITE8_DEVICE_HANDLER( ymf262_data_b_w );


DECLARE_LEGACY_SOUND_DEVICE(YMF262, ymf262);

#endif /* __262INTF_H__ */
