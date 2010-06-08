#pragma once

#ifndef __BEEP_H__
#define __BEEP_H__

#include "devlegcy.h"

void beep_set_state(running_device *device, int on);
void beep_set_frequency(running_device *device, int frequency);
void beep_set_volume(running_device *device, int volume);

DECLARE_LEGACY_SOUND_DEVICE(BEEP, beep);

#endif /* __BEEP_H__ */
