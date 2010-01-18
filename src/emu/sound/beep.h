#pragma once

#ifndef __BEEP_H__
#define __BEEP_H__

void beep_set_state(running_device *device, int on);
void beep_set_frequency(running_device *device, int frequency);
void beep_set_volume(running_device *device, int volume);

DEVICE_GET_INFO( beep );
#define SOUND_BEEP DEVICE_GET_INFO_NAME( beep )

#endif /* __BEEP_H__ */
