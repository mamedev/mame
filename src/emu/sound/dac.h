#pragma once

#ifndef __DAC_H__
#define __DAC_H__

void dac_data_w(int num,UINT8 data);
void dac_signed_data_w(int num,UINT8 data);
void dac_data_16_w(int num,UINT16 data);
void dac_signed_data_16_w(int num,UINT16 data);

WRITE8_HANDLER( dac_0_data_w );
WRITE8_HANDLER( dac_1_data_w );
WRITE8_HANDLER( dac_2_data_w );
WRITE8_HANDLER( dac_0_signed_data_w );
WRITE8_HANDLER( dac_1_signed_data_w );
WRITE8_HANDLER( dac_2_signed_data_w );

SND_GET_INFO( dac );
#define SOUND_DAC SND_GET_INFO_NAME( dac )

#endif /* __DAC_H__ */
