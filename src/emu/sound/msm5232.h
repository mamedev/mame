#pragma once

#ifndef __MSM5232_H__
#define __MSM5232_H__

#include "devlegcy.h"

typedef struct _msm5232_interface msm5232_interface;
struct _msm5232_interface
{
	double capacity[8];	/* in Farads, capacitors connected to pins: 24,25,26,27 and 37,38,39,40 */
	void (*gate_handler)(running_device *device, int state);	/* callback called when the GATE output pin changes state */
};

WRITE8_DEVICE_HANDLER( msm5232_w );

void msm5232_set_clock(running_device *device, int clock);

DECLARE_LEGACY_SOUND_DEVICE(MSM5232, msm5232);

#endif /* __MSM5232_H__ */
