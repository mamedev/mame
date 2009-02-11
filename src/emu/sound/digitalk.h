#ifndef _DIGITALKER_H_
#define _DIGITALKER_H_

void digitalker_0_cs_w(const device_config *device, int line);
void digitalker_0_cms_w(const device_config *device, int line);
void digitalker_0_wr_w(const device_config *device, int line);
int digitalker_0_intr_r(const device_config *device);
WRITE8_DEVICE_HANDLER(digitalker_data_w);

DEVICE_GET_INFO(digitalker);
#define SOUND_DIGITALKER DEVICE_GET_INFO_NAME(digitalker)

#endif
