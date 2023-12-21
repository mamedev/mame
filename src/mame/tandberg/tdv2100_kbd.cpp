// license:BSD-3-Clause
// copyright-holders:Frode van der Meeren
/***************************************************************************

	Tandberg TDV-2100 Series Keyboard

	Two keyboards exist, one using switches and logic to derive keypresses,
	the other based on an OEM design from Key Tronic Corp, based on capacitive
	switches and the 20-04592-013 30293E-013 chip by GI (AY-3-4592 derivative).
	In both cases there is a translation ROM for deriving an ASCII character
	for each keypress, and the Keytronic version also has a ROM for sorting
	out key properties. For the logic-based keyboard all keys have the same
	properties.

	The logic keyboard toggles a relay in order to make a typing-sound, while
	the Keytronic keyboard has a proper buzzer for this click.


	Keytronic keyboard key numbers (as printed on PCB):

				  1  2  3  4  5  6  7  8  9  10 11 12 13 14
		15 16 17  18   19 20 21 22 23 24 25 26 27 28 29 30 31   32  33  34
		35 36 37  38 39 40 41 42 43 44 45 46 47 48 49 50 51 52  53  54  55
		56 57 58   59 60 61 62 63 64 65 66 67 68 69 70 71 72    73  74  75
		76 77 78   79  80 81 82 83 84 85 86 87 88 89  90   91   92  93  94
		95 96 97          98          99              100       101 102 103


		   | 0      1   2   3   4   5   6   7   8   9   10  11  12  13  14
		---+------------------------------------------------------------------
		0  | 90/91* 90* 91  1   11  10  9   8   7   6   5   4   3   2
		1  | 79     79* 30  29  28  27  26  25  24  23  22  21  20  19
		2  | 59     18  50  49  48  47  46  45  44  43  42  41  40  39
		3  | 38/59* 38* 72  60  70  69  68  67  66  65  64  63  62  61
		4  |        12* 100 99  98  89  88  87  86  85  84  83  82  81  12
		5  |        13* 80  93  92  17  94  103 75  73  74  53  54  55  13
		6  |        14* 95  96  97  76  78  77  56  58  57  37  36  35  14
		7  |        71  51  31* 52          102 101 16  15  32  33  34  31/71*

					* Alternate positions, selectable by jumpers


	Keytronic keyboard key-id (bit7 = Shift, bit8 = Ctrl):

		   | 0   1   2   3   4   5   6   7   8   9   10  11  12  13  14
		---+---------------------------------------------------------------
		0  | Sh  7E* 76  6E  66  5E  56  4E  46  3E  36  2E  26  1E
		1  | Sh  7D* 75  6D  65  5D  55  4D  45  3D  35  2D  25  1D
		2  | Lk  7C  74  6C  64  5C  54  4C  44  3C  34  2C  24  1C
		3  | Ct  7B* 73  6B  63  5B  53  4B  43  3B  33  2B  23  1B
		4  |     7A* 72* 6A  62* 5A  52  4A  42  3A  32  2A  22  1A  Di
		5  |     79* 71  69* 61  59  51  49  41  39  31  29  21  19  Di
		6  |     78* 70  68  60  58  50  48  40  38  30  28  20  18  Di
		7  |     77  6F  67* 5F  57* 4F* 47* 3F  37  2F  27  1F  17  Di


				  6E 1E 26 2E 36 3E 46 4E 56 5E 66 Di Di Di
		2F 37 59  7C   1D 25 2D 35 3D 45 4D 55 5D 65 6D 75 Di   27  1F  17
		18 20 28  Ct 1C 24 2C 34 3C 44 4C 54 5C 64 6C 74 6F 5F  29  21  19
		40 30 38   Lk 6B 1B 23 2B 33 3B 43 4B 53 5B 63 77 73    39  31  41
		58 48 50   Sh  71 1A 22 2A 32 3A 42 4A 52 5A  Sh   76   61  69* 51
		70 68 60          62*         6A              72*       3F  47* 49

					* Not verified, assumed value


	Input strobes:

		  Function:                         Hook:
		* Set indicator WAIT                Display Logic module, lamp WAIT signal
		* Set indicator ON LINE             Display Logic module, lamp ON LINE signal
		* Set indicator CARRIER             Display Logic module, lamp CARRIER signal
		* Set indicator ERROR               Display Logic module, lamp ERROR signal
		* Set indicator ENQUIRE             Display Logic module, lamp ENQUIRE signal
		* Set indicator ACK                 Display Logic module, lamp ACK signal
		* Set indicator NAK                 Display Logic module, lamp NAK signal

	Output strobes:

		  Function:                         Hook:
		* Pending character                 Display Logic module, process pending keyboard char
		* CLEAR keyswitch                   Display Logic module, clear screen
		* TRANS keyswitch                   Display Logic module, toggle TRANSMIT
		* LINE keyswitch                    Display Logic module, toggle ON-LINE
		* BREAK keyswitch                   Display Logic module, force UART out high


	TODO:
		* Add click sound and volume potentiometer
		* Add CRT brightness-adjust potentiometer
		* Expose 8-bit output mode and add jumpers for layout options

****************************************************************************/

