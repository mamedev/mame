#pragma once

#ifndef __HC55516_H__
#define __HC55516_H__

#include "devlegcy.h"


/* sets the digit (0 or 1) */
void hc55516_digit_w(device_t *device, int digit);

/* sets the clock state (0 or 1, clocked on the rising edge) */
void hc55516_clock_w(device_t *device, int state);

/* returns whether the clock is currently LO or HI */
int hc55516_clock_state_r(device_t *device);

DECLARE_LEGACY_SOUND_DEVICE(HC55516, hc55516);
DECLARE_LEGACY_SOUND_DEVICE(MC3417, mc3417);
DECLARE_LEGACY_SOUND_DEVICE(MC3418, mc3418);

#endif /* __HC55516_H__ */
