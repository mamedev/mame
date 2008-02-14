/*
    ADC12130/ADC12132/ADC12138

    Self-calibrating 12-bit Plus Sign Serial I/O A/D Converters with MUX and Sample/Hold

    TODO:
        - Only ADC12138 currently supported
*/

#include "driver.h"
#include "adc1213x.h"

#define MAX_ADC1213X_CHIPS		4

typedef struct
{
	int cycle;
	int data_out;
	int data_in;
	int conv_mode;
	int auto_cal;
	int auto_zero;
	int acq_time;
	int data_out_sign;
	int mode;
	int input_shift_reg;
	int output_shift_reg;
	int end_conv;

	double (*input_callback)(int input);
} ADC1213X;

static ADC1213X adc1213x[MAX_ADC1213X_CHIPS];

#define ADC1213X_CONV_MODE_12_MSB_FIRST			0
#define ADC1213X_CONV_MODE_16_MSB_FIRST			1
#define ADC1213X_CONV_MODE_12_LSB_FIRST			2
#define ADC1213X_CONV_MODE_16_LSB_FIRST			3

#define ADC1213X_ACQUISITION_TIME_6_CCLK		0
#define ADC1213X_ACQUISITION_TIME_10_CCLK		1
#define ADC1213X_ACQUISITION_TIME_18_CCLK		2
#define ADC1213X_ACQUISITION_TIME_34_CCLK		3

int adc1213x_do_r(int chip)
{
	//printf("ADC: DO\n");
	return adc1213x[chip].data_out;
}

void adc1213x_di_w(int chip, int state)
{
	adc1213x[chip].data_in = state & 1;
}

static void adc1213x_convert(int chip, int channel, int bits16, int lsbfirst)
{
	int i;
	int bits;
	int input_value;
	double input;

	if (bits16)
		fatalerror("ADC1213X: 16-bit mode not supported\n");

	if (lsbfirst)
		fatalerror("ADC1213X: LSB first not supported\n");

	switch (channel)
	{
		case 0x8:		// H L L L - CH0 (single-ended)
		{
			input = adc1213x[chip].input_callback(0);
			break;
		}
		case 0xc:		// H H L L - CH1 (single-ended)
		{
			input = adc1213x[chip].input_callback(1);
			break;
		}
		case 0x9:		// H L L H - CH2 (single-ended)
		{
			input = adc1213x[chip].input_callback(2);
			break;
		}
		case 0xd:		// H H L H - CH3 (single-ended)
		{
			input = adc1213x[chip].input_callback(3);
			break;
		}
		case 0xa:		// H L H L - CH4 (single-ended)
		{
			input = adc1213x[chip].input_callback(4);
			break;
		}
		case 0xe:		// H H H L - CH5 (single-ended)
		{
			input = adc1213x[chip].input_callback(5);
			break;
		}
		case 0xb:		// H L H H - CH6 (single-ended)
		{
			input = adc1213x[chip].input_callback(6);
			break;
		}
		case 0xf:		// H H H H - CH7 (single-ended)
		{
			input = adc1213x[chip].input_callback(7);
			break;
		}
		default:
		{
			fatalerror("ADC1213X: unsupported channel %02X\n", channel);
		}
	}

	input_value = (int)(input * 2047.0);

	bits = 12;

	// sign-extend if needed
	if (adc1213x[chip].data_out_sign)
	{
		input_value = input_value | ((input_value & 0x800) << 1);
		bits++;
	}

	adc1213x[chip].output_shift_reg = 0;

	for (i=0; i < bits; i++)
	{
		if (input_value & (1 << ((bits-1) - i)))
		{
			adc1213x[chip].output_shift_reg |= (1 << i);
		}
	}

	adc1213x[chip].data_out = adc1213x[chip].output_shift_reg & 1;
	adc1213x[chip].output_shift_reg >>= 1;
}

void adc1213x_cs_w(int chip, int state)
{
	if (state)
	{
		//printf("ADC: CS\n");

		if (adc1213x[chip].cycle >= 7)
		{
			int mode = adc1213x[chip].input_shift_reg >> (adc1213x[chip].cycle - 8);

			switch (mode & 0xf)
			{
				case 0x0:		// X X X X L L L L - 12 or 13 Bit MSB First conversion
				{
					adc1213x_convert(chip, (mode >> 4) & 0xf, 0, 0);
					break;
				}
				case 0x1:		// X X X X L L L H - 16 or 17 Bit MSB First conversion
				{
					adc1213x_convert(chip, (mode >> 4) & 0xf, 1, 0);
					break;
				}
				case 0x4:		// X X X X L H L L - 12 or 13 Bit LSB First conversion
				{
					adc1213x_convert(chip, (mode >> 4) & 0xf, 0, 1);
					break;
				}
				case 0x5:		// X X X X L H L H - 16 or 17 Bit LSB First conversion
				{
					adc1213x_convert(chip, (mode >> 4) & 0xf, 1, 1);
					break;
				}

				default:
				{
					switch (mode)
					{
						case 0x08:		// L L L L H L L L - Auto cal
						{
							adc1213x[chip].auto_cal = 1;
							break;
						}

						case 0x0e:		// L L L L H H H L - Acquisition time 6 CCLK cycles
						{
							adc1213x[chip].acq_time = ADC1213X_ACQUISITION_TIME_6_CCLK;
							break;
						}

						case 0x8d:		// H L L L H H L H - Data out with sign
						{
							adc1213x[chip].data_out_sign = 1;
							break;
						}

						default:
						{
							fatalerror("ADC1213X: unknown config mode %02X\n", mode);
						}
					}
					break;
				}
			}
		}

		adc1213x[chip].cycle = 0;
		adc1213x[chip].input_shift_reg = 0;

		adc1213x[chip].end_conv = 0;
	}
}

void adc1213x_sclk_w(int chip, int state)
{
	if (state)
	{
		//printf("ADC: cycle %d, DI = %d\n", adc1213x[chip].cycle, adc1213x[chip].data_in);

		adc1213x[chip].input_shift_reg <<= 1;
		adc1213x[chip].input_shift_reg |= adc1213x[chip].data_in;

		adc1213x[chip].data_out = adc1213x[chip].output_shift_reg & 1;
		adc1213x[chip].output_shift_reg >>= 1;

		adc1213x[chip].cycle++;
	}
}

void adc1213x_conv_w(int chip, int state)
{
	adc1213x[chip].end_conv = 1;
}

int adc1213x_eoc_r(int chip)
{
	return adc1213x[chip].end_conv;
}

void adc1213x_init(int chip, double (*input_callback)( int input ))
{
	memset(&adc1213x[chip], 0, sizeof(ADC1213X));

	adc1213x[chip].conv_mode = ADC1213X_CONV_MODE_12_MSB_FIRST;
	adc1213x[chip].data_out_sign = 1;
	adc1213x[chip].auto_cal = 0;
	adc1213x[chip].auto_zero = 0;
	adc1213x[chip].acq_time = ADC1213X_ACQUISITION_TIME_10_CCLK;

	adc1213x[chip].input_callback = input_callback;
}
