// license:BSD-3-Clause
// copyright-holders:Frode van der Meeren
/***************************************************************************

	Tandberg TDV-2115 Terminal module, "Display Logic" version

	The "Display Logic" module provided the terminal functionality for
	most of Tandbergs TDV-2100 series products. Most notably, standalone
	it was available as the digital portion of the TDV-2115L terminal.
	Being divided across two boards, Tandberg order-numbers for the module
	is 960309 and 960310. These were connected together in reserved slots
	on a bigger backplane.

	Being a 1976-design, the module does not feature any CPU, but it does
	have an interface to an optional CPU module through the backplane of
	the machines it was installed in. For instance, the TDV-2114 computer
	or the TDV-2116 advanced terminal both has a CPU module featuring the
	Intel 8080 along with 2KB of RAM and up to 8KB of ROM. More could be
	added using additional memory modules. While the TDV-2115/16 only has
	enough space for two extra modules besides the terminal module, the
	TDV-2114 can fit up to 8 extra modules.

	Two keyboards exist, one using switches and logic to derive keypresses,
	the other using a standard circuit based on the 20-04592-013 chip by
	Key-Tronics. In both cases there is a translation ROM for deriving an
	ASCII character for each keypress, and the Key-Tronics version also has
	a ROM for sorting out key properties. For the logic-based keyboard all
	keys have the same properties.

	While the keyboard itself can make a sound when a key is pressed (the
	logic keyboard toggles a relay, the Key-Tronics keyboard has a buzzer),
	the terminal module itself has circuit to beep a dynamic speaker. The
	volume is selectable using a potentiometer on the front.

	For characters, the terminal uses up to four font-ROM, each containing
	sixteen 14x8 bitmaps. Typically three of these ROM slots will be filled,
	for ASCII character 0x20 to 0x7F. There is also two PROM used as logic-
	function lookup-tables for deriving at the correct RAM addresses.

	The UART on the board is the AY-5-1013, but the keyboard has priority
	over the UART in TTY CPU-less mode. All symbols received from keyboard
	and UART are considered chars, and control characters will be proccessed
	if sent to local. The path from keyboard/UART Rx to local is toggeled by
	the LINE key. Likewise, the path from keyboard to UART Tx is controlled
	by the TRANS key. In CPU mode, automatic paths are disabled and the code
	running on the CPU module will ultimately control the data-flow. This is
	done using strobes from the IO-port address-decoder on the CPU module
	itself. An interrupt is used to request action from the CPU, and there
	is also another interrupt triggering on the 50Hz VSync.

	It is also worth noting that the terminal module provides the clock base
	for the CPU module on actual hardware.

	Stobes:

	      Function:                         Hook:
	    * Get Rx char from UART             CPU module, IO E4 Read
	    * Send Tx data to UART              CPU module, IO E4 Write
	    * Get data from local at cursor++   CPU module, IO E5 Read
	    * Send char to local at cursor++    CPU module, IO E5 Write
	    * Get char from keyboard            CPU module, IO E6 Read
	    * Send data to local at cursor++    CPU module, IO E6 Write
	    * Get Terminal status               CPU module, IO E7 Read
	    * Set Terminal control              CPU module, IO E7 Write
	    * Get Interrupt status              CPU module, IO F6 Read
	    * Get UART status                   CPU module, IO F7 Read
	    * Process pending keyboard char     Keyboard, strobe
	    * Clear screen                      Keyboard, CLEAR keyswitch
	    * Toggle TRANSMIT                   Keyboard, TRANS keyswitch
	    * Toggle ON-LINE                    Keyboard, LINE keyswitch
	    * Force UART out high               Keyboard, BREAK keyswitch

	TODO:

	    * Add keyboard w/indicators
	    * Add selectable BAUD rate (110, 300, 600, 1200, 2400, 4800, 9600, 19200)
	    * Add missing DIP switches
	    * Add reset
	    * Add CPU interface
	    * Add interrupts
	    * Make DIP switches selectable

****************************************************************************/

#include "emu.h"
#include "disp_logic.h"

static constexpr XTAL DOT_CLOCK = XTAL(20'275'200);
static constexpr XTAL DOT_CLOCK_3 = DOT_CLOCK/3;

DEFINE_DEVICE_TYPE(TANDBERG_DISPLAY_LOGIC, tandberg_disp_logic_device, "tandberg_display_logic", "Tandberg TDV-2100 series Display Logic terminal module");

