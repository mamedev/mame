/*
 * ADC0831/ADC0832/ADC0834/ADC0838
 *
 * 8-Bit Serial I/O A/D Converters with Muliplexer Options
 *
 */

#include <stdarg.h>
#include "driver.h"
#include "machine/adc083x.h"

#define VERBOSE_LEVEL ( 0 )

INLINE void verboselog( int n_level, const char *s_fmt, ... )
{
	if( VERBOSE_LEVEL >= n_level )
	{
		va_list v;
		char buf[ 32768 ];
		va_start( v, s_fmt );
		vsprintf( buf, s_fmt, v );
		va_end( v );
		if( cpu_getactivecpu() != -1 )
		{
			logerror( "%08x: %s", activecpu_get_pc(), buf );
		}
		else
		{
			logerror( "(timer) : %s", buf );
		}
	}
}

struct adc083x_chip
{
	int type;
	double (*input_callback)(int input);
	INT32 CS;
	INT32 CLK;
	INT32 DI;
	INT32 SE;
	INT32 SARS;
	INT32 DO;
	INT32 SGL;
	INT32 ODD;
	INT32 SEL1;
	INT32 SEL0;
	INT32 state;
	INT32 bit;
	INT32 output;
	INT32 mux_bits;
};

#define STATE_IDLE ( 0 )
#define STATE_WAIT_FOR_START ( 1 )
#define STATE_SHIFT_MUX ( 2 )
#define STATE_MUX_SETTLE ( 3 )
#define STATE_OUTPUT_MSB_FIRST ( 4 )
#define STATE_WAIT_FOR_SE ( 5 )
#define STATE_OUTPUT_LSB_FIRST ( 6 )
#define STATE_FINISHED ( 7 )

static struct adc083x_chip adc083x[ MAX_ADC083X_CHIPS ];

void adc083x_init( int chip, int type, double (*input_callback)(int input) )
{
	struct adc083x_chip *c;

	if( chip >= MAX_ADC083X_CHIPS )
	{
		verboselog( 0, "adc083x_init( %d ) chip %d out of range\n", chip );
		return;
	}

	c = &adc083x[ chip ];

	c->type = type;
	c->input_callback = input_callback;
	c->CS = 0;
	c->CLK = 0;
	c->DI = 0;
	c->SE = 0;
	if( c->type == ADC0834 || c->type == ADC0838 )
	{
		c->SARS = 1;
	}
	else
	{
		c->SARS = 0;
	}
	c->DO = 1;
	c->SGL = 0;
	c->ODD = 0;
	c->SEL1 = 0;
	c->SEL0 = 0;
	c->state = STATE_IDLE;
	c->bit = 0;
	c->output = 0;

	switch( c->type )
	{
	case ADC0831:
		c->mux_bits = 0;
		break;
	case ADC0832:
		c->mux_bits = 2;
		break;
	case ADC0834:
		c->mux_bits = 3;
		break;
	case ADC0838:
		c->mux_bits = 4;
		break;
	}

	state_save_register_item( "adc083x", chip, c->CS );
	state_save_register_item( "adc083x", chip, c->CLK );
	state_save_register_item( "adc083x", chip, c->DI );
	state_save_register_item( "adc083x", chip, c->SE );
	state_save_register_item( "adc083x", chip, c->SARS );
	state_save_register_item( "adc083x", chip, c->DO );
	state_save_register_item( "adc083x", chip, c->SGL );
	state_save_register_item( "adc083x", chip, c->ODD );
	state_save_register_item( "adc083x", chip, c->SEL1 );
	state_save_register_item( "adc083x", chip, c->SEL0 );
	state_save_register_item( "adc083x", chip, c->state );
	state_save_register_item( "adc083x", chip, c->bit );
	state_save_register_item( "adc083x", chip, c->output );
}

