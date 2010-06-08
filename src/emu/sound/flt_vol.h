#pragma once

#ifndef __FLT_VOL_H__
#define __FLT_VOL_H__

#include "devlegcy.h"


void flt_volume_set_volume(running_device *device, float volume);

DECLARE_LEGACY_SOUND_DEVICE(FILTER_VOLUME, filter_volume);

#endif /* __FLT_VOL_H__ */
