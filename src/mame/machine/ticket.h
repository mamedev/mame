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

// device type definition
DECLARE_DEVICE_TYPE(TICKET_DISPENSER, ticket_dispenser_device)



//**************************************************************************
//  DEVICE CONFIGURATION MACROS
//**************************************************************************

// add/remove dispensers
#define MCFG_TICKET_DISPENSER_ADD(_tag, _period_in_msec, _motor_sense, _status_sense) \
	MCFG_DEVICE_ADD(_tag, TICKET_DISPENSER, 0) \
	ticket_dispenser_device::static_set_period(*device, _period_in_msec); \
	ticket_dispenser_device::static_set_senses(*device, _motor_sense, _status_sense, false);

#define MCFG_HOPPER_ADD(_tag, _period_in_msec, _motor_sense, _status_sense) \
	MCFG_DEVICE_ADD(_tag, TICKET_DISPENSER, 0) \
	ticket_dispenser_device::static_set_period(*device, _period_in_msec); \
	ticket_dispenser_device::static_set_senses(*device, _motor_sense, _status_sense, true);

//**************************************************************************
//  CONSTANTS
//**************************************************************************

const uint8_t TICKET_MOTOR_ACTIVE_LOW = 0;    /* Ticket motor is triggered by D7=0 */
const uint8_t TICKET_MOTOR_ACTIVE_HIGH = 1;    /* Ticket motor is triggered by D7=1 */

const uint8_t TICKET_STATUS_ACTIVE_LOW = 0;    /* Ticket is done dispensing when D7=0 */
const uint8_t TICKET_STATUS_ACTIVE_HIGH = 1;    /* Ticket is done dispensing when D7=1 */



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> ticket_dispenser_device

class ticket_dispenser_device : public device_t
{
public:
	// construction/destruction
	ticket_dispenser_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~ticket_dispenser_device();

	// inline configuration helpers
	static void static_set_period(device_t &device, const attotime &period);
	static void static_set_senses(device_t &device, uint8_t motor_sense, uint8_t status_sense, bool hopper_type);

	// read/write handlers
	DECLARE_READ8_MEMBER( read );
	DECLARE_READ_LINE_MEMBER( line_r );
	DECLARE_WRITE8_MEMBER( write );
	DECLARE_WRITE_LINE_MEMBER( motor_w );

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// configuration state
	uint8_t m_motor_sense;
	uint8_t m_status_sense;
	attotime m_period;
	bool m_hopper_type;

	// active state
	uint8_t m_active_bit;
	uint8_t m_motoron;
	uint8_t m_ticketdispensed;
	uint8_t m_ticketnotdispensed;

	uint8_t m_status;
	uint8_t m_power;
	emu_timer *m_timer;
};

#endif // MAME_MACHINE_TICKET_H