#include "emu.h"
#include "tdv2100_kbd.h"
#include "utf8.h"

#include "tdv2100kbd.lh"

DEFINE_DEVICE_TYPE(TANDBERG_TDV2100_KEYBOARD, tandberg_tdv2100_keyboard_device, "tandberg_tdv2100_keyboard", "Tandberg TDV-2100 series Keyboard");

tandberg_tdv2100_keyboard_device::tandberg_tdv2100_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock):
	device_t(mconfig, TANDBERG_TDV2100_KEYBOARD, tag, owner, clock),
	m_keymap(*this, "keyboard_chars_rom"),
	m_keyparams(*this, "keyboard_params_rom"),
	m_matrix(*this, "X%u", 0),
	m_scan_clock(*this, "scan_clock"),
	m_sw_all_cap(*this, "sw_all_cap"),
	m_key_repeat_delay(*this, "key_repeat_delay"),
	m_key_repeat_rate(*this, "key_repeat_rate"),
	m_online_led(*this, "led_online"),
	m_carrier_led(*this, "led_carrier"),
	m_error_led(*this, "led_error"),
	m_enquiry_led(*this, "led_enquiry"),
	m_ack_led(*this, "led_ack"),
	m_nak_led(*this, "led_nak"),
	m_wait_led(*this, "led_wait"),
	m_shiftlock_led(*this, "led_shiftlock"),
	m_write_kstr_cb(*this),
	m_write_cleark_cb(*this),
	m_write_linek_cb(*this),
	m_write_transk_cb(*this),
	m_write_break_cb(*this),
	m_column_counter(0),
	m_shift(false),
	m_shift_lock(false),
	m_control(false),
	m_char_buffer(0x00),
	m_key_nr_in_buffer(0xff),
	m_8_bit_output(false)           // Hardwiered by PCB-trace jumpers 37-39 and 40-42, set at factory
	                                //   true:
	                                //     Use parameter PROM bit 2 to get which keys are inhibited
	                                //     Replace TRANS key strobe with extra character data-bit
	                                //   false:
	                                //     Ignore parameter PROM bit 2
	                                //     Use msb of char-map PROM to get which keys are inhibited
{
	std::fill(std::begin(m_keystate), std::end(m_keystate), 0);
}

///////////////////////////////////////////////////////////////////////////////
//
// General machine state
//

void tandberg_tdv2100_keyboard_device::device_start()
{
	m_key_repeat_trigger = timer_alloc(FUNC(tandberg_tdv2100_keyboard_device::key_repeat), this);
	m_online_led.resolve();
	m_carrier_led.resolve();
	m_error_led.resolve();
	m_enquiry_led.resolve();
	m_ack_led.resolve();
	m_nak_led.resolve();
	m_wait_led.resolve();
	m_shiftlock_led.resolve();

	save_item(NAME(m_shift_lock));
	save_item(NAME(m_column_counter));
	save_item(NAME(m_keystate));
	save_item(NAME(m_shift));
	save_item(NAME(m_control));
	save_item(NAME(m_char_buffer));
	save_item(NAME(m_key_nr_in_buffer));
}

///////////////////////////////////////////////////////////////////////////////
//
// Key Scanning
//

