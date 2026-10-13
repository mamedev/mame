// license:BSD-3-Clause
// copyright-holders:Vincent Halver

#include "emu.h"
#include "cdipcb.h"

#define LOG_CHARS   (1U << 1)

#define VERBOSE (0)
#include "logmacro.h"

constexpr uint8_t ACK = 0x06;


DEFINE_DEVICE_TYPE(CDI_SERVICE_PCB, cdi_service_pcb_device, "cdipcb", "CD-i Service PCB")

cdi_service_pcb_device::cdi_service_pcb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, CDI_SERVICE_PCB, tag, owner, clock)
	, device_serial_interface(mconfig, *this)
	, device_rs232_port_interface(mconfig, *this)
	, m_digits(*this, "digit%u", 0U)
{
}

static INPUT_PORTS_START( cdipcb )
	PORT_START("BUTTONS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Yes") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(cdi_service_pcb_device::button_changed), 'Y')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Test") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(cdi_service_pcb_device::button_changed), 'B')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("No") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(cdi_service_pcb_device::button_changed), 'N')
	PORT_BIT( 0xf8, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

ioport_constructor cdi_service_pcb_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( cdipcb );
}

INPUT_CHANGED_MEMBER(cdi_service_pcb_device::button_changed)
{
	if (newval)
		send_char(uint8_t(param));
}

void cdi_service_pcb_device::device_start()
{
	std::fill(std::begin(m_display), std::end(m_display), ' ');

	save_item(NAME(m_answered));
	save_item(NAME(m_cursor));
	save_item(NAME(m_display));
}

void cdi_service_pcb_device::device_reset()
{
	set_data_frame(1, 8, PARITY_NONE, STOP_BITS_1);
	set_rcv_rate(9600);
	set_tra_rate(9600);

	receive_register_reset();
	transmit_register_reset();
	output_rxd(1);

	m_answered = false;

	std::fill(std::begin(m_display), std::end(m_display), ' ');
	m_cursor = 0;
	refresh_display();

	transmit_register_setup(ACK);
}

void cdi_service_pcb_device::tra_callback()
{
	output_rxd(transmit_register_get_data_bit());
}

void cdi_service_pcb_device::tra_complete()
{
	// ACK announces until the player responds.
	if (!m_answered)
		transmit_register_setup(ACK);
}

void cdi_service_pcb_device::rcv_complete()
{
	receive_register_extract();

	// Wait for a real output character.
	const uint8_t data = get_received_char();
	if (data >= 0x20 && data < 0x7f)
		m_answered = true;

	put_char(data);
}

void cdi_service_pcb_device::put_char(uint8_t data)
{
	LOGMASKED(LOG_CHARS, "put_char: %02x\n", data);

	switch (data)
	{
	case 0x08: // Backspace
		if (m_cursor)
			m_cursor--;
		break;

	case 0x0d: // Carriage return
		m_cursor = 0;
		break;

	case 0x0a: // Line feed
		std::fill(std::begin(m_display), std::end(m_display), ' ');
		m_cursor = 0;
		break;

	default:
		if (data < 0x20 || data >= 0x7f)
			break;
		if (m_cursor < 8)
		{
			m_display[m_cursor] = data;
			m_cursor++;
		}
		else
		{
			std::copy(std::begin(m_display) + 1, std::end(m_display), std::begin(m_display));
			m_display[7] = data;
		}
		break;
	}

	refresh_display();
}

void cdi_service_pcb_device::send_char(uint8_t data)
{
	if (is_transmit_register_empty())
		transmit_register_setup(data);
}

void cdi_service_pcb_device::refresh_display()
{
	// Seven segment bits, for 0x20 to 0x5f. Space, [0-9], [A-Z].
	static constexpr uint8_t s_ascii_to_segments[0x40] =
	{
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x39, 0x0f, 0x00, 0x00, 0x00, 0x40, 0x80, 0x00,
		0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07,
		0x7f, 0x6f, 0x00, 0x00, 0x00, 0x48, 0x00, 0x00,
		0x00, 0x77, 0x7c, 0x39, 0x5e, 0x79, 0x71, 0x3d,
		0x76, 0x06, 0x1e, 0x00, 0x38, 0x00, 0x37, 0x3f,
		0x73, 0x67, 0x50, 0x6d, 0x78, 0x3e, 0x1c, 0x00,
		0x00, 0x6e, 0x5b, 0x39, 0x00, 0x0f, 0x00, 0x08
	};

	for (int i = 0; i < 8; i++)
	{
		const uint8_t c = m_display[i];
		m_digits[i] = (c >= 0x20 && c < 0x60) ? s_ascii_to_segments[c - 0x20] : 0x00;
	}
}
