// license:BSD-3-Clause
// copyright-holders:Vas Crabb

#include "emu.h"
#include "ti8x.h"

#define LOG_GENERAL     (1U <<  0)
#define LOG_BITPROTO    (1U <<  1)
#define LOG_BYTEPROTO   (1U <<  2)

//#define VERBOSE (LOG_GENERAL | LOG_BITPROTO | LOG_BYTEPROTO)
#define LOG_OUTPUT_FUNC device().logerror
#include "logmacro.h"

#define LOGBITPROTO(...)    LOGMASKED(LOG_BITPROTO,  __VA_ARGS__)
#define LOGBYTEPROTO(...)   LOGMASKED(LOG_BYTEPROTO, __VA_ARGS__)



DEFINE_DEVICE_TYPE(TI8X_LINK_PORT, ti8x_link_port_device, "ti8x_link_port", "TI-8x Link Port")


ti8x_link_port_device::ti8x_link_port_device(
		machine_config const &mconfig,
		char const *tag,
		device_t *owner,
		uint32_t clock)
	: ti8x_link_port_device(mconfig, TI8X_LINK_PORT, tag, owner, clock)
{
}


ti8x_link_port_device::ti8x_link_port_device(
		machine_config const &mconfig,
		device_type type,
		char const *tag,
		device_t *owner,
		uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_slot_interface(mconfig, *this)
	, m_tip_handler(*this)
	, m_ring_handler(*this)
	, m_dev(nullptr)
	, m_tip_in(true)
	, m_tip_out(true)
	, m_ring_in(true)
	, m_ring_out(true)
{
}


WRITE_LINE_MEMBER(ti8x_link_port_device::tip_w)
{
	if (bool(state) != m_tip_out)
	{
		m_tip_out = bool(state);
		if (m_dev)
			m_dev->input_tip(m_tip_out ? 1 : 0);
	}
}


WRITE_LINE_MEMBER(ti8x_link_port_device::ring_w)
{
	if (bool(state) != m_ring_out)
	{
		m_ring_out = bool(state);
		if (m_dev)
			m_dev->input_ring(m_ring_out ? 1 : 0);
	}
}


void ti8x_link_port_device::device_start()
{
	m_tip_handler.resolve_safe();
	m_ring_handler.resolve_safe();

	save_item(NAME(m_tip_in));
	save_item(NAME(m_tip_out));
	save_item(NAME(m_ring_in));
	save_item(NAME(m_ring_out));

	m_tip_in = m_tip_out = true;
	m_ring_in = m_ring_out = true;
}


void ti8x_link_port_device::device_config_complete()
{
	m_dev = dynamic_cast<device_ti8x_link_port_interface *>(get_card_device());
}



device_ti8x_link_port_interface::device_ti8x_link_port_interface(
		machine_config const &mconfig,
		device_t &device)
	: device_slot_card_interface(mconfig, device)
	, m_port(dynamic_cast<ti8x_link_port_device *>(device.owner()))
{
}



device_ti8x_link_port_bit_interface::device_ti8x_link_port_bit_interface(
		machine_config const &mconfig,
		device_t &device)
	: device_ti8x_link_port_interface(mconfig, device)
	, m_error_timer(nullptr)
	, m_bit_phase(IDLE)
	, m_tx_bit_buffer(EMPTY)
	, m_tip_in(true)
	, m_ring_in(true)
{
}


void device_ti8x_link_port_bit_interface::interface_pre_start()
{
	device_ti8x_link_port_interface::interface_pre_start();

	if (!m_error_timer)
		m_error_timer = device().machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(device_ti8x_link_port_bit_interface::bit_timeout), this));

	m_bit_phase = IDLE;
	m_tx_bit_buffer = EMPTY;
	m_tip_in = m_ring_in = true;
}


void device_ti8x_link_port_bit_interface::interface_post_start()
{
	device_ti8x_link_port_interface::interface_post_start();

	device().save_item(NAME(m_bit_phase));
	device().save_item(NAME(m_tx_bit_buffer));
	device().save_item(NAME(m_tip_in));
	device().save_item(NAME(m_ring_in));
}


void device_ti8x_link_port_bit_interface::interface_pre_reset()
{
	device_ti8x_link_port_interface::interface_pre_reset();

	m_error_timer->reset();
	m_bit_phase = (m_tip_in && m_ring_in) ? IDLE : WAIT_IDLE;
	m_tx_bit_buffer = EMPTY;

	output_tip(1);
	output_ring(1);
}


