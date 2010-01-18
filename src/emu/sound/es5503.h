#pragma once

#ifndef __ES5503_H__
#define __ES5503_H__

typedef struct _es5503_interface es5503_interface;
struct _es5503_interface
{
	void (*irq_callback)(running_device *device, int state);
	read8_device_func adc_read;
	UINT8 *wave_memory;
};

READ8_DEVICE_HANDLER( es5503_r );
WRITE8_DEVICE_HANDLER( es5503_w );
void es5503_set_base(running_device *device, UINT8 *wavemem);

DEVICE_GET_INFO( es5503 );
#define SOUND_ES5503 DEVICE_GET_INFO_NAME( es5503 )

#endif /* __ES5503_H__ */
