// license:BSD-3-Clause
// copyright-holders:James Wallace
#include "emu.h"
#include "bacta_datalogger.h"

#define LOG_DATA    (1U << 1)

//#define VERBOSE (LOG_DATA)
//#define LOG_OUTPUT_STREAM std::cout

#include "logmacro.h"

#define LOGDATA(...)    LOGMASKED(LOG_DATA,    __VA_ARGS__)

/***********************************************************************************************************
  BACTA Datalogger emulation
  The British Amusement and Catering Trade Association created a standard for the tracking of statistics
  and other features on British AWPs across hardware manufacturers.
  The specification is very simple, a 1200 Baud null modem connection via RS232, with the logger sending an
  ACK command (0x06) on receipt of a valid character, and NAK (0x15) on error.
  In this emulation, the simplest possible device is simulated here, derived from the RS232 null_modem.
  We only handle the feedback for acknowledgment, and limited logging.
************************************************************************************************************/

// device type definition
DEFINE_DEVICE_TYPE(BACTA_DATALOGGER, bacta_datalogger_device, "bacta_datalogger", "BACTA Datalogger")

//-------------------------------------------------
//  bacta_datalogger_device - constructor
//-------------------------------------------------

bacta_datalogger_device::bacta_datalogger_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	bacta_datalogger_device(mconfig, BACTA_DATALOGGER, tag, owner, clock)
{
}

bacta_datalogger_device::bacta_datalogger_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, BACTA_DATALOGGER, tag, owner, clock),
	device_serial_interface(mconfig, *this),
	m_rxd_handler(*this),
	m_last_input(-1),
	m_output_char(0)
{
}

void bacta_datalogger_device::device_start()
{
	m_rxd_handler.resolve_safe();
}

void bacta_datalogger_device::device_reset()
{
	int startbits = 1;
	int databits = 8;
	parity_t parity = device_serial_interface::PARITY_ODD;
	stop_bits_t stopbits =  device_serial_interface::STOP_BITS_1;

	set_data_frame(startbits, databits, parity, stopbits);

	set_tra_rate(1200);

	set_rcv_rate(1200);

	output_rxd(1);
}

void bacta_datalogger_device::tx_queue()
{
	if (is_transmit_register_empty())
	{
		if (m_output_char != 255)
		{
			set_tra_rate(1200);
			transmit_register_setup(m_output_char);
			m_output_char = 255;
		}
	}
}

void bacta_datalogger_device::tra_callback()
{
	output_rxd(transmit_register_get_data_bit());
}

void bacta_datalogger_device::tra_complete()
{
	// Shut down transmitter until there's a character
	set_tra_rate(attotime::never);
	tx_queue();
}

void bacta_datalogger_device::rcv_complete()
{
	u8 data;

	receive_register_extract();

	data = get_received_char();
	if (data != 0x00)
	{
		if (data > 0x80)
		{
			data &= ~0x80;
			if ( data == m_last_input)
			{
				LOGDATA("Retransmission of %x\n",data);
			}
			m_last_input = data;
		}
		switch (data)
		{
			case 0x01:
				LOGDATA("(%c) Prize 1 vend or 0x01\n",data);
				break;
			case 0x02:
				LOGDATA("(%c) Prize 2 vend or 0x02\n",data);
				break;
			case 0x03:
				LOGDATA("(%c) Prize 3 vend or 0x03\n",data);
				break;
			case 0x04:
				LOGDATA("(%c) Prize 4 vend or 0x04\n",data);
				break;
			case 0x05:
				LOGDATA("(%c) Remote Credit or 0x05\n",data);
				break;
			case 0x07:
				LOGDATA("(%c) Idle or 0x07\n",data);
				break;
			case 0x08:
				LOGDATA("(%c) Change or 0x08\n",data);
				break;
			case 0x09:
			case 0x0a:
			case 0x0b:
			case 0x0c:
			case 0x0d:
			case 0x0e:
			case 0x0f:
				LOGDATA("(%c) User defined message or 0x0%x\n",data,data);
				break;
			case 0x2b:
				LOGDATA("(%c) Cashbox door open 0x2b\n",data);
				break;
			case 0x2c:
				LOGDATA("(%c) Cashbox door closed 0x2c\n",data);
				break;
			case 0x2d:
				LOGDATA("(%c) Service door open 0x2d\n",data);
				break;
			case 0x2e:
				LOGDATA("(%c) Service door closed 0x2e\n",data);
				break;
			case 0x60:
				LOGDATA("(%c) Primary message (0x%x), next byte is message length\n",data,data);
				break;
			case 0x61:
				LOGDATA("(%c) Coin Tube / Hopper Levels 0x61 \n",data);
				break;
			case 0x62:
				LOGDATA("(%c) Secondary message (0x%x), next byte is message length\n",data,data);
				break;
			case 0x63:
				LOGDATA("(%c) Critical Fault 0x63\n",data);
				break;
			case 0x64:
				LOGDATA("(%c) Non Critical Fault 0x64\n",data);
				break;
			case 0x65:
				LOGDATA("(%c) Manufacturer Message Header 0x65, next byte is message length\n",data);
				break;
			case 0x66:
				LOGDATA("(%c) Potential Parameter Data request 0x66, not currently supported\n",data);
				break;
			case 0x67:
				LOGDATA("(%c) Potential Parameter Report request 0x67, not currently supported\n",data);
				break;
			case 0x68:
				LOGDATA("(%c) Multi-Stake Multi-Game message 0x68\n",data);
				break;
			case 0x69:
				LOGDATA("(%c) Cashless source 0x69\n",data);
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
				LOGDATA("(%c) User defined message (0x%x), next byte is message length\n",data,data);
				break;

			default:
				LOGDATA("(%c) Received: %02x\n",data, data);
				break;
		}
		m_output_char = 0x06;//ACK
		tx_queue();
	}
	else
	{
		if (data != 0x00)
		{
			m_output_char = 0x15;//NAK
			tx_queue();
		}
	}
}
