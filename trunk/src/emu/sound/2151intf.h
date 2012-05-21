#pragma once

#ifndef __2151INTF_H__
#define __2151INTF_H__

#include "devlegcy.h"

typedef struct _ym2151_interface ym2151_interface;
struct _ym2151_interface
{
	devcb_write_line irqhandler;
	devcb_write8 portwritehandler;
};

READ8_DEVICE_HANDLER( ym2151_r );
WRITE8_DEVICE_HANDLER( ym2151_w );

READ8_DEVICE_HANDLER( ym2151_status_port_r );
WRITE8_DEVICE_HANDLER( ym2151_register_port_w );
WRITE8_DEVICE_HANDLER( ym2151_data_port_w );

DECLARE_LEGACY_SOUND_DEVICE(YM2151, ym2151);

#endif /* __2151INTF_H__ */
