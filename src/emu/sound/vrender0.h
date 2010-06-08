#pragma once

#ifndef __VRENDER0_H__
#define __VRENDER0_H__

#include "devlegcy.h"


typedef struct _vr0_interface vr0_interface;
struct _vr0_interface
{
	UINT32 RegBase;
};

void vr0_snd_set_areas(running_device *device,UINT32 *texture,UINT32 *frame);

READ32_DEVICE_HANDLER( vr0_snd_read );
WRITE32_DEVICE_HANDLER( vr0_snd_write );

DECLARE_LEGACY_SOUND_DEVICE(VRENDER0, vrender0);

#endif /* __VRENDER0_H__ */