void tandberg_tdv2100_keyboard_device::scan_next_column(int state)
{
	if(state)
	{
		int column = m_matrix[m_column_counter]->read();

		for(int row_counter=0; row_counter<8; row_counter++)
		{
			if(BIT(column, row_counter) != BIT(m_keystate[m_column_counter], row_counter))
			{
				// This simulates the enumeration done by the internal LUT of the 20-04592-013
				uint8_t key_nr = (~((m_column_counter-1)*8 + row_counter + 1))&0xff;
				if(BIT(column, row_counter))
				{
					switch(m_column_counter)
					{
						case 0:
							switch(row_counter)
							{
								case 0:
								case 1:
									m_keystate[m_column_counter] |= 1<<row_counter;     // In case the second shift key is let go on the same scan
									m_shift = true;
									break;

								case 2:
									m_shift_lock = !m_shift_lock;
									m_shiftlock_led = !m_shift_lock;
									break;

								case 3:
									m_control = true;
									break;
							}
							break;

						case 14:
							switch(row_counter)
							{
								case 4:
									m_write_cleark_cb(1);
									break;

								case 5:
									if(!m_8_bit_output)
									{
										m_write_transk_cb(1);
									}
									break;

								case 6:
									m_write_linek_cb(1);
									break;

								case 7:
									m_write_break_cb(1);
									break;
							}
							break;

						default:
							new_keystroke(key_nr, (m_shift||m_shift_lock), m_control);
					}
				}
				else
				{
					switch(m_column_counter)
					{
						case 0:
							switch(row_counter)
							{
								case 0:
								case 1:
									m_keystate[m_column_counter] &= ~(1<<row_counter);  // Turn off released shift key
									m_shift = m_keystate[m_column_counter]&0x03;	    // Keep shift state if the other shift key is still down
									break;

								case 3:
									m_control = false;
									break;
							}
							break;

						case 14:
							switch(row_counter)
							{
								case 4:
									m_write_cleark_cb(0);
									break;

								case 5:
									if(!m_8_bit_output)
									{
										m_write_transk_cb(0);
									}
									break;

								case 6:
									m_write_linek_cb(0);
									break;

								case 7:
									m_write_break_cb(0);
									break;
							}
							break;

						default:
							if(key_nr == m_key_nr_in_buffer)
							{
								m_key_repeat_trigger->adjust(attotime::never);
							}
					}
				}
			}
		}
		m_keystate[m_column_counter] = column;

		m_column_counter++;
		if(m_column_counter >= 15)
		{
			m_column_counter = 0;
		}
	}
}

void tandberg_tdv2100_keyboard_device::new_keystroke(uint8_t key_nr, bool shift, bool control)
{
	uint8_t param = m_keyparams[(key_nr&0x7f) | ((control) ? 0x80 : 0x00)];
	bool all_caps = !(m_sw_all_cap->read());
	shift = (shift || ((param&0x01) && all_caps));
	uint8_t key = m_keymap[(key_nr&0x07f) | ((shift) ? 0x080 : 0x000) | ((control) ? 0x100 : 0x000)];

	bool inhibit = (key&0x80);
	if(m_8_bit_output)
	{
		inhibit = (param&0x04);
	}

	if(!inhibit)
	{
		m_key_nr_in_buffer = key_nr;
		m_char_buffer = key&0x7f;
		if(m_8_bit_output)
		{
			m_write_transk_cb((key>>7)&0x01);
		}
		key_trigger();
		if(param&0x02)
		{
			int key_repeat_delay_ms = 500 + m_key_repeat_delay->read()*750/100;    // 500ms -> 1250ms
			m_key_repeat_trigger->adjust(attotime::from_msec(key_repeat_delay_ms));
		}
	}
}

TIMER_CALLBACK_MEMBER(tandberg_tdv2100_keyboard_device::key_repeat)
{
	key_trigger();
	int key_repeat_rate_hz = 10 + m_key_repeat_rate->read()/5;    // 10Hz -> 30Hz
	m_key_repeat_trigger->adjust(attotime::from_hz(key_repeat_rate_hz));
}

///////////////////////////////////////////////////////////////////////////////
//
// Strobes
//

void tandberg_tdv2100_keyboard_device::key_trigger()
{
	m_write_kstr_cb(0x80|m_char_buffer);
	m_write_kstr_cb(m_char_buffer);
}

void tandberg_tdv2100_keyboard_device::waitl_w(int state)
{
	m_wait_led = !state;
}

void tandberg_tdv2100_keyboard_device::onlil_w(int state)
{
	m_online_led = !state;
}

void tandberg_tdv2100_keyboard_device::carl_w(int state)
{
	m_carrier_led = !state;
}

void tandberg_tdv2100_keyboard_device::errorl_w(int state)
{
	m_error_led = !state;
}

void tandberg_tdv2100_keyboard_device::enql_w(int state)
{
	m_enquiry_led = !state;
}

void tandberg_tdv2100_keyboard_device::ackl_w(int state)
{
	m_ack_led = !state;
}

