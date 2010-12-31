#pragma once

#ifndef __ES8712_H__
#define __ES8712_H__

#include "devlegcy.h"

/* An interface for the ES8712 ADPCM chip */

void es8712_play(device_t *device);
void es8712_set_bank_base(device_t *device, int base);
void es8712_set_frequency(device_t *device, int frequency);

WRITE8_DEVICE_HANDLER( es8712_w );

DECLARE_LEGACY_SOUND_DEVICE(ES8712, es8712);

#endif /* __ES8712_H__ */
