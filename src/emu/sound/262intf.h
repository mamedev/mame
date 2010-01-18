#pragma once

#ifndef __262INTF_H__
#define __262INTF_H__


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


DEVICE_GET_INFO( ymf262 );
#define SOUND_YMF262 DEVICE_GET_INFO_NAME( ymf262 )

#endif /* __262INTF_H__ */