tandberg_disp_logic_device::tandberg_disp_logic_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock):
	device_t(mconfig, TANDBERG_DISPLAY_LOGIC, tag, owner, clock),
	m_screen(*this, "screen"),
	m_palette(*this, "palette"),
	m_font(*this, "display_font_rom"),
	m_vram(*this, "video_ram", 0x800, ENDIANNESS_LITTLE),
	m_beep(*this, "beep"),
	m_uart(*this, "uart"),
	m_uart_clock(*this, "uart_clock"),
	m_rs232(*this, "serial")
{
	speed_check = false;

	attribute = 0;
	frame_counter = 0;
	cursor_x = 0;
	cursor_y = 0;
	underline_input = false;
	video_enable = true;
	cursor_x_input = false;
	cursor_y_input = false;

	data_terminal_ready = false;
	request_to_send = false;
	rx_handshake = false;
	tx_handshake = false;
	break_key_held = false;

	                                  // Device settings
	                                  //
	no_parity = true;                 // Rear switch 1
	                                  //    If true, disables parity checking.
	                                  //
	even_parity = false;              // Rear switch 2
	                                  //    If true and Parity enabled, use even parity instead
	                                  //    of odd parity. No effect if parity disabled.
	                                  //
	two_stop_bits = false;            // Rear switch 3
	                                  //    If true, expect two stop bits instead of one.
	                                  //
	ext_echo = false;                 // Rear switch 4
	                                  //    If true, blocks local echo on DTR.
	                                  //
	auto_rx_handshake = true;         // Rear switch 5
	                                  //    If true, DTR loops to Rx handshake.
	                                  //
	auto_tx_handshake = true;         // Rear switch 6
	                                  //    If true, RTS loops to Tx handshake.
	                                  //
	full_inverse_video = false;       // Push-button toggle
	                                  //    Inverts the entire display if activated.

	                                  // Display 1 Settings
	                                  //
	// TODO: add keyboard             // DIP switches U39-1/2
	                                  //    Defines if ASCII handshake signal indicators will be
	                                  //    reset with the clear key, or the SYN control-character.
	                                  //
	dcd_handshake = false;            // DIP switches U39-3/4
	                                  //    Expect DCD as handshake for DTR. Otherwise, the
	                                  //    DSR signal will be expected instead.
	                                  //
	// TODO: Add CPU interface        // DIP switch U39-5
	                                  //    Enables CPU-mode. If enabled, automatic data-paths
	                                  //    in the module itself will be disabled.
	                                  //
	force_on_line = true;             // DIP switch U39-6
	                                  //    If enabled, DTR and RTS will be permanent active.
	                                  //    Otherwise toggled using the LINE and TRANS keys.
	                                  //
	need_tx_for_on_line_led = false;  // DIP switches U39-7/8
	                                  //    DTR + Handshake is needed for ON-LINE LED to light,
	                                  //    but if this is enabled RTS + CTS is needed too.
	                                  //
	auto_roll_up = true;              // DIP switch U39-9
	                                  //    Rolls text one line up if NewLine on lowest line.
	                                  //    If disabled, NewLine has no effect in this case.
	                                  //
	eight_bit_uart = true;            // DIP switch U39-10
	                                  //    If true, UART use 7 bits of data instead of 8.
	                                  //
	hold_line_key_for_rts = false;    // DIP switches U71-1/2
	                                  //    Pressing the LINE key will first raise DTR, and
	                                  //    immediately after handshake RTS will be raised if
	                                  //    the key is still held.
	                                  //
	                                  // DIP switches U71-2/3
	                                  //    Sets current-loop hardware voltage-clamping strategy.

	                                  // Display 2 Settings
	                                  //
	extend_dot7 = true;               // DIP switches U61-1/2
	                                  //    If true, the 9th column will be a copy of the 8th
	                                  //    column of each character. If false, the 9th column
	                                  //    will use the background colour.
	                                  //
	auto_cr_lf = true;                // DIP switch U61-3
	                                  //    Automatic CR/NL if a line is over-typed. Cursor will
	                                  //    stop on the 80th character of a line if disabled.
	                                  //
	underline_mode = false;           // DIP switch U61-4
	                                  //    If enabled, attributes will be disabled but each
	                                  //    value in video-ram will have a dedicated flag for
	                                  //    immediate underline. Otherwise underline will have
	                                  //    to be enabled using attribute-changes.
	                                  //
	blinking_cursor = false;          // DIP switches U61-5/6
	                                  //    Decides if the cursor is static or blinking.
	                                  //
	blank_attr_chars = true;          // DIP switch U61-7
	                                  //    Decides if attribute-change or underline characters
	                                  //    in underline-mode will be hidden or not.
	                                  //
	                                  // DIP switch U61-8
	                                  //    Unused.
	                                  //
	blank_ctrl_chars = true;          // DIP switch U61-9
	                                  //    Decides if control-character values are to be
	                                  //    hidden or not. (These characters can only be put on
	                                  //    the screen using raw writes over the CPU interface.)
	                                  //
	block_cursor = false;             // DIP switch U61-10
	                                  //    Decide if the cursor will be in the form of a block
	                                  //    or an underline.
}