void device_ti8x_link_port_bit_interface::send_bit(bool data)
{
	LOGBITPROTO("queue %d bit\n", data ? 1 : 0);

	if (EMPTY != m_tx_bit_buffer)
		device().logerror("device_ti8x_link_port_bit_interface: warning: transmit buffer overrun\n");

	m_tx_bit_buffer = data ? PENDING_1 : PENDING_0;
	if (IDLE == m_bit_phase)
		check_tx_bit_buffer();
	else if (WAIT_IDLE == m_bit_phase)
		m_error_timer->reset(attotime(1, 0)); // TODO: configurable timeout
}


void device_ti8x_link_port_bit_interface::accept_bit()
{
	switch (m_bit_phase)
	{
	// can't accept a bit that isn't being held
	case IDLE:
	case WAIT_ACK_0:
	case WAIT_ACK_1:
	case WAIT_REL_0:
	case WAIT_REL_1:
	case ACK_0:
	case ACK_1:
	case WAIT_IDLE:
		fatalerror("device_ti8x_link_port_bit_interface: attempt to accept bit when not holding");
		break;

	// release the acknowledgement - if the ring doesn't rise we've lost sync
	case HOLD_0:
		assert(m_tip_in);

		output_ring(1);
		if (m_ring_in)
		{
			LOGBITPROTO("accepted 0 bit\n");
			check_tx_bit_buffer();
		}
		else
		{
			LOGBITPROTO("accepted 0 bit, ring low (collision) - waiting for bus idle\n");
			m_error_timer->reset((EMPTY == m_tx_bit_buffer) ? attotime::never : attotime(1, 0)); // TODO: configurable timeout
			m_bit_phase = WAIT_IDLE;
			bit_collision();
		}
		break;

	// release the acknowledgement - if the tip doesn't rise we've lost sync
	case HOLD_1:
		assert(m_ring_in);

		output_tip(1);
		if (m_tip_in)
		{
			LOGBITPROTO("accepted 1 bit\n");
			check_tx_bit_buffer();
		}
		else
		{
			LOGBITPROTO("accepted 1 bit, tip low (collision) - waiting for bus idle\n");
			m_error_timer->reset((EMPTY == m_tx_bit_buffer) ? attotime::never : attotime(1, 0)); // TODO: configurable timeout
			m_bit_phase = WAIT_IDLE;
			bit_collision();
		}
		break;

	// something very bad happened (heap smash?)
	default:
		throw false;
	}
}


WRITE_LINE_MEMBER(device_ti8x_link_port_bit_interface::input_tip)
{
	m_tip_in = bool(state);
	switch (m_bit_phase)
	{
	// if tip falls while idle, it's the beginning of an incoming 0
	case IDLE:
		if (!m_tip_in)
		{
			LOGBITPROTO("falling edge on tip, acknowledging 0 bit\n");
			m_error_timer->reset(attotime(1, 0)); // TODO: configurable timeout
			m_bit_phase = ACK_0;
			output_ring(0);
		}
		break;

	// we're driving tip low in this state, ignore it
	case WAIT_ACK_0:
	case ACK_1:
	case HOLD_1:
		break;

	// tip must fall to acknowledge outgoing 1
	case WAIT_ACK_1:
		if (!m_tip_in)
		{
			LOGBITPROTO("falling edge on tip, 1 bit acknowledged, confirming\n");
			m_error_timer->reset(attotime(1, 0)); // TODO: configurable timeout
			m_bit_phase = WAIT_REL_1;
			output_ring(1);
		}
		break;

	// if tip falls now, we've lost sync
	case WAIT_REL_0:
	case HOLD_0:
		if (!m_tip_in)
		{
			LOGBITPROTO("falling edge on tip, lost sync, waiting for bus idle\n");
			m_error_timer->reset((EMPTY == m_tx_bit_buffer) ? attotime::never : attotime(1, 0)); // TODO: configurable timeout
			m_bit_phase = WAIT_IDLE;
			output_ring(1);
			bit_collision();
		}
		break;

	// tip must rise to complete outgoing 1 sequence
	case WAIT_REL_1:
		if (m_tip_in)
		{
			assert(!m_ring_in);

			LOGBITPROTO("rising edge on tip, 1 bit sent\n");
			check_tx_bit_buffer();
			bit_sent();
		}
		break;

	// tip must rise to accept our acknowledgement
	case ACK_0:
		if (m_tip_in)
		{
			LOGBITPROTO("rising edge on tip, 0 bit acknowledge confirmed, holding\n");
			m_error_timer->reset();
			m_bit_phase = HOLD_0;
			bit_received(false);
		}
		break;

	// if the bus is available, check for bit to send
	case WAIT_IDLE:
		if (m_tip_in && m_ring_in)
		{
			LOGBITPROTO("rising edge on tip, bus idle detected\n");
			check_tx_bit_buffer();
		}
		break;

	// something very bad happened (heap smash?)
	default:
		throw false;
	}
}


