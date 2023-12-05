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
	for ASCII character 0x20 to 0x7F. There is also two PROM used as logic-
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
	m_write_nakl_cb(*this)
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
	tx_data_pending_kbd = false;
	tx_data_kbd = 0x00;
}

///////////////////////////////////////////////////////////////////////////////
//
// General machine state
//

void tandberg_tdv2100_disp_logic_device::device_start()
{
	m_beep_trigger = timer_alloc(FUNC(tandberg_tdv2100_disp_logic_device::end_beep), this);
	m_speed_ctrl = timer_alloc(FUNC(tandberg_tdv2100_disp_logic_device::expire_speed_check), this);
}

void tandberg_tdv2100_disp_logic_device::device_reset()
{
	cursor_x_input = false;
	cursor_y_input = false;
	char_to_display(0x19);
	attribute = 0;
	frame_counter = 0;
	underline_input = false;
	video_enable = true;
	data_terminal_ready = false;
	request_to_send = false;
	tx_data_pending_kbd = false;
	m_uart->write_xr(1);
	m_uart->write_xr(0);
	w_cleark(1);

	set_uart_state_from_switches();
	bool force_on_line = m_dsw_u39->read()&0x020;
	if(force_on_line)
	{
		w_linek(1);
		w_transk(1);
	}
	else
	{
		update_rs232_lamps();
	}
}

///////////////////////////////////////////////////////////////////////////////
//
// Graphics and character data
//

