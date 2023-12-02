// license:BSD-3-Clause
// copyright-holders:Frode van der Meeren
/***************************************************************************

	Tandberg TDV-2115 Terminal module, "Display Logic" version

****************************************************************************/

#ifndef MAME_TANDBERG_DISP_LOGIC_H
#define MAME_TANDBERG_DISP_LOGIC_H

#pragma once

#include "machine/clock.h"
#include "machine/ram.h"
#include "machine/ay31015.h"
#include "bus/rs232/rs232.h"
#include "sound/beep.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

class tandberg_tdv2100_disp_logic_device : public device_t
{
public:
	tandberg_tdv2100_disp_logic_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	auto write_waitl_callback() { return m_write_waitl_cb.bind(); }
	auto write_onlil_callback() { return m_write_onlil_cb.bind(); }
	auto write_carl_callback() { return m_write_carl_cb.bind(); }
	auto write_errorl_callback() { return m_write_errorl_cb.bind(); }
	auto write_enql_callback() { return m_write_enql_cb.bind(); }
	auto write_ackl_callback() { return m_write_ackl_cb.bind(); }
	auto write_nakl_callback() { return m_write_nakl_cb.bind(); }

	// Input strobes
	void process_keyboard_char(uint8_t key);
	void w_cleark(int state);
	void w_linek(int state);
	void w_transk(int state);
	void w_break(int state);

protected:
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void rs232_rxd_w(int state);
	void rs232_dcd_w(int state);
	void rs232_dsr_w(int state);
	void rs232_cts_w(int state);
	void rs232_txd_w(int state);
	void rs232_ri_w(int state);
	void uart_rx(int state);
	void uart_tx(int state);

	required_device<screen_device>  	m_screen;
	required_device<palette_device> 	m_palette;
	required_memory_region          	m_font;
	memory_share_creator<uint8_t>   	m_vram;
	required_device<beep_device>		m_beep;
	required_device<ay51013_device>		m_uart;
	required_device<clock_device>		m_uart_clock;
	required_device<rs232_port_device>	m_rs232;

private:

	devcb_write_line m_write_waitl_cb;
	devcb_write_line m_write_onlil_cb;
	devcb_write_line m_write_carl_cb;
	devcb_write_line m_write_errorl_cb;
	devcb_write_line m_write_enql_cb;
	devcb_write_line m_write_ackl_cb;
	devcb_write_line m_write_nakl_cb;

	// Video
	uint8_t attribute;
	int frame_counter;
	bool video_enable;
	void data_to_display(uint8_t byte);
	void char_to_display(uint8_t byte);

	// Video settings
	bool blank_ctrl_chars;
	bool blank_attr_chars;
	bool underline_mode;
	bool extend_dot7;
	bool full_inverse_video;

	// Cursor
	int cursor_x;
	int cursor_y;
	bool cursor_x_input;
	bool cursor_y_input;
	void advance_cursor();

	// Cursor settings
	bool block_cursor;
	bool blinking_cursor;
	bool auto_cr_lf;
	bool auto_roll_up;
	bool underline_input;

	// Sound
	bool speed_check;
	emu_timer *m_beep_trigger;
	emu_timer *m_speed_ctrl;
	TIMER_CALLBACK_MEMBER(expire_speed_check);
	TIMER_CALLBACK_MEMBER(end_beep);

	// RS232
	bool data_terminal_ready;
	bool request_to_send;
	bool rx_handshake;
	bool tx_handshake;

	// RS232 settings
	void update_rs232_lamps();
	void check_rs232_rx_error(int state);
	bool force_on_line;
	bool dcd_handshake;
	bool hold_line_key_for_rts;
	bool need_tx_for_on_line_led;
	bool eight_bit_uart;
	bool no_parity;
	bool even_parity;
	bool two_stop_bits;
	bool ext_echo;
	bool auto_rx_handshake;
	bool auto_tx_handshake;

	// Keyboard
	bool break_key_held;
	bool tx_data_pending_kbd;
	uint8_t tx_data_kbd;
	bool clear_ascii_hs_by_cleark;
};

// device type definition
DECLARE_DEVICE_TYPE(TANDBERG_TDV2100_DISPLAY_LOGIC, tandberg_tdv2100_disp_logic_device)

#endif // MAME_TANDBERG_DISP_LOGIC_H
