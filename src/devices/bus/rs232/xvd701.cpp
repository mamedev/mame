// license:BSD-3-Clause
// copyright-holders:smf
#include "xvd701.h"

jvc_xvd701_device::jvc_xvd701_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, JVC_XVD701, "JVC XV-D701", tag, owner, clock, "xvd701", __FILE__),
	device_serial_interface(mconfig, *this),
	device_rs232_port_interface(mconfig, *this),
	m_response_index(0),
	m_timer_response(nullptr)
{
}

static MACHINE_CONFIG_FRAGMENT(xvd701)
MACHINE_CONFIG_END

machine_config_constructor jvc_xvd701_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME(xvd701);
}

static INPUT_PORTS_START(xvd701)
INPUT_PORTS_END

ioport_constructor jvc_xvd701_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(xvd701);
}

void jvc_xvd701_device::device_start()
{
	int startbits = 1;
	int databits = 8;
	parity_t parity = PARITY_ODD;
	stop_bits_t stopbits = STOP_BITS_1;

	set_data_frame(startbits, databits, parity, stopbits);

	int txbaud = 9600;
	set_tra_rate(txbaud);

	int rxbaud = 9600;
	set_rcv_rate(rxbaud);

	output_rxd(1);

	// TODO: make this configurable
	output_dcd(0);
	output_dsr(0);
	output_ri(0);
	output_cts(0);

	m_timer_response = timer_alloc(TIMER_RESPONSE);
}

void jvc_xvd701_device::device_reset()
{
	memset(m_command, 0, sizeof(m_command));

	m_response_index = sizeof(m_response);
}

void jvc_xvd701_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_RESPONSE:
		send_response();
		break;

	default:
		device_serial_interface::device_timer(timer, id, param, ptr);
	}
}

void jvc_xvd701_device::tra_callback()
{
	output_rxd(transmit_register_get_data_bit());
}

void jvc_xvd701_device::tra_complete()
{
	m_timer_response->adjust(attotime::from_msec(100));
}

unsigned char jvc_xvd701_device::sum(unsigned char *buffer, int length)
{
	int sum = 0;

	for (int i = 0; i < length; i++)
		sum += buffer[i];

	return sum & 0x7f;
}

void jvc_xvd701_device::send_response()
{
	if (m_response_index < sizeof(m_response) && is_transmit_register_empty())
	{
//      printf("sending %02x\n", m_response[m_response_index]);
		transmit_register_setup(m_response[m_response_index++]);
	}
}

void jvc_xvd701_device::rcv_complete()
{
	receive_register_extract();

	for (int i = 0; i < sizeof(m_command) - 1; i++)
		m_command[i] = m_command[i + 1];

	m_command[sizeof(m_command) - 1] = get_received_char();

	if (m_command[0] == 0xff &&
		m_command[1] == 0xff &&
		m_command[2] == 0x21 &&
		sum(m_command, sizeof(m_command)) == 0)
	{
		// printf("xvd701");

		//for (int i = 0; i < sizeof(m_command); i++)
		//  printf(" %02x", m_command[i]);

		//printf("\n");

		// FF FF 21 3E 40 70 00 00 00 00 73 DEVICE ON
		// FF FF 21 3E 40 60 00 00 00 00 03 DEVICE OFF
		// FF FF 21 0C 44 60 00 00 00 00 31 STOP
		// FF FF 21 0C 43 75 00 00 00 00 1D PLAY
		// FF FF 21 0C 43 6D 00 00 00 00 25 PAUSE
		// FF FF 21 0C 50 20 00 00 00 00 63 SEEK TO SPECIFIC CHAPTER
		// FF FF 21 0C 50 73 00 00 00 00 12 FF (SEEK TO NEXT CHAPTER)
		// FF FF 21 0C 50 61 00 00 00 00 24 PREV (SEEK TO PREVIOUS CHAPTER)

		m_response[0] = 0xff;
		m_response[1] = 0xfe;
		m_response[2] = 0x7f;
		m_response[3] = 0x7e;
		m_response[4] = 0x7d;
		m_response[5] = 0x7c;
		m_response[6] = 0x7b;
		m_response[7] = 0x7a;
		m_response[8] = 0x79;
		m_response[9] = 0x78;
		m_response[10] = 0x77;
		m_response_index = 0;

		m_timer_response->adjust(attotime::from_msec(100));
	}
}

const device_type JVC_XVD701 = &device_creator<jvc_xvd701_device>;
