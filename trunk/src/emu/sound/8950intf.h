#pragma once

#ifndef __8950INTF_H__
#define __8950INTF_H__

#include "devlegcy.h"

typedef struct _y8950_interface y8950_interface;
struct _y8950_interface
{
	void (*handler)(device_t *device, int linestate);

	read8_device_func keyboardread;
	write8_device_func keyboardwrite;
	read8_device_func portread;
	write8_device_func portwrite;
};

READ8_DEVICE_HANDLER( y8950_r );
WRITE8_DEVICE_HANDLER( y8950_w );

READ8_DEVICE_HANDLER( y8950_status_port_r );
READ8_DEVICE_HANDLER( y8950_read_port_r );
WRITE8_DEVICE_HANDLER( y8950_control_port_w );
WRITE8_DEVICE_HANDLER( y8950_write_port_w );

DECLARE_LEGACY_SOUND_DEVICE(Y8950, y8950);

#endif /* __8950INTF_H__ */
