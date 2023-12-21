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

	The terminal module has circuit to beep a dynamic speaker. The volume
	is selectable using a potentiometer on the front. This beeps a 2KHz
	short tone on manual typing at column 72, as well as on the BEL ASCII
	control-code. It is not possible to turn this entirely off.

	For characters, the terminal uses up to four font-ROMs, each containing
	thirty-two 14x8 bitmaps. Typically three of these ROM slots will be filled,
	for ASCII character 0x20 to 0x7f. There is also two PROM used as logic-
	function lookup-tables for deriving at the correct RAM addresses.

	The UART on the board is the AY-5-1013, and the keyboard has priority
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


	Input stobes:

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
	    * Process pending keyboard char     Keyboard, pending character
	    * Clear screen                      Keyboard, CLEAR keyswitch
	    * Toggle TRANSMIT                   Keyboard, TRANS keyswitch
	    * Toggle ON-LINE                    Keyboard, LINE keyswitch
	    * Force UART out high               Keyboard, BREAK keyswitch

	Output strobes:

	      Function:                         Hook:
	    * VSync                             CPU module, Interrupt 1
	    * State change                      CPU module, Interrupt 3
	    * WAIT lamp                         Keyboard, WAIT indicator
	    * ON LINE lamp                      Keyboard, ON LINE indicator
	    * CARRIER lamp                      Keyboard, CARRIER indicator
	    * ERROR lamp                        Keyboard, ERROR indicator
	    * ENQUIRE lamp                      Keyboard, ENQUIRE indicator
	    * ACK lamp                          Keyboard, ACK indicator
	    * NAK lamp                          Keyboard, NAK indicator


	TODO:

	    * Add CPU interface and strobes
	    * Add CPU interrupts

****************************************************************************/

#include "emu.h"
#include "tdv2100_disp_logic.h"

static constexpr XTAL DOT_CLOCK = XTAL(20'275'200);
static constexpr XTAL DOT_CLOCK_3 = DOT_CLOCK/3;

DEFINE_DEVICE_TYPE(TANDBERG_TDV2100_DISPLAY_LOGIC, tandberg_tdv2100_disp_logic_device, "tandberg_tdv2100_disp_logic", "Tandberg TDV-2100 series Display Logic terminal module");

tandberg_tdv2100_disp_logic_device::tandberg_tdv2100_disp_logic_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock):
	device_t(mconfig, TANDBERG_TDV2100_DISPLAY_LOGIC, tag, owner, clock),
	m_screen(*this, "screen"),
	m_palette(*this, "palette"),
	m_font(*this, "display_font_rom"),
	m_addr_offsets(*this, "row_addr_offsets"),
	m_vram(*this, "video_ram", 0x800, ENDIANNESS_LITTLE),
	m_beep(*this, "beep"),
	m_uart(*this, "uart"),
	m_uart_clock(*this, "uart_clock"),
	m_rs232(*this, "serial"),
	m_sw_invert_video(*this, "sw_invert_video"),
	m_sw_rs232_baud(*this, "sw_rs232_baud"),
	m_sw_rs232_settings(*this, "sw_rs232_settings"),
	m_dsw_u39(*this, "dsw_u39"),
	m_dsw_u73(*this, "dsw_u73"),
	m_dsw_u61(*this, "dsw_u61"),
	m_write_waitl_cb(*this),
	m_write_onlil_cb(*this),
	m_write_carl_cb(*this),
	m_write_errorl_cb(*this),
	m_write_enql_cb(*this),
	m_write_ackl_cb(*this),
	m_write_nakl_cb(*this),
	m_vblank_state(false),
	m_speed_check(false),
	m_attribute(0),
	m_char_max_row(0),
	m_char_max_col(0),
	m_rx_handshake(false),
	m_tx_handshake(false),
	m_clear_key_held(false),
	m_line_key_held(false),
	m_trans_key_held(false),
	m_break_key_held(false)
{}

///////////////////////////////////////////////////////////////////////////////
//
// General machine state
//

void tandberg_tdv2100_disp_logic_device::device_start()
{
	m_beep_trigger = timer_alloc(FUNC(tandberg_tdv2100_disp_logic_device::end_beep), this);
	m_speed_ctrl = timer_alloc(FUNC(tandberg_tdv2100_disp_logic_device::expire_speed_check), this);

	save_item(NAME(m_page_roll));
	save_item(NAME(m_cursor_row));
	save_item(NAME(m_cursor_col));
	save_item(NAME(m_cursor_row_input));
	save_item(NAME(m_cursor_col_input));
	save_item(NAME(m_underline_input));
	save_item(NAME(m_video_enable));
	save_item(NAME(m_speed_check));
	save_item(NAME(m_data_terminal_ready));
	save_item(NAME(m_request_to_send));
	save_item(NAME(m_rx_handshake));
	save_item(NAME(m_tx_handshake));
	save_item(NAME(m_frame_counter));
	save_item(NAME(m_vblank_state));
	save_item(NAME(m_attribute));
	save_item(NAME(m_char_max_row));
	save_item(NAME(m_char_max_col));
	save_item(NAME(m_clear_key_held));
	save_item(NAME(m_line_key_held));
	save_item(NAME(m_trans_key_held));
	save_item(NAME(m_break_key_held));
	save_item(NAME(m_data_pending_kbd));
	save_item(NAME(m_data_kbd));
}

