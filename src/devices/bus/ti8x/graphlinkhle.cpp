// license:BSD-3-Clause
// copyright-holders:Vas Crabb

#include "emu.h"
#include "graphlinkhle.h"

#include "bus/rs232/rs232.h"

#include "diserial.h"

#include <memory>


namespace {

class graph_link_hle_device
		: public device_t
		, public device_ti8x_link_port_byte_interface
		, public device_serial_interface
{
public:
	graph_link_hle_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void byte_collision() override;
	virtual void byte_send_timeout() override;
	virtual void byte_receive_timeout() override;
	virtual void byte_sent() override;
	virtual void byte_received(u8 data) override;

	virtual void rcv_complete() override;
	virtual void tra_callback() override;
	virtual void tra_complete() override;

private:
	static constexpr unsigned BUFLEN = 1U << 16;

	required_device<rs232_port_device>  m_serial_port;
	std::unique_ptr<u8 []>              m_buffer;
	unsigned                            m_head, m_tail;
	bool                                m_empty, m_ready;
};


graph_link_hle_device::graph_link_hle_device(
		machine_config const &mconfig,
		char const *tag,
		device_t *owner,
		uint32_t clock)
	: device_t(mconfig, TI8X_GRAPH_LINK_HLE, tag, owner, clock)
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
	m_buffer = std::make_unique<u8 []>(BUFLEN);

	save_pointer(NAME(m_buffer), BUFLEN);
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


void graph_link_hle_device::device_add_mconfig(machine_config &config)
{
	RS232_PORT(config, m_serial_port, default_rs232_devices, nullptr);
	m_serial_port->rxd_handler().set(FUNC(graph_link_hle_device::rx_w));
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

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(TI8X_GRAPH_LINK_HLE, device_ti8x_link_port_interface, graph_link_hle_device, "ti8x_glinkhle", "TI-Graph Link (grey, HLE)")