void tandberg_tdv2100_keyboard_device::nakl_w(int state)
{
	m_nak_led = !state;
}

///////////////////////////////////////////////////////////////////////////////
//
// Driver config
//

void tandberg_tdv2100_keyboard_device::device_add_mconfig(machine_config &mconfig)
{
	CLOCK(mconfig, m_scan_clock, 8235);
	m_scan_clock->signal_handler().set(FUNC(tandberg_tdv2100_keyboard_device::scan_next_column));

	mconfig.set_default_layout(layout_tdv2100kbd);
}

static INPUT_PORTS_START( tdv2115l )

	PORT_START("key_repeat_delay")
		PORT_ADJUSTER(40, "Key-repeat delay (500ms - 1250ms)")

	PORT_START("key_repeat_rate")
		PORT_ADJUSTER(50, "Key-repeat rate (10Hz - 30Hz)")

	PORT_START("sw_all_cap")
		PORT_CONFNAME(0x1, 0x1, "ALL CAP")
			PORT_CONFSETTING(0x1, DEF_STR( Off ))
			PORT_CONFSETTING(0x0, DEF_STR( On ))

	PORT_START("X0")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_NAME("SHIFT (Right)")      PORT_CODE(KEYCODE_RSHIFT)       PORT_CHAR(UCHAR_SHIFT_1)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_NAME("SHIFT (Left)")       PORT_CODE(KEYCODE_LSHIFT)       PORT_CHAR(UCHAR_SHIFT_1)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_NAME("SHIFT LOCK")         PORT_CODE(KEYCODE_CAPSLOCK)     PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_NAME("CTRL")               PORT_CODE(KEYCODE_LCONTROL)     PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X1")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                   PORT_CODE(KEYCODE_1)            PORT_CHAR('1') PORT_CHAR('!')
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_BACKSLASH)    PORT_CHAR(':') PORT_CHAR('*')

	PORT_START("X2")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_NAME("RETURN")             PORT_CODE(KEYCODE_ENTER_PAD)    PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD))
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_EQUALS)       PORT_CHAR('@') PORT_CHAR('`')
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_CLOSEBRACE)   PORT_CHAR(';') PORT_CHAR('+')
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_NAME("DEL")                PORT_CODE(KEYCODE_DEL)          PORT_CHAR(UCHAR_MAMEKEY(DEL))
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_NAME("Z")                  PORT_CODE(KEYCODE_Z)            PORT_CHAR('z') PORT_CHAR('Z')
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_F9)           PORT_CHAR(UCHAR_MAMEKEY(F9))
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_NAME("LF")                 PORT_CODE(KEYCODE_ENTER)        PORT_CHAR(13)

	PORT_START("X3")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_NAME("ER LINE")            PORT_CODE(KEYCODE_F11)          PORT_CHAR(UCHAR_MAMEKEY(F11))
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_TILDE)        PORT_CHAR('^') PORT_CHAR(0x00ac)    // Logic NOT
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_NAME(u8"Å")                PORT_CODE(KEYCODE_OPENBRACE)    PORT_CHAR(0x00e5) PORT_CHAR(0x00c5)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_NAME("A")                  PORT_CODE(KEYCODE_A)            PORT_CHAR('a') PORT_CHAR('A')
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_NAME("(Space)")            PORT_CODE(KEYCODE_SPACE)        PORT_CHAR(' ')
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_DOWN)         PORT_CHAR(UCHAR_MAMEKEY(DOWN))
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X4")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_NAME("(Blank 3)")
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_SLASH)        PORT_CHAR('-') PORT_CHAR('=')
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_NAME("P")                  PORT_CODE(KEYCODE_P)            PORT_CHAR('p') PORT_CHAR('P')
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_NAME(u8"Æ")                PORT_CODE(KEYCODE_QUOTE)        PORT_CHAR(0x00e6) PORT_CHAR(0x00c6)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_NAME("0 (NumPad)")         PORT_CODE(KEYCODE_0_PAD)        PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_F10)          PORT_CHAR(UCHAR_MAMEKEY(F10))
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_NAME("CR")                 PORT_CODE(KEYCODE_BACKSPACE)    PORT_CHAR(10)

	PORT_START("X5")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_NAME("(Blank 2)")
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_0)            PORT_CHAR('0') PORT_CHAR('_')
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_NAME("O")                  PORT_CODE(KEYCODE_O)            PORT_CHAR('o') PORT_CHAR('O')
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_NAME(u8"Ø")                PORT_CODE(KEYCODE_COLON)        PORT_CHAR(0x00f8) PORT_CHAR(0x00d8)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_MINUS)        PORT_CHAR('/') PORT_CHAR('?')
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_F3)           PORT_CHAR(UCHAR_MAMEKEY(F3))
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_LEFT)         PORT_CHAR(UCHAR_MAMEKEY(LEFT))
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X6")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_NAME("(Blank 1)")
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_9)            PORT_CHAR('9') PORT_CHAR(')')
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_NAME("I")                  PORT_CODE(KEYCODE_I)            PORT_CHAR('i') PORT_CHAR('I')
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_NAME("L")                  PORT_CODE(KEYCODE_L)            PORT_CHAR('l') PORT_CHAR('L')
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_STOP)         PORT_CHAR('.') PORT_CHAR('>')
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_NAME(". (NumPad)")         PORT_CODE(KEYCODE_DEL_PAD)      PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_RIGHT)        PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X7")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_PGDN)         PORT_CHAR(UCHAR_MAMEKEY(PGDN))
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_8)            PORT_CHAR('8') PORT_CHAR('(')
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_NAME("U")                  PORT_CODE(KEYCODE_U)            PORT_CHAR('u') PORT_CHAR('U')
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_NAME("K")                  PORT_CODE(KEYCODE_K)            PORT_CHAR('k') PORT_CHAR('K')
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_COMMA)        PORT_CHAR(',') PORT_CHAR('<')
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_NAME("- (NumPad)")         PORT_CODE(KEYCODE_MINUS_PAD)    PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD))
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_NAME("Cursor Home")        PORT_CODE(KEYCODE_HOME)         PORT_CHAR(UCHAR_MAMEKEY(HOME))
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("X8")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_PGUP)         PORT_CHAR(UCHAR_MAMEKEY(PGUP))
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_7)            PORT_CHAR('7') PORT_CHAR('`')
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_NAME("Y")                  PORT_CODE(KEYCODE_Y)            PORT_CHAR('y') PORT_CHAR('Y')
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_NAME("J")                  PORT_CODE(KEYCODE_J)            PORT_CHAR('j') PORT_CHAR('J')
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_NAME("M")                  PORT_CODE(KEYCODE_M)            PORT_CHAR('m') PORT_CHAR('M')
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_NAME("3 (NumPad)")         PORT_CODE(KEYCODE_3_PAD)        PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_F7)           PORT_CHAR(UCHAR_MAMEKEY(F7))
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_NAME("SPACE (NumPad)")

	PORT_START("X9")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_NAME("VIDEO ON")           PORT_CODE(KEYCODE_NUMLOCK)      PORT_CHAR(UCHAR_MAMEKEY(NUMLOCK))
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_6)            PORT_CHAR('6') PORT_CHAR('&')
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_NAME("T")                  PORT_CODE(KEYCODE_T)            PORT_CHAR('t') PORT_CHAR('T')
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_NAME("H")                  PORT_CODE(KEYCODE_H)            PORT_CHAR('h') PORT_CHAR('H')
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_NAME("N")                  PORT_CODE(KEYCODE_N)            PORT_CHAR('n') PORT_CHAR('N')
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_NAME("1 (NumPad)")         PORT_CODE(KEYCODE_1_PAD)        PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_F8)           PORT_CHAR(UCHAR_MAMEKEY(F8))
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_F2)           PORT_CHAR(UCHAR_MAMEKEY(F2))

	PORT_START("X10")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_NAME("VIDEO OFF")          PORT_CODE(KEYCODE_SCRLOCK)      PORT_CHAR(UCHAR_MAMEKEY(SCRLOCK))
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_5)            PORT_CHAR('5') PORT_CHAR('%')
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_NAME("R")                  PORT_CODE(KEYCODE_R)            PORT_CHAR('r') PORT_CHAR('R')
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_NAME("G")                  PORT_CODE(KEYCODE_G)            PORT_CHAR('g') PORT_CHAR('G')
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_NAME("B")                  PORT_CODE(KEYCODE_B)            PORT_CHAR('b') PORT_CHAR('B')
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_NAME("2 (NumPad)")         PORT_CODE(KEYCODE_2_PAD)        PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_UP)           PORT_CHAR(UCHAR_MAMEKEY(UP))
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_F1)           PORT_CHAR(UCHAR_MAMEKEY(F1))

	PORT_START("X11")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_NAME("UNDER LINE")         PORT_CODE(KEYCODE_INSERT)       PORT_CHAR(UCHAR_MAMEKEY(INSERT))
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_4)            PORT_CHAR('4') PORT_CHAR('$')
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_NAME("E")                  PORT_CODE(KEYCODE_E)            PORT_CHAR('e') PORT_CHAR('E')
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_NAME("F")                  PORT_CODE(KEYCODE_F)            PORT_CHAR('f') PORT_CHAR('F')
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_NAME("V")                  PORT_CODE(KEYCODE_V)            PORT_CHAR('v') PORT_CHAR('V')
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_NAME("4 (NumPad)")         PORT_CODE(KEYCODE_4_PAD)        PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_F6)           PORT_CHAR(UCHAR_MAMEKEY(F6))
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_NAME("7 (NumPad)")         PORT_CODE(KEYCODE_7_PAD)        PORT_CHAR(UCHAR_MAMEKEY(7_PAD))

	PORT_START("X12")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_NAME("NORM")               PORT_CODE(KEYCODE_RCONTROL)     PORT_CHAR(UCHAR_MAMEKEY(RCONTROL))
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_3)            PORT_CHAR('3') PORT_CHAR(0x00a3)    // £
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_NAME("W")                  PORT_CODE(KEYCODE_W)            PORT_CHAR('w') PORT_CHAR('W')
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_NAME("D")                  PORT_CODE(KEYCODE_D)            PORT_CHAR('d') PORT_CHAR('D')
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_NAME("C")                  PORT_CODE(KEYCODE_C)            PORT_CHAR('c') PORT_CHAR('C')
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_NAME("5 (NumPad)")         PORT_CODE(KEYCODE_5_PAD)        PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_F5)           PORT_CHAR(UCHAR_MAMEKEY(F5))
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_NAME("8 (NumPad)")         PORT_CODE(KEYCODE_8_PAD)        PORT_CHAR(UCHAR_MAMEKEY(8_PAD))

	PORT_START("X13")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_NAME("ER PAGE")            PORT_CODE(KEYCODE_F12)          PORT_CHAR(UCHAR_MAMEKEY(F12))
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_2)            PORT_CHAR('2') PORT_CHAR('"')
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_NAME("Q")                  PORT_CODE(KEYCODE_Q)            PORT_CHAR('q') PORT_CHAR('Q')
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_NAME("S")                  PORT_CODE(KEYCODE_S)            PORT_CHAR('s') PORT_CHAR('S')
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_NAME("X")                  PORT_CODE(KEYCODE_X)            PORT_CHAR('x') PORT_CHAR('X')
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_NAME("6 (NumPad)")         PORT_CODE(KEYCODE_6_PAD)        PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_F4)           PORT_CHAR(UCHAR_MAMEKEY(F4))
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_NAME("9 (NumPad)")         PORT_CODE(KEYCODE_9_PAD)        PORT_CHAR(UCHAR_MAMEKEY(9_PAD))

	PORT_START("X14")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_NAME("CLEAR")              PORT_CODE(KEYCODE_ESC)          PORT_CHAR(UCHAR_MAMEKEY(ESC))
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_NAME("TRANS")              PORT_CODE(KEYCODE_RALT)         PORT_CHAR(UCHAR_MAMEKEY(RALT))
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_NAME("LINE")               PORT_CODE(KEYCODE_LALT)         PORT_CHAR(UCHAR_MAMEKEY(LALT))
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD)    PORT_NAME("BREAK")              PORT_CODE(KEYCODE_PAUSE)        PORT_CHAR(UCHAR_MAMEKEY(PAUSE))

INPUT_PORTS_END

ioport_constructor tandberg_tdv2100_keyboard_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( tdv2115l );
}

ROM_START( tdv2115l )
	ROM_REGION( 0x0200, "keyboard_chars_rom", ROMREGION_ERASEFF )
	ROM_LOAD( "prom.82s147.z17", 0x0000, 0x0200, CRC(5455d48e) SHA1(b85021efbfc794c0c21c1919333c2bf08785c330))

	ROM_REGION( 0x0100, "keyboard_params_rom", ROMREGION_ERASEFF )
	ROM_LOAD_NIB_LOW( "prom.82s129.z15", 0x0000, 0x0100, CRC(6497bc08) SHA1(a8985ae2c90f8e7361c3e4418314925b17e72b10))
ROM_END

const tiny_rom_entry *tandberg_tdv2100_keyboard_device::device_rom_region() const
{
	return ROM_NAME( tdv2115l );
}
