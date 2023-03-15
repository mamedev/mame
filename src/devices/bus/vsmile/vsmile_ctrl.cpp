// license:BSD-3-Clause
// copyright-holders:Vas Crabb

#include "emu.h"
#include "vsmile_ctrl.h"

#include <algorithm>
#include <cassert>

//#define VERBOSE 1
#include "logmacro.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(VSMILE_CTRL_PORT, vsmile_ctrl_port_device, "vsmile_ctrl_port", "V.Smile Controller Port")


//**************************************************************************
//    V.Smile controller interface
//**************************************************************************

device_vsmile_ctrl_interface::device_vsmile_ctrl_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "vsmilectrl")
	, m_port(dynamic_cast<vsmile_ctrl_port_device *>(device.owner()))
{
}

device_vsmile_ctrl_interface::~device_vsmile_ctrl_interface()
{
}

void device_vsmile_ctrl_interface::interface_validity_check(validity_checker &valid) const
{
	if (device().owner() && !m_port)
	{
		osd_printf_error(
				"Owner device %s (%s) is not a vsmile_ctrl_port_device\n",
				device().owner()->tag(),
				device().owner()->name());
	}
}

void device_vsmile_ctrl_interface::interface_pre_start()
{
	if (m_port && !m_port->started())
		throw device_missing_dependencies();
}


//**************************************************************************
//    V.Smile controller port
//**************************************************************************

vsmile_ctrl_port_device::vsmile_ctrl_port_device(
		machine_config const &mconfig,
		char const *tag,
		device_t *owner,
		uint32_t clock)
	: device_t(mconfig, VSMILE_CTRL_PORT, tag, owner, clock)
	, device_single_card_slot_interface<device_vsmile_ctrl_interface>(mconfig, *this)
	, m_rts_cb(*this)
	, m_data_cb(*this)
{
}

vsmile_ctrl_port_device::~vsmile_ctrl_port_device()
{
}

void vsmile_ctrl_port_device::device_resolve_objects()
{
	m_device = get_card_device();

	m_rts_cb.resolve_safe();
	m_data_cb.resolve_safe();
}

void vsmile_ctrl_port_device::device_start()
{
	if (m_device)
		m_device->select_w(0);
}


//**************************************************************************
//    V.Smile controller HLE base
//**************************************************************************

