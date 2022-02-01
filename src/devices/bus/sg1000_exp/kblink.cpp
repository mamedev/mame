// license:BSD-3-Clause
// copyright-holders:Enik Land
/**********************************************************************

    Sega SK-1100 keyboard link cable emulation

The cable is used only to link two Mark III's through keyboard, what
is supported by the game F-16 Fighting Falcon for its 2 players mode.

Keyboard link cable info (originally from http://homepage3.nifty.com/st-2/,
but taken from http://www.smspower.org/Games/F16FightingFalcon-SMS-Talk):

- Cable is 7-pin DIN.
- Crossover scheme of the cable to connect pins

  From  To
  1     1
  2     6
  3     3
  4     5
  5     4
  6     2
  7     7

Pinout of the printer port (from Charles MacDonald's sc3000h-20040729.txt
document, with the function of pin 6 corrected to /FAULT).
Numbering in counterclockwise/anticlockwise direction:

 1 : Unused (not connected to anything)
 2 : PPI PC5 (DATA output)
 3 : PPI PC7 (/FEED output)
 4 : PPI PB6 (BUSY input)
 5 : PPI PC6 (/RESET output)
 6 : PPI PB5 (/FAULT input)
 7 : GND

**********************************************************************/

#include "emu.h"
#include "kblink.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(SK1100_LINK_CABLE, sk1100_link_cable_device, "sk1100_link_cable", "SK-1100 Link Cable")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  sk1100_link_cable_device - constructor
//-------------------------------------------------

sk1100_link_cable_device::sk1100_link_cable_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, SK1100_LINK_CABLE, tag, owner, clock),
	device_sk1100_printer_port_interface(mconfig, *this),
	m_stream(*this, "stream"),
	m_input_count(0),
	m_input_index(0),
	m_timer_poll(nullptr),
	m_timer_send(nullptr),
	m_timer_read(nullptr),
	m_update_received_data(true),
	m_data(0),
	m_reset(0),
	m_feed(0),
	m_busy(0),
	m_fault(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sk1100_link_cable_device::device_start()
{
	m_timer_poll = timer_alloc(TIMER_POLL);
	m_timer_send = timer_alloc(TIMER_SEND);
	m_timer_read = timer_alloc(TIMER_READ);

	/* register for state saving */
	save_item(NAME(m_data));
	save_item(NAME(m_reset));
	save_item(NAME(m_feed));
	save_item(NAME(m_busy));
	save_item(NAME(m_fault));
	save_item(NAME(m_update_received_data));
	save_item(NAME(m_input_count));
	save_item(NAME(m_input_index));
	save_pointer(NAME(m_input_buffer), sizeof(m_input_buffer));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void sk1100_link_cable_device::device_reset()
{
	queue();
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void sk1100_link_cable_device::device_add_mconfig(machine_config &config)
{
	BITBANGER(config, m_stream, 0);
}


void sk1100_link_cable_device::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	switch (id)
	{
	case TIMER_POLL:
		queue();
		break;

	case TIMER_SEND:
		m_stream->output(u8(param));
		break;

	case TIMER_READ:
		m_update_received_data = true;
		break;

	default:
		break;
	}
}

void sk1100_link_cable_device::queue()
{
	if (m_input_index == m_input_count)
	{
		m_input_index = 0;
		m_input_count = m_stream->input(m_input_buffer, sizeof(m_input_buffer));
		if (!m_input_count)
		{
			m_timer_poll->adjust(attotime::from_hz(XTAL(10'738'635)/3));
		}
	}
}

void sk1100_link_cable_device::set_data_read()
{
	// Check if a new byte from the input buffer was read for this timeslice.
	if (m_update_received_data == true)
	{
		if (m_input_count != 0)
		{
			u8 byte = m_input_buffer[m_input_index++];
			// there is no way to read what was sent from peer as feed bit.
			m_fault = BIT(byte, 0); // sent from peer as data bit
			m_busy = BIT(byte, 1); // sent from peer as reset bit
			queue();
		}
		// Set to read next byte only after the end of this timeslice.
		m_update_received_data = false;
		m_timer_read->adjust(attotime::zero);
	}
}

void sk1100_link_cable_device::set_data_transfer()
{
	u8 byte = (m_feed << 2) | (m_reset << 1) | m_data;
	m_timer_send->adjust(attotime::zero, byte);
}