void tandberg_tdv2100_disp_logic_device::device_reset()
{
	m_cursor_row_input = false;
	m_cursor_col_input = false;
	m_page_roll = 0;
	char_to_display(0x19);
	m_frame_counter = 0;
	m_underline_input = false;
	m_video_enable = true;
	m_data_terminal_ready = false;
	m_request_to_send = false;
	m_data_pending_kbd = false;

	m_uart->write_xr(1);
	m_uart->write_xr(0);
	clear_key_handler();

	bool force_on_line = BIT(m_dsw_u39->read(), 5);
	set_uart_state_from_switches();
	if(force_on_line)
	{
		// Use conditional catch to force lines active
		clock_on_line_flip_flop();
		clock_transmit_flip_flop();
	}
	update_all_rs232_signal_paths();
}

///////////////////////////////////////////////////////////////////////////////
//
// Graphics and character data
//

void tandberg_tdv2100_disp_logic_device::char_to_display(uint8_t byte)
{
	// Note: There are no checks in hardware to catch an invalid cursor-position.
	//       To clear an invalid cursor placement, the cursor-movement mechanism
	//       may eventually catch it later, but it can also be manually sorted
	//       out by pressing the CR and/or HOME key.
	if(m_cursor_row_input)
	{
		place_cursor(byte, m_cursor_col);
		m_cursor_col_input = true;
		m_cursor_row_input = false;
	}
	else if(m_cursor_col_input)
	{
		place_cursor(m_cursor_row, byte);
		m_cursor_col_input = false;
	}
	else
	{
		// Ordinary input
		if((byte&0x7f) < 0x20 || (byte&0x7f) == 0x7f)
		{
			// Control characters
			int u39 = m_dsw_u39->read();
			bool auto_roll_up = !BIT(u39, 8);
			bool clear_ascii_hs_by_cleark = ((u39&0x003) == 0x001);
			switch(byte&0x7f)
			{
				// STX, Turn off video
				case 0x02:
					m_video_enable = false;
					break;

				// ETX, Turn on video
				case 0x03:
					m_video_enable = true;
					break;

				// UL, Turn on Underline on new input
				case 0x0e:
					m_underline_input = true;
					break;

				// NO, Normal input (Underline on input off)
				case 0x0f:
					m_underline_input = false;
					break;

				// ERLIN, Erase line and move cursor to line start
				case 0x04:
					erase_row(m_cursor_row);
					char_to_display(0x0d);
					break;

				// ERPAG, Clear screen and move cursor home
				case 0x19:
					for(int row=0; row<25; row++)
					{
						erase_row(row);
					}
					char_to_display(0x1d);
					break;

				// CURL, Cursor left
				case 0x08:
					if(m_cursor_col > 0)
					{
						place_cursor(m_cursor_row, m_cursor_col-1);
					}
					break;

				// CURR, Cursor right
				case 0x18:
					advance_cursor();
					break;

				// CURUP, Cursor up
				case 0x1c:
					if(m_cursor_row > 0)
					{
						place_cursor(m_cursor_row-1, m_cursor_col);
					}
					break;

				// CURD, Cursor down
				case 0x0b:
					if((m_cursor_row&0x18) != 24)
					{
						place_cursor(m_cursor_row+1, m_cursor_col);
					}
					break;

				// LF, Move cursor to next line
				case 0x0a:
					if((m_cursor_row&0x18) == 24 && auto_roll_up)
					{
						char_to_display(0x0c);
					}
					char_to_display(0x0b);
					break;

				// CR, Carridge return
				case 0x0d:
					place_cursor(m_cursor_row, 0);
					break;

				// CURH, Cursor home
				case 0x1d:
					place_cursor(0, 0);
					break;

				// ROLUP, Roll text up
				case 0x0c:
					if((m_page_roll&0x18) != 24)
					{
						m_page_roll++;
					}
					else
					{
						m_page_roll = 0;
					}
					erase_row(24);
					break;

				// RLDWN, Roll text down
				case 0x17:
					if(m_page_roll > 0)
					{
						m_page_roll--;
					}
					else
					{
						m_page_roll = 24;
					}
					erase_row(0);
					break;

				// DLE, Set new cursor position
				case 0x10:
					m_cursor_row_input = true;
					break;

				// BEL, Sound beeper
				case 0x07:
					m_beep->set_state(1);
					m_beep_trigger->adjust(attotime::from_usec(70000));
					break;

				// ENQ, Light ENQIRY indicator
				case 0x05:
					m_write_enql_cb(1);
					break;

				// ACK, Light ACK indicator
				case 0x06:
					m_write_ackl_cb(1);
					break;

				// NAK, Light NACK indicator
				case 0x15:
					m_write_nakl_cb(1);
					break;

				// SYN, Clear ENQIRY/ACK/NACK indicators if DIP switches set accordingly
				case 0x16:
					if(!clear_ascii_hs_by_cleark)
					{
						m_write_enql_cb(0);
						m_write_ackl_cb(0);
						m_write_nakl_cb(0);
					}
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

void tandberg_tdv2100_disp_logic_device::data_to_display(uint8_t byte)
{
	if(m_underline_input)
	{
		byte = byte|0x80;
	}

	m_vram[get_ram_addr(m_cursor_row, m_cursor_col)] = byte;
	advance_cursor();
}

void tandberg_tdv2100_disp_logic_device::advance_cursor()
{
	bool auto_cr_lf = BIT(m_dsw_u61->read(), 2);
	if((m_cursor_col&0x4f) != 79 || auto_cr_lf)
	{
		place_cursor(m_cursor_row, m_cursor_col+1);
	}

	m_speed_check = true;
	m_speed_ctrl->adjust(attotime::from_usec(16121));
}

void tandberg_tdv2100_disp_logic_device::place_cursor(int row, int col)
{
	bool in_eol_region = ((m_cursor_col&0x48) == 72);

	m_cursor_col = col&0x7f;
	m_cursor_row = row&0x1f;

	if((m_cursor_col&0x48) == 72 && !in_eol_region && !m_speed_check)
	{
		// End of line warning beep
		char_to_display(0x07);
	}
	if((m_cursor_col&0x50) == 80)
	{
		// End of line action
		char_to_display(0x0d);
		char_to_display(0x0a);
	}
}

void tandberg_tdv2100_disp_logic_device::clear_key_handler()
{
	bool clear_ascii_hs_by_cleark = ((m_dsw_u39->read()&0x003) == 0x001);
	m_write_errorl_cb(0);
	if(clear_ascii_hs_by_cleark)
	{
		m_write_enql_cb(0);
		m_write_ackl_cb(0);
		m_write_nakl_cb(0);
	}
}

int tandberg_tdv2100_disp_logic_device::get_ram_addr(int row, int col)
{
	int abs_row = (row&0x1f) + m_page_roll;
	return ((abs_row<<7) + (col&0x7f) + (m_addr_offsets[abs_row]<<4))&0x7ff;
}

void tandberg_tdv2100_disp_logic_device::erase_row(int row)
{
	for(int col = 0; col<80; col++)
	{
		m_vram[get_ram_addr(row, col)] = 0;
	}
}

TIMER_CALLBACK_MEMBER(tandberg_tdv2100_disp_logic_device::end_beep)
{
	m_beep->set_state(0);
}

TIMER_CALLBACK_MEMBER(tandberg_tdv2100_disp_logic_device::expire_speed_check)
{
	m_speed_check = false;
}

void tandberg_tdv2100_disp_logic_device::vblank(int state)
{
	if(state && !m_vblank_state)
	{
		// TODO: VSync interrupt for CPU interface
		m_frame_counter++;
		m_screen->update_now();
	}
	m_vblank_state = state;
}

void tandberg_tdv2100_disp_logic_device::update_attribute(int at_row, int at_col, int from_row, int from_col)
{
	if(at_row >= 0 && at_row < 25 && at_col >= 0 && at_col < 80)
	{
		bool first_round = true;
		for(int char_row = from_row; char_row <= 25; char_row++)
		{
			if(char_row == 25)
			{
				char_row = 0;
			}
			for(int char_col = (first_round)? from_col : 0; char_col < 80; char_col++)
			{
				if(char_col == 0 && char_row == 0)
				{
					m_attribute = 0;
				}
				int const vram_data = m_vram[get_ram_addr(char_row, char_col)];
				if(vram_data&0x80)
				{
					m_attribute = (vram_data&0x70)>>4;
				}

				if(char_row == at_row && char_col == at_col)
				{
					return;
				}
			}
			if(first_round)
			{
				first_round = false;
			}
		}
	}
}

uint32_t tandberg_tdv2100_disp_logic_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	rgb_t const *const palette = m_palette->palette()->entry_list_raw();

	int u61 = m_dsw_u61->read();
	bool extend_dot7 = ((u61&0x003) == 0x001);
	bool underline_mode = BIT(u61, 3);
	bool blinking_cursor = ((u61&0x030) == 0x010);
	bool blank_attr_chars = !BIT(u61, 6);
	bool blank_ctrl_chars = !BIT(u61, 8);
	bool block_cursor = BIT(u61, 9);
	bool full_inverse_video = !m_sw_invert_video->read();

	int const char_min_col = cliprect.min_x/9;
	int const char_min_row = cliprect.min_y/14;
	update_attribute(char_min_row, char_min_col, m_char_max_row, m_char_max_col);
	m_char_max_col = cliprect.max_x/9;
	m_char_max_row = cliprect.max_y/14;
	bool const blink_strobe = BIT(m_frame_counter, 3);
	bool const cursor_blink_strobe = blink_strobe && !m_speed_check;

	for(int char_row_nr = char_min_row; char_row_nr <= m_char_max_row; char_row_nr++)
	{
		// This is at the start of a char row
		int const chr_y_pos = char_row_nr*14;
		if(char_row_nr != char_min_row)
		{
			update_attribute(char_row_nr, char_min_col, char_row_nr-1, m_char_max_col+1);
		}

		for(int char_col_nr = char_min_col; char_col_nr <= m_char_max_col; char_col_nr++)
		{
			// This is at the start of a single character
			int const chr_x_pos = char_col_nr*9;
			int const in_line_char_nr = char_col_nr - 4;
			int const vram_address = get_ram_addr(char_row_nr, in_line_char_nr);
			bool const is_cursor = (m_cursor_col == in_line_char_nr && m_cursor_row == char_row_nr);

			for(int line = 0; line < 14; line++)
			{
				// This is at the start of a 9-dot raster-line segment in a single character
				uint8_t data = 0x00;
				int dot_nr8 = 0;
				int extra_intensity = 1;
				if(in_line_char_nr >= 0 && in_line_char_nr < 80)
				{
					bool const draw_cursor = (is_cursor && (line == 12 || block_cursor) && (!blinking_cursor || cursor_blink_strobe));
					int const vram_data = m_vram[vram_address];

					// Pattern generator & character blanking
					if(m_video_enable && (vram_data&0x60 || !blank_ctrl_chars) && !(vram_data&0x80 && blank_attr_chars))
					{
						data = m_font[((vram_data&0x7f)<<4) + line];
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
							data = 0xff;
							dot_nr8 = 0x01;
						}
					}
					else if(m_video_enable) // attribute mode
					{
						if(vram_data&0x80)
						{
							m_attribute = (vram_data&0x70)>>4;
						}
						else
						{
							if(m_attribute == 2)
							{
								// Low intensity
								extra_intensity = 0;
							}
							else if(m_attribute == 3 && blink_strobe && !draw_cursor)
							{
								// Blink
								data = 0x00;
								dot_nr8 = 0x00;
							}
							else if(m_attribute == 4)
							{
								// Inverse
								data = ~data;
								dot_nr8 = ~dot_nr8;
							}
							else if(m_attribute == 5 && line == 13)
							{
								// Underline
								data = 0xff;
								dot_nr8 = 0x01;
							}
							else if(m_attribute == 6)
							{
								// Invissible
								data = 0x00;
								dot_nr8 = 0x00;
							}
						}
					}

					if(draw_cursor)
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
				auto *pix = &bitmap.pix(chr_y_pos + line, chr_x_pos);
				for(int dot = 0; dot < 8; dot++)
				{
					pix[dot] = palette[BIT(data, 7 - dot)<<extra_intensity];
				}
				pix[8] = palette[BIT(dot_nr8, 0)<<extra_intensity];
			}
		}
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
//
// UART
//

void tandberg_tdv2100_disp_logic_device::update_rs232_lamps()
{
	m_write_waitl_cb(m_data_terminal_ready && !m_rx_handshake);
	bool need_tx_for_on_line_led = ((m_dsw_u39->read()&0x0c0) == 0x080);
	m_write_onlil_cb(m_data_terminal_ready && m_rx_handshake && (m_tx_handshake || !need_tx_for_on_line_led));
}

// Resolves RS232 RxD to UART SI data path only
void tandberg_tdv2100_disp_logic_device::rs232_rxd_w(int state)
{
	if(!(m_data_terminal_ready && m_rx_handshake))
	{
		state = 1;
	}

	bool ext_echo = BIT(m_sw_rs232_settings->read(), 3);
	if(!(ext_echo && m_data_terminal_ready))
	{
		state = ((state && m_uart->so_r()) ? 1 : 0);
	}
	m_uart->write_si(state);
}

// Resolves UART SO to RS232 TxD data-path, but also refreshes Rx data-path
void tandberg_tdv2100_disp_logic_device::rs232_txd_w(int state)
{
	// Handle local echo
	rs232_rxd_w(m_rs232->rxd_r());

	if(!(m_request_to_send && m_tx_handshake))
	{
		state = 1;
	}
	else if(m_break_key_held)
	{
		state = 0;
	}
	m_rs232->write_txd(state);
}

void tandberg_tdv2100_disp_logic_device::rs232_dcd_w(int state)
{
	m_write_carl_cb(state);

	bool dcd_handshake = ((m_dsw_u39->read()&0x00c) == 0x008);
	if(dcd_handshake)
	{
		m_rx_handshake = state;
		update_rs232_lamps();
		rs232_rxd_w(m_rs232->rxd_r());
	}
}

void tandberg_tdv2100_disp_logic_device::rs232_dsr_w(int state)
{
	bool dcd_handshake = ((m_dsw_u39->read()&0x00c) == 0x008);
	if(!dcd_handshake)
	{
		bool auto_rx_handshake = BIT(m_sw_rs232_settings->read(), 4);
		if(auto_rx_handshake)
		{
			state = m_data_terminal_ready;
		}
		m_rx_handshake = state;
		update_rs232_lamps();
		rs232_rxd_w(m_rs232->rxd_r());
	}
}

void tandberg_tdv2100_disp_logic_device::rs232_cts_w(int state)
{
	bool auto_tx_handshake = BIT(m_sw_rs232_settings->read(), 5);
	if(auto_tx_handshake)
	{
		state = m_request_to_send;
	}
	m_tx_handshake = state;
	update_rs232_lamps();
	rs232_txd_w(m_uart->so_r());
}

INPUT_CHANGED_MEMBER(tandberg_tdv2100_disp_logic_device::rs232_changed)
{
	update_all_rs232_signal_paths();
}

void tandberg_tdv2100_disp_logic_device::update_all_rs232_signal_paths()
{
	rs232_dcd_w(m_rs232->dcd_r());
	rs232_dsr_w(m_rs232->dsr_r());
	rs232_cts_w(m_rs232->cts_r());      // Updates LEDs, refreshes data-paths
}

void tandberg_tdv2100_disp_logic_device::rs232_ri_w(int state)
{
	// TODO: Ring Indicator interrupt for CPU interface
}

void tandberg_tdv2100_disp_logic_device::check_rs232_rx_error(int state)
{
	m_write_errorl_cb(m_uart->pe_r() && m_uart->dav_r());
}

void tandberg_tdv2100_disp_logic_device::uart_rx(int state)
{
	if(state)
	{
		bool tty_mode = BIT(m_dsw_u39->read(), 4);
		if(tty_mode)
		{
			char_to_display(m_uart->receive());
			m_uart->write_rdav(0);
		}
		else
		{
			// TODO: Rx interrupt for CPU interface
		}
	}
}

void tandberg_tdv2100_disp_logic_device::uart_tx(int state)
{
	if(state)
	{
		bool tty_mode = BIT(m_dsw_u39->read(), 4);
		if(tty_mode)
		{
			if(m_data_pending_kbd)
			{
				m_uart->transmit(m_data_kbd);
				m_data_pending_kbd = false;
			}
		}
		else
		{
			// TODO: Tx interrupt for CPU interface
		}
	}
}

INPUT_CHANGED_MEMBER(tandberg_tdv2100_disp_logic_device::uart_changed)
{
	set_uart_state_from_switches();
}

void tandberg_tdv2100_disp_logic_device::set_uart_state_from_switches()
{
	int baud_setting = m_sw_rs232_baud->read();
	int u46_divisor = (baud_setting == 0x0) ? 15 : 11;
	int u22_divisor = 256>>baud_setting;
	m_uart_clock->set_unscaled_clock(DOT_CLOCK_3 / (u46_divisor * u22_divisor));

	int rs232_settings = m_sw_rs232_settings->read();
	m_uart->write_cs(true);
	m_uart->write_nb1(BIT(m_dsw_u39->read(), 9));
	m_uart->write_nb2(true);
	m_uart->write_np(BIT(rs232_settings, 0));
	m_uart->write_eps(BIT(rs232_settings, 1));
	m_uart->write_tsb(BIT(rs232_settings, 2));
}

// Resolves Rx flow-control logic
void tandberg_tdv2100_disp_logic_device::clock_on_line_flip_flop()
{
	bool force_on_line = BIT(m_dsw_u39->read(), 5);
	if(force_on_line)
	{
		m_data_terminal_ready = true;
	}
	else
	{
		// Try to toggle RS232 DTR
		if(m_rx_handshake)
		{
			m_data_terminal_ready = false;
		}
		else
		{
			m_data_terminal_ready = !m_data_terminal_ready;
		}
	}

	if(!m_data_terminal_ready)
	{
		m_request_to_send = false;
		m_rs232->write_rts((m_request_to_send) ? 1 : 0);
	}
	m_rs232->write_dtr((m_data_terminal_ready) ? 1 : 0);

	update_all_rs232_signal_paths();
	update_rs232_lamps();

	bool hold_line_key_for_rts = ((m_dsw_u73->read()&0x3) == 0x1);
	if(hold_line_key_for_rts && m_data_terminal_ready && m_line_key_held && !m_trans_key_held)
	{
		clock_transmit_flip_flop();
	}
}

// Resolves Tx flow-control logic
void tandberg_tdv2100_disp_logic_device::clock_transmit_flip_flop()
{
	bool force_on_line = BIT(m_dsw_u39->read(), 5);
	if(force_on_line)
	{
		m_request_to_send = true;
	}
	else if(!m_data_terminal_ready)
	{
		m_request_to_send = false;
	}
	else
	{
		// Try to toggle RS232 RTS
		if(m_tx_handshake)
		{
			m_request_to_send = false;
		}
		else
		{
			m_request_to_send = !m_request_to_send;
		}
	}
	m_rs232->write_rts((m_request_to_send) ? 1 : 0);

	update_all_rs232_signal_paths();
	update_rs232_lamps();
}

INPUT_CHANGED_MEMBER(tandberg_tdv2100_disp_logic_device::rs232_lock_changed)
{
	bool force_on_line = BIT(m_dsw_u39->read(), 5);
	if(force_on_line)
	{
		// Use conditional catch to force lines active
		clock_on_line_flip_flop();
		clock_transmit_flip_flop();
	}
}

///////////////////////////////////////////////////////////////////////////////
//
// Strobes for Keyboard
//

void tandberg_tdv2100_disp_logic_device::process_keyboard_char(uint8_t key)
{
	m_data_kbd = key&0x7f;
	if(key&0x80)
	{
		m_data_pending_kbd = true;
		bool tty_mode = BIT(m_dsw_u39->read(), 4);
		if(tty_mode && m_uart->tbmt_r())
		{
			uart_tx(1);
		}
		else
		{
			// TODO: Keyboard interrupt for CPU interface
		}
	}
}

void tandberg_tdv2100_disp_logic_device::break_w(int state)
{
	bool old_state = m_break_key_held;
	m_break_key_held = state;
	if(state == !old_state)
	{
		rs232_txd_w(m_uart->so_r());
	}
}

void tandberg_tdv2100_disp_logic_device::cleark_w(int state)
{
	if(state && !m_clear_key_held)
	{
		clear_key_handler();
	}
	m_clear_key_held = state;
}

void tandberg_tdv2100_disp_logic_device::linek_w(int state)
{
	if(state && !m_line_key_held)
	{
		clock_on_line_flip_flop();
	}
	m_line_key_held = state;
}

void tandberg_tdv2100_disp_logic_device::transk_w(int state)
{
	if(state && !m_trans_key_held)
	{
		clock_transmit_flip_flop();
	}
	m_trans_key_held = state;
}

///////////////////////////////////////////////////////////////////////////////
//
// Driver config
//

void tandberg_tdv2100_disp_logic_device::device_add_mconfig(machine_config &mconfig)
{
	/* Video hardware */
	SCREEN(mconfig, m_screen, SCREEN_TYPE_RASTER, rgb_t::green());
	m_screen->set_raw(DOT_CLOCK, 999, 0, 783, 402, 0, 350);
	m_screen->set_screen_update(FUNC(tandberg_tdv2100_disp_logic_device::screen_update));
	m_screen->screen_vblank().set(FUNC(tandberg_tdv2100_disp_logic_device::vblank));
	PALETTE(mconfig, m_palette, palette_device::MONOCHROME_HIGHLIGHT);

	/* Sound */
	SPEAKER(mconfig, "mono").front_center();
	BEEP(mconfig, m_beep, 2000);
	m_beep->add_route(ALL_OUTPUTS, "mono", 1.0);

	RS232_PORT(mconfig, m_rs232, default_rs232_devices, nullptr);
	m_rs232->rxd_handler().set(FUNC(tandberg_tdv2100_disp_logic_device::rs232_rxd_w));
	m_rs232->dcd_handler().set(FUNC(tandberg_tdv2100_disp_logic_device::rs232_dcd_w));
	m_rs232->dsr_handler().set(FUNC(tandberg_tdv2100_disp_logic_device::rs232_dsr_w));
	m_rs232->cts_handler().set(FUNC(tandberg_tdv2100_disp_logic_device::rs232_cts_w));
	m_rs232->ri_handler().set(FUNC(tandberg_tdv2100_disp_logic_device::rs232_ri_w));

	AY51013(mconfig, m_uart);
	m_uart->write_so_callback().set(FUNC(tandberg_tdv2100_disp_logic_device::rs232_txd_w));
	m_uart->write_dav_callback().set(FUNC(tandberg_tdv2100_disp_logic_device::uart_rx));
	m_uart->write_dav_callback().append(FUNC(tandberg_tdv2100_disp_logic_device::check_rs232_rx_error));
	m_uart->write_tbmt_callback().set(FUNC(tandberg_tdv2100_disp_logic_device::uart_tx));
	m_uart->write_pe_callback().set(FUNC(tandberg_tdv2100_disp_logic_device::check_rs232_rx_error));

	CLOCK(mconfig, m_uart_clock, 1);
	// NOTE: Frequency set with the rest of the UART settings
	m_uart_clock->signal_handler().set(m_uart, FUNC(ay51013_device::write_rcp));
	m_uart_clock->signal_handler().append(m_uart, FUNC(ay51013_device::write_tcp));
}

static INPUT_PORTS_START( tdv2115l )

	PORT_START("sw_invert_video")
		PORT_CONFNAME(0x1, 0x1, "INVERSE VIDEO")
			PORT_CONFSETTING(0x1, DEF_STR( Off ))
			PORT_CONFSETTING(0x0, DEF_STR( On ))

	PORT_START("sw_rs232_baud")
		PORT_CONFNAME(0x7, 0x6, "SPEED SELECT [Note: Baud-rate]")                                PORT_CHANGED_MEMBER(DEVICE_SELF, tandberg_tdv2100_disp_logic_device, uart_changed, 0)
			PORT_CONFSETTING(0x0, "0: 110")
			PORT_CONFSETTING(0x1, "1: 300")
			PORT_CONFSETTING(0x2, "2: 600")
			PORT_CONFSETTING(0x3, "3: 1200")
			PORT_CONFSETTING(0x4, "4: 2400")
			PORT_CONFSETTING(0x5, "5: 4800")
			PORT_CONFSETTING(0x6, "6: 9600")
			PORT_CONFSETTING(0x7, "7: 19200")

	PORT_START("sw_rs232_settings")
		PORT_CONFNAME(0x01, 0x01, "RS-232 Parity checking")                                     PORT_CHANGED_MEMBER(DEVICE_SELF, tandberg_tdv2100_disp_logic_device, uart_changed, 0)   // NO PARITY [YES/NO]
			PORT_CONFSETTING(0x01, DEF_STR( Off ))
			PORT_CONFSETTING(0x00, DEF_STR( On ))
		PORT_CONFNAME(0x02, 0x02, "RS-232 Parity type")                                         PORT_CHANGED_MEMBER(DEVICE_SELF, tandberg_tdv2100_disp_logic_device, uart_changed, 0)   // EVEN PARITY [NO/YES]
			PORT_CONFSETTING(0x00, "Odd")
			PORT_CONFSETTING(0x02, "Even")
		PORT_CONFNAME(0x04, 0x04, "RS-232 Number of stop bits")                                 PORT_CHANGED_MEMBER(DEVICE_SELF, tandberg_tdv2100_disp_logic_device, uart_changed, 0)   // TWO STOP BITS [NO/YES]
			PORT_CONFSETTING(0x00, "One")
			PORT_CONFSETTING(0x04, "Two")
		PORT_CONFNAME(0x08, 0x08, "RS-232 Internal local echo when on-line (for half duplex)")  PORT_CHANGED_MEMBER(DEVICE_SELF, tandberg_tdv2100_disp_logic_device, rs232_changed, 0)  // EXT. ECHO [YES/NO]
			PORT_CONFSETTING(0x08, DEF_STR( Off ))
			PORT_CONFSETTING(0x00, DEF_STR( On ))
		PORT_CONFNAME(0x10, 0x00, "RS-232 Automatic RTS with CTS")                              PORT_CHANGED_MEMBER(DEVICE_SELF, tandberg_tdv2100_disp_logic_device, rs232_changed, 0)  // AUTO RFS [NO/YES]
			PORT_CONFSETTING(0x00, DEF_STR( No ))
			PORT_CONFSETTING(0x10, DEF_STR( Yes ))
		PORT_CONFNAME(0x20, 0x00, "RS-232 Automatic DSR with DTR")                              PORT_CHANGED_MEMBER(DEVICE_SELF, tandberg_tdv2100_disp_logic_device, rs232_changed, 0)  // AUTO DSR [NO/YES]
			PORT_CONFSETTING(0x00, DEF_STR( No ))
			PORT_CONFSETTING(0x20, DEF_STR( Yes ))

	PORT_START("dsw_u39")
		PORT_DIPNAME(0x003, 0x001, "ACK/NACK/ENQUIRY lamps")
			PORT_DIPSETTING(0x001, "Reset with CLEAR key")                      // 1: OFF 2: ON
			PORT_DIPSETTING(0x002, "Reset with SYN ctrl-char (^V)")             // 1: ON  2: OFF
		PORT_DIPNAME(0x00c, 0x004, "Rx handshake source")                                       PORT_CHANGED_MEMBER(DEVICE_SELF, tandberg_tdv2100_disp_logic_device, rs232_changed, 0)
			PORT_DIPSETTING(0x004, "DSR")                                       // 3: OFF 4: ON
			PORT_DIPSETTING(0x008, "DCD")                                       // 3: ON  4: Off
		PORT_DIPNAME(0x010, 0x010, "Operating mode")
			PORT_DIPSETTING(0x010, "TTY mode (no CPU)")                         // 5: OFF
			PORT_DIPSETTING(0x000, "CPU mode (CPU module required)")            // 5: ON
		PORT_DIPNAME(0x020, 0x020, "DTR/RTS signals")                                           PORT_CHANGED_MEMBER(DEVICE_SELF, tandberg_tdv2100_disp_logic_device, rs232_lock_changed, 0)
			PORT_DIPSETTING(0x020, "Permanently asserted")                      // 6: OFF
			PORT_DIPSETTING(0x000, "Affected by LINE/TRANS keys")               // 6: ON
		PORT_DIPNAME(0x0c0, 0x040, "Require RTS + CTS for ON LINE lamp")                        PORT_CHANGED_MEMBER(DEVICE_SELF, tandberg_tdv2100_disp_logic_device, rs232_changed, 0)
			PORT_DIPSETTING(0x040, DEF_STR( Off ))                              // 7: OFF 8: ON
			PORT_DIPSETTING(0x080, DEF_STR( On ))                               // 7: ON  8: OFF
		PORT_DIPNAME(0x100, 0x000, "Automatic page-roll after end of page")
			PORT_DIPSETTING(0x100, DEF_STR( Off ))                              // 9: OFF
			PORT_DIPSETTING(0x000, DEF_STR( On ))                               // 9: ON
		PORT_DIPNAME(0x200, 0x000, "RS-232 data length")                                        PORT_CHANGED_MEMBER(DEVICE_SELF, tandberg_tdv2100_disp_logic_device, uart_changed, 0)
			PORT_DIPSETTING(0x200, "8 data bits")                               // 10: OFF
			PORT_DIPSETTING(0x000, "7 data bits")                               // 10: ON

	PORT_START("dsw_u73")
		PORT_DIPNAME(0x3, 0x1, "Automatic RTS with DTR")
			PORT_DIPSETTING(0x2, DEF_STR( Off ))                                // 1: ON  2: OFF
			PORT_DIPSETTING(0x1, DEF_STR( On ))                                 // 1: OFF 2: ON
		PORT_DIPNAME(0xc, 0x8, "Current-Loop RxD line (Unused)")
			PORT_DIPSETTING(0x4, "10V zener-diode in series")                   // 3: OFF 4: ON
			PORT_DIPSETTING(0x8, "47 Ohm series resistor")                      // 3: ON  4: OFF

	PORT_START("dsw_u61")
		PORT_DIPNAME(0x003, 0x001, "Horizontal space between chars")
			PORT_DIPSETTING(0x001, "Duplicate edge of previous char")           // 1: OFF 2: ON
			PORT_DIPSETTING(0x002, "Draw gap")                                  // 1: ON  2: OFF
		PORT_DIPNAME(0x004, 0x004, "Automatic CR+LF after end of line")
			PORT_DIPSETTING(0x000, DEF_STR( Off ))                              // 3: ON
			PORT_DIPSETTING(0x004, DEF_STR( On ))                               // 3: OFF
		PORT_DIPNAME(0x008, 0x000, "Display mode")
			PORT_DIPSETTING(0x008, "Undeline mode")                             // 4: OFF
			PORT_DIPSETTING(0x000, "Attribute mode")                            // 4: ON
		PORT_DIPNAME(0x030, 0x020, "Cursor blinking")
			PORT_DIPSETTING(0x020, DEF_STR( Off ))                              // 5: ON  6: OFF
			PORT_DIPSETTING(0x010, DEF_STR( On ))                               // 5: OFF 6: ON
		PORT_DIPNAME(0x040, 0x000, "Hide Attribute-changes or Underlined chars")
			PORT_DIPSETTING(0x040, DEF_STR( Off ))                              // 7: OFF
			PORT_DIPSETTING(0x000, DEF_STR( On ))                               // 7: ON
		PORT_DIPNAME(0x080, 0x080, DEF_STR( Unused ))
			PORT_DIPSETTING(0x080, DEF_STR( Off ))                              // 8: OFF
			PORT_DIPSETTING(0x000, DEF_STR( On ))                               // 8: ON
		PORT_DIPNAME(0x100, 0x000, "Hide ASCII control characters")
			PORT_DIPSETTING(0x100, DEF_STR( Off ))                              // 9: OFF
			PORT_DIPSETTING(0x000, DEF_STR( On ))                               // 9: ON
			// NOTE: CPU module needed to write these to display RAM, as well as a ROM set with a 4th font ROM
		PORT_DIPNAME(0x200, 0x000, "Cursor shape")
			PORT_DIPSETTING(0x200, "Block")                                     // 10: OFF
			PORT_DIPSETTING(0x000, "Line")                                      // 10: ON

INPUT_PORTS_END

ioport_constructor tandberg_tdv2100_disp_logic_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( tdv2115l );
}

ROM_START(tdv2115l)
	ROM_REGION( 0x0800, "display_font_rom", ROMREGION_ERASEFF )
	ROM_LOAD( "960558-0 1.82s141.u18", 0x0200, 0x0200, CRC(002856d2) SHA1(d86afc7190163b3c0d7a43516adb14046b978a4f))
	ROM_LOAD( "960558-0 2.82s141.u19", 0x0400, 0x0200, CRC(422f1959) SHA1(aad1b74ed194c486e53d7fe6469b15c407e08441))
	ROM_LOAD( "960558-0 3.82s141.u20", 0x0600, 0x0200, CRC(f662df8e) SHA1(c585731612aa1bea8d95223a49011099953cf34e))

	ROM_REGION( 0x40, "row_addr_offsets", ROMREGION_ERASEFF )
	ROM_LOAD( "prom.82s123.u23", 0x00, 0x20, CRC(b92bfa61) SHA1(3af8108269a0504268ebb8d5bb6ae235d811460c))
	ROM_LOAD( "prom.82s123.u24", 0x20, 0x20, CRC(a64d8378) SHA1(014d524d977927c140237c47e8ba692c1a89397a))
ROM_END

const tiny_rom_entry *tandberg_tdv2100_disp_logic_device::device_rom_region() const
{
	return ROM_NAME(tdv2115l);
}
