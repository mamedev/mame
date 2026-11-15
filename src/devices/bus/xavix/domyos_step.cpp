// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn
/***************************************************************************

    Wired six-zone step mats for domfitex, domfitch, domdance and domstepc.
    The connector accepts one direct mat or the optional hub with two mats.
    Two-mat gameplay has only been verified in domstepc.
    Each mat owns the same independent contacts, with instance-specific host
    bindings. The console's panel remains separate.

    A direct mat supplies decoded accessory bytes to the console's 40-bit
    report interface, without modelling the cable signalling. The hub instead
    exchanges discovery and active-low contact packets through a bidirectional
    command/report handshake. Its first two numbered sockets are populated;
    the remaining sockets are reported empty.

    TODO:
    - Confirm the connector pin numbering and physical hub response latency.

***************************************************************************/

#include "emu.h"
#include "domyos_step.h"

#include <bit>

DEFINE_DEVICE_TYPE(DOMYOS_STEP_PORT, domyos_step_port_device, "domyos_step_port", "Domyos Step Mat Port")
DEFINE_DEVICE_TYPE(DOMYOS_STEP_MAT, domyos_step_mat_device, "domyos_step_mat", "Domyos Step Mat")
DEFINE_DEVICE_TYPE(DOMYOS_STEP_HUB, domyos_step_hub_device, "domyos_step_hub", "Domyos Step Mat Hub")

domyos_step_port_device::domyos_step_port_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, DOMYOS_STEP_PORT, tag, owner, clock)
	, device_single_card_slot_interface<device_domyos_step_interface>(mconfig, *this)
	, m_event_cb(*this)
	, m_peripheral(nullptr)
{
	set_options([] (device_slot_interface &port)
	{
		port.option_add("mat", DOMYOS_STEP_MAT);
		port.option_add("hub", DOMYOS_STEP_HUB);
	}, "mat", false);
}

device_domyos_step_interface::device_domyos_step_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "domyosstep")
	, m_port(dynamic_cast<domyos_step_port_device *>(device.owner()))
{
}

domyos_step_mat_device::domyos_step_mat_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, DOMYOS_STEP_MAT, tag, owner, clock)
	, device_domyos_step_interface(mconfig, *this)
	, m_contacts(*this, "STEP_SENSOR")
	, m_key_layout(key_layout::SINGLE)
{
}

domyos_step_hub_device::domyos_step_hub_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, DOMYOS_STEP_HUB, tag, owner, clock)
	, device_domyos_step_interface(mconfig, *this)
	, m_mats(*this, "mat%u", 1U)
	, m_event_timer(nullptr)
{
}

static INPUT_PORTS_START( domyos_step_mat )
	auto const &mat = downcast<domyos_step_mat_device const &>(owner);
	bool const single = mat.get_key_layout() == domyos_step_mat_device::key_layout::SINGLE;
	bool const second = mat.get_key_layout() == domyos_step_mat_device::key_layout::PLAYER2;

	PORT_START("STEP_SENSOR")
	// Six independent floor contacts, numbered across the upper row and then the lower row.
	// Upper-zone left/right is inferred from the mat artwork and ROM direction mapping.
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Upper Left / Up") PORT_CODE(single ? KEYCODE_W : second ? KEYCODE_U : KEYCODE_E)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Upper Right / Down") PORT_CODE(single ? KEYCODE_E : second ? KEYCODE_I : KEYCODE_R)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Far Left / Back / Cancel") PORT_CODE(second ? KEYCODE_H : KEYCODE_A)
	if (second)
	{
		PORT_CODE(MOUSECODE_BUTTON3_INDEXED(1)) PORT_CODE(GUNCODE_BUTTON2_INDEXED(1))
	}
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Lower Left / Select / Previous") PORT_CODE(second ? KEYCODE_J : KEYCODE_S)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Lower Right / Select / Next") PORT_CODE(second ? KEYCODE_K : KEYCODE_D)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("Far Right / Enter") PORT_CODE(second ? KEYCODE_L : KEYCODE_F)
	if (second)
	{
		PORT_CODE(MOUSECODE_BUTTON1_INDEXED(1)) PORT_CODE(GUNCODE_BUTTON1_INDEXED(1))
	}
	// The factory test checks these report states separately; their physical source is unidentified.
	// PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Factory Test Input 1") PORT_CODE(INPUT_CODE_INVALID)
	// PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Factory Test Input 2") PORT_CODE(INPUT_CODE_INVALID)
	PORT_BIT( 0x50, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

ioport_constructor domyos_step_mat_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(domyos_step_mat);
}

void domyos_step_hub_device::device_add_mconfig(machine_config &config)
{
	DOMYOS_STEP_MAT(config, m_mats[0]).set_key_layout(domyos_step_mat_device::key_layout::PLAYER1);
	DOMYOS_STEP_MAT(config, m_mats[1]).set_key_layout(domyos_step_mat_device::key_layout::PLAYER2);
}