void adc083x_cs_write( int chip, int cs )
{
	struct adc083x_chip *c;

	if( chip >= MAX_ADC083X_CHIPS )
	{
		verboselog( 0, "adc083x_cs_write( %d ) chip %d out of range\n", chip );
		return;
	}

	c = &adc083x[ chip ];

	if( c->CS != cs )
	{
		verboselog( 2, "adc083x_cs_write( %d, %d )\n", chip, cs );
	}

	if( c->CS == 0 && cs != 0 )
	{
		c->state = STATE_IDLE;
		if( c->type == ADC0834 || c->type == ADC0838 )
		{
			c->SARS = 1;
		}
		c->DO = 1;
	}
	if( c->CS != 0 && cs == 0 )
	{
		if( c->type == ADC0831 )
		{
			c->state = STATE_MUX_SETTLE;
		}
		else
		{
			c->state = STATE_WAIT_FOR_START;
		}
		if( c->type == ADC0834 || c->type == ADC0838 )
		{
			c->SARS = 1;
		}
		c->DO = 1;
	}

	c->CS = cs;
}

static int adc083x_conversion( int chip )
{
	struct adc083x_chip *c = &adc083x[ chip ];
	int result;
	int positive_channel = ADC083X_AGND;
	int negative_channel = ADC083X_AGND;
	double positive = 0;
	double negative = 0;
	double gnd = c->input_callback( ADC083X_AGND );
	double vref = c->input_callback( ADC083X_VREF );

	switch( c->type )
	{
	case ADC0831:
		positive_channel = ADC083X_CH0;
		negative_channel = ADC083X_CH1;
		break;
	case ADC0832:
		positive_channel = ADC083X_CH0 + c->ODD;
		if( c->SGL == 0 )
		{
			negative_channel = positive_channel ^ 1;
		}
		else
		{
			negative_channel = ADC083X_AGND;
		}
		break;
	case ADC0834:
		positive_channel = ADC083X_CH0 + c->ODD + ( c->SEL1 * 2 );
		if( c->SGL == 0 )
		{
			negative_channel = positive_channel ^ 1;
		}
		else
		{
			negative_channel = ADC083X_AGND;
		}
		break;
	case ADC0838:
		positive_channel = ADC083X_CH0 + c->ODD + ( c->SEL0 * 2 ) + ( c->SEL1 * 4 );
		if( c->SGL == 0 )
		{
			negative_channel = positive_channel ^ 1;
		}
		else
		{
			negative_channel = ADC083X_COM;
		}
		break;
	}

	if( positive_channel != ADC083X_AGND )
	{
		positive = c->input_callback( positive_channel ) - gnd;
	}
	if( negative_channel != ADC083X_AGND )
	{
		negative = c->input_callback( negative_channel ) - gnd;
	}

	result = (int)( ( ( positive - negative ) * 255 ) / vref );
	if( result < 0 )
	{
		result = 0;
	}
	else if( result > 255 )
	{
		result = 255;
	}

	return result;
}

