// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    ticket.c

    Generic ticket dispensing device.

***************************************************************************/

#include "emu.h"
#include "machine/ticket.h"


//**************************************************************************
//  DEBUGGING
//**************************************************************************

#define DEBUG_TICKET 0
#define LOG(x) do { if (DEBUG_TICKET) logerror x; } while (0)



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
//  ticket_dispenser_device - constructor
//-------------------------------------------------

ticket_dispenser_device::ticket_dispenser_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_motor_sense(TICKET_MOTOR_ACTIVE_LOW)
	, m_status_sense(TICKET_STATUS_ACTIVE_LOW)
	, m_period(attotime::from_msec(100))
	, m_hopper_type(false)
	, m_motoron(0)
	, m_ticketdispensed(0)
	, m_ticketnotdispensed(0)
	, m_status(0)
	, m_power(0)
	, m_timer(nullptr)
	, m_output(*this, tag) // TODO: change to "tag:status"
{
}

ticket_dispenser_device::ticket_dispenser_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ticket_dispenser_device(mconfig, TICKET_DISPENSER, tag, owner, clock)
{
}

hopper_device::hopper_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ticket_dispenser_device(mconfig, HOPPER, tag, owner, clock)
{
}

//-------------------------------------------------
//  ~ticket_dispenser_device - destructor
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

READ_LINE_MEMBER( ticket_dispenser_device::line_r )
{
	return m_status ? 1 : 0;
}


//-------------------------------------------------
//  motor_w - write the control line
//-------------------------------------------------

WRITE_LINE_MEMBER( ticket_dispenser_device::motor_w )
{
	// On an activate signal, start dispensing!
	if (bool(state) == m_motoron)
	{
		if (!m_power)
		{
			LOG(("%s: Ticket Power On\n", machine().describe_context()));
			m_timer->adjust(m_period);
			m_power = true;
			m_status = m_ticketnotdispensed;
		}
	}
	else
	{
		if (m_power)
		{
			if (m_hopper_type == false || m_status == m_ticketnotdispensed)
			{
				LOG(("%s: Ticket Power Off\n", machine().describe_context()));
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
	m_motoron = (m_motor_sense == TICKET_MOTOR_ACTIVE_HIGH);
	m_ticketdispensed = (m_status_sense == TICKET_STATUS_ACTIVE_HIGH);
	m_ticketnotdispensed = !m_ticketdispensed;

	m_timer = timer_alloc();

	m_output.resolve();

	save_item(NAME(m_status));
	save_item(NAME(m_power));
}


//-------------------------------------------------
//  device_reset - handle device startup
//-------------------------------------------------

void ticket_dispenser_device::device_reset()
{
	m_status = m_ticketnotdispensed;
	m_power = false;
}


//-------------------------------------------------
//  device_timer - handle timer callbacks
//-------------------------------------------------

void ticket_dispenser_device::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	// if we still have power, keep toggling ticket states
	if (m_power)
	{
		m_status = !m_status;
		LOG(("Ticket Status Changed to %02X\n", m_status));
		m_timer->adjust(m_period);
	}
	else if (m_hopper_type)
	{
		m_status = !m_status;
		LOG(("%s: Ticket Power Off\n", machine().describe_context()));
		m_timer->adjust(attotime::never);
		m_output = 0;
	}

	// update output status
	m_output = m_status == m_ticketdispensed;

	// if we just dispensed, increment global count
	if (m_status == m_ticketdispensed)
	{
		machine().bookkeeping().increment_dispensed_tickets(1);
		LOG(("Ticket Dispensed\n"));
	}
}
