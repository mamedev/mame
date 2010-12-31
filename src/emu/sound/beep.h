#pragma once

#ifndef __BEEP_H__
#define __BEEP_H__

#include "devlegcy.h"

void beep_set_state(device_t *device, int on);
void beep_set_frequency(device_t *device, int frequency);
void beep_set_volume(device_t *device, int volume);

DECLARE_LEGACY_SOUND_DEVICE(BEEP, beep);

#endif /* __BEEP_H__ */
