#pragma once

#ifndef __3812INTF_H__
#define __3812INTF_H__

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

DEVICE_GET_INFO( ym3812 );
#define SOUND_YM3812 DEVICE_GET_INFO_NAME( ym3812 )

#endif /* __3812INTF_H__ */
