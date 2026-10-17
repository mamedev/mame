// license:BSD-3-Clause
// copyright-holders:vklachkov

#ifndef MAME_GRIDCOMP_GRIDMODEM_H
#define MAME_GRIDCOMP_GRIDMODEM_H

#pragma once

#include "emu.h"

#include <cstdint>
#include <memory>
#include <string>

class grid_modem_device : public device_t
{
public:
	grid_modem_device(
			const machine_config &mconfig,
			const char *tag,
			device_t *owner,
			u32 clock);
	~grid_modem_device();

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

	void set_remote_host(const char *host);
	void set_remote_port(const char *port);

	auto int_cb() { return m_int_func.bind(); }

protected:
	grid_modem_device(
			const machine_config &mconfig,
			device_type type,
			const char *tag,
			device_t *owner,
			u32 clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_stop() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_post_load() override;

private:
	class network_context;

	enum class protocol_state : uint8_t
	{
		IDLE,
		EXPECT_SELECTOR,
		WRITE_REGISTER_SELECTED,
		WRITE_REGISTER_VALUE,
		READ_STATUS_SELECTED,
		EXEC_COMMAND_SELECTED
	};

	static constexpr uint8_t C4_READY = 0x80;
	static constexpr uint8_t C4_DATA_READY = 0x20;

	static constexpr uint8_t STATUS1_OPERATION_COMPLETE = 0x01;
	static constexpr uint8_t STATUS1_LINE_READY = 0x04;
	static constexpr uint8_t STATUS1_CARRIER_DETECTED = 0x10;

	static constexpr uint8_t STATUS2_TX_READY = 0x02;
	static constexpr uint8_t STATUS2_RX_BYTE_AVAILABLE = 0x04;
	static constexpr uint8_t STATUS2_LINE_EVENT = 0x80;
	static constexpr uint16_t RX_QUEUE_SIZE = 256;
	static constexpr uint8_t SERIAL_FRAME_BITS = 10;

	static constexpr uint8_t STATUS3_MODEM_VERSION = 20;

	void command_port_w(uint8_t data);
	void data_port_w(uint8_t data);

	void begin_protocol_transaction();
	void commit_protocol_transaction();

	void write_internal_register(uint8_t reg, uint8_t data);
	void handle_reg_1_write(uint8_t data);
	void handle_reg_6_write(uint8_t data);
	void handle_reg_7_write(uint8_t data);
	void enter_online_data_mode();
	void leave_online_data_mode();
	void execute_modem_command(uint8_t command);
	void execute_off_hook_command();
	void execute_hang_up_command();
	void execute_handshake_command(uint8_t command);
	void execute_pulse_dial_command(uint8_t command);

	uint8_t read_status_index(uint8_t index);
	uint8_t read_status1() const;
	uint8_t read_status2();

	void reset_hle_state();
	void raise_line_event();
	void raise_rx_event();
	void raise_tx_event();
	attotime serial_byte_delay() const;
	u64 socket_byte_delay_nsec() const;
	attotime next_serial_delay(attotime &ready_time) const;
	void reset_serial_timing();
	void schedule_rx_ready_event(attotime delay);
	void schedule_tx_ready_event(attotime delay);
	void acknowledge_irq();
	void clear_irq();

	void start_socket_connect();
	void stop_socket();
	bool socket_connected() const;
	void process_network_events();
	void handle_socket_connected();
	void handle_socket_connect_failed(const std::string &message);
	void handle_socket_disconnected(const std::string &message);
	void arm_socket_rx();
	void send_tx_byte(uint8_t data);
	void queue_rx_byte(uint8_t data);
	uint8_t dequeue_rx_byte();
	bool rx_queue_empty() const;
	bool rx_queue_full() const;
	void clear_rx_queue();

	void log_tx_byte(uint8_t data);
	void log_reg_2_write(uint8_t data);

	TIMER_CALLBACK_MEMBER(rx_poll_timer);
	TIMER_CALLBACK_MEMBER(rx_ready_timer);
	TIMER_CALLBACK_MEMBER(tx_ready_timer);

	protocol_state m_protocol_state = protocol_state::IDLE;

	uint8_t m_selected_reg = 0;
	uint8_t m_requested_status = 0;
	uint8_t m_return_value = 0;
	uint8_t m_command = 0;

	uint8_t m_bus_status = C4_READY;

	uint8_t m_internal_regs[9]{};

	uint8_t m_control_byte = 0;

	bool m_raw_reset_pending = false;

	bool m_off_hook = false;
	bool m_carrier = false;
	bool m_online = false;
	bool m_rx_ready = true;
	bool m_tx_ready = true;
	uint16_t m_baud_rate = 1200;
	attotime m_rx_ready_time;
	attotime m_tx_ready_time;

	bool m_irq_pending = false;
	uint8_t m_irq_status2 = STATUS2_TX_READY;

	emu_timer *m_rx_poll_timer = nullptr;
	emu_timer *m_rx_ready_timer = nullptr;
	emu_timer *m_tx_ready_timer = nullptr;

	uint8_t m_rx_queue[RX_QUEUE_SIZE]{};
	uint16_t m_rx_queue_count = 0;

	std::string m_remote_host = "localhost";
	uint16_t m_remote_port = 15112;

	std::unique_ptr<network_context> m_network;

	devcb_write_line m_int_func;
};


DECLARE_DEVICE_TYPE(GRID_MODEM, grid_modem_device)

#endif
