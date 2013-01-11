/*********************************************************************

    bitbngr.c

    TRS style "bitbanger" serial port

*********************************************************************/

#include "emu.h"
#include "bitbngr.h"



/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

const device_type BITBANGER = &device_creator<bitbanger_device>;

/*-------------------------------------------------
    ctor
-------------------------------------------------*/

bitbanger_device::bitbanger_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, BITBANGER, "Bitbanger", tag, owner, clock),
		device_image_interface(mconfig, *this)
{
	m_output_timer = NULL;
	m_input_timer = NULL;
	m_output_value = 0;
	m_build_count = 0;
	m_build_byte = 0;
	m_idle_delay = attotime::zero;
	m_current_baud = attotime::zero;
	m_input_buffer_size = 0;
	m_input_buffer_cursor = 0;
	m_mode = 0;
	m_baud = 0;
	m_tune = 0;
	m_current_input = 0;
	memset(m_input_buffer, 0, sizeof(m_input_buffer));
}



/*-------------------------------------------------
    native_output - outputs data to a file
-------------------------------------------------*/

void bitbanger_device::native_output(UINT8 data)
{
	if (exists())
	{
		fwrite(&data, 1);
	}
}


/*-------------------------------------------------
    native_input - inputs data from a file
-------------------------------------------------*/

UINT32 bitbanger_device::native_input(void *buffer, UINT32 length)
{
	if (exists())
	{
		return fread(buffer, length);
	}
	return 0;
}



/*-------------------------------------------------
    mode_string
-------------------------------------------------*/

const char *bitbanger_device::mode_string(void)
{
	static const char *const modes[] = {"Printer", "Modem"};
	return modes[m_mode];
}



/*-------------------------------------------------
    inc_mode
-------------------------------------------------*/

bool bitbanger_device::inc_mode(bool test)
{
	int adjust_mode = (int)m_mode + 1;

	if( adjust_mode >= BITBANGER_MODE_MAX )
		return FALSE;

	if( !test)
		m_mode = adjust_mode;

	return TRUE;
}



/*-------------------------------------------------
    dec_mode
-------------------------------------------------*/

bool bitbanger_device::dec_mode(bool test)
{
	int adjust_mode = m_mode - 1;

	if( adjust_mode < 0 )
		return FALSE;

	if( !test)
		m_mode = adjust_mode;

	return TRUE;
}



/*-------------------------------------------------
    tune_string
-------------------------------------------------*/

const char *bitbanger_device::tune_string(void)
{
	static const char *const tunes[] = {"-2.0%", "-1.75%", "-1.5%", "-1.25%", "-1.0%", "-0.75%", "-0.5", "-0.25%", "\xc2\xb1""0%",
									"+0.25%",  "+0.5%", "+0.75%", "+1.0%", "+1.25%", "+1.5%", "+1.75%", "+2.0%"};
	return tunes[m_tune];
}



/*-------------------------------------------------
    tune_value
-------------------------------------------------*/

float bitbanger_device::tune_value(void)
{
	static const float tunes[] = {0.97f, 0.9825f, 0.985f, 0.9875f, 0.99f, 0.9925f, 0.995f, 0.9975f, 1.0f,
					1.0025f, 1.005f, 1.0075f, 1.01f, 1.0125f, 1.015f, 1.0175f, 1.02f};
	return tunes[m_tune];
}



/*-------------------------------------------------
    baud_value
-------------------------------------------------*/

UINT32 bitbanger_device::baud_value(void)
{
	static const float bauds[] = { 150.0f, 300.0f, 600.0f, 1200.0f, 2400.0f, 4800.0f, 9600.0f,
			14400.0f, 19200.0f, 28800.0f, 38400.0f, 57600.0f, 115200.0f};
	float result = tune_value() * bauds[m_baud];
	return (UINT32)result;
}


/*-------------------------------------------------
    baud_string
-------------------------------------------------*/

const char *bitbanger_device::baud_string(void)
{
	static const char *const bauds[] = { "150", "300", "600", "1200", "2400", "4800",
						"9600", "14400", "19200", "28800", "38400", "57600", "115200"};

	return(bauds[m_baud]);
}



/*-------------------------------------------------
    inc_baud
-------------------------------------------------*/

bool bitbanger_device::inc_baud(bool test)
{
	int adjust_baud = (int)m_baud + 1;

	if( adjust_baud >= BITBANGER_BAUD_MAX )
		return FALSE;

	if( !test)
	{
		m_baud = adjust_baud;
		m_current_baud = attotime::from_hz(baud_value());
	}

	return TRUE;
}


/*-------------------------------------------------
    dec_baud
-------------------------------------------------*/

bool bitbanger_device::dec_baud(bool test)
{
	int adjust_baud = m_baud - 1;

	if( adjust_baud < 0 )
		return FALSE;

	if( !test)
	{
		m_baud = adjust_baud;
		m_current_baud = attotime::from_hz(baud_value());
	}

	return TRUE;
}


/*-------------------------------------------------
    inc_tune
-------------------------------------------------*/

bool bitbanger_device::inc_tune(bool test)
{
	int adjust_tune = (int)m_tune + 1;

	if( adjust_tune >= BITBANGER_TUNE_MAX )
		return FALSE;

	if (!test)
	{
		m_tune = adjust_tune;
		m_current_baud = attotime::from_hz(baud_value());
	}

	return TRUE;
}



/*-------------------------------------------------
    dec_tune
-------------------------------------------------*/

bool bitbanger_device::dec_tune(bool test)
{
	int adjust_tune = m_tune - 1;

	if( adjust_tune < 0 )
		return FALSE;

	if( !test)
	{
		m_tune = adjust_tune;
		m_current_baud = attotime::from_hz(baud_value());
	}

	return TRUE;
}



