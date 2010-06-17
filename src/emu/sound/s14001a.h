#pragma once

#ifndef __S14001A_H__
#define __S14001A_H__

#include "devlegcy.h"

int s14001a_bsy_r(running_device *device);  		/* read BUSY pin */
void s14001a_reg_w(running_device *device, int data);		/* write to input latch */
void s14001a_rst_w(running_device *device, int data);		/* write to RESET pin */
void s14001a_set_clock(running_device *device, int clock);     /* set VSU-1000 clock */
void s14001a_set_volume(running_device *device, int volume);    /* set VSU-1000 volume control */

DECLARE_LEGACY_SOUND_DEVICE(S14001A, s14001a);

#endif /* __S14001A_H__ */

