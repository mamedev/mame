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

//	output_rxd(1);
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
		if (data != 0x00)
		{
			if (LOG_DATA) 
			{
				switch (data)
				{
					case 0x01:
						logerror("(%c) Prize 1 vend or 0x01\n",data);
						break;
					case 0x02:
						logerror("(%c) Prize 2 vend or 0x02\n",data);
						break;
					case 0x03:
						logerror("(%c) Prize 3 vend or 0x03\n",data);
						break;
					case 0x04:
						logerror("(%c) Prize 4 vend or 0x04\n",data);
						break;
					case 0x05:
						logerror("(%c) Remote Credit or 0x05\n",data);
						break;
					case 0x07:
						logerror("(%c) Idle or 0x07\n",data);
						break;
					case 0x08:
						logerror("(%c) Change or 0x08\n",data);
						break;
					case 0x09:
					case 0x0a:
					case 0x0b:
					case 0x0c:
					case 0x0d:
					case 0x0e:
					case 0x0f:
						logerror("(%c) User defined message or 0x%x\n",data,data);
						break;
					case 0x2b:
						logerror("(%c) Cashbox door open 0x2b\n",data);
					case 0x2c:
						logerror("(%c) Cashbox door closed 0x2c\n",data);
					case 0x2d:
						logerror("(%c) Service door open 0x2d\n",data);
					case 0x2e:
						logerror("(%c) Service door closed 0x2e\n",data);
					case 0x61:
						logerror("(%c) Coin Tube / Hopper Levels 0x61 \n",data);
						break;
					case 0x62:
						logerror("(%c) Secondary message (0x%x), next byte is message length\n",data,data);
						break;
					case 0x63:
						logerror("(%c) Critical Fault 0x63\n",data);
						break;
					case 0x64:
						logerror("(%c) Non Critical Fault 0x64\n",data);
						break;
					case 0x65:
						logerror("(%c) Manufacturer Message Header 0x65, next byte is message length\n",data);
						break;
					case 0x66:
						logerror("(%c) Potential Parameter Data request 0x66, not currently supported\n",data);
						break;
					case 0x67:
						logerror("(%c) Potential Parameter Report request 0x67, not currently supported\n",data);
						break;
					case 0x68:
						logerror("(%c) Multi-Stake Multi-Game message 0x68\n",data);
						break;
					case 0x69:
						logerror("(%c) Cashless source 0x69\n",data);
						break;
					case 0x70:
					case 0x71:
					case 0x72:
					case 0x73:
					case 0x74:
					case 0x75:
					case 0x76:
					case 0x77:
					case 0x78:
					case 0x79:
					case 0x7a:
					case 0x7b:
					case 0x7c:
					case 0x7d:
					case 0x7e:
					case 0x7f:
						logerror("(%c) User defined message (0x%x), next byte is message length\n",data,data);
						break;
					
					default:
						logerror("(%c) Received: %02x\n",data, data);
						break;
				}
				m_output_char = 0x06;//ACK
				queue();
			}
		}
	}
	else
	{
		m_output_char = 0x15;//NAK
		queue();
	}
}