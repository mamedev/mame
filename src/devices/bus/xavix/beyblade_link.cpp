// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn
/***************************************************************************

    Beyblade Ultimate battle cable port and a local HLE second console.
    The optional peer exchanges its identity, buttons, dial movement and
    filtered ripcord speed through the two-line handshake. This is a local
    approximation of another complete console, not a link between emulators.

    The diagnostic profile represents unidentified factory equipment and is
    not available in the slot menu.

    TODO:
    - Measure physical link turnaround, dial detent scale/polarity and
      ripcord encoder counts.
    - Implement transport between two emulated consoles.

***************************************************************************/

#include "emu.h"
#include "beyblade_link.h"

#define VERBOSE 0
#include "logmacro.h"

DEFINE_DEVICE_TYPE(XAVIX_BEYBLADE_LINK_PORT, xavix_beyblade_link_port_device, "xavix_beyblade_link_port", "XaviX Beyblade Link Port")
DEFINE_DEVICE_TYPE(XAVIX_BEYBLADE_A55, xavix_beyblade_a55_device, "xavix_beyblade_a55", "Beyblade A-55 Dragoon / Tyson Second Console (HLE)")
DEFINE_DEVICE_TYPE(XAVIX_BEYBLADE_A62, xavix_beyblade_a62_device, "xavix_beyblade_a62", "Beyblade A-62 Dranzer / Kai Second Console (HLE)")

static INPUT_PORTS_START( xavix_beyblade_peer )
	PORT_START("BUTTONS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P2 Enter / Trigger") PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P2 Cancel / Signature Move") PORT_PLAYER(2)
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("DIAL")
	// Match Player 1 host deltas; the physical detent scale and polarity are unmeasured.
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_NAME("P2 Select / Dial Controller") PORT_SENSITIVITY(100) PORT_KEYDELTA(4) PORT_CODE_DEC(KEYCODE_D) PORT_CODE_INC(KEYCODE_G) PORT_PLAYER(2)

	PORT_START("RIPCORD")
	// Match Player 1 host scaling; the physical encoder count is unmeasured.
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_NAME("P2 Ripcord") PORT_SENSITIVITY(100) PORT_KEYDELTA(12) PORT_CODE_DEC(INPUT_CODE_INVALID) PORT_CODE_INC(KEYCODE_Q) PORT_PLAYER(2)
INPUT_PORTS_END

xavix_beyblade_link_port_device::xavix_beyblade_link_port_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, XAVIX_BEYBLADE_LINK_PORT, tag, owner, clock)
	, device_single_card_slot_interface<device_xavix_beyblade_link_interface>(mconfig, *this)
	, m_controls_cb(*this, 0)
	, m_host_reset_cb(*this, 0)
	, m_peer(nullptr)
{
	set_options([] (device_slot_interface &port)
	{
		port.option_add("a55", XAVIX_BEYBLADE_A55);
		port.option_add("a62", XAVIX_BEYBLADE_A62);
		// The factory protocol is known, but its physical test equipment is unidentified.
		// port.option_add("factory", XAVIX_BEYBLADE_A55).machine_config([] (device_t *device)
		// {
		//  downcast<xavix_beyblade_peer_device &>(*device).set_model(xavix_beyblade_peer_device::model::FACTORY);
		// });
	}, nullptr, false);
}

device_xavix_beyblade_link_interface::device_xavix_beyblade_link_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "xavixbeybladelink")
	, m_port(dynamic_cast<xavix_beyblade_link_port_device *>(device.owner()))
{
}

void xavix_beyblade_link_port_device::device_start()
{
	m_peer = get_card_device();
}

u8 xavix_beyblade_link_port_device::io_r(offs_t, u8)
{
	return (m_controls_cb() & 0x3f) | (m_peer ? m_peer->lines_r() : 0);
}

void xavix_beyblade_link_port_device::io_w(offs_t, u8 levels, u8 direction)
{
	if (m_peer)
		m_peer->lines_w(levels, direction);
}

void xavix_beyblade_link_port_device::vblank_w(int state)
{
	if (m_peer)
		m_peer->vblank_w(state);
}

