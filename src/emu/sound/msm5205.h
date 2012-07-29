#pragma once

#ifndef __MSM5205_H__
#define __MSM5205_H__

#include "devlegcy.h"

/* an interface for the MSM5205 and similar chips */

/* prescaler selector defines   */
/* MSM5205 default master clock is 384KHz */
#define MSM5205_S96_3B 0     /* prescaler 1/96(4KHz) , data 3bit */
#define MSM5205_S48_3B 1     /* prescaler 1/48(8KHz) , data 3bit */
#define MSM5205_S64_3B 2     /* prescaler 1/64(6KHz) , data 3bit */
#define MSM5205_SEX_3B 3     /* VCLK slave mode      , data 3bit */
#define MSM5205_S96_4B 4     /* prescaler 1/96(4KHz) , data 4bit */
#define MSM5205_S48_4B 5     /* prescaler 1/48(8KHz) , data 4bit */
#define MSM5205_S64_4B 6     /* prescaler 1/64(6KHz) , data 4bit */
#define MSM5205_SEX_4B 7     /* VCLK slave mode      , data 4bit */

/* MSM6585 default master clock is 640KHz */
#define MSM6585_S160  (4+8)  /* prescaler 1/160(4KHz), data 4bit */
#define MSM6585_S40   (5+8)  /* prescaler 1/40(16KHz), data 4bit */
#define MSM6585_S80   (6+8)  /* prescaler 1/80 (8KHz), data 4bit */
#define MSM6585_S20   (7+8)  /* prescaler 1/20(32KHz), data 4bit */

typedef struct _msm5205_interface msm5205_interface;
struct _msm5205_interface
{
	void (*vclk_callback)(device_t *);   /* VCLK callback              */
	int select;       /* prescaler / bit width selector        */
};

/* reset signal should keep for 2cycle of VCLK      */
void msm5205_reset_w (device_t *device, int reset);
/* adpcmata is latched after vclk_interrupt callback */
void msm5205_data_w (device_t *device, int data);
/* VCLK slave mode option                                        */
/* if VCLK and reset or data is changed at the same time,        */
/* Call msm5205_vclk_w after msm5205_data_w and msm5205_reset_w. */
void msm5205_vclk_w (device_t *device, int reset);
/* option , selected pin seletor */
void msm5205_playmode_w(device_t *device, int _select);

void msm5205_set_volume(device_t *device,int volume);

void msm5205_change_clock_w(device_t *device, INT32 clock);

DECLARE_LEGACY_SOUND_DEVICE(MSM5205, msm5205);
DECLARE_LEGACY_SOUND_DEVICE(MSM6585, msm6585);

#endif /* __MSM5205_H__ */