vsmile_ctrl_device_base::vsmile_ctrl_device_base(
		machine_config const &mconfig,
		device_type type,
		char const *tag,
		device_t *owner,
		uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_vsmile_ctrl_interface(mconfig, *this)
	, m_tx_timer(nullptr)
	, m_rts_timer(nullptr)
	, m_tx_fifo_head(0U)
	, m_tx_fifo_tail(0U)
	, m_tx_fifo_empty(true)
	, m_tx_active(false)
	, m_select(false)
{
	std::fill(std::begin(m_tx_fifo), std::end(m_tx_fifo), 0);
}

vsmile_ctrl_device_base::~vsmile_ctrl_device_base()
{
}

void vsmile_ctrl_device_base::device_start()
{
	// allocate a timer to limit transmit rate to something realistic
	m_tx_timer = timer_alloc(FUNC(vsmile_ctrl_device_base::tx_timer_expired), this);

	// allocate a timer for RTS timeouts
	m_rts_timer = timer_alloc(FUNC(vsmile_ctrl_device_base::rts_timer_expired), this);

	// start with transmit queue empty
	m_tx_fifo_head = m_tx_fifo_tail = 0U;
	m_tx_fifo_empty = true;
	m_tx_active = false;

	// register for save states
	save_item(NAME(m_tx_fifo));
	save_item(NAME(m_tx_fifo_head));
	save_item(NAME(m_tx_fifo_tail));
	save_item(NAME(m_tx_fifo_empty));
	save_item(NAME(m_select));
}

bool vsmile_ctrl_device_base::queue_tx(uint8_t data)
{
	// return false on overrun and drop byte
	bool const was_empty(m_tx_fifo_empty);
	if (!was_empty && (m_tx_fifo_head == m_tx_fifo_tail))
	{
		LOG(
				"%s: discarding byte %02X because FIFO is full (length %u, Tx %sactive)\n",
				machine().describe_context(),
				data,
				std::size(m_tx_fifo),
				m_tx_active ? "" : "in");
		return false;
	}

	// queue the byte
	m_tx_fifo[m_tx_fifo_tail] = data;
	m_tx_fifo_tail = (m_tx_fifo_tail + 1) % std::size(m_tx_fifo);
	m_tx_fifo_empty = false;

	// assert RTS and start transmitting if necessary
	if (was_empty)
	{
		rts_out(1);
		if (m_select)
		{
			LOG("%s: transmitting byte %02X immediately (Tx was %sactive)\n", machine().describe_context(), data, m_tx_active ? "" : "in");
			m_tx_active = true;
			m_tx_timer->adjust(attotime::from_hz(9600 / 10));
		}
		else
		{
			LOG("%s: asserting RTS to transmit byte %02X\n", machine().describe_context(), data);
			m_rts_timer->adjust(attotime::from_msec(500));
		}
	}
	else
	{
		unsigned const fifo_used((m_tx_fifo_tail + std::size(m_tx_fifo) - m_tx_fifo_head) % std::size(m_tx_fifo));
		LOG("%s: queued byte %02X (%u bytes queued, Tx %sactive)\n", machine().describe_context(), data, fifo_used, m_tx_active ? "" : "in");
	}

	// data was queued
	return true;
}

void vsmile_ctrl_device_base::select_w(int state)
{
	if (bool(state) != m_select)
	{
		if (state && !m_tx_fifo_empty && !m_tx_active)
		{
			m_rts_timer->reset();
			unsigned const fifo_used((m_tx_fifo_tail + std::size(m_tx_fifo) - m_tx_fifo_head) % std::size(m_tx_fifo));
			LOG("%s: select asserted, starting transmission (%u bytes queued)\n", machine().describe_context(), fifo_used);
			m_tx_active = true;
			m_tx_timer->adjust(attotime::from_hz(9600 / 10));
		}
		else
		{
			LOG("%s: select %sasserted (Tx %sactive)\n", machine().describe_context(), state ? "" : "de", m_tx_active ? "" : "in");
		}
		m_select = bool(state);
	}
}

void vsmile_ctrl_device_base::data_w(uint8_t data)
{
	LOG(
			"%s: received byte %02X (select %sasserted, Tx %sactive)\n",
			machine().describe_context(),
			data,
			m_select ? "" : "de",
			m_tx_active ? "" : "in");
	rx_complete(data, m_select);
}

TIMER_CALLBACK_MEMBER(vsmile_ctrl_device_base::tx_timer_expired)
{
	assert(!m_tx_fifo_empty);
	assert(m_tx_active);

	// deliver the byte to the host (bits have shifted out now)
	uint8_t const data(m_tx_fifo[m_tx_fifo_head]);
	m_tx_fifo_head = (m_tx_fifo_head + 1) % std::size(m_tx_fifo);
	if (m_tx_fifo_head == m_tx_fifo_tail)
		m_tx_fifo_empty = true;
	data_out(data);

	// if queue is drained give implmentation a chance to queue more before dropping RTS
	if (m_tx_fifo_empty)
	{
		LOG("transmitted byte %02X, queue empty (select %sasserted)\n", data, m_select ? "" : "de");
		tx_complete();
	}
	else
	{
		unsigned const fifo_used((m_tx_fifo_tail + std::size(m_tx_fifo) - m_tx_fifo_head) % std::size(m_tx_fifo));
		LOG("transmitted byte %02X (%u bytes queued, select %sasserted)\n", data, fifo_used, m_select ? "" : "de");
	}

	// drop RTS if no more data, otherwise keep transmitting if CTS is still high
	if (m_tx_fifo_empty)
	{
		LOG("nothing to transmit, deasserting RTS\n");
		m_tx_active = false;
		rts_out(0);
	}
	else if (m_select)
	{
		LOG("select asserted, transmitting next byte %02X\n", m_tx_fifo[m_tx_fifo_head]);
		m_tx_timer->adjust(attotime::from_hz(9600 / 10));
	}
	else
	{
		LOG("select deasserted, waiting to transmit\n");
		m_tx_active = false;
		//m_rts_timer->adjust(attotime::from_msec(2000));
	}
}


TIMER_CALLBACK_MEMBER(vsmile_ctrl_device_base::rts_timer_expired)
{
	assert(!m_tx_fifo_empty);
	assert(!m_tx_active);

	// clear out anything queued and let the implementation deal with it
	if (!m_tx_fifo_empty)
	{
		unsigned const fifo_used((m_tx_fifo_tail + std::size(m_tx_fifo) - m_tx_fifo_head) % std::size(m_tx_fifo));
		LOG("timeout waiting for select after asserting RTS (%u bytes queued)\n", fifo_used);
		m_tx_fifo_head = m_tx_fifo_tail = 0U;
		m_tx_fifo_empty = true;
		tx_timeout();
		if (m_tx_fifo_empty)
			rts_out(0);
	}
}


#include "pad.h"
#include "mat.h"
#include "keyboard.h"

void vsmile_controllers(device_slot_interface &device)
{
	device.option_add("joy", VSMILE_PAD);
	device.option_add("mat", VSMILE_MAT);
	device.option_add("smartkb_us", VSMILE_KEYBOARD_US);
	device.option_add("smartkb_fr", VSMILE_KEYBOARD_FR);
	device.option_add("smartkb_ge", VSMILE_KEYBOARD_GE);
}