xavix_beyblade_peer_device::xavix_beyblade_peer_device(
		const machine_config &mconfig,
		device_type type,
		const char *tag,
		device_t *owner,
		u32 clock,
		model variant)
	: device_t(mconfig, type, tag, owner, clock)
	, device_xavix_beyblade_link_interface(mconfig, *this)
	, m_model(variant)
	, m_peer_buttons(*this, "BUTTONS")
	, m_peer_dial(*this, "DIAL")
	, m_peer_ripcord(*this, "RIPCORD")
{
}

xavix_beyblade_a55_device::xavix_beyblade_a55_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: xavix_beyblade_peer_device(mconfig, XAVIX_BEYBLADE_A55, tag, owner, clock, model::A55)
{
}

xavix_beyblade_a62_device::xavix_beyblade_a62_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: xavix_beyblade_peer_device(mconfig, XAVIX_BEYBLADE_A62, tag, owner, clock, model::A62)
{
}

ioport_constructor xavix_beyblade_peer_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( xavix_beyblade_peer );
}

void xavix_beyblade_peer_device::device_start()
{
	m_transmit_timer = timer_alloc(FUNC(xavix_beyblade_peer_device::transmit_timer), this);
	m_idle_timer = timer_alloc(FUNC(xavix_beyblade_peer_device::idle_timer), this);

	save_item(NAME(m_response));
	save_item(NAME(m_peer_rpm));
	save_item(NAME(m_phase));
	save_item(NAME(m_host_lines));
	save_item(NAME(m_host_direction));
	save_item(NAME(m_peer_lines));
	save_item(NAME(m_receive_byte));
	save_item(NAME(m_receive_bits));
	save_item(NAME(m_response_bytes));
	save_item(NAME(m_response_byte));
	save_item(NAME(m_response_bit));
	save_item(NAME(m_peer_button_levels));
	save_item(NAME(m_peer_button_edges));
	save_item(NAME(m_last_peer_buttons));
	save_item(NAME(m_peer_dial_position));
	save_item(NAME(m_peer_dial_delta));
	save_item(NAME(m_peer_ripcord_position));
	save_item(NAME(m_peer_ripcord_history));
	save_item(NAME(m_peer_ripcord_history_position));
	save_item(NAME(m_peer_inputs_valid));
	save_item(NAME(m_expect_diagnostic_token));
	save_item(NAME(m_factory_stage));
}

void xavix_beyblade_peer_device::device_reset()
{
	m_peer_rpm = 0;
	m_peer_button_levels = 0;
	m_peer_button_edges = 0;
	m_last_peer_buttons = 0;
	m_peer_dial_position = 0;
	m_peer_dial_delta = 0;
	m_peer_ripcord_position = 0;
	m_peer_ripcord_history.fill(0);
	m_peer_ripcord_history_position = 0;
	m_peer_inputs_valid = false;
	m_expect_diagnostic_token = false;
	m_factory_stage = FACTORY_ENTRY;
	reset_protocol();
}

void xavix_beyblade_peer_device::reset_protocol()
{
	m_transmit_timer->adjust(attotime::never);
	m_idle_timer->adjust(attotime::never);
	m_phase = WAIT_CONNECT;
	m_host_lines = 0;
	m_host_direction = 0;
	m_peer_lines = 0;
	m_receive_byte = 0;
	m_receive_bits = 0;
	m_response.fill(0);
	m_response_bytes = 0;
	m_response_byte = 0;
	m_response_bit = 0;
	m_expect_diagnostic_token = false;
	if (m_model == model::FACTORY && m_factory_stage != FACTORY_READY)
	{
		m_phase = TEST_CONNECT;
		m_peer_lines = 0x80;
	}
}

