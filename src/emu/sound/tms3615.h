#pragma once

#ifndef __TMS3615_H__
#define __TMS3615_H__

extern void tms3615_enable_w(running_device *device, int enable);

#define TMS3615_FOOTAGE_8	0
#define TMS3615_FOOTAGE_16	1

DEVICE_GET_INFO( tms3615 );
#define SOUND_TMS3615 DEVICE_GET_INFO_NAME( tms3615 )

#endif /* __TMS3615_H__ */
