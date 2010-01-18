#pragma once

#ifndef __FLT_VOL_H__
#define __FLT_VOL_H__

void flt_volume_set_volume(running_device *device, float volume);

DEVICE_GET_INFO( filter_volume );
#define SOUND_FILTER_VOLUME DEVICE_GET_INFO_NAME( filter_volume )

#endif /* __FLT_VOL_H__ */
