// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    ticket.h

    Generic ticket dispensing device.

***************************************************************************/
#ifndef MAME_MACHINE_TICKET_H
#define MAME_MACHINE_TICKET_H

#pragma once


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DECLARE_DEVICE_TYPE(TICKET_DISPENSER, ticket_dispenser_device)
DECLARE_DEVICE_TYPE(HOPPER, hopper_device)


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> ticket_dispenser_device

class ticket_dispenser_device : public device_t
{
public:
	static inline constexpr uint8_t MOTOR_ACTIVE_LOW = 0;    // activated by state = 0
	static inline constexpr uint8_t MOTOR_ACTIVE_HIGH = 1;   // activated by state = 1

	static inline constexpr uint8_t STATUS_ACTIVE_LOW = 0;   // output 0 when dispensing is done
	static inline constexpr uint8_t STATUS_ACTIVE_HIGH = 1;  // output 1 when dispensing is done

	// construction/destruction
	ticket_dispenser_device(const machine_config &mconfig, const char *tag, device_t *owner, const attotime &period, uint8_t motor_sense, uint8_t status_sense)
		: ticket_dispenser_device(mconfig, tag, owner)
	{
		set_period(period);
		set_senses(motor_sense, status_sense, false);
	}
	ticket_dispenser_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
	virtual ~ticket_dispenser_device();

	// inline configuration helpers
	void set_period(const attotime &period) { m_period = period; }
	void set_senses(uint8_t motor_sense, uint8_t status_sense, bool hopper_type)
	{
		m_motor_sense = motor_sense;
		m_status_sense = status_sense;
		m_hopper_type = hopper_type;
	}

	auto dispense_handler() { return m_dispense_handler.bind(); }

	// read/write handlers
	int line_r();
	void motor_w(int state);

protected:
	ticket_dispenser_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock = 0);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	TIMER_CALLBACK_MEMBER(update_output_state);

	// configuration state
	uint8_t m_motor_sense;
	uint8_t m_status_sense;
	attotime m_period;
	bool m_hopper_type;

	// active state
	bool m_motoron;
	bool m_ticketdispensed;
	bool m_ticketnotdispensed;

	bool m_status;
	bool m_power;
	emu_timer *m_timer;
	output_finder<> m_output;
	devcb_write_line m_dispense_handler;
};

class hopper_device : public ticket_dispenser_device
{
public:
	// construction/destruction
	hopper_device(const machine_config &mconfig, const char *tag, device_t *owner, const attotime &period, uint8_t motor_sense, uint8_t status_sense)
		: hopper_device(mconfig, tag, owner)
	{
		set_period(period);
		set_senses(motor_sense, status_sense, true);
	}
	hopper_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

private:

};

#endif // MAME_MACHINE_TICKET_H
