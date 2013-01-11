/***************************************************************************

    ticket.c

    Generic ticket dispensing device.

****************************************************************************

    Copyright Aaron Giles
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

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
const device_type TICKET_DISPENSER = &device_creator<ticket_dispenser_device>;



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  speaker_device - constructor
//-------------------------------------------------

ticket_dispenser_device::ticket_dispenser_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, TICKET_DISPENSER, "Ticket Dispenser", tag, owner, clock),
		m_motor_sense(TICKET_MOTOR_ACTIVE_LOW),
		m_status_sense(TICKET_STATUS_ACTIVE_LOW),
		m_period(attotime::from_msec(100)),
		m_active_bit(0x80),
		m_motoron(0),
		m_ticketdispensed(0),
		m_ticketnotdispensed(0),
		m_status(0),
		m_power(0),
		m_timer(NULL)
{
}


//-------------------------------------------------
//  ~ticket_dispenser_device - destructor
//-------------------------------------------------

ticket_dispenser_device::~ticket_dispenser_device()
{
}


//**************************************************************************
//  CONFIGURATION HELPERS
//**************************************************************************

//-------------------------------------------------
//  static_set_period - configure the clock period
//  for dispensing
//-------------------------------------------------

void ticket_dispenser_device::static_set_period(device_t &device, attotime period)
{
	downcast<ticket_dispenser_device &>(device).m_period = period;
}


//-------------------------------------------------
//  static_set_senses - configure the senses of
//  the motor and status bits
//-------------------------------------------------

void ticket_dispenser_device::static_set_senses(device_t &device, UINT8 motor_sense, UINT8 status_sense)
{
	ticket_dispenser_device &ticket = downcast<ticket_dispenser_device &>(device);
	ticket.m_motor_sense = motor_sense;
	ticket.m_status_sense = status_sense;
}



//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

//-------------------------------------------------
//  read - read the status line via the active bit
//-------------------------------------------------

READ8_MEMBER( ticket_dispenser_device::read )
{
	LOG(("%s: Ticket Status Read = %02X\n", machine().describe_context(), m_status));
	return m_status;
}


//-------------------------------------------------
//  line_r - read the status line as a proper line
//-------------------------------------------------

READ_LINE_MEMBER( ticket_dispenser_device::line_r )
{
	return m_status ? 1 : 0;
}


//-------------------------------------------------
//  write - write the control line via the active
//  bit
//-------------------------------------------------

WRITE8_MEMBER( ticket_dispenser_device::write )
{
	// On an activate signal, start dispensing!
	if ((data & m_active_bit) == m_motoron)
	{
		if (!m_power)
		{
			LOG(("%s: Ticket Power On\n", machine().describe_context()));
			m_timer->adjust(m_period);
			m_power = 1;
			m_status = m_ticketnotdispensed;
		}
	}
	else
	{
		if (m_power)
		{
			LOG(("%s: Ticket Power Off\n", machine().describe_context()));
			m_timer->adjust(attotime::never);
			set_led_status(machine(), 2,0);
			m_power = 0;
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
	m_active_bit = 0x80;
	m_motoron = (m_motor_sense == TICKET_MOTOR_ACTIVE_HIGH) ? m_active_bit : 0;
	m_ticketdispensed = (m_status_sense == TICKET_STATUS_ACTIVE_HIGH) ? m_active_bit : 0;
	m_ticketnotdispensed = m_ticketdispensed ^ m_active_bit;

	m_timer = timer_alloc();

	save_item(NAME(m_status));
	save_item(NAME(m_power));
}


//-------------------------------------------------
//  device_reset - handle device startup
//-------------------------------------------------

void ticket_dispenser_device::device_reset()
{
	m_status = m_ticketnotdispensed;
	m_power = 0x00;
}


//-------------------------------------------------
//  device_timer - handle timer callbacks
//-------------------------------------------------

void ticket_dispenser_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	// if we still have power, keep toggling ticket states
	if (m_power)
	{
		m_status ^= m_active_bit;
		LOG(("Ticket Status Changed to %02X\n", m_status));
		m_timer->adjust(m_period);
	}

	// update LED status (fixme: should map to an output)
	set_led_status(machine(), 2, (m_status == m_ticketdispensed));

	// if we just dispensed, increment global count
	if (m_status == m_ticketdispensed)
	{
		increment_dispensed_tickets(machine(), 1);
		LOG(("Ticket Dispensed\n"));
	}
}
