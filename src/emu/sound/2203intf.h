#pragma once

#ifndef __2203INTF_H__
#define __2203INTF_H__

#include "devlegcy.h"

#include "ay8910.h"

void ym2203_update_request(void *param);

typedef struct _ym2203_interface ym2203_interface;
struct _ym2203_interface
{
	const ay8910_interface ay8910_intf;
	devcb_write_line irqhandler;
};

READ8_DEVICE_HANDLER( ym2203_r );
WRITE8_DEVICE_HANDLER( ym2203_w );

READ8_DEVICE_HANDLER( ym2203_status_port_r );
READ8_DEVICE_HANDLER( ym2203_read_port_r );
WRITE8_DEVICE_HANDLER( ym2203_control_port_w );
WRITE8_DEVICE_HANDLER( ym2203_write_port_w );

DECLARE_LEGACY_SOUND_DEVICE(YM2203, ym2203);

#endif /* __2203INTF_H__ */
