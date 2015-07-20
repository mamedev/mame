// license:BSD-3-Clause
// copyright-holders:Mariusz Wojcieszek
/*
    Microtouch touch screen controller

    Written by Mariusz Wojcieszek

    Notes/ToDo:
    - calibration mode (command CX)
    - only tablet format and decimal format are supported for returning touch screen state

*/

#include "microtch.h"

#define LOG 0

const device_type MICROTOUCH = &device_creator<microtouch_device>;

microtouch_device::microtouch_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, MICROTOUCH, "Microtouch Touchscreen", tag, owner, clock, "microtouch", __FILE__),
	device_serial_interface(mconfig, *this),
	m_out_stx_func(*this),
	m_touch(*this, "TOUCH"),
	m_touchx(*this, "TOUCH_X"),
	m_touchy(*this, "TOUCH_Y")
{
}

int microtouch_device::check_command( const char* commandtocheck, int command_len, UINT8* command_data )
{
	if ( (command_len == (strlen(commandtocheck) + 2)) &&
			(command_data[0] == 0x01) &&
			(strncmp(commandtocheck, (const char*)command_data + 1, strlen(commandtocheck)) == 0) &&
			(command_data[command_len-1] == 0x0d) )
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

void microtouch_device::send_format_table_packet(UINT8 flag, int x, int y)
{
	m_tx_buffer[m_tx_buffer_num++] = flag;
	// lower byte (7bits) of x coordinate
	m_tx_buffer[m_tx_buffer_num++] = x & 0x7f;
	// higher byte (7bits) of x coordinate
	m_tx_buffer[m_tx_buffer_num++] = (x >> 7) & 0x7f;
	// lower byte (7bits) of y coordinate
	m_tx_buffer[m_tx_buffer_num++] = y & 0x7f;
	// higher byte (7bits) of y coordinate
	m_tx_buffer[m_tx_buffer_num++] = (y >> 7) & 0x7f;
}

void microtouch_device::send_format_decimal_packet(int x, int y)
{
	int decx, decy;

	decx = x / 16;
	if ( decx > 999 )
		decx = 999;
	decy = y / 16;
	if ( decy > 999 )
		decy = 999;

	// header byte
	m_tx_buffer[m_tx_buffer_num++] = 0x01;
	// x coordinate in decimal mode
	m_tx_buffer[m_tx_buffer_num++] = (decx / 100) + '0';
	m_tx_buffer[m_tx_buffer_num++] = ((decx / 10) % 10) + '0';
	m_tx_buffer[m_tx_buffer_num++] = (decx % 10) + '0';
	// comma (separator)
	m_tx_buffer[m_tx_buffer_num++] = ',';
	// y coordinate in decimal mode
	m_tx_buffer[m_tx_buffer_num++] = (decy / 100) + '0';
	m_tx_buffer[m_tx_buffer_num++] = ((decy / 10) % 10) + '0';
	m_tx_buffer[m_tx_buffer_num++] = (decy % 10) + '0';
	// terminator
	m_tx_buffer[m_tx_buffer_num++] = 0x0d;
}

void microtouch_device::send_touch_packet()
{
	int tx = m_touchx->read();
	int ty = m_touchy->read();

	if ( m_out_touch_cb.isnull() ||
			m_out_touch_cb( &tx, &ty ) != 0 )
	{
		ty = 0x4000 - ty;

		switch( m_format )
		{
			case FORMAT_TABLET:
				send_format_table_packet(0xc8, tx, ty);
				break;
			case FORMAT_DECIMAL:
				send_format_decimal_packet(tx, ty);
				break;
			case FORMAT_UNKNOWN:
				break;
		}
		m_last_touch_state = 1;
		m_last_x = tx;
		m_last_y = ty;
	}
}

void microtouch_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	if(id)
	{
		device_serial_interface::device_timer(timer, id, param, ptr);
		return;
	}

	if ( m_tx_buffer_ptr < m_tx_buffer_num )
	{
		m_output = m_tx_buffer[m_tx_buffer_ptr++];
		m_output_valid = true;
		if(is_transmit_register_empty())
			tra_complete();

		if ( m_tx_buffer_ptr == m_tx_buffer_num )
		{
			m_tx_buffer_ptr = m_tx_buffer_num = 0;
		}
		return;
	}

	if ( (m_reset_done == 0) ||
			(m_format == FORMAT_UNKNOWN) ||
			(m_mode != MODE_STREAM))
	{
		return;
	}

	// send format tablet packet
	if (m_touch->read())
	{
		send_touch_packet();
	}
	else
	{
		if ( m_last_touch_state == 1 )
		{
			m_last_touch_state = 0;
			switch( m_format )
			{
				case FORMAT_TABLET:
					send_format_table_packet(0x88, m_last_x, m_last_y);
					break;
				case FORMAT_DECIMAL:
					send_format_decimal_packet(m_last_x, m_last_y);
					break;
				case FORMAT_UNKNOWN:
					break;
			}
		}
	}
}

