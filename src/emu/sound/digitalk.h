#ifndef _DIGITALKER_H_
#define _DIGITALKER_H_

#include "devlegcy.h"

void digitalker_0_cs_w(running_device *device, int line);
void digitalker_0_cms_w(running_device *device, int line);
void digitalker_0_wr_w(running_device *device, int line);
int digitalker_0_intr_r(running_device *device);
WRITE8_DEVICE_HANDLER(digitalker_data_w);

DECLARE_LEGACY_SOUND_DEVICE(DIGITALKER, digitalker);

#endif
