/*
    Microtouch touch screen controller

    Written by Mariusz Wojcieszek

    Notes/ToDo:
    - calibration mode (command CX)
    - only tablet format and decimal format are supported for returning touch screen state

*/

#include "emu.h"
#include "microtch.h"

#define LOG 0

enum MICROTOUCH_FORMAT
{
	FORMAT_UNKNOWN,
	FORMAT_TABLET,
	FORMAT_DECIMAL
};

ALLOW_SAVE_TYPE(MICROTOUCH_FORMAT);

enum MICROTOUCH_MODE
{
	MODE_INACTIVE,
	MODE_STREAM,
	MODE_POINT
};

ALLOW_SAVE_TYPE(MICROTOUCH_MODE);

static struct
{
	UINT8		rx_buffer[16];
	int			rx_buffer_ptr;
	emu_timer*	timer;
	UINT8		tx_buffer[16];
	UINT8		tx_buffer_num;
	UINT8		tx_buffer_ptr;
	int			reset_done;
	MICROTOUCH_FORMAT	format;
	MICROTOUCH_MODE		mode;
	int			last_touch_state;
	int			last_x;
	int			last_y;
	microtouch_tx_func	tx_callback;
	microtouch_touch_func	touch_callback;
} microtouch;


static int microtouch_check_command( const char* commandtocheck, int command_len, UINT8* command_data )
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

static void microtouch_send_format_table_packet(UINT8 flag, int x, int y)
{
	microtouch.tx_buffer[microtouch.tx_buffer_num++] = flag;
	// lower byte (7bits) of x coordinate
	microtouch.tx_buffer[microtouch.tx_buffer_num++] = x & 0x7f;
	// higher byte (7bits) of x coordinate
	microtouch.tx_buffer[microtouch.tx_buffer_num++] = (x >> 7) & 0x7f;
	// lower byte (7bits) of y coordinate
	microtouch.tx_buffer[microtouch.tx_buffer_num++] = y & 0x7f;
	// higher byte (7bits) of y coordinate
	microtouch.tx_buffer[microtouch.tx_buffer_num++] = (y >> 7) & 0x7f;
};

static void microtouch_send_format_decimal_packet(int x, int y)
{
	int decx, decy;

	decx = x / 16;
	if ( decx > 999 )
		decx = 999;
	decy = y / 16;
	if ( decy > 999 )
		decy = 999;

	// header byte
	microtouch.tx_buffer[microtouch.tx_buffer_num++] = 0x01;
	// x coordinate in decimal mode
	microtouch.tx_buffer[microtouch.tx_buffer_num++] = (decx / 100) + '0';
	microtouch.tx_buffer[microtouch.tx_buffer_num++] = ((decx / 10) % 10) + '0';
	microtouch.tx_buffer[microtouch.tx_buffer_num++] = (decx % 10) + '0';
	// comma (separator)
	microtouch.tx_buffer[microtouch.tx_buffer_num++] = ',';
	// y coordinate in decimal mode
	microtouch.tx_buffer[microtouch.tx_buffer_num++] = (decy / 100) + '0';
	microtouch.tx_buffer[microtouch.tx_buffer_num++] = ((decy / 10) % 10) + '0';
	microtouch.tx_buffer[microtouch.tx_buffer_num++] = (decy % 10) + '0';
	// terminator
	microtouch.tx_buffer[microtouch.tx_buffer_num++] = 0x0d;
}

static void microtouch_send_touch_packet(running_machine &machine)
{
	int tx = input_port_read(machine, "TOUCH_X");
	int ty = input_port_read(machine, "TOUCH_Y");

	if ( microtouch.touch_callback == NULL ||
		 microtouch.touch_callback( machine, &tx, &ty ) != 0 )
	{
		ty = 0x4000 - ty;

		switch( microtouch.format )
		{
			case FORMAT_TABLET:
				microtouch_send_format_table_packet(0xc8, tx, ty);
				break;
			case FORMAT_DECIMAL:
				microtouch_send_format_decimal_packet(tx, ty);
				break;
			case FORMAT_UNKNOWN:
				break;
		}
		microtouch.last_touch_state = 1;
		microtouch.last_x = tx;
		microtouch.last_y = ty;
	}
}

static TIMER_CALLBACK(microtouch_timer_callback)
{
	if ( microtouch.tx_buffer_ptr < microtouch.tx_buffer_num )
	{
		microtouch.tx_callback( machine, microtouch.tx_buffer[microtouch.tx_buffer_ptr++] );
		if ( microtouch.tx_buffer_ptr == microtouch.tx_buffer_num )
		{
			microtouch.tx_buffer_ptr = microtouch.tx_buffer_num = 0;
		}
		return;
	}

	if ( (microtouch.reset_done == 0) ||
		 (microtouch.format == FORMAT_UNKNOWN) ||
		 (microtouch.mode != MODE_STREAM))
	{
		return;
	}

	// send format tablet packet
	if ( input_port_read(machine, "TOUCH") & 0x01 )
	{
		microtouch_send_touch_packet(machine);
	}
	else
	{
		if ( microtouch.last_touch_state == 1 )
		{
			microtouch.last_touch_state = 0;
			switch( microtouch.format )
			{
				case FORMAT_TABLET:
					microtouch_send_format_table_packet(0x88, microtouch.last_x, microtouch.last_y);
					break;
				case FORMAT_DECIMAL:
					microtouch_send_format_decimal_packet(microtouch.last_x, microtouch.last_y);
					break;
				case FORMAT_UNKNOWN:
					break;
			}
		}
	}
};

