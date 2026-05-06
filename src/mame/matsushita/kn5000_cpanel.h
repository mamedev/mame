// license:GPL2+
// copyright-holders:Felipe Sanches
/***************************************************************************

	KN5000 control panel HLE

	Emulates the two Mitsubishi M37471M2196S MCUs on the control panel.
	Since no ROM dumps are available, this uses High Level Emulation based
	on reverse engineering of the main CPU firmware protocol.

	Protocol documentation: https://felipesanches.github.io/kn5000-docs/control-panel-protocol/

***************************************************************************/

#ifndef MAME_MATSUSHITA_KN5000_CPANEL_H
#define MAME_MATSUSHITA_KN5000_CPANEL_H

#pragma once

#include <queue>

class kn5000_cpanel_device :
	public device_t
{
public:
	kn5000_cpanel_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// Serial interface from main CPU
	void rxd(int state);
	void sioclk(int state);
	void tx_start(int state);  // Called when CPU starts a new byte transmission

	// Callbacks to main CPU
	auto txd() { return m_txd_cb.bind(); }
	auto sclk_out() { return m_sclk_out_cb.bind(); }
	auto inta() { return m_inta_cb.bind(); }

	// Configuration
	void set_baudrate(uint16_t br);

	// Button input port setters (called from main driver)
	void set_cpl_port(int n, ioport_port *port) { m_cpl_ports[n] = port; }
	void set_cpr_port(int n, ioport_port *port) { m_cpr_ports[n] = port; }

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(timer_callback);
	TIMER_CALLBACK_MEMBER(idle_detect_callback);
	TIMER_CALLBACK_MEMBER(self_clock_callback);
	TIMER_CALLBACK_MEMBER(button_scan_callback);

private:
	// Serial communication
	void send_byte(uint8_t data);
	void process_received_byte(uint8_t data);
	void process_command();

	// Response generation
	void send_sync_packet();
	void send_button_packet(int segment, bool is_left_panel);
	void send_all_button_states(bool is_left_panel);

	// LED control
	void process_led_command(uint8_t row, uint8_t data);

	// Read button state from input ports
	uint8_t read_button_segment(int segment, bool is_left_panel);

	// Timers
	emu_timer *m_timer;
	emu_timer *m_idle_detect_timer;
	emu_timer *m_self_clock_timer;
	emu_timer *m_button_scan_timer;
	uint16_t m_baud_rate;

	// Serial RX state
	uint8_t m_rx_clock_count;
	uint8_t m_rx_shift_register;
	uint8_t m_rxd;
	uint8_t m_sioclk_state;

	// Serial TX state
	uint8_t m_tx_clock_count;
	uint8_t m_tx_shift_register;
	std::queue<uint8_t> m_tx_queue;
	bool m_tx_skip_first_falling;  // Skip first falling edge after pre-outputting bit 0

	// Command buffer (2-byte commands)
	uint8_t m_cmd_buffer[2];
	uint8_t m_cmd_index;

	// Protocol state
	bool m_initialized;
	bool m_self_clocking;
	bool m_inta_asserted;
	bool m_accept_next_byte;   // false = next received byte is phantom (PFFC-off), skip it
	bool m_tx_output_enabled;  // false = suppress TX output during phantom byte clock edges
	bool m_next_accept;        // Deferred accept_next_byte (applied at next byte boundary)
	bool m_next_tx_output_enabled;  // Deferred tx_output_enabled (applied at next byte boundary)
	bool m_rx_waiting_for_start;    // Ignore RX edges until next tx_start
	uint8_t m_self_clock_bytes_sent;  // Bytes sent in current INTA cycle
	uint8_t m_last_button_state[22];  // 11 segments * 2 panels (confirmed)
	uint8_t m_pending_button_state[22];  // Per-segment confirmation buffer

	// Callbacks
	devcb_write_line m_txd_cb;
	devcb_write_line m_sclk_out_cb;
	devcb_write_line m_inta_cb;

	// Input port pointers (set by main driver)
	ioport_port *m_cpl_ports[11];  // Left panel segments 0-10
	ioport_port *m_cpr_ports[11];  // Right panel segments 0-10

	// LED outputs
	output_finder<50> m_cpl_leds;  // Left panel LEDs (CPL_0 through CPL_49)
	output_finder<69> m_cpr_leds;  // Right panel LEDs (CPR_0 through CPR_68)
};

DECLARE_DEVICE_TYPE(KN5000_CPANEL, kn5000_cpanel_device)

#endif // MAME_MATSUSHITA_KN5000_CPANEL_H
