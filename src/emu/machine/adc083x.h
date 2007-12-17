/*
 * ADC0831/ADC0832/ADC0834/ADC0838
 *
 * 8-Bit Serial I/O A/D Converters with Muliplexer Options
 *
 */

#if !defined( ADC083X_H )
#define ADC083X_H ( 1 )

#define MAX_ADC083X_CHIPS ( 1 )

#define ADC083X_CH0 ( 0 )
#define ADC083X_CH1 ( 1 )
#define ADC083X_CH2 ( 2 )
#define ADC083X_CH3 ( 3 )
#define ADC083X_CH4 ( 4 )
#define ADC083X_CH5 ( 5 )
#define ADC083X_CH6 ( 6 )
#define ADC083X_CH7 ( 7 )
#define ADC083X_COM ( 8 )
#define ADC083X_AGND ( 9 )
#define ADC083X_VREF ( 10 )

#define ADC0831 ( 0 )
#define ADC0832 ( 1 )
#define ADC0834 ( 2 )
#define ADC0838 ( 3 )

void adc083x_init( int chip, int type, double (*input_callback)( int input ) );
extern void adc083x_cs_write( int chip, int cs );
extern void adc083x_clk_write( int chip, int clk );
extern void adc083x_di_write( int chip, int di );
extern void adc083x_se_write( int chip, int se );
extern int adc083x_sars_read( int chip );
extern int adc083x_do_read( int chip );

#endif
