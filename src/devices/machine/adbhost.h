// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*
    adbhost.h - Apple Desktop Bus host HLE mix-in
	by R. Belmont
*/

#ifndef MAME_MACHINE_ADBHOST_H
#define MAME_MACHINE_ADBHOST_H

#pragma once

class adb_hle_host_interface : public device_interface
{
public:
	enum class adb_host_status : u8
	{
		OK,             // transaction completed
		NO_RESPONSE,    // Talk: no start bit within the response window
		DATA_LOST,      // Talk: malformed bit cell or missing edge mid-byte; or the line never rose after the command stop bit
		OVERRUN,        // the owner refused a received byte
		NO_DATA         // Listen ended without any data byte
	};

	// owner -> engine
	void adb_host_line_w(int state);
	void adb_host_reset();
	void adb_host_command(u8 command);
	bool adb_host_listen_data_w(u8 data);
	void adb_host_abort();
	bool adb_host_idle() const { return m_adb_phase == adb_phase::IDLE; }

protected:
	adb_hle_host_interface(const machine_config &mconfig, device_t &device);

	// engine -> owner
	virtual void adb_host_drive_line(int state) = 0;
	virtual void adb_host_tx_empty() { }
	virtual bool adb_host_listen_more() const { return false; }
	virtual void adb_host_command_sent(bool srq) { }
	virtual bool adb_host_rx_byte(u8 data) { return true; }
	virtual void adb_host_transaction_done(adb_host_status status) { }
	virtual void adb_host_reset_done() { }

	// device_interface implementation
	virtual void interface_pre_start() override;
	virtual void interface_post_start() override;
	virtual void interface_post_reset() override;

private:
	struct adb_phase
	{
		enum : u8
		{
			IDLE,
			RESET_LOW,
			ATTENTION_LOW,
			SYNC_HIGH,
			TRANSMIT_LOW,
			TRANSMIT_HIGH,
			COMMAND_STOP_LOW,
			COMMAND_STOP_WAIT,
			LISTEN_TURNAROUND,
			LISTEN_START_LOW,
			LISTEN_START_HIGH,
			LISTEN_WAIT_DATA,
			LISTEN_STOP_LOW,
			TALK_WAIT,
			TALK_START_LOW,
			TALK_BIT_LOW,
			TALK_BIT_HIGH,
			TALK_STOP_HIGH
		};
	};

	TIMER_CALLBACK_MEMBER(adb_tick);

	void adb_start_transmit_bit();
	void adb_finish_transmit_byte();
	void adb_finish_command_stop(bool srq);
	void adb_start_listen();
	bool adb_load_listen_byte();
	void adb_start_talk();
	void adb_receive_bit(bool bit);
	void adb_receive_byte();
	void adb_finish_talk(adb_host_status status);
	static u32 adb_microseconds(attotime duration);

	emu_timer *m_adb_timer;
	u8 m_adb_phase;
	bool m_adb_line;
	bool m_adb_tx_data_valid;
	bool m_adb_transmitting_command;
	bool m_adb_listen_started;
	u8 m_adb_command;
	u8 m_adb_tx_data;
	u8 m_adb_tx_shift;
	u8 m_adb_tx_bit;
	u8 m_adb_rx_shift;
	u8 m_adb_rx_bit;
	bool m_adb_received_byte;
	u32 m_adb_last_low;
	attotime m_adb_last_edge;
};

#endif // MAME_MACHINE_ADBHOST_H
