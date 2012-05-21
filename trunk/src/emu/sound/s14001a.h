#pragma once

#ifndef __S14001A_H__
#define __S14001A_H__

#include "devlegcy.h"

int s14001a_bsy_r(device_t *device);		/* read BUSY pin */
void s14001a_reg_w(device_t *device, int data);		/* write to input latch */
void s14001a_rst_w(device_t *device, int data);		/* write to RESET pin */
void s14001a_set_clock(device_t *device, int clock);     /* set VSU-1000 clock */
void s14001a_set_volume(device_t *device, int volume);    /* set VSU-1000 volume control */

DECLARE_LEGACY_SOUND_DEVICE(S14001A, s14001a);

#endif /* __S14001A_H__ */

