#pragma once

#ifndef __2151INTF_H__
#define __2151INTF_H__

typedef struct _ym2151_interface ym2151_interface;
struct _ym2151_interface
{
	void (*irqhandler)(running_device *device, int irq);
	write8_device_func portwritehandler;
};

READ8_DEVICE_HANDLER( ym2151_r );
WRITE8_DEVICE_HANDLER( ym2151_w );

READ8_DEVICE_HANDLER( ym2151_status_port_r );
WRITE8_DEVICE_HANDLER( ym2151_register_port_w );
WRITE8_DEVICE_HANDLER( ym2151_data_port_w );

DEVICE_GET_INFO( ym2151 );
#define SOUND_YM2151 DEVICE_GET_INFO_NAME( ym2151 )

#endif /* __2151INTF_H__ */
