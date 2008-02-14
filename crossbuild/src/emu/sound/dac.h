#ifndef DAC_H
#define DAC_H

void DAC_data_w(int num,UINT8 data);
void DAC_signed_data_w(int num,UINT8 data);
void DAC_data_16_w(int num,UINT16 data);
void DAC_signed_data_16_w(int num,UINT16 data);

WRITE8_HANDLER( DAC_0_data_w );
WRITE8_HANDLER( DAC_1_data_w );
WRITE8_HANDLER( DAC_2_data_w );
WRITE8_HANDLER( DAC_0_signed_data_w );
WRITE8_HANDLER( DAC_1_signed_data_w );
WRITE8_HANDLER( DAC_2_signed_data_w );

#endif
