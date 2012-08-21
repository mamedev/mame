#pragma once

#ifndef __SOCR_SND_H__
#define __SOCR_SND_H__

void socrates_snd_reg0_w(device_t *device, int data);
void socrates_snd_reg1_w(device_t *device, int data);
void socrates_snd_reg2_w(device_t *device, int data);
void socrates_snd_reg3_w(device_t *device, int data);
void socrates_snd_reg4_w(device_t *device, int data);

DECLARE_LEGACY_SOUND_DEVICE(SOCRATES, socrates_snd);

#endif /* __SOCR_SND_H__ */

