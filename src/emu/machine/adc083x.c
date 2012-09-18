/***************************************************************************

    National Semiconductor ADC0831 / ADC0832 / ADC0834 / ADC0838

    8-Bit serial I/O A/D Converters with Muliplexer Options

***************************************************************************/

#include "emu.h"
#include "adc083x.h"

#define VERBOSE_LEVEL ( 0 )

INLINE void ATTR_PRINTF( 3, 4 ) verboselog( int n_level, running_machine &machine, const char *s_fmt, ... )
{
	if( VERBOSE_LEVEL >= n_level )
	{
		va_list v;
		char buf[ 32768 ];
		va_start( v, s_fmt );
		vsprintf( buf, s_fmt, v );
		va_end( v );
		logerror( "%s: %s", machine.describe_context( ), buf );
	}
}

/***************************************************************************
    PARAMETERS
***************************************************************************/

enum
{
	STATE_IDLE,
	STATE_WAIT_FOR_START,
	STATE_SHIFT_MUX,
	STATE_MUX_SETTLE,
	STATE_OUTPUT_MSB_FIRST,
	STATE_WAIT_FOR_SE,
	STATE_OUTPUT_LSB_FIRST,
	STATE_FINISHED
};

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct adc0831_state
{
	adc083x_input_convert_func input_callback_r;

	INT32 cs;
	INT32 clk;
	INT32 di;
	INT32 se;
	INT32 sars;
	INT32 _do;
	INT32 sgl;
	INT32 odd;
	INT32 sel1;
	INT32 sel0;
	INT32 state;
	INT32 bit;
	INT32 output;
	INT32 mux_bits;
};



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

INLINE adc0831_state *get_safe_token( device_t *device )
{
	assert( device != NULL );
	assert( ( device->type() == ADC0831 ) || ( device->type() == ADC0832 ) || ( device->type() == ADC0834 ) || ( device->type() == ADC0838 ) );
	return (adc0831_state *) downcast<adc0831_device *>(device)->token();
}

INLINE const adc083x_interface *get_interface( device_t *device )
{
	assert( device != NULL );
	assert( ( device->type() == ADC0831 ) || ( device->type() == ADC0832 ) || ( device->type() == ADC0834 ) || ( device->type() == ADC0838 ) );
	return (const adc083x_interface *) device->static_config();
}


/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    adc083x_clear_sars
-------------------------------------------------*/

static void adc083x_clear_sars( device_t *device, adc0831_state *adc083x )
{
	if( device->type() == ADC0834 ||device->type() == ADC0838 )
	{
		adc083x->sars = 1;
	}
	else
	{
		adc083x->sars = 0;
	}
}

/*-------------------------------------------------
    adc083x_cs_write
-------------------------------------------------*/

WRITE_LINE_DEVICE_HANDLER( adc083x_cs_write )
{
	adc0831_state *adc083x = get_safe_token( device );

	if( adc083x->cs != state )
	{
		verboselog( 2, device->machine(), "adc083x_cs_write( %s, %d )\n", device->tag(), state );
	}

	if( adc083x->cs == 0 && state != 0 )
	{
		adc083x->state = STATE_IDLE;
		adc083x_clear_sars( device, adc083x );
		adc083x->_do = 1;
	}

	if( adc083x->cs != 0 && state == 0 )
	{
		if( device->type() == ADC0831 )
		{
			adc083x->state = STATE_MUX_SETTLE;
		}
		else
		{
			adc083x->state = STATE_WAIT_FOR_START;
		}

		adc083x_clear_sars( device, adc083x );
		adc083x->_do = 1;
	}

	adc083x->cs = state;
}

/*-------------------------------------------------
    adc083x_conversion
-------------------------------------------------*/

