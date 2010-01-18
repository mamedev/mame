#pragma once

#ifndef __SP0250_H__
#define __SP0250_H__

struct sp0250_interface {
	void (*drq_callback)(running_device *device, int state);
};

WRITE8_DEVICE_HANDLER( sp0250_w );
UINT8 sp0250_drq_r(running_device *device);

DEVICE_GET_INFO( sp0250 );
#define SOUND_SP0250 DEVICE_GET_INFO_NAME( sp0250 )

#endif /* __SP0250_H__ */