void microtouch_device::device_start()
{
	memset(m_rx_buffer, 0, sizeof(m_rx_buffer));
	memset(m_tx_buffer, 0, sizeof(m_tx_buffer));
	m_rx_buffer_ptr = 0;
	m_tx_buffer_ptr = 0;
	m_tx_buffer_num = 0;
	m_reset_done = 0;
	m_format = 0;
	m_mode = 0;
	m_last_x = 0;
	m_last_y = 0;
	m_last_touch_state = -1;

	m_timer = timer_alloc();
	m_timer->adjust(attotime::from_hz(167*5), 0, attotime::from_hz(167*5));

	m_format = FORMAT_UNKNOWN;
	m_mode = MODE_INACTIVE;

	save_item(NAME(m_reset_done));
	save_item(NAME(m_last_touch_state));
	save_item(NAME(m_last_x));
	save_item(NAME(m_last_y));
	save_item(NAME(m_rx_buffer));
	save_item(NAME(m_rx_buffer_ptr));
	save_item(NAME(m_tx_buffer));
	save_item(NAME(m_tx_buffer_num));
	save_item(NAME(m_tx_buffer_ptr));
	save_item(NAME(m_format));
	save_item(NAME(m_mode));
	set_data_frame(1, 8, PARITY_NONE, STOP_BITS_1); //8N1?
	set_tra_rate(clock());
	set_rcv_rate(clock());
	m_out_stx_func.resolve_safe();
	m_output_valid = false;

	save_item(NAME(m_output_valid));
	save_item(NAME(m_output));
}


void microtouch_device::rcv_complete()
{
	receive_register_extract();
	m_rx_buffer[m_rx_buffer_ptr] = get_received_char();
	m_rx_buffer_ptr++;
	if(m_rx_buffer_ptr == 16)
		return;

	if (m_rx_buffer_ptr > 0 && (m_rx_buffer[m_rx_buffer_ptr-1] == 0x0d))
	{
		if (LOG)
		{
			char command[16];
			memset(command, 0, sizeof(command));
			strncpy( command, (const char*)m_rx_buffer + 1, m_rx_buffer_ptr - 2 );
			logerror("Microtouch: received command %s\n", command);
		}
		// check command
		if ( check_command( "MS", m_rx_buffer_ptr, m_rx_buffer ) )
		{
			m_mode = MODE_STREAM;
		}
		else if ( check_command( "MI", m_rx_buffer_ptr, m_rx_buffer ) )
		{
			m_mode = MODE_INACTIVE;
		}
		else if ( check_command( "MP", m_rx_buffer_ptr, m_rx_buffer ) )
		{
			m_mode = MODE_POINT;
		}
		else if ( check_command( "R", m_rx_buffer_ptr, m_rx_buffer ) )
		{
			m_tx_buffer_num = 0;
			m_reset_done = 1;
		}
		else if ( check_command( "FT", m_rx_buffer_ptr, m_rx_buffer ) )
		{
			m_format = FORMAT_TABLET;
		}
		else if ( check_command( "FD", m_rx_buffer_ptr, m_rx_buffer ) )
		{
			m_format = FORMAT_DECIMAL;
		}
		else if ( check_command("OI", m_rx_buffer_ptr, m_rx_buffer ) )
		{
			// output identity - SMT3, ver 01.00
			m_tx_buffer[m_tx_buffer_num++] = 0x01;
			m_tx_buffer[m_tx_buffer_num++] = 'Q';
			m_tx_buffer[m_tx_buffer_num++] = '1';
			m_tx_buffer[m_tx_buffer_num++] = '0';
			m_tx_buffer[m_tx_buffer_num++] = '1';
			m_tx_buffer[m_tx_buffer_num++] = '0';
			m_tx_buffer[m_tx_buffer_num++] = '0';
			m_tx_buffer[m_tx_buffer_num++] = 0x0d;
			m_rx_buffer_ptr = 0;
			return;
		}
		// send response
		m_tx_buffer[m_tx_buffer_num++] = 0x01;
		m_tx_buffer[m_tx_buffer_num++] = 0x30;
		m_tx_buffer[m_tx_buffer_num++] = 0x0d;
		m_rx_buffer_ptr = 0;
	}
}

INPUT_CHANGED_MEMBER( microtouch_device::touch )
{
	if ( newval && ( m_mode == MODE_POINT ) )
	{
		send_touch_packet();
	}
}

static INPUT_PORTS_START(microtouch)
	PORT_START("TOUCH")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME( "Touch screen" ) PORT_CHANGED_MEMBER( DEVICE_SELF,microtouch_device, touch, 0 )
	PORT_START("TOUCH_X")
	PORT_BIT( 0x3fff, 0x2000, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(45) PORT_KEYDELTA(15)
	PORT_START("TOUCH_Y")
	PORT_BIT( 0x3fff, 0x2000, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(45) PORT_KEYDELTA(15)
INPUT_PORTS_END

ioport_constructor microtouch_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(microtouch);
}

void microtouch_device::tra_callback()
{
	m_out_stx_func(transmit_register_get_data_bit());
}

void microtouch_device::tra_complete()
{
	if(m_output_valid)
	{
		transmit_register_setup(m_output);
		m_output_valid = false;
	}
}