void xavix_beyblade_peer_device::vblank_w(int state)
{
	if (!state)
		return;
	if (host_reset_r())
	{
		// Re-arm the cable protocol while the local processor is held in physical reset.
		m_factory_stage = FACTORY_ENTRY;
		reset_protocol();
	}

	u8 const buttons = m_peer_buttons->read() & 0x03;
	u8 const dial_position = m_peer_dial->read();
	u8 const ripcord_position = m_peer_ripcord->read();
	m_peer_button_levels = buttons;

	if (!m_peer_inputs_valid)
	{
		m_last_peer_buttons = buttons;
		m_peer_dial_position = dial_position;
		m_peer_ripcord_position = ripcord_position;
		m_peer_inputs_valid = true;
		return;
	}

	m_peer_button_edges = buttons & ~m_last_peer_buttons;
	m_last_peer_buttons = buttons;
	m_peer_dial_delta = dial_position - m_peer_dial_position;
	m_peer_dial_position = dial_position;

	int const ripcord_delta = s8(ripcord_position - m_peer_ripcord_position);
	m_peer_ripcord_position = ripcord_position;
	m_peer_ripcord_history[m_peer_ripcord_history_position] = ripcord_delta < 0 ? -ripcord_delta : ripcord_delta;
	m_peer_ripcord_history_position = (m_peer_ripcord_history_position + 1) % m_peer_ripcord_history.size();

	// Match the launcher's unsigned 35-sample speed filter, including 16-bit wraparound.
	u16 rpm = 0;
	for (unsigned age = 0; age < RPM_FILTER.size(); age++)
	{
		unsigned const history = (m_peer_ripcord_history_position + RPM_FILTER.size() - 1 - age) % RPM_FILTER.size();
		rpm += u16(m_peer_ripcord_history[history]) * RPM_FILTER[age];
	}
	m_peer_rpm = rpm;
}

void xavix_beyblade_peer_device::prepare_response(u8 command)
{
	m_response.fill(0);
	m_response_byte = 0;
	m_response_bit = 0;
	if (m_expect_diagnostic_token)
	{
		m_expect_diagnostic_token = false;
		if (command == 0xa5)
		{
			m_response[0] = 0xd2;
			m_response_bytes = 1;
			return;
		}
	}

	switch (command)
	{
	case 0x01:
		if (m_model == model::FACTORY)
		{
			m_response_bytes = 0;
			break;
		}
		// A fresh linked launcher exposes only its matching lead character.
		m_response[0] = 1U << (u8(m_model) - 1);
		m_response[4] = u8(m_model) - 1;
		m_response_bytes = 5;
		break;

	case 0x02:
		m_response[0] = BIT(m_peer_button_levels, 0);
		m_response[1] = BIT(m_peer_button_edges, 0);
		m_response[2] = BIT(m_peer_button_levels, 1);
		m_response[3] = BIT(m_peer_button_edges, 1);
		m_response[4] = m_peer_dial_delta;
		m_response[5] = m_peer_rpm;
		m_response[6] = m_peer_rpm >> 8;
		m_response_bytes = 7;
		break;

	case 0x0e:
		m_response_bytes = 0;
		break;

	case 0xf0:
		m_response_bytes = 0;
		m_expect_diagnostic_token = true;
		break;

	default:
		LOG("Unknown command %02x\n", command);
		m_response_bytes = 0;
		break;
	}
}

TIMER_CALLBACK_MEMBER(xavix_beyblade_peer_device::transmit_timer)
{
	if (m_phase == TRANSMIT_DELAY)
	{
		bool const bit = BIT(m_response[m_response_byte], 7 - m_response_bit);
		m_peer_lines = bit ? 0x80 : 0x40;
		m_phase = TRANSMIT_ACK;
	}
}

TIMER_CALLBACK_MEMBER(xavix_beyblade_peer_device::idle_timer)
{
	// With no disconnect token, an idle timeout lets a local HLE peer accept a later reconnection.
	reset_protocol();
}

void xavix_beyblade_peer_device::begin_transmit_turnaround()
{
	m_phase = TRANSMIT_TURNAROUND;
}

