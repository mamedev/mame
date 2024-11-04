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

	DECLARE_INPUT_CHANGED_MEMBER(rs232_changed);
	DECLARE_INPUT_CHANGED_MEMBER(rs232_lock_changed);
	DECLARE_INPUT_CHANGED_MEMBER(uart_changed);

	// Input strobes
	void process_keyboard_char(uint8_t key);
	void cleark_w(int state);
	void linek_w(int state);
	void transk_w(int state);
	void break_w(int state);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	// Video, Cursor & Sound
	void vblank(int state);
	void update_attribute(int at_row, int at_col, int from_row, int from_col);
	void char_to_display(uint8_t byte);
	void data_to_display(uint8_t byte);
	void advance_cursor();
	void place_cursor(int row, int col);
	void clear_key_handler();
	int get_ram_addr(int row, int col);
	void erase_row(int row);
	TIMER_CALLBACK_MEMBER(expire_speed_check);
	TIMER_CALLBACK_MEMBER(end_beep);

	// RS-232
	void rs232_rxd_w(int state);
	void rs232_txd_w(int state);
	void rs232_dcd_w(int state);
	void rs232_dsr_w(int state);
	void rs232_cts_w(int state);
	void update_all_rs232_signal_paths();
	void rs232_ri_w(int state);
	void uart_rx(int state);
	void uart_tx(int state);
	void update_rs232_lamps();
	void check_rs232_rx_error(int state);
	void set_uart_state_from_switches();
	void clock_on_line_flip_flop();
	void clock_transmit_flip_flop();

	required_device<screen_device>      m_screen;
	required_device<palette_device>     m_palette;
	required_region_ptr<uint8_t>        m_font;
	required_region_ptr<uint8_t>        m_addr_offsets;
	memory_share_creator<uint8_t>       m_vram;
	required_device<beep_device>        m_beep;
	required_device<ay51013_device>     m_uart;
	required_device<clock_device>       m_uart_clock;
	required_device<rs232_port_device>  m_rs232;
	required_ioport                     m_sw_invert_video;
	required_ioport                     m_sw_rs232_baud;
	required_ioport                     m_sw_rs232_settings;
	required_ioport                     m_dsw_u39;
	required_ioport                     m_dsw_u73;
	required_ioport                     m_dsw_u61;

	devcb_write_line                    m_write_waitl_cb;
	devcb_write_line                    m_write_onlil_cb;
	devcb_write_line                    m_write_carl_cb;
	devcb_write_line                    m_write_errorl_cb;
	devcb_write_line                    m_write_enql_cb;
	devcb_write_line                    m_write_ackl_cb;
	devcb_write_line                    m_write_nakl_cb;

	// Video, Cursor & Sound
	uint8_t m_frame_counter;
	bool m_video_enable;
	bool m_vblank_state;
	uint8_t m_cursor_row;
	uint8_t m_cursor_col;
	uint8_t m_page_roll;
	bool m_cursor_row_input;
	bool m_cursor_col_input;
	bool m_underline_input;
	bool m_speed_check;
	uint8_t m_attribute;
	uint8_t m_char_max_row;
	uint8_t m_char_max_col;
	emu_timer *m_beep_trigger;
	emu_timer *m_speed_ctrl;

	// RS-232
	bool m_data_terminal_ready;
	bool m_request_to_send;
	bool m_rx_handshake;
	bool m_tx_handshake;

	// Keyboard
	bool m_clear_key_held;
	bool m_line_key_held;
	bool m_trans_key_held;
	bool m_break_key_held;
	bool m_data_pending_kbd;
	uint8_t m_data_kbd;
};

// device type definition
DECLARE_DEVICE_TYPE(TANDBERG_TDV2100_DISPLAY_LOGIC, tandberg_tdv2100_disp_logic_device)

#endif // MAME_TANDBERG_DISP_LOGIC_H
