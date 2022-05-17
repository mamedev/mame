// license:BSD-3-Clause
// copyright-holders:Mariusz Wojcieszek
/*
    Microtouch touch screen controller

    Written by Mariusz Wojcieszek

    Notes/ToDo:
    - calibration mode (command CX)
    - only tablet format and decimal format are supported for returning touch screen state

Known MicroTouch boards use an 80C32 (compatible: Siemens, Winbond, Philips, MHS, etc.) with external ROM and a custom MicroTouch MCU.
It seems there are different MicroTouch MCU versions, named "Kahuna", "Excalibur", and probably others, with unknown differences.


ISA board ((c) 1997 MicroTouch Systems Inc. FAB 5405800 REV 2.3)
_________________________________________________________________
|                                    ______   ______             |
|                                   MM74HC04M HC125A   ____      |
|                                                      34072   __|__
|                  _______      ____       ____  ____  ____    |    |
|                  R11APB18     78M05      34072 34072 93C46S  | DB |
|                ___________  __________   ____________        | 25 |
|                |SIEMENS   | |U1 BIOS  |  |MicroTouch|        |(P1)|
|                |SAB 80C32 | |         |  |Excalibur |        |____|
|                |__________| |_________|  |__________|          |
|             ________  _______   __________                     O <- LED
|             |_LS240_| DM74LS30M |TI       |                    |
| JP2->:::::                      |TL16C450FN  ________          |
|          JP1->::::::::::::::    |_________|  |_LS245_|         |
|__       ___                                             _____  |
   |_|_|_|   |_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|     |_|
                         ISA EDGE CONNECTOR

JP1 = ADDRESS / INTERRUPT (A1, A2, A3, A4, A5, A6, I2, I3, I4, I5, I10, I11, I12, I15)
JP2 = TEST (T1, T2, T3, T4, T5, T6)
Y1 = XTAL R11APB18
U3 = MicroTouch Excalibur
U4 = 80C32
U11 = 93C46S Serial EEPROM
P1 = RS-232

    JP1 ADDRESS JUMPERS
COM  ADDRESS A1 A2 A3 A4 A5 A6
--------------------------------
COM1 3F8-3FF ON  : ON  : ON  :
COM2 2F8-2FF  : ON ON  : ON  :
COM3 3E8-3EF ON  :  : ON ON  : <- DEFAULT
COM4 2E8-2EF  : ON  : ON ON  :
COM5 3E0-2E7  : ON  : ON  : ON
COM6 2F0-2F7  : ON ON  :  : ON
COM7 3E0-3E7 ON  :  : ON  : ON
COM8 3F0-3F7 ON  : ON  :  : ON

*/

#include "emu.h"
#include "microtch.h"

//#define VERBOSE 1
#include "logmacro.h"


DEFINE_DEVICE_TYPE(MICROTOUCH, microtouch_device, "microtouch", "Microtouch Touchscreen")

microtouch_device::microtouch_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, MICROTOUCH, tag, owner, clock),
	device_serial_interface(mconfig, *this),
	m_rx_buffer_ptr(0), m_tx_buffer_num(0), m_tx_buffer_ptr(0), m_reset_done(0), m_format(0), m_mode(0), m_last_touch_state(0),
	m_last_x(0), m_last_y(0),
	m_out_touch_cb(*this),
	m_out_stx_func(*this),
	m_touch(*this, "TOUCH"),
	m_touchx(*this, "TOUCH_X"),
	m_touchy(*this, "TOUCH_Y"),
	m_timer(nullptr),
	m_output_valid(false), m_output(0)
{
}