void tandberg_tdv2100_disp_logic_device::char_to_display(uint8_t byte){
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
			bool auto_roll_up = !(m_dsw_u39->read()&0x100);
			bool clear_ascii_hs_by_cleark = ((m_dsw_u39->read()&0x003) == 0x001);
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

void tandberg_tdv2100_disp_logic_device::data_to_display(uint8_t byte){
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

void tandberg_tdv2100_disp_logic_device::advance_cursor()
{
	cursor_x++;
	if(cursor_x == 72 && !speed_check){
		char_to_display(0x07);
	}
	else if(cursor_x > 79)
	{
		bool auto_cr_lf = m_dsw_u61->read()&0x004;
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

TIMER_CALLBACK_MEMBER(tandberg_tdv2100_disp_logic_device::end_beep)
{
	m_beep->set_state(0);
}

TIMER_CALLBACK_MEMBER(tandberg_tdv2100_disp_logic_device::expire_speed_check)
{
	speed_check = false;
}

uint32_t tandberg_tdv2100_disp_logic_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
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
			bool const is_cursor = (cursor_x == in_line_char_nr && cursor_y == char_nr_y);
			bool const blink_strobe = frame_counter&0x08;
			bool const cursor_blink_strobe = blink_strobe && !speed_check;

			for(int line = 0; line < 14; line++)
			{
				// This is at the start of a 9-dot raster-line segment in a single character
				uint8_t data = 0x00;
				int dot_nr8 = 0;
				int extra_intensity = 1;
				if(in_line_char_nr >= 0 && in_line_char_nr < 80)
				{
					bool block_cursor = m_dsw_u61->read()&0x200;
					bool blinking_cursor = ((m_dsw_u61->read()&0x030) == 0x010);
					bool const draw_cursor = (is_cursor && (line == 12 || block_cursor) && (!blinking_cursor || cursor_blink_strobe));
					int const vram_data = m_vram[vram_address];

					// Pattern generator & character blanking
					bool blank_ctrl_chars = !(m_dsw_u61->read()&0x100);
					bool blank_attr_chars = !(m_dsw_u61->read()&0x040);
					if(video_enable && (vram_data&0x60 || !blank_ctrl_chars) && !(vram_data&0x80 && blank_attr_chars))
					{
						data = m_font->base()[((vram_data&0x7F)<<4) + line];
						bool extend_dot7 = ((m_dsw_u61->read()&0x003) == 1);
						if(extend_dot7)
						{
							dot_nr8 = data&0x01;
						}
					}

					// Pattern modifiers
					bool underline_mode = m_dsw_u61->read()&0x008;
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

					if(draw_cursor)
					{
						data = ~data;
						dot_nr8 = ~dot_nr8;
					}
				}

				bool full_inverse_video = !m_sw_invert_video->read();
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

void tandberg_tdv2100_disp_logic_device::update_rs232_lamps()
{
	m_write_waitl_cb(data_terminal_ready && !rx_handshake);
	bool need_tx_for_on_line_led = ((m_dsw_u39->read()&0x0C0) == 0x080);
	m_write_onlil_cb(data_terminal_ready && rx_handshake && (tx_handshake || !need_tx_for_on_line_led));
}

void tandberg_tdv2100_disp_logic_device::rs232_rxd_w(int state)
{
	if(!(data_terminal_ready && rx_handshake))
	{
		state = 1;
	}
	bool ext_echo = m_sw_rs232_settings->read()&0x08;
	if(!(ext_echo && data_terminal_ready))
	{
		state = (state && m_uart->so_r());
	}
	m_uart->write_si(state);
}

void tandberg_tdv2100_disp_logic_device::rs232_dcd_w(int state)
{
	m_write_carl_cb(state);
	bool dcd_handshake = ((m_dsw_u39->read()&0x00C) == 0x008);
	if(dcd_handshake)
	{
		rx_handshake = state;
	}
	update_rs232_lamps();
}

void tandberg_tdv2100_disp_logic_device::rs232_dsr_w(int state)
{
	bool auto_rx_handshake = m_sw_rs232_settings->read()&0x10;
	bool dcd_handshake = ((m_dsw_u39->read()&0x00C) == 0x008);
	if(!dcd_handshake && !auto_rx_handshake)
	{
		rx_handshake = state;
	}
	update_rs232_lamps();
}

void tandberg_tdv2100_disp_logic_device::rs232_cts_w(int state)
{
	bool auto_tx_handshake = m_sw_rs232_settings->read()&0x20;
	if(!auto_tx_handshake)
	{
		tx_handshake = state;
	}
	update_rs232_lamps();
}

void tandberg_tdv2100_disp_logic_device::rs232_txd_w(int state)
{
	bool ext_echo = m_sw_rs232_settings->read()&0x08;
	if(!(ext_echo && data_terminal_ready))
	{
		m_uart->write_si(state);
	}
	if(!(request_to_send && tx_handshake))
	{
		state = 1;
	}
	else if(break_key_held)
	{
		state = 0;
	}
	m_rs232->write_txd(state);
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
		bool rtt_mode = m_dsw_u39->read()&0x010;
		if(rtt_mode)
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
		bool rtt_mode = m_dsw_u39->read()&0x010;
		if(rtt_mode)
		{
			if(tx_data_pending_kbd)
			{
				m_uart->transmit(tx_data_kbd);
				tx_data_pending_kbd = false;
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
	m_uart->write_cs(true);
	m_uart->write_nb1(m_dsw_u39->read()&0x200);
	m_uart->write_nb2(true);
	m_uart->write_np(m_sw_rs232_settings->read()&0x01);
	m_uart->write_eps(m_sw_rs232_settings->read()&0x02);
	m_uart->write_tsb(m_sw_rs232_settings->read()&0x04);
}

///////////////////////////////////////////////////////////////////////////////
//
// Strobes for Keyboard
//

void tandberg_tdv2100_disp_logic_device::process_keyboard_char(uint8_t key)
{
	if(key&0x80)
	{
		tx_data_kbd = key&0x7F;
		tx_data_pending_kbd = true;
		bool rtt_mode = m_dsw_u39->read()&0x010;
		if(rtt_mode)
		{
			if(m_uart->tbmt_r())
			{
				uart_tx(1);
			}
		}
		else
		{
			// TODO: Keyboard interrupt for CPU interface
		}
	}
}

void tandberg_tdv2100_disp_logic_device::w_break(int state)
{
	break_key_held = state;
}

void tandberg_tdv2100_disp_logic_device::w_cleark(int state)
{
	if(state)
	{
		m_write_errorl_cb(0);
		bool clear_ascii_hs_by_cleark = ((m_dsw_u39->read()&0x003) == 0x001);
		if(clear_ascii_hs_by_cleark)
		{
			m_write_enql_cb(0);
			m_write_ackl_cb(0);
			m_write_nakl_cb(0);
		}
	}
}

void tandberg_tdv2100_disp_logic_device::w_linek(int state)
{
	if(state)
	{
		bool force_on_line = m_dsw_u39->read()&0x020;
		if(force_on_line)
		{
			data_terminal_ready = true;
		}
		else
		{
			// Try to toggle RS232 DTR
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
			bool auto_tx_handshake = m_sw_rs232_settings->read()&0x20;
			if(auto_tx_handshake)
			{
				tx_handshake = request_to_send;
			}
		}
		m_rs232->write_dtr(data_terminal_ready);

		bool auto_rx_handshake = m_sw_rs232_settings->read()&0x10;
		if(auto_rx_handshake)
		{
			rx_handshake = data_terminal_ready;
		}
		bool hold_line_key_for_rts = ((m_dsw_u73->read()&0x3) == 0x1);
		if(data_terminal_ready && hold_line_key_for_rts)
		{
			w_transk(1);
		}
		update_rs232_lamps();
	}
}

void tandberg_tdv2100_disp_logic_device::w_transk(int state)
{
	if(state)
	{
		bool force_on_line = m_dsw_u39->read()&0x020;
		if(force_on_line)
		{
			request_to_send = true;
		}
		else if(!data_terminal_ready)
		{
			request_to_send = false;
		}
		else
		{
			// Try to toggle RS232 RTS
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

		bool auto_tx_handshake = m_sw_rs232_settings->read()&0x20;
		if(auto_tx_handshake)
		{
			tx_handshake = request_to_send;
		}
		update_rs232_lamps();
	}
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
			PORT_CONFSETTING(0x0, "Toggled in")
			PORT_CONFSETTING(0x1, "Toggled out")

	PORT_START("sw_rs232_baud")
		PORT_CONFNAME(0x7, 0x6, "SPEED SELECT (Baud-rate)")                                                             PORT_CHANGED_MEMBER(DEVICE_SELF, tandberg_tdv2100_disp_logic_device, uart_changed, 0)
			PORT_CONFSETTING(0x0, "0: 110")
			PORT_CONFSETTING(0x1, "1: 300")
			PORT_CONFSETTING(0x2, "2: 600")
			PORT_CONFSETTING(0x3, "3: 1200")
			PORT_CONFSETTING(0x4, "4: 2400")
			PORT_CONFSETTING(0x5, "5: 4800")
			PORT_CONFSETTING(0x6, "6: 9600")
			PORT_CONFSETTING(0x7, "7: 19200")

	PORT_START("sw_rs232_settings")
		PORT_CONFNAME(0x01, 0x01, "NO PARITY")              PORT_CONFSETTING(0x01, "YES") PORT_CONFSETTING(0x00, "NO")  PORT_CHANGED_MEMBER(DEVICE_SELF, tandberg_tdv2100_disp_logic_device, uart_changed, 0)
		PORT_CONFNAME(0x02, 0x02, "EVEN PARITY")            PORT_CONFSETTING(0x02, "YES") PORT_CONFSETTING(0x00, "NO")  PORT_CHANGED_MEMBER(DEVICE_SELF, tandberg_tdv2100_disp_logic_device, uart_changed, 0)
		PORT_CONFNAME(0x04, 0x04, "TWO STOP BITS")          PORT_CONFSETTING(0x04, "YES") PORT_CONFSETTING(0x00, "NO")  PORT_CHANGED_MEMBER(DEVICE_SELF, tandberg_tdv2100_disp_logic_device, uart_changed, 0)
		PORT_CONFNAME(0x08, 0x08, "EXT. ECHO")              PORT_CONFSETTING(0x08, "YES") PORT_CONFSETTING(0x00, "NO")
		PORT_CONFNAME(0x10, 0x00, "AUTO RFS (RTS)")         PORT_CONFSETTING(0x10, "YES") PORT_CONFSETTING(0x00, "NO")
		PORT_CONFNAME(0x20, 0x00, "AUTO DSR")               PORT_CONFSETTING(0x20, "YES") PORT_CONFSETTING(0x00, "NO")

	PORT_START("dsw_u39")
		PORT_DIPNAME(0x003, 0x001, "ACK/NACK/ENQUIRY lamps")
			PORT_DIPSETTING(0x001, "Reset with CLEAR key")                      // 1: OFF 2: ON
			PORT_DIPSETTING(0x002, "Reset with SYN ctrl-char (^V)")             // 1: ON  2: OFF
		PORT_DIPNAME(0x00C, 0x004, "Rx handshake source")
			PORT_DIPSETTING(0x004, "DSR")                                       // 3: OFF 4: ON
			PORT_DIPSETTING(0x008, "DCD")                                       // 3: ON  4: Off
		PORT_DIPNAME(0x010, 0x010, "Operating mode")
			PORT_DIPSETTING(0x010, "RTT mode (no CPU)")                         // 5: OFF
			PORT_DIPSETTING(0x000, "CPU mode (CPU module required)")            // 5: ON
		PORT_DIPNAME(0x020, 0x020, "DTR/RTS signals")
			PORT_DIPSETTING(0x020, "Permanently asserted")                      // 6: OFF
			PORT_DIPSETTING(0x000, "Affected by LINE/TRANS keys")               // 6: ON
		PORT_DIPNAME(0x0C0, 0x040, "ON LINE lamp condition")
			PORT_DIPSETTING(0x040, "DTR + Rx handshake")                        // 7: OFF 8: ON
			PORT_DIPSETTING(0x080, "DTR/RTS + Rx/Tx handshake")                 // 7: ON  8: OFF
		PORT_DIPNAME(0x100, 0x000, "End of page NewLine action")
			PORT_DIPSETTING(0x100, "No automatic action")                       // 9: OFF
			PORT_DIPSETTING(0x000, "Automatic page scroll")                     // 9: ON
		PORT_DIPNAME(0x200, 0x000, "RS-232 data length")                                                                PORT_CHANGED_MEMBER(DEVICE_SELF, tandberg_tdv2100_disp_logic_device, uart_changed, 0)
			PORT_DIPSETTING(0x200, "8 data bits")                               // 10: OFF
			PORT_DIPSETTING(0x000, "7 data bits")                               // 10: ON

	PORT_START("dsw_u73")
		PORT_DIPNAME(0x3, 0x1, "RTS signal")
			PORT_DIPSETTING(0x1, "Automatic with DTR")                          // 1: OFF 2: ON
			PORT_DIPSETTING(0x2, "Asserted by TRANS key")                       // 1: ON  2: OFF
		PORT_DIPNAME(0xC, 0x8, "Current-Loop RxD line (Unused)")
			PORT_DIPSETTING(0x4, "10V zener-diode in series")                   // 3: OFF 4: ON
			PORT_DIPSETTING(0x8, "47 Ohm series resistor")                      // 3: ON  4: OFF

	PORT_START("dsw_u61")
		PORT_DIPNAME(0x003, 0x001, "Horizontal space between chars")
			PORT_DIPSETTING(0x001, "Duplicate edge of previous char")           // 1: OFF 2: ON
			PORT_DIPSETTING(0x002, "Draw gap")                                  // 1: ON  2: OFF
		PORT_DIPNAME(0x004, 0x004, "End of line input action")
			PORT_DIPSETTING(0x004, "Automatic CartridgeReturn + NewLine")       // 3: OFF
			PORT_DIPSETTING(0x000, "No automatic action")                       // 3: ON
		PORT_DIPNAME(0x008, 0x000, "Display mode")
			PORT_DIPSETTING(0x008, "Undeline mode")                             // 4: OFF
			PORT_DIPSETTING(0x000, "Attribute mode")                            // 4: ON
		PORT_DIPNAME(0x030, 0x020, "Cursor behaviour")
			PORT_DIPSETTING(0x010, "Blinking")                                  // 5: OFF 6: ON
			PORT_DIPSETTING(0x020, "Static")                                    // 5: ON  6: OFF
		PORT_DIPNAME(0x040, 0x000, "Attribute-spesific characters")
			PORT_DIPSETTING(0x040, "Draw underline chars (for UL mode)")        // 7: OFF
			PORT_DIPSETTING(0x000, "Hide attr-change chars (for Attr mode)")    // 7: ON
		PORT_DIPNAME(0x080, 0x080, DEF_STR( Unused ))
			PORT_DIPSETTING(0x080, DEF_STR( Off ))                              // 8: OFF
			PORT_DIPSETTING(0x000, DEF_STR( On ))                               // 8: ON
		PORT_DIPNAME(0x100, 0x000, "Control Characters")
			PORT_DIPSETTING(0x100, "Draw ASCII chars 0x00 - 0x1F")              // 9: OFF
			PORT_DIPSETTING(0x000, "Hide ASCII chars 0x00 - 0x1F")              // 9: ON
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
	ROM_LOAD( "960558-0 1.82s141.u18", 0x0200, 0x0200, CRC(002856D2) SHA1(D86AFC7190163B3C0D7A43516ADB14046B978A4F))
	ROM_LOAD( "960558-0 2.82s141.u19", 0x0400, 0x0200, CRC(422F1959) SHA1(AAD1B74ED194C486E53D7FE6469B15C407E08441))
	ROM_LOAD( "960558-0 3.82s141.u20", 0x0600, 0x0200, CRC(F662DF8E) SHA1(C585731612AA1BEA8D95223A49011099953CF34E))
ROM_END

const tiny_rom_entry *tandberg_tdv2100_disp_logic_device::device_rom_region() const
{
	return ROM_NAME(tdv2115l);
}
