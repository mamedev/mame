#pragma once

#ifndef __ES8712_H__
#define __ES8712_H__

/* An interface for the ES8712 ADPCM chip */

void es8712_play(const device_config *device);
void es8712_set_bank_base(const device_config *device, int base);
void es8712_set_frequency(const device_config *device, int frequency);

WRITE8_DEVICE_HANDLER( es8712_w );

DEVICE_GET_INFO( es8712 );
#define SOUND_ES8712 DEVICE_GET_INFO_NAME( es8712 )

#endif /* __ES8712_H__ */
