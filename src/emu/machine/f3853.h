/*
  fairchild f3853 static ram interface smi
  with integrated interrupt controller and timer

  timer shift register basically the same as in f3851!
*/

#pragma once

#ifndef __F3853_H__
#define __F3853_H__

#include "devlegcy.h"

DECLARE_LEGACY_DEVICE(F3853, f3853);

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _f3853_config f3853_config;
struct _f3853_config
{
    void (*interrupt_request)(running_device *device, UINT16 addr, int level);
};


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MDRV_F3853_ADD(_tag, _clock, _intrf) \
	MDRV_DEVICE_ADD(_tag, F3853, _clock) \
	MDRV_DEVICE_CONFIG(_intrf)


READ8_DEVICE_HANDLER(f3853_r);
WRITE8_DEVICE_HANDLER(f3853_w);

void f3853_set_external_interrupt_in_line(running_device *device, int level);
void f3853_set_priority_in_line(running_device *device, int level);

#endif /* __F3853_H__ */
