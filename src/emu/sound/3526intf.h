#pragma once

#ifndef __3526INTF_H__
#define __3526INTF_H__

typedef struct _ym3526_interface ym3526_interface;
struct _ym3526_interface
{
	void (*handler)(running_device *device, int linestate);
};

READ8_DEVICE_HANDLER( ym3526_r );
WRITE8_DEVICE_HANDLER( ym3526_w );

READ8_DEVICE_HANDLER( ym3526_status_port_r );
READ8_DEVICE_HANDLER( ym3526_read_port_r );
WRITE8_DEVICE_HANDLER( ym3526_control_port_w );
WRITE8_DEVICE_HANDLER( ym3526_write_port_w );

DEVICE_GET_INFO( ym3526 );
#define SOUND_YM3526 DEVICE_GET_INFO_NAME( ym3526 )

#endif /* __3526INTF_H__ */
