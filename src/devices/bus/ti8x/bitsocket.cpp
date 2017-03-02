// license:BSD-3-Clause
// copyright-holders:Vas Crabb

#include "bitsocket.h"


device_type const TI8X_BIT_SOCKET = device_creator<bus::ti8x::bit_socket_device>;


namespace bus { namespace ti8x {

namespace {

MACHINE_CONFIG_FRAGMENT(bit_socket)
	MCFG_DEVICE_ADD("stream", BITBANGER, 0)
MACHINE_CONFIG_END

} // anonymous namespace


bit_socket_device::bit_socket_device(
		machine_config const &mconfig,
		char const *tag,
		device_t *owner,
		uint32_t clock)
	: device_t(mconfig, TI8X_BIT_SOCKET, "TI-8x Bit Socket", tag, owner, clock, "ti8xbitsock", __FILE__)
	, device_ti8x_link_port_interface(mconfig, *this)
	, m_stream(*this, "stream")
	, m_poll_timer(nullptr)
	, m_tip_in(true)
	, m_ring_in(true)
{
}


machine_config_constructor bit_socket_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME(bit_socket);
}


void bit_socket_device::device_start()
{
	m_poll_timer = timer_alloc(TIMER_ID_POLL);

	save_item(NAME(m_tip_in));
	save_item(NAME(m_ring_in));

	m_tip_in = m_ring_in = true;

	m_poll_timer->adjust(attotime::from_hz(200000), 0, attotime::from_hz(200000));
}


void bit_socket_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_ID_POLL:
		{
			u8 data;
			while (m_stream->input(&data, 1))
			{
				if (BIT(data, 1)) output_tip(BIT(data, 0));
				if (BIT(data, 2)) output_ring(BIT(data, 0));
			}
		}
		break;

	default:
		break;
	}
}


WRITE_LINE_MEMBER(bit_socket_device::input_tip)
{
	m_tip_in = bool(state);
	m_stream->output((m_tip_in ? 0x01 : 0x00) | 0x02);
}


WRITE_LINE_MEMBER(bit_socket_device::input_ring)
{
	m_ring_in = bool(state);
	m_stream->output((m_ring_in ? 0x01 : 0x00) | 0x04);
}

} } // namespace bus::ti8x
