// license:BSD-3-Clause
// copyright-holders:smf,Carl
#include "emu.h"
#include "null_modem.h"

null_modem_device::null_modem_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, NULL_MODEM, tag, owner, clock),
	device_serial_interface(mconfig, *this),
	device_rs232_port_interface(mconfig, *this),
	m_stream(*this, "stream"),
	m_rs232_txbaud(*this, "RS232_TXBAUD"),
	m_rs232_rxbaud(*this, "RS232_RXBAUD"),
	m_rs232_databits(*this, "RS232_DATABITS"),
	m_rs232_parity(*this, "RS232_PARITY"),
	m_rs232_stopbits(*this, "RS232_STOPBITS"),
	m_flow(*this, "FLOW_CONTROL"),
	m_input_count(0),
	m_input_index(0),
	m_timer_poll(nullptr),
	m_rts(0),
	m_dtr(0),
	m_xoff(0),
	m_cr(false)
{
}

void null_modem_device::device_add_mconfig(machine_config &config)
{
	BITBANGER(config, m_stream, 0);
}

static INPUT_PORTS_START(null_modem)
	PORT_RS232_BAUD("RS232_TXBAUD", RS232_BAUD_9600, "TX Baud", null_modem_device, update_serial)
	PORT_RS232_BAUD("RS232_RXBAUD", RS232_BAUD_9600, "RX Baud", null_modem_device, update_serial)
	PORT_RS232_DATABITS("RS232_DATABITS", RS232_DATABITS_8, "Data Bits", null_modem_device, update_serial)
	PORT_RS232_PARITY("RS232_PARITY", RS232_PARITY_NONE, "Parity", null_modem_device, update_serial)
	PORT_RS232_STOPBITS("RS232_STOPBITS", RS232_STOPBITS_1, "Stop Bits", null_modem_device, update_serial)

	PORT_START("FLOW_CONTROL")
	PORT_CONFNAME(0x07, 0x00, "Flow Control")
	PORT_CONFSETTING(   0x00, DEF_STR(Off))
	PORT_CONFSETTING(   0x01, "RTS")
	PORT_CONFSETTING(   0x02, "DTR")
	PORT_CONFSETTING(   0x04, "XON/XOFF")
	PORT_CONFNAME(0x18, 0x00, "CR/LF Input Conversion")
	PORT_CONFSETTING(   0x00, DEF_STR(Off))
	PORT_CONFSETTING(   0x08, "LF to CR")
	PORT_CONFSETTING(   0x10, "CR/LF to CR")
	PORT_CONFSETTING(   0x18, "LF to CR/LF")
	PORT_CONFNAME(0xe0, 0x00, "Delay After CR")
	PORT_CONFSETTING(   0x00, DEF_STR(None))
	PORT_CONFSETTING(   0x20, "10 ms")
	PORT_CONFSETTING(   0x40, "20 ms")
	PORT_CONFSETTING(   0x60, "40 ms")
	PORT_CONFSETTING(   0x80, "80 ms")
	PORT_CONFSETTING(   0xa0, "160 ms")
	PORT_CONFSETTING(   0xc0, "320 ms")
	PORT_CONFSETTING(   0xe0, "640 ms")
INPUT_PORTS_END

ioport_constructor null_modem_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(null_modem);
}

void null_modem_device::device_start()
{
	m_timer_poll = timer_alloc(FUNC(null_modem_device::update_queue), this);
}

void null_modem_device::update_serial(int state)
{
	int startbits = 1;
	int databits = convert_databits(m_rs232_databits->read());
	parity_t parity = convert_parity(m_rs232_parity->read());
	stop_bits_t stopbits = convert_stopbits(m_rs232_stopbits->read());

	set_data_frame(startbits, databits, parity, stopbits);

	int txbaud = convert_baud(m_rs232_txbaud->read());
	set_tra_rate(txbaud);

	int rxbaud = convert_baud(m_rs232_rxbaud->read());
	set_rcv_rate(rxbaud);

	output_rxd(1);

	// TODO: make this configurable
	output_dcd(0);
	output_dsr(0);
	output_cts(0);

	m_xoff = 0;
	m_cr = false;
}

void null_modem_device::device_reset()
{
	update_serial(0);
	update_queue(0);
}

void null_modem_device::update_input_buffer()
{
	if (m_input_index == m_input_count)
	{
		m_input_index = 0;
		m_input_count = m_stream->input(m_input_buffer, sizeof(m_input_buffer));
	}
}

TIMER_CALLBACK_MEMBER(null_modem_device::update_queue)
{
	if (is_transmit_register_empty())
	{
		auto const fc = m_flow->read();
		if ((fc & (m_rts | m_dtr << 1 | m_xoff << 2)) == 0)
		{
			if (m_cr && (fc & 0x18) == 0x18)
			{
				transmit_register_setup(0x0a);
				m_timer_poll->adjust(attotime::never);
				m_cr = false;
				return;
			}

			update_input_buffer();

			if (m_input_count != 0)
			{
				if (m_cr && (fc & 0x18) == 0x10)
				{
					while (m_input_index != m_input_count && (m_input_buffer[m_input_index] == 0x0a || m_input_buffer[m_input_index] == 0x0d))
					{
						// suppress redundant CRs
						m_cr = m_input_buffer[m_input_index++] == 0x0d;
						update_input_buffer();
					}
				}

				if (m_input_index != m_input_count)
				{
					uint8_t data = m_input_buffer[m_input_index++];

					if ((fc & 0x08) && data == 0x0a)
					{
						m_cr = data == 0x0a;
						if (m_cr)
							data = 0x0d;
					}
					else
						m_cr = data == 0x0d;

					transmit_register_setup(data);
					m_timer_poll->adjust(attotime::never);
					return;
				}
			}
		}

		int const txbaud = convert_baud(m_rs232_txbaud->read());
		m_timer_poll->adjust(attotime::from_hz(txbaud));
	}
}

void null_modem_device::tra_callback()
{
	output_rxd(transmit_register_get_data_bit());
}

void null_modem_device::tra_complete()
{
	auto const fc = m_flow->read();
	if (m_cr && (fc & 0xe0) != 0)
		m_timer_poll->adjust(attotime::from_msec(5 << (fc >> 5)));
	else
		update_queue(0);
}

void null_modem_device::rcv_complete()
{
	receive_register_extract();

	uint8_t const data = get_received_char();
	if (m_flow->read() != 4)
		m_stream->output(data);
	else
	{
		switch (data)
		{
		case 0x13: // XOFF
			m_xoff = 1;
			return;

		case 0x11: // XON
			m_xoff = 0;
			return;

		default:
			break;
		}
		m_stream->output(data);
	}
}

DEFINE_DEVICE_TYPE(NULL_MODEM, null_modem_device, "null_modem", "RS-232 Null Modem")