void domyos_step_port_device::device_start()
{
	m_event_cb.resolve_safe();

	m_peripheral = get_card_device();
}

void domyos_step_mat_device::device_start()
{
}

void domyos_step_hub_device::device_start()
{
	m_event_timer = timer_alloc(FUNC(domyos_step_hub_device::event_timer), this);

	save_item(NAME(m_report));
	save_item(NAME(m_command));
	save_item(NAME(m_bit));
	save_item(NAME(m_command_bits));
	save_item(NAME(m_report_bytes));
	save_item(NAME(m_request));
	save_item(NAME(m_request_output));
	save_item(NAME(m_clock));
	save_item(NAME(m_host_transmit));
	save_item(NAME(m_active));
	save_item(NAME(m_transfer));
}

void domyos_step_hub_device::device_reset()
{
	m_report.fill(0);
	m_command.fill(0);
	m_bit = 0;
	m_command_bits = 0;
	m_report_bytes = 0;
	m_request = false;
	m_request_output = false;
	m_clock = false;
	m_host_transmit = true;
	m_active = false;
	m_transfer = false;
	m_event_timer->adjust(attotime::never);
}

u16 device_domyos_step_interface::report_r()
{
	// A hub supplies its own packets instead of the direct-mat report.
	return 0;
}

u8 device_domyos_step_interface::pio_r()
{
	return 0;
}

void device_domyos_step_interface::pio_w(u8, u8)
{
}

void device_domyos_step_interface::trigger_event()
{
	if (m_port)
		m_port->trigger_event();
}

u16 domyos_step_port_device::report_r()
{
	return m_peripheral ? m_peripheral->report_r() : 0;
}

u8 domyos_step_port_device::pio_r()
{
	return m_peripheral ? m_peripheral->pio_r() : 0;
}

void domyos_step_port_device::pio_w(offs_t, u8 data, u8 direction)
{
	if (m_peripheral)
		m_peripheral->pio_w(data, direction);
}

void domyos_step_port_device::trigger_event()
{
	m_event_cb();
}

u8 domyos_step_mat_device::contacts_r()
{
	return m_contacts->read();
}

u16 domyos_step_mat_device::report_r()
{
	u8 const contacts = contacts_r();
	return 0x20 | (contacts & 0x0f) | ((contacts & 0xf0) << 8);
}

TIMER_CALLBACK_MEMBER(domyos_step_hub_device::event_timer)
{
	m_transfer = true;
	trigger_event();
}

void domyos_step_hub_device::latch_report()
{
	m_report.fill(0);

	if (m_command[0] == 0xc1)
	{
		m_report[1] = 0xd0;
		m_report[2] = 0xd0;
		m_report_bytes = 5;
	}
	else if (m_command[0] == 0x82)
	{
		m_report[1] = ~m_mats[0]->contacts_r();
		m_report[2] = ~m_mats[1]->contacts_r();
		m_report_bytes = 3;
	}
	else
	{
		m_report_bytes = 1;
	}

	// Header bits 6-5 contain the population count of bit 7 and the payload modulo four.
	unsigned ones = 1;
	for (unsigned index = 1; index < m_report_bytes; index++)
		ones += std::popcount(m_report[index]);
	m_report[0] = 0x80 | ((ones & 3) << 5);
	m_bit = 0;
}

void domyos_step_hub_device::command_bit_w(int state)
{
	if (m_command_bits < (8 * m_command.size()))
	{
		u8 &byte = m_command[m_command_bits / 8];
		byte = (byte << 1) | state;
		m_command_bits++;
	}
}

u8 domyos_step_hub_device::pio_r()
{
	return (!m_host_transmit && BIT(m_report[m_bit / 8], 7 - (m_bit % 8))) ? 0x04 : 0;
}

void domyos_step_hub_device::pio_w(u8 data, u8 direction)
{
	u8 const output = data & direction;
	bool const request = BIT(output, 1);
	bool const request_output = BIT(direction, 1);
	bool const clock = BIT(output, 4);

	if (request_output && m_request_output && m_request && !request)
	{
		m_active = true;
		m_transfer = false;
		if (m_host_transmit)
		{
			m_command.fill(0);
			m_command_bits = 0;
		}
		else
		{
			latch_report();
		}

		// A short response delay lets the host arm its timeout; physical hub latency is unknown.
		m_event_timer->adjust(attotime::from_usec(10));
	}
	else if (request_output && m_request_output && !m_request && request && m_active)
	{
		m_host_transmit = !m_host_transmit;
		m_active = false;
		m_transfer = false;
	}

	if (request_output && !request && m_transfer)
	{
		if (m_host_transmit && !clock)
			command_bit_w(BIT(output, 3));
		else if (!m_host_transmit && !m_clock && clock && m_bit < (8 * m_report_bytes - 1))
			m_bit++;
	}

	m_request = request;
	m_request_output = request_output;
	m_clock = clock;
}