///////////////////////////////////////////////////////////////////////////////
//
// General machine state
//

void tandberg_disp_logic_device::device_start()
{
	m_beep_trigger = timer_alloc(FUNC(tandberg_disp_logic_device::end_beep), this);
	m_speed_ctrl = timer_alloc(FUNC(tandberg_disp_logic_device::expire_speed_check), this);

	m_uart->write_cs(true);
	m_uart->write_nb1(eight_bit_uart);
	m_uart->write_nb2(true);
	m_uart->write_np(no_parity);
	m_uart->write_eps(even_parity);
	m_uart->write_tsb(two_stop_bits);

	if(force_on_line)
	{
		rs232_dtr_toggle();
		rs232_rts_toggle();
	}
}

void tandberg_disp_logic_device::device_reset()
{
	uint8_t attribute_chars[6] = {' ', '0', 'A', 'Q', 'a', 'q'};
	char_to_display(0x19);
	for(char c: "TDV-2115") char_to_display(c);
	char_to_display(0x0A);
	for(int l=0; l<2; l++){
		char_to_display(0x0A);
		for(int j=0; j<8; j++){
			char_to_display(0x0D);
			char_to_display(0x18);
			for(int k=0; k<3; k++){
				char_to_display(0x0E);
				char_to_display(attribute_chars[l*3 + k]);
				char_to_display(0x0F);
				for(int i=0; i<16; i++) data_to_display(16*j+i);
				char_to_display(0x18);
				char_to_display(0x18);
				char_to_display(0x18);
				char_to_display(0x0E);
				char_to_display(attribute_chars[5]);
				char_to_display(0x0F);
				char_to_display(0x18);
			}
			char_to_display(0x0A);
		}
	}
	char_to_display(0x10);
	char_to_display(23);
	char_to_display(20);
	for(char c: "Testing random cursor position") char_to_display(c);

	char_to_display(0x07);
}

///////////////////////////////////////////////////////////////////////////////
//
// Graphics and character data
//

