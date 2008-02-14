/*
    ADC12130/ADC12132/ADC12138

    Self-calibrating 12-bit Plus Sign Serial I/O A/D Converters with MUX and Sample/Hold
*/

#ifndef ADC1213X_H
#define ADC1213X_H

int adc1213x_do_r(int chip);
void adc1213x_di_w(int chip, int state);
void adc1213x_cs_w(int chip, int state);
void adc1213x_sclk_w(int chip, int state);
void adc1213x_conv_w(int chip, int state);
int adc1213x_eoc_r(int chip);
void adc1213x_init(int chip, double (*input_callback)( int input ));

#endif
