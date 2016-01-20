// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    ticket.h

    Generic ticket dispensing device.

***************************************************************************/

#pragma once


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
extern const device_type TICKET_DISPENSER;



//**************************************************************************
//  DEVICE CONFIGURATION MACROS
//**************************************************************************

// add/remove dispensers
#define MCFG_TICKET_DISPENSER_ADD(_tag, _period_in_msec, _motor_sense, _status_sense) \
	MCFG_DEVICE_ADD(_tag, TICKET_DISPENSER, 0) \
	ticket_dispenser_device::static_set_period(*device, _period_in_msec); \
	ticket_dispenser_device::static_set_senses(*device, _motor_sense, _status_sense);


//**************************************************************************
//  CONSTANTS
//**************************************************************************

const UINT8 TICKET_MOTOR_ACTIVE_LOW = 0;    /* Ticket motor is triggered by D7=0 */
const UINT8 TICKET_MOTOR_ACTIVE_HIGH = 1;    /* Ticket motor is triggered by D7=1 */

const UINT8 TICKET_STATUS_ACTIVE_LOW = 0;    /* Ticket is done dispensing when D7=0 */
const UINT8 TICKET_STATUS_ACTIVE_HIGH = 1;    /* Ticket is done dispensing when D7=1 */



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> ticket_dispenser_device

class ticket_dispenser_device : public device_t
{
public:
	// construction/destruction
	ticket_dispenser_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	virtual ~ticket_dispenser_device();

	// inline configuration helpers
	static void static_set_period(device_t &device, const attotime &period);
	static void static_set_senses(device_t &device, UINT8 motor_sense, UINT8 status_sense);

	// read/write handlers
	DECLARE_READ8_MEMBER( read );
	DECLARE_READ_LINE_MEMBER( line_r );
	DECLARE_WRITE8_MEMBER( write );

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// configuration state
	UINT8 m_motor_sense;
	UINT8 m_status_sense;
	attotime m_period;

	// active state
	UINT8 m_active_bit;
	UINT8 m_motoron;
	UINT8 m_ticketdispensed;
	UINT8 m_ticketnotdispensed;

	UINT8 m_status;
	UINT8 m_power;
	emu_timer *m_timer;
};
