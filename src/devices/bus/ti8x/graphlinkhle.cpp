// license:BSD-3-Clause
// copyright-holders:Vas Crabb

#include "emu.h"
#include "graphlinkhle.h"


device_type const TI8X_GRAPH_LINK_HLE = device_creator<bus::ti8x::graph_link_hle_device>;


namespace bus { namespace ti8x {

namespace {

MACHINE_CONFIG_FRAGMENT(graph_link_hle)
	MCFG_RS232_PORT_ADD("rs232", default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(WRITELINE(graph_link_hle_device, rx_w))
MACHINE_CONFIG_END

} // anonymous namespace


graph_link_hle_device::graph_link_hle_device(
		machine_config const &mconfig,
		char const *tag,
		device_t *owner,
		uint32_t clock)
	: device_t(mconfig, TI8X_GRAPH_LINK_HLE, "TI-Graph Link (grey, HLE)", tag, owner, clock, "glinkhle", __FILE__)
	, device_ti8x_link_port_byte_interface(mconfig, *this)
	, device_serial_interface(mconfig, *this)
	, m_serial_port(*this, "rs232")
	, m_buffer()
	, m_head(0)
	, m_tail(0)
	, m_empty(true)
	, m_ready(true)
{
}


void graph_link_hle_device::device_start()
{
	device_serial_interface::register_save_state(machine().save(), this);

	m_buffer = std::make_unique<u8 []>(BUFLEN);

	save_pointer(NAME(m_buffer.get()), BUFLEN);
}


void graph_link_hle_device::device_reset()
{
	set_data_frame(1, 8, PARITY_NONE, STOP_BITS_1);
	set_rate(9600);
	receive_register_reset();
	transmit_register_reset();

	m_head = m_tail = 0;
	m_empty = true;
	m_ready = true;
}


void graph_link_hle_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	device_ti8x_link_port_byte_interface::device_timer(timer, id, param, ptr);
	device_serial_interface::device_timer(timer, id, param, ptr);
}


machine_config_constructor graph_link_hle_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME(graph_link_hle);
}


void graph_link_hle_device::byte_collision()
{
	if (m_empty)
	{
		m_ready = true;
	}
	else
	{
		send_byte(m_buffer[m_tail]);
		m_tail = (m_tail + 1) % BUFLEN;
		m_empty = m_head == m_tail;
	}
}


void graph_link_hle_device::byte_send_timeout()
{
	if (m_empty)
	{
		m_ready = true;
	}
	else
	{
		send_byte(m_buffer[m_tail]);
		m_tail = (m_tail + 1) % BUFLEN;
		m_empty = m_head == m_tail;
	}
}


void graph_link_hle_device::byte_receive_timeout()
{
}


void graph_link_hle_device::byte_sent()
{
	if (m_empty)
	{
		m_ready = true;
	}
	else
	{
		send_byte(m_buffer[m_tail]);
		m_tail = (m_tail + 1) % BUFLEN;
		m_empty = m_head == m_tail;
	}
}


void graph_link_hle_device::byte_received(u8 data)
{
	transmit_register_setup(data);
}


void graph_link_hle_device::rcv_complete()
{
	receive_register_extract();
	if (m_ready)
	{
		assert(m_empty);

		send_byte(get_received_char());
		m_ready = false;
	}
	else
	{
		m_buffer[m_head] = get_received_char();
		m_head = (m_head + 1) % BUFLEN;
		m_empty = false;
	}
}


void graph_link_hle_device::tra_callback()
{
	m_serial_port->write_txd(transmit_register_get_data_bit());
}


void graph_link_hle_device::tra_complete()
{
	accept_byte();
}

} } // namespace bus::ti8x
