#pragma once

#ifndef __3812INTF_H__
#define __3812INTF_H__

#include "devlegcy.h"

typedef struct _ym3812_interface ym3812_interface;
struct _ym3812_interface
{
	void (*handler)(running_device *device, int linestate);
};

READ8_DEVICE_HANDLER( ym3812_r );
WRITE8_DEVICE_HANDLER( ym3812_w );

READ8_DEVICE_HANDLER( ym3812_status_port_r );
READ8_DEVICE_HANDLER( ym3812_read_port_r );
WRITE8_DEVICE_HANDLER( ym3812_control_port_w );
WRITE8_DEVICE_HANDLER( ym3812_write_port_w );

DECLARE_LEGACY_SOUND_DEVICE(YM3812, ym3812);

#endif /* __3812INTF_H__ */