void xavix_beyblade_peer_device::advance_transmit()
{
	if (++m_response_bit == 8)
	{
		m_response_bit = 0;
		m_response_byte++;
	}

	if (m_response_byte == m_response_bytes)
	{
		m_peer_lines = 0;
		if (m_model == model::FACTORY && m_factory_stage != FACTORY_READY)
		{
			if (m_factory_stage == FACTORY_ENTRY)
			{
				m_factory_stage = FACTORY_LINK_CHECK;
				m_phase = WAIT_CONNECT;
			}
			else
			{
				m_factory_stage = FACTORY_REPLY;
				m_phase = RECEIVE_ASSERT;
			}
		}
		else
		{
			m_phase = RECEIVE_ASSERT;
		}
	}
	else
	{
		begin_transmit_turnaround();
	}
}

void xavix_beyblade_peer_device::lines_w(u8 levels, u8 direction)
{
	u8 const host_direction = direction & 0xc0;
	u8 const host_lines = levels & host_direction;
	if (host_lines == m_host_lines && host_direction == m_host_direction)
		return;
	m_host_lines = host_lines;
	m_host_direction = host_direction;

	switch (m_phase)
	{
	case TEST_CONNECT:
		if (host_lines == 0x40)
		{
			m_peer_lines = 0;
			m_phase = TEST_CONNECT_RELEASE;
		}
		break;

	case TEST_CONNECT_RELEASE:
		if (!host_lines)
		{
			// Request entry first, then exchange the factory link-check challenge.
			m_response[0] = m_factory_stage == FACTORY_ENTRY ? 0xf0 : 0x0f;
			m_response[1] = 0x5a;
			m_response_bytes = m_factory_stage == FACTORY_ENTRY ? 1 : 2;
			m_response_byte = 0;
			m_response_bit = 0;
			begin_transmit_turnaround();
		}
		break;

	case WAIT_CONNECT:
		if (host_lines == 0x80)
		{
			m_peer_lines = 0x40;
			m_phase = CONNECT_RELEASE;
		}
		break;

	case CONNECT_RELEASE:
		if (!host_lines)
		{
			m_peer_lines = 0;
			m_phase = RECEIVE_ASSERT;
		}
		break;

	case RECEIVE_ASSERT:
		if (host_lines == 0x40 || host_lines == 0x80)
		{
			m_receive_byte = (m_receive_byte << 1) | BIT(host_lines, 7);
			m_peer_lines = host_lines == 0x40 ? 0x80 : 0x40;
			m_phase = RECEIVE_RELEASE;
		}
		break;

	case RECEIVE_RELEASE:
		if (!host_lines)
		{
			m_peer_lines = 0;
			if (++m_receive_bits == 8)
			{
				bool const factory_reply = m_model == model::FACTORY && m_factory_stage == FACTORY_REPLY && m_receive_byte == 0x4b;
				if (factory_reply)
				{
					m_factory_stage = FACTORY_READY;
					m_response_bytes = 0;
				}
				else
				{
					prepare_response(m_receive_byte);
				}
				m_receive_byte = 0;
				m_receive_bits = 0;
				if (factory_reply)
					m_phase = WAIT_CONNECT;
				else if (m_response_bytes)
					begin_transmit_turnaround();
				else
					m_phase = RECEIVE_ASSERT;
			}
			else
			{
				m_phase = RECEIVE_ASSERT;
			}
		}
		break;

	case TRANSMIT_TURNAROUND:
		break;

	case TRANSMIT_DELAY:
		break;

	case TRANSMIT_ACK:
		if (host_lines == (m_peer_lines == 0x40 ? 0x80 : 0x40))
		{
			m_peer_lines = 0;
			m_phase = TRANSMIT_RELEASE;
		}
		break;

	case TRANSMIT_RELEASE:
		if (!host_lines)
			advance_transmit();
		break;

	default:
		break;
	}

	if (m_phase == TRANSMIT_TURNAROUND && !host_direction)
	{
		m_phase = TRANSMIT_DELAY;
		// Allow the sender to observe ACK release; exact physical peer turnaround is unmeasured.
		m_transmit_timer->adjust(attotime::from_usec(1));
	}

	if (m_phase != WAIT_CONNECT)
		m_idle_timer->adjust(attotime::from_seconds(1));
}