void tandberg_disp_logic_device::char_to_display(uint8_t byte){
	if(cursor_y_input)
	{
		cursor_y = byte&0x7F;
		cursor_x_input = true;
		cursor_y_input = false;
	}
	else if(cursor_x_input)
	{
		cursor_x = byte&0x1F;
		cursor_x_input = false;
	}
	else
	{
		// Ordinary input
		if((byte&0x7F) < 0x20 || (byte&0x7F) == 0x7F)
		{
			// Control characters
			switch(byte&0x7F)
			{
				// STX, Turn off video
				case 0x02:
					video_enable = false;
					break;

				// ETX, Turn on video
				case 0x03:
					video_enable = true;
					break;

				// UL, Turn on Underline on new input
				case 0x0E:
					underline_input = true;
					break;

				// NO, Normal input (Underline on input off)
				case 0x0F:
					underline_input = false;
					break;

				// ERLIN, Erase line and move cursor to line start
				case 0x04:
					for(int i=0; i<80; i++)
					{
						m_vram[cursor_y*80 + i] = 0x00;
					}
					char_to_display(0x0D);
					break;

				// ERPAG, Clear screen and move cursor home
				case 0x19:
					for(int j=0; j<25; j++)
					{
						for(int i=0; i<80; i++)
						{
							m_vram[j*80+i] = 0x00;
						}
					}
					char_to_display(0x1D);
					break;

				// CURL, Cursor left
				case 0x08:
					if(cursor_x > 0)
					{
						cursor_x--;
					}
					break;

				// CURR, Cursor right
				case 0x18:
					advance_cursor();
					break;

				// CURUP, Cursor up
				case 0x1C:
					if(cursor_y > 0)
					{
						cursor_y--;
					}
					break;

				// CURD, Cursor down
				case 0x0B:
					if(cursor_y < 24)
					{
						cursor_y++;
					}
					break;

				// LF, Move cursor to next line
				case 0x0A:
					if(cursor_y < 24)
					{
						cursor_y++;
					}
					else if(auto_roll_up)
					{
						char_to_display(0x0C);
					}
					break;

				// CR, Carridge return
				case 0x0D:
					cursor_x = 0;
					break;

				// CURH, Cursor home
				case 0x1D:
					cursor_x = 0;
					cursor_y = 0;
					break;

				// ROLUP, Roll text up
				case 0x0C:
					for(int j=0; j<24; j++)
					{
						for(int i=0; i<80; i++)
						{
							m_vram[j*80+i] = m_vram[(j+1)*80+i];
						}
					}
					for(int i=0; i<80; i++)
					{
							m_vram[24*80+i] = 0x00;
					}
					break;

				// RLDWN, Roll text down
				case 0x17:
					for(int j=24; j>0; j--)
					{
						for(int i=0; i<80; i++)
						{
							m_vram[j*80+i] = m_vram[(j-1)*80+i];
						}
					}
					for(int i=0; i<80; i++)
					{
							m_vram[i] = 0x00;
					}
					break;

				// DLE, Set new cursor position
				case 0x10:
					cursor_y_input = true;
					break;

				// BEL, Sound beeper
				case 0x07:
					m_beep->set_state(1);
					m_beep_trigger->adjust(attotime::from_usec(70000));
					break;

				// ENQ, Light ENQIRY indicator
				case 0x05:
					// TODO
					break;

				// ACK, Light ACK indicator
				case 0x06:
					// TODO
					break;

				// NAK, Light NACK indicator
				case 0x15:
					// TODO
					break;

				// SYN, Clear ENQIRY/ACK/NACK indicators if DIP switches set accordingly
				case 0x16:
					// TODO
					break;

				default:
					break;
			}
		}
		else
		{
			data_to_display(byte);
		}
	}
}

void tandberg_disp_logic_device::data_to_display(uint8_t byte){
	if(underline_input)
	{
		byte = byte|0x80;
	}
	if (cursor_x < 80 && cursor_y < 25)
	{
		m_vram[cursor_y*80 + cursor_x] = byte;
	}
	advance_cursor();
}

void tandberg_disp_logic_device::advance_cursor()
{
	cursor_x++;
	if(cursor_x == 72 && !speed_check){
		char_to_display(0x07);
	}
	else if(cursor_x > 79)
	{
		if(auto_cr_lf)
		{
			char_to_display(0x0D);
			char_to_display(0x0A);
		}
		else
		{
			cursor_x = 79;
		}
	}
	speed_check = true;
	m_speed_ctrl->adjust(attotime::from_usec(16121));
}

TIMER_CALLBACK_MEMBER(tandberg_disp_logic_device::end_beep)
{
	m_beep->set_state(0);
}

TIMER_CALLBACK_MEMBER(tandberg_disp_logic_device::expire_speed_check)
{
	speed_check = false;
}