WRITE_LINE_MEMBER(device_ti8x_link_port_bit_interface::input_ring)
{
	m_ring_in = bool(state);
	switch (m_bit_phase)
	{
	// if ring falls while idle, it's the beginning of an incoming 1
	case IDLE:
		if (!m_ring_in)
		{
			LOGBITPROTO("falling edge on ring, acknowledging 1 bit\n");
			m_error_timer->reset(attotime(1, 0)); // TODO: configurable timeout
			m_bit_phase = ACK_1;
			output_tip(0);
		}
		break;

	// ring must fall to acknowledge outgoing 0
	case WAIT_ACK_0:
		if (!m_ring_in)
		{
			LOGBITPROTO("falling edge on ring, 0 bit acknowledged, confirming\n");
			m_error_timer->reset(attotime(1, 0)); // TODO: configurable timeout
			m_bit_phase = WAIT_REL_0;
			output_tip(1);
		}
		break;

	// we're driving ring low in this state, ignore it
	case WAIT_ACK_1:
	case ACK_0:
	case HOLD_0:
		break;

	// ring must rise to complete outgoing 0 sequence
	case WAIT_REL_0:
		if (m_ring_in)
		{
			assert(!m_tip_in);

			LOGBITPROTO("rising edge on ring, 0 bit sent\n");
			check_tx_bit_buffer();
			bit_sent();
		}
		break;

	// if ring falls now, we've lost sync
	case WAIT_REL_1:
	case HOLD_1:
		if (!m_ring_in)
		{
			LOGBITPROTO("falling edge on ring, lost sync, waiting for bus idle\n");
			m_error_timer->reset((EMPTY == m_tx_bit_buffer) ? attotime::never : attotime(1, 0)); // TODO: configurable timeout
			m_bit_phase = WAIT_IDLE;
			output_tip(1);
			bit_collision();
		}
		break;

	// ring must rise to accept our acknowledgement
	case ACK_1:
		if (m_ring_in)
		{
			LOGBITPROTO("rising edge on ring, 1 bit acknowledge confirmed, holding\n");
			m_error_timer->reset();
			m_bit_phase = HOLD_1;
			bit_received(true);
		}
		break;

	// if the bus is available, check for bit to send
	case WAIT_IDLE:
		if (m_tip_in && m_ring_in)
		{
			LOGBITPROTO("rising edge on tip, bus idle detected\n");
			check_tx_bit_buffer();
		}
		break;

	// something very bad happened (heap smash?)
	default:
		throw false;
	}
}


TIMER_CALLBACK_MEMBER(device_ti8x_link_port_bit_interface::bit_timeout)
{
	switch (m_bit_phase)
	{
	// something very bad happened (heap smash?)
	case IDLE:
	case HOLD_0:
	case HOLD_1:
	default:
		throw false;

	// receive timeout
	case ACK_0:
	case ACK_1:
		LOGBITPROTO("timeout acknowledging %d bit\n", (ACK_0 == m_bit_phase) ? 0 : 1);
		output_tip(1);
		output_ring(1);
		if (m_tip_in && m_ring_in)
		{
			check_tx_bit_buffer();
		}
		else
		{
			LOGBITPROTO("waiting for bus idle\n");
			m_error_timer->reset((EMPTY == m_tx_bit_buffer) ? attotime::never : attotime(1, 0)); // TODO: configurable timeout
			m_bit_phase = WAIT_IDLE;
		}
		bit_receive_timeout();
		break;

	// send timeout:
	case WAIT_IDLE:
		assert(EMPTY != m_tx_bit_buffer);
	case WAIT_ACK_0:
	case WAIT_ACK_1:
	case WAIT_REL_0:
	case WAIT_REL_1:
		LOGBITPROTO("timeout sending bit\n");
		m_error_timer->reset();
		m_bit_phase = (m_tip_in && m_ring_in) ? IDLE : WAIT_IDLE;
		m_tx_bit_buffer = EMPTY;
		output_tip(1);
		output_ring(1);
		bit_send_timeout();
		break;
	}
}


void device_ti8x_link_port_bit_interface::check_tx_bit_buffer()
{
	assert(m_tip_in);
	assert(m_ring_in);

	switch (m_tx_bit_buffer)
	{
	// nothing to do
	case EMPTY:
		LOGBITPROTO("no pending bit, entering idle state\n");
		m_error_timer->reset();
		m_bit_phase = IDLE;
		break;

	// pull tip low and wait for acknowledgement
	case PENDING_0:
		LOGBITPROTO("sending 0 bit, pulling tip low\n");
		m_error_timer->reset(attotime(1, 0)); // TODO: configurable timeout
		m_bit_phase = WAIT_ACK_0;
		m_tx_bit_buffer = EMPTY;
		output_tip(0);
		break;

	// pull ring low and wait for acknowledgement
	case PENDING_1:
		LOGBITPROTO("sending 1 bit, pulling ring low\n");
		m_error_timer->reset(attotime(1, 0)); // TODO: configurable timeout
		m_bit_phase = WAIT_ACK_1;
		m_tx_bit_buffer = EMPTY;
		output_ring(0);
		break;

	// something very bad happened (heap smash?)
	default:
		throw false;
	}
}



