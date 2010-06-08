#pragma once

#ifndef __TMS3615_H__
#define __TMS3615_H__

#include "devlegcy.h"

extern void tms3615_enable_w(running_device *device, int enable);

#define TMS3615_FOOTAGE_8	0
#define TMS3615_FOOTAGE_16	1

DECLARE_LEGACY_SOUND_DEVICE(TMS3615, tms3615);

#endif /* __TMS3615_H__ */
