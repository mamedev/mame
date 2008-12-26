#pragma once

#ifndef __ES8712_H__
#define __ES8712_H__

/* An interface for the ES8712 ADPCM chip */

void es8712_play(int which);
void es8712_set_bank_base(int which, int base);
void es8712_set_frequency(int which, int frequency);

WRITE8_HANDLER( es8712_data_0_w );
WRITE8_HANDLER( es8712_data_1_w );
WRITE8_HANDLER( es8712_data_2_w );
WRITE16_HANDLER( es8712_data_0_lsb_w );
WRITE16_HANDLER( es8712_data_1_lsb_w );
WRITE16_HANDLER( es8712_data_2_lsb_w );
WRITE16_HANDLER( es8712_data_0_msb_w );
WRITE16_HANDLER( es8712_data_1_msb_w );
WRITE16_HANDLER( es8712_data_2_msb_w );

SND_GET_INFO( es8712 );

#endif /* __ES8712_H__ */