int microtouch_device::check_command( const char* commandtocheck, int command_len, uint8_t* command_data )
{
	if ((command_len == (strlen(commandtocheck) + 2)) &&
			(command_data[0] == 0x01) &&
			(strncmp(commandtocheck, (const char*)command_data + 1, strlen(commandtocheck)) == 0) &&
			(command_data[command_len-1] == 0x0d))
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

void microtouch_device::send_format_table_packet(uint8_t flag, int x, int y)
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
	if (decx > 999)
		decx = 999;
	decy = y / 16;
	if (decy > 999)
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

	if (m_out_touch_cb.isnull() || m_out_touch_cb(&tx, &ty) != 0)
	{
		ty = 0x4000 - ty;

		switch (m_format)
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

TIMER_CALLBACK_MEMBER(microtouch_device::update_output)
{
	if (m_tx_buffer_ptr < m_tx_buffer_num)
	{
		if (is_transmit_register_empty())
		{
			m_output = m_tx_buffer[m_tx_buffer_ptr++];
			m_output_valid = true;
			tra_complete();
		}

		if (m_tx_buffer_ptr == m_tx_buffer_num)
		{
			m_tx_buffer_ptr = m_tx_buffer_num = 0;
		}
		return;
	}

	if (m_reset_done == 0 || m_format == FORMAT_UNKNOWN || m_mode != MODE_STREAM)
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
		if (m_last_touch_state == 1)
		{
			m_last_touch_state = 0;
			switch (m_format)
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

	m_timer = timer_alloc(FUNC(microtouch_device::update_output), this);
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
	m_out_touch_cb.resolve();
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
	if (m_rx_buffer_ptr == 16)
		return;

	if (m_rx_buffer_ptr > 0 && m_rx_buffer[m_rx_buffer_ptr-1] == 0x0d)
	{
		if (VERBOSE)
		{
			char command[16];
			memset(command, 0, sizeof(command));
			strncpy( command, (const char*)m_rx_buffer + 1, m_rx_buffer_ptr - 2 );
			LOG("Microtouch: received command %s\n", command);
		}
		// check command
		if (check_command("MS", m_rx_buffer_ptr, m_rx_buffer))
		{
			m_mode = MODE_STREAM;
		}
		else if (check_command("MI", m_rx_buffer_ptr, m_rx_buffer))
		{
			m_mode = MODE_INACTIVE;
		}
		else if (check_command("MP", m_rx_buffer_ptr, m_rx_buffer))
		{
			m_mode = MODE_POINT;
		}
		else if (check_command("R", m_rx_buffer_ptr, m_rx_buffer))
		{
			m_tx_buffer_num = 0;
			m_reset_done = 1;
		}
		else if (check_command("FT", m_rx_buffer_ptr, m_rx_buffer))
		{
			m_format = FORMAT_TABLET;
		}
		else if (check_command("FD", m_rx_buffer_ptr, m_rx_buffer))
		{
			m_format = FORMAT_DECIMAL;
		}
		else if (check_command("OI", m_rx_buffer_ptr, m_rx_buffer))
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
		else if (check_command("OS", m_rx_buffer_ptr, m_rx_buffer))
		{
			// output status
			m_tx_buffer[m_tx_buffer_num++] = 0x01;

			// ---- ---x    RAM error
			// ---- --x-    ROM error
			// ---- -x--    Analog-to-digital error
			// ---- x---    NOVRAM error
			// ---x ----    ASIC error
			// --x- ----    Power on flag
			// -x-- ----    Always 1
			// x--- ----    Always 0
			m_tx_buffer[m_tx_buffer_num++] = 0x40;

			// ---- ---x    Cable NOVRAM error
			// ---- --x-    Hard NOVRAM error
			// ---x xx--    Reserved
			// --x- ----    Software reset flag
			// -x-- ----    Always 1
			// x--- ----    Always 0
			m_tx_buffer[m_tx_buffer_num++] = 0x40 | (m_reset_done << 5);
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

INPUT_CHANGED_MEMBER(microtouch_device::touch)
{
	if (newval && (m_mode == MODE_POINT))
	{
		send_touch_packet();
	}
}

// BIOS not hooked up
ROM_START(microtouch)
	ROM_REGION(0x10000, "bios", 0)
	ROM_SYSTEM_BIOS(0, "microtouch_5_6", "MicroTouch Rev.5.6") // "Excalibur", ISA card
	ROMX_LOAD("microtouch_5604010_rev_5.6.u1", 0x0000, 0x10000, CRC(d19ee080) SHA1(c695405ec8c2ac4408a63bacfc68a5a4b878928c), ROM_BIOS(0)) // 27c512
	ROM_SYSTEM_BIOS(1, "microtouch_5_5", "MicroTouch Rev.5.5") // "Kahuna", daughterboard integrated on monitor
	ROMX_LOAD("microtouch_5603920_rev_5.5.u1", 0x0000, 0x08000, CRC(5cc164e7) SHA1(753277ce54e8be1b759c37ae760fe4a6846d1fae), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "microtouch_2_2", "MicroTouch Rev.2.2") // "Excalibur, ISA card
	ROMX_LOAD("microtouch_5604340_rev_2.2.u1", 0x0000, 0x10000, CRC(110f312f) SHA1(5a60d7d1f8a0b3898aabcabe43dc51e9f90306f7), ROM_BIOS(2)) // 27c512
ROM_END

static INPUT_PORTS_START(microtouch)
	PORT_START("TOUCH")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Touch screen") PORT_CHANGED_MEMBER(DEVICE_SELF, microtouch_device, touch, 0)
	PORT_START("TOUCH_X")
	PORT_BIT(0x3fff, 0x2000, IPT_LIGHTGUN_X) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(45) PORT_KEYDELTA(15)
	PORT_START("TOUCH_Y")
	PORT_BIT(0x3fff, 0x2000, IPT_LIGHTGUN_Y) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(45) PORT_KEYDELTA(15)
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
	if (m_output_valid)
	{
		transmit_register_setup(m_output);
		m_output_valid = false;
	}
}

const tiny_rom_entry *microtouch_device::device_rom_region() const
{
	return ROM_NAME(microtouch);
}
