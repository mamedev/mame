// license:BSD-3-Clause
// copyright-holders:James Wallace
#include "emu.h"
#include "bacta_datalogger.h"

#define LOG_DATA 1
/***********************************************************************************************************
  BACTA Datalogger emulation
  The British Amusement and Catering Trade Association created a standard for the tracking of statistics
  and other features on British AWPs across hardware manufacturers.
  The specification is very simple, a 1200 baud null modem via RS232, with the logger sending an ACK
  command (0x06) on receipt of a valid character.
  As this is RS232, the logger itself can be of any design provided the acknowledgment is made where demanded
  (by, for example, Protocol game ROMS).
  In this emulation, the simplest functioning device is simulated here, derived from the rs232 null_modem.
  We only handle the feedback for acknowledgment, not any other feature.
************************************************************************************************************/

// device type definition
DEFINE_DEVICE_TYPE(BACTA_DATALOGGER, bacta_datalogger_device, "bacta_datalogger", "BACTA Datalogger")

//-------------------------------------------------
//  bacta_datalogger_device - constructor
//-------------------------------------------------

bacta_datalogger_device::bacta_datalogger_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: bacta_datalogger_device(mconfig, BACTA_DATALOGGER, tag, owner, clock)
{
}

bacta_datalogger_device::bacta_datalogger_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BACTA_DATALOGGER, tag, owner, clock),
	device_serial_interface(mconfig, *this),
	m_rxd_handler(*this),
	m_output_char(0),
	m_timer_poll(nullptr)
{
}

void bacta_datalogger_device::device_start()
{
	m_rxd_handler.resolve_safe();

	m_timer_poll = timer_alloc(TIMER_POLL);
}


WRITE_LINE_MEMBER(bacta_datalogger_device::update_serial)
{
	int startbits = 1;
	int databits = 8;
	parity_t parity = device_serial_interface::PARITY_ODD;
	stop_bits_t stopbits = 	device_serial_interface::STOP_BITS_1; // 1 stop bit

	set_data_frame(startbits, databits, parity, stopbits);

	set_tra_rate(1200);

	set_rcv_rate(1200);

	output_rxd(1);
}

void bacta_datalogger_device::device_reset()
{
	update_serial(0);
	queue();
}

void bacta_datalogger_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_POLL:
		queue();
		break;

	default:
		break;
	}
}

void bacta_datalogger_device::queue()
{
	if (m_output_char != 0)
	{
		if (is_transmit_register_empty())
		{
			transmit_register_setup(m_output_char);
			m_timer_poll->adjust(attotime::never);
			return;

			m_timer_poll->adjust(attotime::from_hz(1200));
		}
	}
}

void bacta_datalogger_device::tra_callback()
{
	output_rxd(transmit_register_get_data_bit());
}

void bacta_datalogger_device::tra_complete()
{
	queue();
}

void bacta_datalogger_device::rcv_complete()
{
	u8 data;

	receive_register_extract();

	data = get_received_char();
	if (!is_receive_parity_error())
	{
		if (data == 0x00)//clean up logging
		{
			if (LOG_DATA) 
			{
				logerror("Received: %02x () \n",data);
			}
		
		}
		else 
		{
			if (LOG_DATA) 
			{
				logerror("Received: %02x (%c) \n",data, data);
			}
		}
		m_output_char = 0x06;//ACK
	}
	else
	{
		m_output_char = 0x15;//NAK
	}
	queue();
}