#pragma once

#ifndef __3526INTF_H__
#define __3526INTF_H__

#include "devlegcy.h"

typedef struct _ym3526_interface ym3526_interface;
struct _ym3526_interface
{
	devcb_write_line out_int_func;
};

READ8_DEVICE_HANDLER( ym3526_r );
WRITE8_DEVICE_HANDLER( ym3526_w );

READ8_DEVICE_HANDLER( ym3526_status_port_r );
READ8_DEVICE_HANDLER( ym3526_read_port_r );
WRITE8_DEVICE_HANDLER( ym3526_control_port_w );
WRITE8_DEVICE_HANDLER( ym3526_write_port_w );

DECLARE_LEGACY_SOUND_DEVICE(YM3526, ym3526);

#endif /* __3526INTF_H__ */