void adc083x_clk_write( int chip, int clk )
{
	struct adc083x_chip *c;

	if( chip >= MAX_ADC083X_CHIPS )
	{
		verboselog( 0, "adc083x_clk_write( %d ) chip %d out of range\n", chip );
		return;
	}

	c = &adc083x[ chip ];

	if( c->CLK != clk )
	{
		verboselog( 2, "adc083x_clk_write( %d, %d )\n", chip, clk );
	}

	if( c->CS == 0 )
	{
		if( c->CLK == 0 && clk != 0 )
		{
			switch( c->state )
			{
			case STATE_WAIT_FOR_START:
				if( c->DI != 0 )
				{
					verboselog( 1, "adc083x %d got start bit\n", chip );
					c->state = STATE_SHIFT_MUX;
					c->SARS = 0;
					c->SGL = 0;
					c->ODD = 0;
					c->SEL1 = 0;
					c->SEL0 = 0;
					c->bit = 0;
				}
				else
				{
					verboselog( 1, "adc083x %d not start bit\n", chip );
				}
				break;
			case STATE_SHIFT_MUX:
				switch( c->bit )
				{
				case 0:
					if( c->DI != 0 )
					{
						c->SGL = 1;
					}
					verboselog( 1, "adc083x %d SGL <- %d\n", chip, c->SGL );
					break;
				case 1:
					if( c->DI != 0 )
					{
						c->ODD = 1;
					}
					verboselog( 1, "adc083x %d ODD <- %d\n", chip, c->ODD );
					break;
				case 2:
					if( c->DI != 0 )
					{
						c->SEL1 = 1;
					}
					verboselog( 1, "adc083x %d SEL1 <- %d\n", chip, c->SEL1 );
					break;
				case 3:
					if( c->DI != 0 )
					{
						c->SEL0 = 1;
					}
					verboselog( 1, "adc083x %d SEL0 <- %d\n", chip, c->SEL0 );
					break;
				}
				c->bit++;
				if( c->bit == c->mux_bits )
				{
					c->state = STATE_MUX_SETTLE;
				}
				break;
			case STATE_WAIT_FOR_SE:
				c->SARS = 0;
				if( c->type == ADC0838 && c->SE != 0 )
				{
					verboselog( 1, "adc083x %d not SE\n", chip );
				}
				else
				{
					verboselog( 1, "adc083x %d got SE\n", chip );
					c->state = STATE_OUTPUT_LSB_FIRST;
					c->bit = 1;
				}
				break;
			}
		}
		if( c->CLK != 0 && clk == 0 )
		{
			switch( c->state )
			{
			case STATE_MUX_SETTLE:
				verboselog( 1, "adc083x %d mux settle\n", chip );
				c->output = adc083x_conversion( chip );
				c->state = STATE_OUTPUT_MSB_FIRST;
				c->bit = 7;
				if( c->type == ADC0834 || c->type == ADC0838 )
				{
					c->SARS = 1;
				}
				c->DO = 0;
				break;
			case STATE_OUTPUT_MSB_FIRST:
				c->DO = ( c->output >> c->bit ) & 1;
				verboselog( 1, "adc083x %d msb %d -> %d\n", chip, c->bit, c->DO );
				c->bit--;
				if( c->bit < 0 )
				{
					if( c->type == ADC0831 )
					{
						c->state = STATE_FINISHED;
					}
					else
					{
						c->state = STATE_WAIT_FOR_SE;
					}
				}
				break;
			case STATE_OUTPUT_LSB_FIRST:
				c->DO = ( c->output >> c->bit ) & 1;
				verboselog( 1, "adc083x %d lsb %d -> %d\n", chip, c->bit, c->DO );
				c->bit++;
				if( c->bit == 8 )
				{
					c->state = STATE_FINISHED;
				}
				break;
			case STATE_FINISHED:
				c->state = STATE_IDLE;
				c->DO = 0;
				break;
			}
		}
	}

	c->CLK = clk;
}

void adc083x_di_write( int chip, int di )
{
	struct adc083x_chip *c;

	if( chip >= MAX_ADC083X_CHIPS )
	{
		verboselog( 0, "adc083x_di_write( %d ) chip %d out of range\n", chip );
		return;
	}

	c = &adc083x[ chip ];

	if( c->DI != di )
	{
		verboselog( 2, "adc083x_di_write( %d, %d )\n", chip, di );
	}

	c->DI = di;
}

void adc083x_se_write( int chip, int se )
{
	struct adc083x_chip *c;

	if( chip >= MAX_ADC083X_CHIPS )
	{
		verboselog( 0, "adc083x_se_write( %d ) chip %d out of range\n", chip );
		return;
	}

	c = &adc083x[ chip ];

	if( c->SE != se )
	{
		verboselog( 2, "adc083x_se_write( %d, %d )\n", chip, se );
	}

	c->SE = se;
}

int adc083x_sars_read( int chip )
{
	struct adc083x_chip *c;

	if( chip >= MAX_ADC083X_CHIPS )
	{
		verboselog( 0, "adc083x_sars_read( %d ) chip %d out of range\n", chip );
		return 0;
	}

	c = &adc083x[ chip ];

	verboselog( 1, "adc083x_sars_read( %d ) %d\n", chip, c->SARS );
	return c->SARS;
}

int adc083x_do_read( int chip )
{
	struct adc083x_chip *c;

	if( chip >= MAX_ADC083X_CHIPS )
	{
		verboselog( 0, "adc083x_do_read( %d ) chip %d out of range\n", chip );
		return 0;
	}

	c = &adc083x[ chip ];

	verboselog( 1, "adc083x_do_read( %d ) %d\n", chip, c->DO );
	return c->DO;
}