/*-------------------------------------------------
    bytes_to_bits_81N
-------------------------------------------------*/

void bitbanger_device::bytes_to_bits_81N(void)
{
	UINT8 byte_buffer[100];
	UINT32 byte_buffer_size, bit_buffer_size;
	int i, j;
	static const UINT8 bitmask[8] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };

	bit_buffer_size = 0;
	byte_buffer_size = native_input(byte_buffer, sizeof(byte_buffer));

	/* Translate byte buffer into bit buffer using: 1 start bit, 8 data bits, 1 stop bit, no parity */

	for( i=0; i<byte_buffer_size; i++ )
	{
		m_input_buffer[bit_buffer_size++] = 0;

		for( j=0; j<8; j++ )
		{
			if( byte_buffer[i] & bitmask[j] )
				m_input_buffer[bit_buffer_size++] = 1;
			else
				m_input_buffer[bit_buffer_size++] = 0;
		}

		m_input_buffer[bit_buffer_size++] = 1;
	}

	m_input_buffer_size = bit_buffer_size;
	m_input_buffer_cursor = 0;
}



/*-------------------------------------------------
    device_start
-------------------------------------------------*/

void bitbanger_device::device_start(void)
{
	/* output config */
	m_build_count = 0;
	m_output_timer = timer_alloc(TIMER_OUTPUT);

	/* input config */
	m_input_timer = timer_alloc(TIMER_INPUT);
	m_idle_delay = attotime::from_seconds(1);
	m_input_buffer_size = 0;
	m_input_buffer_cursor = 0;

	/* defaults */
	m_mode = m_default_mode;
	m_baud = m_default_baud;
	m_tune = m_default_tune;
	m_current_baud = attotime::from_hz(baud_value());

	/* callback */
	m_input_func.resolve(m_input_callback, *this);
}



/*-------------------------------------------------
    device_config_complete
-------------------------------------------------*/

void bitbanger_device::device_config_complete(void)
{
	const bitbanger_config *intf = reinterpret_cast<const bitbanger_config *>(static_config());
	if(intf != NULL)
	{
		*static_cast<bitbanger_config *>(this) = *intf;
	}
	else
	{
		memset(&m_input_callback, 0, sizeof(m_input_callback));
		m_default_mode = 0;
		m_default_baud = 0;
		m_default_tune = 0;
	}
	update_names();
}



/*-------------------------------------------------
    device_timer
-------------------------------------------------*/

void bitbanger_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch(id)
	{
		case TIMER_OUTPUT:
			timer_output();
			break;

		case TIMER_INPUT:
			timer_input();
			break;
	}
}



/*-------------------------------------------------
    timer_output
-------------------------------------------------*/

void bitbanger_device::timer_output(void)
{
	/* this ia harded coded for 8-1-N */
	if( m_output_value )
		m_build_byte |= 0x200;

	m_build_byte >>= 1;
	m_build_count--;

	if(m_build_count == 0)
	{
		if( m_output_value == 1 )
			native_output(m_build_byte);
		else
			logerror("Bitbanger: Output framing error.\n" );

		m_output_timer->reset();
	}
}



/*-------------------------------------------------
    timer_input
-------------------------------------------------*/

void bitbanger_device::timer_input(void)
{
	if(m_input_buffer_cursor == m_input_buffer_size)
	{
		/* get more data */
		bytes_to_bits_81N();

		if(m_input_buffer_size == 0)
		{
			/* no more data, wait and try again */
			m_idle_delay = min(m_idle_delay + attotime::from_msec(100), attotime::from_seconds(1));
			m_input_timer->adjust(m_idle_delay);

			if( m_mode == BITBANGER_MODEM )
				set_input_line(1);
			else
				set_input_line(0);
			return;
		}
		else
		{
			m_idle_delay = m_current_baud;
			m_input_timer->adjust(m_idle_delay, 0, m_idle_delay);
		}
	}

	/* send bit to driver */
	set_input_line(m_input_buffer[(m_input_buffer_cursor)++]);
}



/*-------------------------------------------------
    set_input_line
-------------------------------------------------*/

void bitbanger_device::set_input_line(UINT8 line)
{
	/* normalize */
	line = line ? ASSERT_LINE : CLEAR_LINE;

	/* only act when the state changes */
	if (m_current_input != line)
	{
		m_current_input = line;
		m_input_func(line ? ASSERT_LINE : CLEAR_LINE);
	}
}



/*-------------------------------------------------
    output - outputs data to a bitbanger port
-------------------------------------------------*/

void bitbanger_device::output(int value)
{
	attotime one_point_five_baud;

	if( m_build_count == 0 && m_output_value == 1 && value == 0 )
	{
		/* we found our start bit */
		/* eight bits of data, plus one of stop */
		m_build_count = 9;
		m_build_byte = 0;

		one_point_five_baud = m_current_baud + m_current_baud / 2;
		m_output_timer->adjust(one_point_five_baud, 0, m_current_baud);
	}

	//fprintf(stderr,"%s, %d\n", device->machine().time().as_string(9), value);
	m_output_value = value;
}



/*-------------------------------------------------
    call_load
-------------------------------------------------*/

bool bitbanger_device::call_load(void)
{
	m_input_timer->enable(true);
	m_input_timer->adjust(attotime::zero, 0, attotime::from_seconds(1));

	/* we don't need to do anything special */
	return IMAGE_INIT_PASS;
}



/*-------------------------------------------------
    call_unload
-------------------------------------------------*/

void bitbanger_device::call_unload(void)
{
	m_input_timer->enable(false);
}
