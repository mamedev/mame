#ifndef _DIGITALKER_H_
#define _DIGITALKER_H_

#include "devlegcy.h"

void digitalker_0_cs_w(device_t *device, int line);
void digitalker_0_cms_w(device_t *device, int line);
void digitalker_0_wr_w(device_t *device, int line);
int digitalker_0_intr_r(device_t *device);
WRITE8_DEVICE_HANDLER(digitalker_data_w);

DECLARE_LEGACY_SOUND_DEVICE(DIGITALKER, digitalker);

#endif
