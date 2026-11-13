// license:BSD-3-Clause
// copyright-holders: Olivier Galibert

// ADB - Apple Desktop Bus
//
// High-level device protocol engine

#ifndef MAME_BUS_ADB_ADBHLE_H
#define MAME_BUS_ADB_ADBHLE_H

#pragma once

#include "adb.h"

#include <span>

class adb_hle_device : public adb_device_interface, public adb_slot_card_interface
{
public:
	enum class address_mode : u8
	{
		MOVABLE,
		EXTENDED
	};

	u8 adb_address() const
	{
		return m_address;
	}

	u8 adb_handler() const
	{
		return m_handler;
	}

	bool adb_service_request_enabled() const
	{
		return m_service_request_enabled;
	}

	bool adb_collision() const
	{
		return m_collision;
	}

	void set_service_request(bool state)
	{
		m_service_request = state;
	}

	void set_exceptional_event(bool state)
	{
		m_exceptional_event = state;
	}

protected:
	adb_hle_device(
			const machine_config &mconfig,
			device_type type,
			const char *tag,
			device_t *owner,
			u32 clock,
			u8 default_address,
			u8 default_handler,
			address_mode mode = address_mode::MOVABLE);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void adb_poll_inputs() override final;

	// Registers zero through two are supplied by the concrete device.  Talk data
	// is not consumed until adb_talk_complete reports successful arbitration.
	virtual unsigned adb_talk(u8 reg, std::span<u8> data) = 0;
	virtual void adb_talk_complete(u8 reg, bool success)
	{
	}

	virtual unsigned adb_listen_length(u8 reg) const
	{
		return 2;
	}

	virtual void adb_listen(u8 reg, std::span<u8 const> data)
	{
	}

	virtual void adb_poll()
	{
	}

	virtual void adb_flush()
	{
	}

	virtual void adb_bus_reset()
	{
	}

	// Handler changes not implemented by a concrete device are rejected.
	virtual bool adb_set_handler(u8 handler)
	{
		return handler == m_default_handler;
	}

	virtual bool adb_activator_pressed() const
	{
		return false;
	}

	virtual bool adb_self_test()
	{
		return true;
	}

	// Extended-address devices return their unique two-to-eight-byte address.
	virtual unsigned adb_extended_address(std::span<u8> data) const
	{
		return 0;
	}

	virtual unsigned adb_service_request_time() const
	{
		return 300;
	}

private:
	struct phase
	{
		enum : u8
		{
			IDLE,
			SYNC,
			COMMAND,
			COMMAND_STOP,
			LISTEN_WAIT,
			LISTEN_START,
			LISTEN_DATA,
			LISTEN_STOP,
			TALK_WAIT,
			TRANSMIT,
			RESET_HOLD
		};
	};

	struct action
	{
		enum : u8
		{
			NONE,
			SEND_RESET,
			FLUSH,
			LISTEN,
			TALK
		};
	};

	virtual void adb_w(int state) override final;

	TIMER_CALLBACK_MEMBER(reset_tick);
	TIMER_CALLBACK_MEMBER(receive_timeout);
	TIMER_CALLBACK_MEMBER(service_request_done);
	TIMER_CALLBACK_MEMBER(talk_turnaround_done);
	TIMER_CALLBACK_MEMBER(transmit_tick);

	void line_fell(attotime duration);
	void line_rose(attotime duration);
	void begin_attention();
	void begin_command_bit();
	void command_stop_started();
	void command_stop_finished();
	void begin_listen();
	void finish_listen();
	void abort_receive();

	void prepare_talk(u8 reg);
	void begin_talk();
	void begin_transmit_cell();
	void finish_talk(bool success, bool collision);
	bool transmit_bit() const;

	void assert_service_request();
	void reset_protocol(bool notify_device, u8 next_phase);
	void prepare_register_three();
	void listen_register_three();
	void listen_extended_address();
	u8 next_random_nibble();

	static u32 microseconds(attotime duration);
	static bool valid_bit_low(u32 duration);
	static bool valid_cell(u32 low, u32 high);
	static bool decode_bit(u32 duration);

	u8 const m_default_address;
	u8 const m_default_handler;
	address_mode const m_address_mode;

	emu_timer *m_reset_timer;
	emu_timer *m_receive_timer;
	emu_timer *m_service_request_timer;
	emu_timer *m_talk_timer;
	emu_timer *m_transmit_timer;

	u8 m_phase;
	u8 m_action;
	bool m_line_state;
	bool m_service_request;
	bool m_service_request_enabled;
	bool m_service_request_asserted;
	bool m_exceptional_event;
	bool m_collision;
	bool m_self_test_failed;
	bool m_active;
	bool m_talk_prepared;
	bool m_talk_common;
	bool m_transmit_low;
	bool m_driving_low;

	u8 m_address;
	u8 m_handler;
	u8 m_command;
	u8 m_command_bits;
	u8 m_action_register;
	u8 m_receive_byte;
	u8 m_receive_bits;
	u8 m_receive_length;
	u8 m_receive_buffer[8];
	u8 m_transmit_length;
	u8 m_transmit_cell;
	u8 m_transmit_buffer[8];
	u8 m_extended_length;
	u8 m_extended_buffer[8];
	u8 m_random;

	u32 m_last_low_duration;
	attotime m_last_edge_time;
	attotime m_command_bit_zero_time;
};

#endif // MAME_BUS_ADB_ADBHLE_H