uint32_t tandberg_disp_logic_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	rgb_t const *const palette = m_palette->palette()->entry_list_raw();

	int const char_min_x = cliprect.min_x/9;
	int const char_max_x = cliprect.max_x/9;
	int const char_min_y = cliprect.min_y/14;
	int const char_max_y = cliprect.max_y/14;
	for(int char_nr_y = char_min_y; char_nr_y <= char_max_y; char_nr_y++)
	{
		// This is at the start of a char line
		for (int char_nr_x = char_min_x; char_nr_x <= char_max_x; char_nr_x++)
		{
			// This is at the start of a single character
			if(char_nr_x == 0 && char_nr_y == 0)
			{
				attribute = 0;
				frame_counter++;
			}

			int const in_line_char_nr = char_nr_x - 4;
			int const chr_x_pos = char_nr_x*9;
			int const chr_y_pos = char_nr_y*14;
			int const vram_address = in_line_char_nr + char_nr_y*80;    //NOTE: On real hardware, a pair of 4-bit PROM are used for this
			bool const draw_cursor = (cursor_x == in_line_char_nr && cursor_y == char_nr_y);
			bool const blink_strobe = frame_counter&0x08;

			for(int line = 0; line < 14; line++)
			{
				// This is at the start of a 9-dot raster-line segment in a single character
				uint8_t data = 0x00;
				int dot_nr8 = 0;
				int extra_intensity = 1;
				if(in_line_char_nr >= 0 && in_line_char_nr < 80)
				{
					int vram_data = m_vram[vram_address];

					// Pattern generator & character blanking
					if(video_enable && (vram_data&0x60 || !blank_ctrl_chars) && !(vram_data&0x80 && blank_attr_chars))
					{
						data = m_font->base()[((vram_data&0x7F)<<4) + line];
						if(extend_dot7)
						{
							dot_nr8 = data&0x01;
						}
					}

					// Pattern modifiers
					if(underline_mode)
					{
						// Note: The underline in underline-mode is not affected by blanking!
						if((vram_data&0x80) && line == 13)
						{
							data = 0xFF;
							dot_nr8 = 0x01;
						}
					}
					else if(video_enable) // attribute mode
					{
						if(vram_data&0x80)
						{
							attribute = (vram_data&0x70)>>4;
						}
						else
						{
							if(attribute == 2)
							{
								// Low intensity
								extra_intensity = 0;
							}
							else if(attribute == 3 && blink_strobe && !draw_cursor)
							{
								// Blink
								data = 0x00;
								dot_nr8 = 0x00;
							}
							else if(attribute == 4)
							{
								// Inverse
								data = ~data;
								dot_nr8 = ~dot_nr8;
							}
							else if(attribute == 5 && line == 13)
							{
								// Underline
								data = 0xFF;
								dot_nr8 = 0x01;
							}
							else if(attribute == 6)
							{
								// Invissible
								data = 0x00;
								dot_nr8 = 0x00;
							}
						}
					}

					if(draw_cursor && (line == 12 || block_cursor) && (!blinking_cursor || blink_strobe))
					{
						data = ~data;
						dot_nr8 = ~dot_nr8;
					}
				}

				if(full_inverse_video)
				{
					data = ~data;
					dot_nr8 = ~dot_nr8;
				}
				for(int dot = 0; dot < 8; dot++)
				{
					bitmap.pix(chr_y_pos + line, chr_x_pos + dot) = palette[((data>>(7-dot))&0x01)<<extra_intensity];
				}
				bitmap.pix(chr_y_pos + line, chr_x_pos + 8) = palette[(dot_nr8&0x01)<<extra_intensity];
			}
		}
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
//
// UART
//

void tandberg_disp_logic_device::rs232_rxd_w(int state)
{
	m_uart->write_si(state && data_terminal_ready && rx_handshake);
}

void tandberg_disp_logic_device::rs232_dcd_w(int state)
{
	if(dcd_handshake && !auto_rx_handshake)
	{
		rx_handshake = (bool)state;
	}
	// TODO: Update lamps
}

void tandberg_disp_logic_device::rs232_dsr_w(int state)
{
	if(!dcd_handshake && !auto_rx_handshake)
	{
		rx_handshake = (bool)state;
	}
	// TODO: Update lamps
}

void tandberg_disp_logic_device::rs232_cts_w(int state)
{
	if(!auto_tx_handshake)
	{
		tx_handshake = (bool)state;
	}
	// TODO: Update lamps
}

void tandberg_disp_logic_device::rs232_txd_w(int state)
{
	m_rs232->write_txd(state && request_to_send && tx_handshake && !break_key_held);
	m_uart->write_si(state && !(ext_echo && data_terminal_ready));
}

void tandberg_disp_logic_device::rs232_dtr_toggle()
{
	if(force_on_line)
	{
		data_terminal_ready = true;
	}
	else
	{
		if(rx_handshake)
		{
			data_terminal_ready = false;
		}
		else
		{
			data_terminal_ready = !data_terminal_ready;
		}
	}

	if(!data_terminal_ready)
	{
		request_to_send = false;
		m_rs232->write_rts(request_to_send);
	}
	m_rs232->write_dtr(data_terminal_ready);

	if(auto_rx_handshake)
	{
		rx_handshake = data_terminal_ready;
	}
	if(data_terminal_ready && hold_line_key_for_rts)
	{
		rs232_rts_toggle();
	}
	// TODO: Update lamps
}

void tandberg_disp_logic_device::rs232_rts_toggle()
{
	if(force_on_line)
	{
		request_to_send = true;
	}
	else
	{
		if(tx_handshake)
		{
			request_to_send = false;
		}
		else
		{
			request_to_send = !request_to_send;
		}
	}
	m_rs232->write_rts(request_to_send);

	if(auto_tx_handshake)
	{
		tx_handshake = request_to_send;
	}
	// TODO: Update lamps
}

void tandberg_disp_logic_device::rs232_ri_w(int state)
{
	// TODO: Ring indicator for CPU interface
}

void tandberg_disp_logic_device::uart_rx(int state)
{
	// TODO: Changed behaviour if CPU-mode DIP switch
	if(state)
	{
		char_to_display(m_uart->receive());
		m_uart->write_rdav(0);
	}
}

///////////////////////////////////////////////////////////////////////////////
//
// Driver config
//

void tandberg_disp_logic_device::device_add_mconfig(machine_config &mconfig)
{
	/* Video hardware */
	SCREEN(mconfig, m_screen, SCREEN_TYPE_RASTER, rgb_t::green());
	m_screen->set_raw(DOT_CLOCK, 999, 0, 783, 402, 0, 350);
	m_screen->set_screen_update(FUNC(tandberg_disp_logic_device::screen_update));
	PALETTE(mconfig, m_palette, palette_device::MONOCHROME_HIGHLIGHT);

	/* Sound */
	SPEAKER(mconfig, "mono").front_center();
	BEEP(mconfig, m_beep, 2000);
	m_beep->add_route(ALL_OUTPUTS, "mono", 1.0);

	RS232_PORT(mconfig, m_rs232, default_rs232_devices, nullptr);
	m_rs232->rxd_handler().set(FUNC(tandberg_disp_logic_device::rs232_rxd_w));
	m_rs232->dcd_handler().set(FUNC(tandberg_disp_logic_device::rs232_dcd_w));
	m_rs232->dsr_handler().set(FUNC(tandberg_disp_logic_device::rs232_dsr_w));
	m_rs232->cts_handler().set(FUNC(tandberg_disp_logic_device::rs232_cts_w));
	m_rs232->ri_handler().set(FUNC(tandberg_disp_logic_device::rs232_ri_w));
	AY51013(mconfig, m_uart);
	m_uart->write_so_callback().set(FUNC(tandberg_disp_logic_device::rs232_txd_w));
	m_uart->write_dav_callback().set(FUNC(tandberg_disp_logic_device::uart_rx));
	CLOCK(mconfig, m_uart_clock, DOT_CLOCK_3 / (11 * 4));
	m_uart_clock->signal_handler().set(m_uart, FUNC(ay51013_device::write_rcp));
	m_uart_clock->signal_handler().append(m_uart, FUNC(ay51013_device::write_tcp));
}

ROM_START(tdv2115l)
	ROM_REGION( 0x0800, "display_font_rom", ROMREGION_ERASEFF )
	ROM_LOAD( "960558-0 1.82s141.u18", 0x0200, 0x0200, CRC(002856D2) SHA1(D86AFC7190163B3C0D7A43516ADB14046B978A4F))
	ROM_LOAD( "960558-0 2.82s141.u19", 0x0400, 0x0200, CRC(422F1959) SHA1(AAD1B74ED194C486E53D7FE6469B15C407E08441))
	ROM_LOAD( "960558-0 3.82s141.u20", 0x0600, 0x0200, CRC(F662DF8E) SHA1(C585731612AA1BEA8D95223A49011099953CF34E))

	ROM_REGION( 0x0200, "keyboard_chars_rom", ROMREGION_ERASEFF )
	ROM_LOAD( "prom.82s147.z17", 0x0000, 0x0200, CRC(5455D48E) SHA1(B85021EFBFC794C0C21C1919333C2BF08785C330))

	ROM_REGION( 0x0100, "keyboard_params_rom", ROMREGION_ERASEFF )
	ROM_LOAD_NIB_LOW( "prom.82s129.z15", 0x0000, 0x0100, CRC(6497BC08) SHA1(A8985AE2C90F8E7361C3E4418314925B17E72B10))
ROM_END

const tiny_rom_entry *tandberg_disp_logic_device::device_rom_region() const
{
	return ROM_NAME(tdv2115l);
}
