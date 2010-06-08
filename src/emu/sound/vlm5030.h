#pragma once

#ifndef __VLM5030_H__
#define __VLM5030_H__

#include "devlegcy.h"

typedef struct _vlm5030_interface vlm5030_interface;
struct _vlm5030_interface
{
	int memory_size;    /* memory size of speech rom (0=memory region length) */
};

/* set speech rom address */
void vlm5030_set_rom(running_device *device, void *speech_rom);

/* get BSY pin level */
int vlm5030_bsy(running_device *device);
/* latch contoll data */
WRITE8_DEVICE_HANDLER( vlm5030_data_w );
/* set RST pin level : reset / set table address A8-A15 */
void vlm5030_rst (running_device *device, int pin );
/* set VCU pin level : ?? unknown */
void vlm5030_vcu(running_device *device, int pin );
/* set ST pin level  : set table address A0-A7 / start speech */
void vlm5030_st(running_device *device, int pin );

DECLARE_LEGACY_SOUND_DEVICE(VLM5030, vlm5030);

#endif /* __VLM5030_H__ */
