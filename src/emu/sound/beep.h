#pragma once

#ifndef __BEEP_H__
#define __BEEP_H__

void beep_set_state(const device_config *device, int on);
void beep_set_frequency(const device_config *device, int frequency);
void beep_set_volume(const device_config *device, int volume);

DEVICE_GET_INFO( beep );
#define SOUND_BEEP DEVICE_GET_INFO_NAME( beep )

#endif /* __BEEP_H__ */
