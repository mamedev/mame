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
	// construction/destruction
	ticket_dispenser_device(const machine_config &mconfig, const char *tag, device_t *owner, const attotime &period)
		: ticket_dispenser_device(mconfig, tag, owner)
	{
		set_period(period);
	}
	ticket_dispenser_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
	virtual ~ticket_dispenser_device();

	// inline configuration helpers
	void set_period(const attotime &period) { m_period = period; }
	auto dispense_handler() { return m_dispense_handler.bind(); }

	// read/write handlers
	int line_r();
	void motor_w(int state);

protected:
	ticket_dispenser_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock = 0);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(update_output_state);

	// configuration state
	attotime m_period;
	bool m_hopper_type;

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
	hopper_device(const machine_config &mconfig, const char *tag, device_t *owner, const attotime &period)
		: hopper_device(mconfig, tag, owner)
	{
		set_period(period);
	}
	hopper_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
};

#endif // MAME_MACHINE_TICKET_H
