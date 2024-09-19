// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    ticket.cpp

    Generic ticket dispensing device.

***************************************************************************/

#include "emu.h"
#include "ticket.h"

#define VERBOSE (0)
#include "logmacro.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(TICKET_DISPENSER, ticket_dispenser_device, "ticket_dispenser", "Ticket Dispenser")
DEFINE_DEVICE_TYPE(HOPPER, hopper_device, "coin_hopper", "Coin Hopper")



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  constructor
//-------------------------------------------------

ticket_dispenser_device::ticket_dispenser_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_period(attotime::from_msec(100))
	, m_hopper_type(false)
	, m_status(false)
	, m_power(false)
	, m_timer(nullptr)
	, m_output(*this, tag) // TODO: change to "tag:status"
	, m_dispense_handler(*this)
{
}

ticket_dispenser_device::ticket_dispenser_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ticket_dispenser_device(mconfig, TICKET_DISPENSER, tag, owner, clock)
{
	m_hopper_type = false;
}

hopper_device::hopper_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ticket_dispenser_device(mconfig, HOPPER, tag, owner, clock)
{
	m_hopper_type = true;
}


//-------------------------------------------------
//  destructor
//-------------------------------------------------

ticket_dispenser_device::~ticket_dispenser_device()
{
}



//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

//-------------------------------------------------
//  line_r - read the status line
//-------------------------------------------------

int ticket_dispenser_device::line_r()
{
	return m_status ? 1 : 0;
}


//-------------------------------------------------
//  motor_w - write the control line
//-------------------------------------------------

void ticket_dispenser_device::motor_w(int state)
{
	// On rising edge, start dispensing!
	if (state)
	{
		if (!m_power)
		{
			LOG("%s: Ticket Power On\n", machine().describe_context());
			m_timer->adjust(m_period);
			m_power = true;
			m_status = false;
		}
	}
	else
	{
		if (m_power)
		{
			if (!m_hopper_type || !m_status)
			{
				LOG("%s: Ticket Power Off\n", machine().describe_context());
				m_timer->adjust(attotime::never);
				m_output = 0;
			}
			m_power = false;
		}
	}
}


//**************************************************************************
//  DEVICE INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_start - handle device startup
//-------------------------------------------------

void ticket_dispenser_device::device_start()
{
	m_timer = timer_alloc(FUNC(ticket_dispenser_device::update_output_state), this);

	m_output.resolve();

	save_item(NAME(m_status));
	save_item(NAME(m_power));
}


//-------------------------------------------------
//  device_reset - handle device startup
//-------------------------------------------------

void ticket_dispenser_device::device_reset()
{
	m_status = false;
	m_power = false;
}


//-------------------------------------------------
//  update_output_state -
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(ticket_dispenser_device::update_output_state)
{
	// if we still have power, keep toggling ticket states
	if (m_power)
	{
		m_status = !m_status;
		LOG("Ticket Status Changed to %02X\n", m_status);
		m_timer->adjust(m_period);
	}
	else if (m_hopper_type)
	{
		m_status = !m_status;
		LOG("%s: Ticket Power Off\n", machine().describe_context());
		m_timer->adjust(attotime::never);
		m_output = 0;
	}

	// update output status
	m_output = m_status;

	if (m_hopper_type)
	{
		m_dispense_handler(m_status);
	}

	// if we just dispensed, increment global count
	if (m_status)
	{
		machine().bookkeeping().increment_dispensed_tickets(1);
		LOG("Ticket Dispensed\n");
	}
}
