// license:BSD-3-Clause
// copyright-holders:Ryan Holtz, Vas Crabb
#ifndef MAME_BUS_VSMILE_KEYBOARD_H
#define MAME_BUS_VSMILE_KEYBOARD_H

#pragma once

#include "vsmile_ctrl.h"

#include "machine/keyboard.h"

/***************************************************************************
 TYPE DEFINITIONS
 ***************************************************************************/

// ======================> vsmile_keyboard_device

class vsmile_keyboard_device : public vsmile_ctrl_device_base, protected device_matrix_keyboard_interface<5U>
{
public:
	// construction/destruction
	virtual ~vsmile_keyboard_device();

	// input handlers
	DECLARE_INPUT_CHANGED_MEMBER(joy_changed);
	DECLARE_INPUT_CHANGED_MEMBER(key_changed);
	DECLARE_INPUT_CHANGED_MEMBER(button_changed);

	enum stale_non_key_inputs : uint16_t
	{
		STALE_NONE          = 0U,
		STALE_LEFT_RIGHT    = 1U << 0,
		STALE_UP_DOWN       = 1U << 1,
		STALE_OK            = 1U << 3,
		STALE_QUIT          = 1U << 4,
		STALE_HELP          = 1U << 5,

		STALE_JOY           = STALE_LEFT_RIGHT | STALE_UP_DOWN,
		STALE_BUTTONS       = STALE_OK | STALE_QUIT | STALE_HELP,
		STALE_ALL           = STALE_JOY | STALE_BUTTONS
	};

	enum comms_state : uint8_t
	{
		STATE_POWERUP           = 0U,
		STATE_HELLO_MESSAGE,
		STATE_HELLO_RECEIVE_BYTE1,
		STATE_HELLO_RECEIVE_BYTE2,
		STATE_HELLO_REPLY_BYTE1,
		STATE_HELLO_REPLY_BYTE2,
		STATE_HELLO_REPLY_BYTE3,
		STATE_RUNNING
	};

protected:
	vsmile_keyboard_device(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, uint32_t clock, uint8_t layout_type);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// vsmile_ctrl_device_base implementation
	virtual void tx_complete() override;
	virtual void tx_timeout() override;
	virtual void rx_complete(uint8_t data, bool cts) override;

	// device_matrix_keyboard_interface implementation
	virtual void key_make(uint8_t row, uint8_t column) override;
	virtual void key_break(uint8_t row, uint8_t column) override;

	uint16_t stale_all() { return STALE_ALL; }
	bool translate(uint8_t code, uint8_t &translated) const;

	void enter_active_state();

	void uart_tx_fifo_push(uint8_t data);

	required_ioport m_io_joy;
	required_ioport_array<5> m_io_keys;
	required_ioport m_io_buttons;

	const uint8_t m_layout_type;

	uint8_t m_state;

	uint16_t m_sent_joy, m_sent_buttons;

	uint16_t m_stale;
	bool m_active;
	emu_timer *m_idle_timer;
	emu_timer *m_hello_timer;
	emu_timer *m_hello_timeout_timer;

private:
	TIMER_CALLBACK_MEMBER(handle_idle);
	TIMER_CALLBACK_MEMBER(handle_hello);
	TIMER_CALLBACK_MEMBER(handle_hello_timeout);

	uint8_t m_ctrl_probe_history[2];
};

class vsmile_keyboard_us_device : public vsmile_keyboard_device
{
public:
	// construction/destruction
	vsmile_keyboard_us_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock = 0U);

protected:
	// device_t implementation
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
};

class vsmile_keyboard_fr_device : public vsmile_keyboard_device
{
public:
	// construction/destruction
	vsmile_keyboard_fr_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock = 0U);

protected:
	// device_t implementation
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
};

class vsmile_keyboard_ge_device : public vsmile_keyboard_device
{
public:
	// construction/destruction
	vsmile_keyboard_ge_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock = 0U);

protected:
	// device_t implementation
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
};

/***************************************************************************
 DEVICE TYPES
 ***************************************************************************/

DECLARE_DEVICE_TYPE(VSMILE_KEYBOARD_US, vsmile_keyboard_us_device)
DECLARE_DEVICE_TYPE(VSMILE_KEYBOARD_FR, vsmile_keyboard_fr_device)
DECLARE_DEVICE_TYPE(VSMILE_KEYBOARD_GE, vsmile_keyboard_ge_device)

#endif // MAME_BUS_VSMILE_KEYBOARD_H