device_ti8x_link_port_byte_interface::device_ti8x_link_port_byte_interface(
		machine_config const &mconfig,
		device_t &device)
	: device_ti8x_link_port_bit_interface(mconfig, device)
	, m_tx_byte_buffer(0U)
	, m_rx_byte_buffer(0U)
{
}


void device_ti8x_link_port_byte_interface::interface_pre_start()
{
	device_ti8x_link_port_bit_interface::interface_pre_start();

	m_tx_byte_buffer = m_rx_byte_buffer = 0U;
}


void device_ti8x_link_port_byte_interface::interface_post_start()
{
	device_ti8x_link_port_bit_interface::interface_post_start();

	device().save_item(NAME(m_tx_byte_buffer));
	device().save_item(NAME(m_rx_byte_buffer));
}


void device_ti8x_link_port_byte_interface::interface_pre_reset()
{
	device_ti8x_link_port_bit_interface::interface_pre_reset();

	m_tx_byte_buffer = m_rx_byte_buffer = 0U;
}


void device_ti8x_link_port_byte_interface::send_byte(u8 data)
{
	if (m_tx_byte_buffer)
		device().logerror("device_ti8x_link_port_byte_interface: warning: transmit buffer overrun\n");

	LOGBYTEPROTO("sending byte 0x%02X\n", data);
	m_tx_byte_buffer = 0x0080 | u16(data >> 1);
	send_bit(BIT(data, 0));
}


void device_ti8x_link_port_byte_interface::accept_byte()
{
	assert(BIT(m_rx_byte_buffer, 8));

	LOGBYTEPROTO("accepting final bit of byte\n");
	m_rx_byte_buffer = 0U;
	accept_bit();
}


void device_ti8x_link_port_byte_interface::bit_collision()
{
	LOGBYTEPROTO("bit collection, clearing byte buffers\n");
	m_tx_byte_buffer = m_rx_byte_buffer = 0U;
	byte_collision();
}


void device_ti8x_link_port_byte_interface::bit_send_timeout()
{
	LOGBYTEPROTO("bit send timeout, clearing send byte buffer\n");
	m_tx_byte_buffer = 0U;
	byte_send_timeout();
}


void device_ti8x_link_port_byte_interface::bit_receive_timeout()
{
	LOGBYTEPROTO("bit receive timeout, clearing receive byte buffer\n");
	m_rx_byte_buffer = 0U;
	byte_receive_timeout();
}


void device_ti8x_link_port_byte_interface::bit_sent()
{
	assert(m_tx_byte_buffer);

	bool const data(BIT(m_tx_byte_buffer, 0));
	if (m_tx_byte_buffer >>= 1)
	{
		LOGBYTEPROTO("bit sent, sending next bit of byte\n");
		send_bit(data);
	}
	else
	{
		assert(data);

		LOGBYTEPROTO("final bit of byte sent\n");
		byte_sent();
	}
}


void device_ti8x_link_port_byte_interface::bit_received(bool data)
{
	assert(!BIT(m_rx_byte_buffer, 8));

	m_rx_byte_buffer = (!m_rx_byte_buffer ? 0x8000 : (m_rx_byte_buffer >> 1)) | (data ? 0x0080U : 0x0000U);
	if (BIT(m_rx_byte_buffer, 8))
	{
		LOGBYTEPROTO("received final bit of byte 0x%02X\n", u8(m_rx_byte_buffer));
		byte_received(u8(m_rx_byte_buffer));
	}
	else
	{
		LOGBYTEPROTO("bit received, accepting\n");
		accept_bit();
	}
}



#include "bitsocket.h"
#include "graphlinkhle.h"
#include "teeconn.h"
#include "tispeaker.h"

void default_ti8x_link_devices(device_slot_interface &device)
{
	device.option_add("bitsock",       TI8X_BIT_SOCKET);
	device.option_add("glinkhle",      TI8X_GRAPH_LINK_HLE);
	device.option_add("tee",           TI8X_TEE_CONNECTOR);
	device.option_add("monospkr",      TI8X_SPEAKER_MONO);
	device.option_add("stereospkr",    TI8X_SPEAKER_STEREO);
}
