// license:BSD-3-Clause
// copyright-holders:smf
/***************************************************************************

    National Semiconductor ADC0831 / ADC0832 / ADC0834 / ADC0838

    8-Bit serial I/O A/D Converters with Muliplexer Options

***************************************************************************/

#include "emu.h"
#include "adc083x.h"

#define VERBOSE_LEVEL ( 0 )

static inline void ATTR_PRINTF( 3, 4 ) verboselog( int n_level, device_t &device, const char *s_fmt, ... )
{
	if( VERBOSE_LEVEL >= n_level )
	{
		va_list v;
		char buf[ 32768 ];
		va_start( v, s_fmt );
		vsprintf( buf, s_fmt, v );
		va_end( v );
		device.logerror( "%s: %s", device.machine().describe_context( ), buf );
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

const device_type ADC0831 = &device_creator<adc0831_device>;
const device_type ADC0832 = &device_creator<adc0832_device>;
const device_type ADC0834 = &device_creator<adc0834_device>;
const device_type ADC0838 = &device_creator<adc0838_device>;

adc083x_device::adc083x_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
	m_cs(0),
	m_clk(0),
	m_di(0),
	m_se(0),
	m_do(1),
	m_sgl(0),
	m_odd(0),
	m_sel1(0),
	m_sel0(0),
	m_state(STATE_IDLE),
	m_bit(0),
	m_output(0)
{
}

adc0831_device::adc0831_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: adc083x_device(mconfig, ADC0831, "ADC0831", tag, owner, clock, "adc0831", __FILE__)
{
	m_mux_bits = 0;
}

adc0832_device::adc0832_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: adc083x_device(mconfig, ADC0832, "ADC0832", tag, owner, clock, "adc0832", __FILE__)
{
	m_mux_bits = 2;
}

adc0834_device::adc0834_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: adc083x_device(mconfig, ADC0834, "ADC0834", tag, owner, clock, "adc0834", __FILE__)
{
	m_mux_bits = 3;
}

adc0838_device::adc0838_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: adc083x_device(mconfig, ADC0838, "ADC0838", tag, owner, clock, "adc0838", __FILE__)
{
	m_mux_bits = 4;
}

/*-------------------------------------------------
    adc083x_device::device_start
-------------------------------------------------*/

void adc083x_device::device_start()
{
	clear_sars();

	/* resolve callbacks */
	m_input_callback.bind_relative_to(*owner());

	/* register for state saving */
	save_item( NAME(m_cs) );
	save_item( NAME(m_clk) );
	save_item( NAME(m_di) );
	save_item( NAME(m_se) );
	save_item( NAME(m_sars) );
	save_item( NAME(m_do) );
	save_item( NAME(m_sgl) );
	save_item( NAME(m_odd) );
	save_item( NAME(m_sel1) );
	save_item( NAME(m_sel0) );
	save_item( NAME(m_state) );
	save_item( NAME(m_bit) );
	save_item( NAME(m_output) );
	save_item( NAME(m_mux_bits) );
}

/*-------------------------------------------------
    adc083x_device::clear_sars
-------------------------------------------------*/

void adc083x_device::clear_sars()
{
	if( type() == ADC0834 || type() == ADC0838 )
	{
		m_sars = 1;
	}
	else
	{
		m_sars = 0;
	}
}

/*-------------------------------------------------
    adc083x_device::cs_write
-------------------------------------------------*/

WRITE_LINE_MEMBER( adc083x_device::cs_write )
{
	if( m_cs != state )
	{
		verboselog( 2, *this, "adc083x_cs_write( %s, %d )\n", tag().c_str(), state );
	}

	if( m_cs == 0 && state != 0 )
	{
		m_state = STATE_IDLE;
		clear_sars();
		m_do = 1;
	}

	if( m_cs != 0 && state == 0 )
	{
		if( type() == ADC0831 )
		{
			m_state = STATE_MUX_SETTLE;
		}
		else
		{
			m_state = STATE_WAIT_FOR_START;
		}

		clear_sars();
		m_do = 1;
	}

	m_cs = state;
}

/*-------------------------------------------------
    adc083x_device::conversion
-------------------------------------------------*/

UINT8 adc083x_device::conversion()
{
	int result;
	int positive_channel = ADC083X_AGND;
	int negative_channel = ADC083X_AGND;
	double positive = 0;
	double negative = 0;
	double gnd = m_input_callback(ADC083X_AGND);
	double vref = m_input_callback(ADC083X_VREF);

	if( type() == ADC0831 )
	{
		positive_channel = ADC083X_CH0;
		negative_channel = ADC083X_CH1;
	}
	else if( type() == ADC0832 )
	{
		positive_channel = ADC083X_CH0 + m_odd;
		if( m_sgl == 0 )
		{
			negative_channel = positive_channel ^ 1;
		}
		else
		{
			negative_channel = ADC083X_AGND;
		}
	}
	else if( type() == ADC0834 )
	{
		positive_channel = ADC083X_CH0 + m_odd + ( m_sel1 * 2 );
		if( m_sgl == 0 )
		{
			negative_channel = positive_channel ^ 1;
		}
		else
		{
			negative_channel = ADC083X_AGND;
		}
	}
	else if( type() == ADC0838 )
	{
		positive_channel = ADC083X_CH0 + m_odd + ( m_sel0 * 2 ) + ( m_sel1 * 4 );
		if( m_sgl == 0 )
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
		positive = m_input_callback(positive_channel) - gnd;
	}

	if( negative_channel != ADC083X_AGND )
	{
		negative = m_input_callback(negative_channel) - gnd;
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
    adc083x_device::clk_write
-------------------------------------------------*/

WRITE_LINE_MEMBER( adc083x_device::clk_write )
{
	if( m_clk != state )
	{
		verboselog( 2, *this, "adc083x_clk_write( %s, %d )\n", tag().c_str(), state );
	}

	if( m_cs == 0 )
	{
		if( m_clk == 0 && state != 0 )
		{
			switch( m_state )
			{
			case STATE_WAIT_FOR_START:
				if( m_di != 0 )
				{
					verboselog( 1, *this, "adc083x %s got start bit\n", tag().c_str() );
					m_state = STATE_SHIFT_MUX;
					m_sars = 0;
					m_sgl = 0;
					m_odd = 0;
					m_sel1 = 0;
					m_sel0 = 0;
					m_bit = 0;
				}
				else
				{
					verboselog( 1, *this, "adc083x %s not start bit\n", tag().c_str() );
				}
				break;

			case STATE_SHIFT_MUX:
				switch( m_bit )
				{
				case 0:
					if( m_di != 0 )
					{
						m_sgl = 1;
					}
					verboselog( 1, *this, "adc083x %s sgl <- %d\n", tag().c_str(), m_sgl );
					break;

				case 1:
					if( m_di != 0 )
					{
						m_odd = 1;
					}
					verboselog( 1, *this, "adc083x %s odd <- %d\n", tag().c_str(), m_odd );
					break;

				case 2:
					if( m_di != 0 )
					{
						m_sel1 = 1;
					}
					verboselog( 1, *this, "adc083x %s sel1 <- %d\n", tag().c_str(), m_sel1 );
					break;

				case 3:
					if( m_di != 0 )
					{
						m_sel0 = 1;
					}
					verboselog( 1, *this, "adc083x %s sel0 <- %d\n", tag().c_str(), m_sel0 );
					break;
				}

				m_bit++;
				if( m_bit == m_mux_bits )
				{
					m_state = STATE_MUX_SETTLE;
				}

				break;

			case STATE_WAIT_FOR_SE:
				m_sars = 0;
				if( type() == ADC0838 && m_se != 0 )
				{
					verboselog( 1, *this, "adc083x %s not se\n", tag().c_str() );
				}
				else
				{
					verboselog( 1, *this, "adc083x %s got se\n", tag().c_str() );
					m_state = STATE_OUTPUT_LSB_FIRST;
					m_bit = 1;
				}
				break;
			}
		}

		if( m_clk != 0 && state == 0 )
		{
			switch( m_state )
			{
			case STATE_MUX_SETTLE:
				verboselog( 1, *this, "adc083x %s mux settle\n", tag().c_str() );
				m_output = conversion();
				m_state = STATE_OUTPUT_MSB_FIRST;
				m_bit = 7;
				clear_sars();
				m_do = 0;
				break;

			case STATE_OUTPUT_MSB_FIRST:
				m_do = ( m_output >> m_bit ) & 1;
				verboselog( 1, *this, "adc083x %s msb %d -> %d\n", tag().c_str(), m_bit, m_do );

				m_bit--;
				if( m_bit < 0 )
				{
					if( type() == ADC0831 )
					{
						m_state = STATE_FINISHED;
					}
					else
					{
						m_state = STATE_WAIT_FOR_SE;
					}
				}
				break;

			case STATE_OUTPUT_LSB_FIRST:
				m_do = ( m_output >> m_bit ) & 1;
				verboselog( 1, *this, "adc083x %s lsb %d -> %d\n", tag().c_str(), m_bit, m_do );

				m_bit++;
				if( m_bit == 8 )
				{
					m_state = STATE_FINISHED;
				}
				break;

			case STATE_FINISHED:
				m_state = STATE_IDLE;
				m_do = 0;
				break;
			}
		}
	}

	m_clk = state;
}

/*-------------------------------------------------
    adc083x_device::di_write
-------------------------------------------------*/

WRITE_LINE_MEMBER( adc083x_device::di_write )
{
	if( m_di != state )
	{
		verboselog( 2, *this, "adc083x_di_write( %s, %d )\n", tag().c_str(), state );
	}

	m_di = state;
}

/*-------------------------------------------------
    adc083x_device::se_write
-------------------------------------------------*/

WRITE_LINE_MEMBER( adc083x_device::se_write )
{
	if( m_se != state )
	{
		verboselog( 2, *this, "adc083x_se_write( %s, %d )\n", tag().c_str(), state );
	}

	m_se = state;
}

/*-------------------------------------------------
    adc083x_device::sars_read
-------------------------------------------------*/

READ_LINE_MEMBER( adc083x_device::sars_read )
{
	verboselog( 1, *this, "adc083x_sars_read( %s ) %d\n", tag().c_str(), m_sars );
	return m_sars;
}

/*-------------------------------------------------
    adc083x_device::do_read
-------------------------------------------------*/

READ_LINE_MEMBER( adc083x_device::do_read )
{
	verboselog( 1, *this, "adc083x_do_read( %s ) %d\n", tag().c_str(), m_do );
	return m_do;
}