void microtouch_init(running_machine &machine, microtouch_tx_func tx_cb, microtouch_touch_func touch_cb)
{
	memset(&microtouch, 0, sizeof(microtouch));

	microtouch.last_touch_state = -1;
	microtouch.tx_callback = tx_cb;
	microtouch.touch_callback = touch_cb;

	microtouch.timer = machine.scheduler().timer_alloc(FUNC(microtouch_timer_callback));
	microtouch.timer->adjust(attotime::from_hz(167*5), 0, attotime::from_hz(167*5));

	microtouch.format = FORMAT_UNKNOWN;
	microtouch.mode = MODE_INACTIVE;

	state_save_register_item(machine, "microtouch", NULL, 0, microtouch.reset_done);
	state_save_register_item(machine, "microtouch", NULL, 0, microtouch.last_touch_state);
	state_save_register_item(machine, "microtouch", NULL, 0, microtouch.last_x);
	state_save_register_item(machine, "microtouch", NULL, 0, microtouch.last_y);
	state_save_register_item_array(machine, "microtouch", NULL, 0, microtouch.rx_buffer);
	state_save_register_item(machine, "microtouch", NULL, 0, microtouch.rx_buffer_ptr);
	state_save_register_item_array(machine, "microtouch", NULL, 0, microtouch.tx_buffer);
	state_save_register_item(machine, "microtouch", NULL, 0, microtouch.tx_buffer_num);
	state_save_register_item(machine, "microtouch", NULL, 0, microtouch.tx_buffer_ptr);
	state_save_register_item(machine, "microtouch", NULL, 0, microtouch.format);
	state_save_register_item(machine, "microtouch", NULL, 0, microtouch.mode);
};


void microtouch_rx(int count, UINT8* data)
{
	int i;

	for ( i = 0; (i < count) && ((microtouch.rx_buffer_ptr + i) < 16); i++ )
	{
		microtouch.rx_buffer[i+microtouch.rx_buffer_ptr] = data[i];
		microtouch.rx_buffer_ptr++;
	}

	if (microtouch.rx_buffer_ptr > 0 && (microtouch.rx_buffer[microtouch.rx_buffer_ptr-1] == 0x0d))
	{
		if (LOG)
		{
			char command[16];
			memset(command, 0, sizeof(command));
			strncpy( command, (const char*)microtouch.rx_buffer + 1, microtouch.rx_buffer_ptr - 2 );
			logerror("Microtouch: received command %s\n", command);
		}
		// check command
		if ( microtouch_check_command( "MS", microtouch.rx_buffer_ptr, microtouch.rx_buffer ) )
		{
			microtouch.mode = MODE_STREAM;
		}
		else if ( microtouch_check_command( "MI", microtouch.rx_buffer_ptr, microtouch.rx_buffer ) )
		{
			microtouch.mode = MODE_INACTIVE;
		}
		else if ( microtouch_check_command( "MP", microtouch.rx_buffer_ptr, microtouch.rx_buffer ) )
		{
			microtouch.mode = MODE_POINT;
		}
		else if ( microtouch_check_command( "R", microtouch.rx_buffer_ptr, microtouch.rx_buffer ) )
		{
			microtouch.tx_buffer_num = 0;
			microtouch.reset_done = 1;
		}
		else if ( microtouch_check_command( "FT", microtouch.rx_buffer_ptr, microtouch.rx_buffer ) )
		{
			microtouch.format = FORMAT_TABLET;
		}
		else if ( microtouch_check_command( "FD", microtouch.rx_buffer_ptr, microtouch.rx_buffer ) )
		{
			microtouch.format = FORMAT_DECIMAL;
		}
		else if ( microtouch_check_command("OI", microtouch.rx_buffer_ptr, microtouch.rx_buffer ) )
		{
			// output identity - SMT3, ver 01.00
			microtouch.tx_buffer[microtouch.tx_buffer_num++] = 0x01;
			microtouch.tx_buffer[microtouch.tx_buffer_num++] = 'Q';
			microtouch.tx_buffer[microtouch.tx_buffer_num++] = '1';
			microtouch.tx_buffer[microtouch.tx_buffer_num++] = '0';
			microtouch.tx_buffer[microtouch.tx_buffer_num++] = '1';
			microtouch.tx_buffer[microtouch.tx_buffer_num++] = '0';
			microtouch.tx_buffer[microtouch.tx_buffer_num++] = '0';
			microtouch.tx_buffer[microtouch.tx_buffer_num++] = 0x0d;
			microtouch.rx_buffer_ptr = 0;
			return;
		}
		// send response
		microtouch.tx_buffer[microtouch.tx_buffer_num++] = 0x01;
		microtouch.tx_buffer[microtouch.tx_buffer_num++] = 0x30;
		microtouch.tx_buffer[microtouch.tx_buffer_num++] = 0x0d;
		microtouch.rx_buffer_ptr = 0;
	}
};

static INPUT_CHANGED( microtouch_touch )
{
	if ( newval && ( microtouch.mode == MODE_POINT ) )
	{
		microtouch_send_touch_packet( field.machine() );
	}
}

INPUT_PORTS_START(microtouch)
	PORT_START("TOUCH")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME( "Touch screen" ) PORT_CHANGED( microtouch_touch, 0 )
	PORT_START("TOUCH_X")
	PORT_BIT( 0x3fff, 0x2000, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(45) PORT_KEYDELTA(15)
	PORT_START("TOUCH_Y")
	PORT_BIT( 0x3fff, 0x2000, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(45) PORT_KEYDELTA(15)
INPUT_PORTS_END