static UINT8 adc083x_conversion( device_t *device )
{
	adc0831_state *adc083x = get_safe_token( device );
	int result;
	int positive_channel = ADC083X_AGND;
	int negative_channel = ADC083X_AGND;
	double positive = 0;
	double negative = 0;
	double gnd = adc083x->input_callback_r( device, ADC083X_AGND );
	double vref = adc083x->input_callback_r( device, ADC083X_VREF );

	if( device->type() == ADC0831 )
	{
		positive_channel = ADC083X_CH0;
		negative_channel = ADC083X_CH1;
	}
	else if( device->type() == ADC0832 )
	{
		positive_channel = ADC083X_CH0 + adc083x->odd;
		if( adc083x->sgl == 0 )
		{
			negative_channel = positive_channel ^ 1;
		}
		else
		{
			negative_channel = ADC083X_AGND;
		}
	}
	else if( device->type() == ADC0834 )
	{
		positive_channel = ADC083X_CH0 + adc083x->odd + ( adc083x->sel1 * 2 );
		if( adc083x->sgl == 0 )
		{
			negative_channel = positive_channel ^ 1;
		}
		else
		{
			negative_channel = ADC083X_AGND;
		}
	}
	else if( device->type() == ADC0838 )
	{
		positive_channel = ADC083X_CH0 + adc083x->odd + ( adc083x->sel0 * 2 ) + ( adc083x->sel1 * 4 );
		if( adc083x->sgl == 0 )
		{
			negative_channel = positive_channel ^ 1;
		}
		else
		{
			negative_channel = ADC083X_COM;
		}
	}

	if( positive_channel != ADC083X_AGND )
	{
		positive = adc083x->input_callback_r( device, positive_channel ) - gnd;
	}

	if( negative_channel != ADC083X_AGND )
	{
		negative = adc083x->input_callback_r( device, negative_channel ) - gnd;
	}

	result = (int) ( ( ( positive - negative ) * 255 ) / vref );
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

/*-------------------------------------------------
    adc083x_clk_write
-------------------------------------------------*/

WRITE_LINE_DEVICE_HANDLER( adc083x_clk_write )
{
	adc0831_state *adc083x = get_safe_token( device );

	if( adc083x->clk != state )
	{
		verboselog( 2, device->machine(), "adc083x_clk_write( %s, %d )\n", device->tag(), state );
	}

	if( adc083x->cs == 0 )
	{
		if( adc083x->clk == 0 && state != 0 )
		{
			switch( adc083x->state )
			{
			case STATE_WAIT_FOR_START:
				if( adc083x->di != 0 )
				{
					verboselog( 1, device->machine(), "adc083x %s got start bit\n", device->tag() );
					adc083x->state = STATE_SHIFT_MUX;
					adc083x->sars = 0;
					adc083x->sgl = 0;
					adc083x->odd = 0;
					adc083x->sel1 = 0;
					adc083x->sel0 = 0;
					adc083x->bit = 0;
				}
				else
				{
					verboselog( 1, device->machine(), "adc083x %s not start bit\n", device->tag() );
				}
				break;

			case STATE_SHIFT_MUX:
				switch( adc083x->bit )
				{
				case 0:
					if( adc083x->di != 0 )
					{
						adc083x->sgl = 1;
					}
					verboselog( 1, device->machine(), "adc083x %s sgl <- %d\n", device->tag(), adc083x->sgl );
					break;

				case 1:
					if( adc083x->di != 0 )
					{
						adc083x->odd = 1;
					}
					verboselog( 1, device->machine(), "adc083x %s odd <- %d\n", device->tag(), adc083x->odd );
					break;

				case 2:
					if( adc083x->di != 0 )
					{
						adc083x->sel1 = 1;
					}
					verboselog( 1, device->machine(), "adc083x %s sel1 <- %d\n", device->tag(), adc083x->sel1 );
					break;

				case 3:
					if( adc083x->di != 0 )
					{
						adc083x->sel0 = 1;
					}
					verboselog( 1, device->machine(), "adc083x %s sel0 <- %d\n", device->tag(), adc083x->sel0 );
					break;
				}

				adc083x->bit++;
				if( adc083x->bit == adc083x->mux_bits )
				{
					adc083x->state = STATE_MUX_SETTLE;
				}

				break;

			case STATE_WAIT_FOR_SE:
				adc083x->sars = 0;
				if( device->type() == ADC0838 && adc083x->se != 0 )
				{
					verboselog( 1, device->machine(), "adc083x %s not se\n", device->tag() );
				}
				else
				{
					verboselog( 1, device->machine(), "adc083x %s got se\n", device->tag() );
					adc083x->state = STATE_OUTPUT_LSB_FIRST;
					adc083x->bit = 1;
				}
				break;
			}
		}

		if( adc083x->clk != 0 && state == 0 )
		{
			switch( adc083x->state )
			{
			case STATE_MUX_SETTLE:
				verboselog( 1, device->machine(), "adc083x %s mux settle\n", device->tag() );
				adc083x->output = adc083x_conversion( device );
				adc083x->state = STATE_OUTPUT_MSB_FIRST;
				adc083x->bit = 7;
				adc083x_clear_sars( device, adc083x );
				adc083x->_do = 0;
				break;

			case STATE_OUTPUT_MSB_FIRST:
				adc083x->_do = ( adc083x->output >> adc083x->bit ) & 1;
				verboselog( 1, device->machine(), "adc083x %s msb %d -> %d\n", device->tag(), adc083x->bit, adc083x->_do );

				adc083x->bit--;
				if( adc083x->bit < 0 )
				{
					if( device->type() == ADC0831 )
					{
						adc083x->state = STATE_FINISHED;
					}
					else
					{
						adc083x->state = STATE_WAIT_FOR_SE;
					}
				}
				break;

			case STATE_OUTPUT_LSB_FIRST:
				adc083x->_do = ( adc083x->output >> adc083x->bit ) & 1;
				verboselog( 1, device->machine(), "adc083x %s lsb %d -> %d\n", device->tag(), adc083x->bit, adc083x->_do );

				adc083x->bit++;
				if( adc083x->bit == 8 )
				{
					adc083x->state = STATE_FINISHED;
				}
				break;

			case STATE_FINISHED:
				adc083x->state = STATE_IDLE;
				adc083x->_do = 0;
				break;
			}
		}
	}

	adc083x->clk = state;
}

/*-------------------------------------------------
    adc083x_di_write
-------------------------------------------------*/

WRITE_LINE_DEVICE_HANDLER( adc083x_di_write )
{
	adc0831_state *adc083x = get_safe_token( device );

	if( adc083x->di != state )
	{
		verboselog( 2, device->machine(), "adc083x_di_write( %s, %d )\n", device->tag(), state );
	}

	adc083x->di = state;
}

/*-------------------------------------------------
    adc083x_se_write
-------------------------------------------------*/

WRITE_LINE_DEVICE_HANDLER( adc083x_se_write )
{
	adc0831_state *adc083x = get_safe_token( device );

	if( adc083x->se != state )
	{
		verboselog( 2, device->machine(), "adc083x_se_write( %s, %d )\n", device->tag(), state );
	}

	adc083x->se = state;
}

/*-------------------------------------------------
    adc083x_sars_read
-------------------------------------------------*/

READ_LINE_DEVICE_HANDLER( adc083x_sars_read )
{
	adc0831_state *adc083x = get_safe_token( device );

	verboselog( 1, device->machine(), "adc083x_sars_read( %s ) %d\n", device->tag(), adc083x->sars );
	return adc083x->sars;
}

/*-------------------------------------------------
    adc083x_do_read
-------------------------------------------------*/

READ_LINE_DEVICE_HANDLER( adc083x_do_read )
{
	adc0831_state *adc083x = get_safe_token( device );

	verboselog( 1, device->machine(), "adc083x_do_read( %s ) %d\n", device->tag(), adc083x->_do );
	return adc083x->_do;
}


/*-------------------------------------------------
    DEVICE_START( adc083x )
-------------------------------------------------*/

static DEVICE_START( adc0831 )
{
	adc0831_state *adc083x = get_safe_token( device );
	const adc083x_interface *intf = get_interface( device );

	adc083x->cs = 0;
	adc083x->clk = 0;
	adc083x->di = 0;
	adc083x->se = 0;
	adc083x_clear_sars( device, adc083x );
	adc083x->_do = 1;
	adc083x->sgl = 0;
	adc083x->odd = 0;
	adc083x->sel1 = 0;
	adc083x->sel0 = 0;
	adc083x->state = STATE_IDLE;
	adc083x->bit = 0;
	adc083x->output = 0;

	if( device->type() == ADC0831 )
	{
		adc083x->mux_bits = 0;
	}
	else if( device->type() == ADC0832 )
	{
		adc083x->mux_bits = 2;
	}
	else if( device->type() == ADC0834 )
	{
		adc083x->mux_bits = 3;
	}
	else if( device->type() == ADC0838 )
	{
		adc083x->mux_bits = 4;
	}

	/* resolve callbacks */
	adc083x->input_callback_r = intf->input_callback_r;

	/* register for state saving */
	device->save_item( NAME(adc083x->cs) );
	device->save_item( NAME(adc083x->clk) );
	device->save_item( NAME(adc083x->di) );
	device->save_item( NAME(adc083x->se) );
	device->save_item( NAME(adc083x->sars) );
	device->save_item( NAME(adc083x->_do) );
	device->save_item( NAME(adc083x->sgl) );
	device->save_item( NAME(adc083x->odd) );
	device->save_item( NAME(adc083x->sel1) );
	device->save_item( NAME(adc083x->sel0) );
	device->save_item( NAME(adc083x->state) );
	device->save_item( NAME(adc083x->bit) );
	device->save_item( NAME(adc083x->output) );
	device->save_item( NAME(adc083x->mux_bits) );
}


/*-------------------------------------------------
    DEVICE_RESET( adc083x )
-------------------------------------------------*/

static DEVICE_RESET( adc0831 )
{
	adc0831_state *adc083x = get_safe_token( device );

	adc083x_clear_sars( device, adc083x );
	adc083x->_do = 1;
	adc083x->state = STATE_IDLE;
}

const device_type ADC0831 = &device_creator<adc0831_device>;

adc0831_device::adc0831_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, ADC0831, "A/D Converters 0831", tag, owner, clock)
{
	m_token = global_alloc_clear(adc0831_state);
}
adc0831_device::adc0831_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, type, name, tag, owner, clock)
{
	m_token = global_alloc_clear(adc0831_state);
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void adc0831_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void adc0831_device::device_start()
{
	DEVICE_START_NAME( adc0831 )(this);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void adc0831_device::device_reset()
{
	DEVICE_RESET_NAME( adc0831 )(this);
}


const device_type ADC0832 = &device_creator<adc0832_device>;

adc0832_device::adc0832_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: adc0831_device(mconfig, ADC0832, "A/D Converters 0832", tag, owner, clock)
{
}


const device_type ADC0834 = &device_creator<adc0834_device>;

adc0834_device::adc0834_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: adc0831_device(mconfig, ADC0834, "A/D Converters 0834", tag, owner, clock)
{
}


const device_type ADC0838 = &device_creator<adc0838_device>;

adc0838_device::adc0838_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: adc0831_device(mconfig, ADC0838, "A/D Converters 0838", tag, owner, clock)
{
}


