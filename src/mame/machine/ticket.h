/***************************************************************************

    ticket.h

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

#pragma once


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
extern const device_type TICKET_DISPENSER;



//**************************************************************************
//  DEVICE CONFIGURATION MACROS
//**************************************************************************

// add/remove speakers
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
	ticket_dispenser_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~ticket_dispenser_device();

	// inline configuration helpers
	static void static_set_period(device_t &device, attotime period);
	static void static_set_senses(device_t &device, UINT8 motor_sense, UINT8 status_sense);

	// read/write handlers
	DECLARE_READ8_MEMBER( read );
	DECLARE_READ_LINE_MEMBER( line_r );
	DECLARE_WRITE8_MEMBER( write );

protected:
	// device-level overrides
	virtual void device_start() ATTR_COLD;
	virtual void device_reset() ATTR_COLD;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

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
