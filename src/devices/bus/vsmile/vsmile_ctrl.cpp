// license:BSD-3-Clause
// copyright-holders:Ryan Holtz

#include "emu.h"
#include "vsmile_ctrl.h"

#include <algorithm>
#include <cassert>


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(VSMILE_CTRL_PORT, vsmile_ctrl_port_device, "vsmile_ctrl_port", "V.Smile Controller Port")


//**************************************************************************
//    V.Smile controller interface
//**************************************************************************

device_vsmile_ctrl_interface::device_vsmile_ctrl_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device)
	, m_port(dynamic_cast<vsmile_ctrl_port_device *>(device.owner()))
{
}

device_vsmile_ctrl_interface::~device_vsmile_ctrl_interface()
{
}

void device_vsmile_ctrl_interface::interface_validity_check(validity_checker &valid) const
{
	device_slot_card_interface::interface_validity_check(valid);

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
	device_slot_card_interface::interface_pre_start();

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
	, device_slot_interface(mconfig, *this)
	, m_rts_cb(*this)
	, m_data_cb(*this)
{
}

vsmile_ctrl_port_device::~vsmile_ctrl_port_device()
{
}

void vsmile_ctrl_port_device::device_validity_check(validity_checker &valid) const
{
	device_t *const card(get_card_device());
	if (card && !dynamic_cast<device_vsmile_ctrl_interface *>(card))
	{
		osd_printf_error(
				"Card device %s (%s) does not implement device_vsmile_ctrl_interface\n",
				card->tag(),
				card->name());
	}
}

void vsmile_ctrl_port_device::device_resolve_objects()
{
	device_vsmile_ctrl_interface *const card(dynamic_cast<device_vsmile_ctrl_interface *>(get_card_device()));
	if (card)
		m_device = card;

	m_rts_cb.resolve_safe();
	m_data_cb.resolve_safe();
}

void vsmile_ctrl_port_device::device_start()
{
	device_t *const card(get_card_device());
	if (card)
	{
		if (!m_device)
		{			throw emu_fatalerror(
					"vsmile_ctrl_port_device: card device %s (%s) does not implement device_vsmile_ctrl_interface\n",
					card->tag(),
					card->name());
		}
		else
		{
			m_device->select_w(0);
		}
	}
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
	m_tx_timer = machine().scheduler().timer_alloc(
			timer_expired_delegate(FUNC(vsmile_ctrl_device_base::tx_timer_expired), this));

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
		return false;

	// queue the byte
	m_tx_fifo[m_tx_fifo_tail] = data;
	m_tx_fifo_tail = (m_tx_fifo_tail + 1) % ARRAY_LENGTH(m_tx_fifo);
	m_tx_fifo_empty = false;

	// assert RTS and start transmitting if necessary
	if (was_empty)
	{
		rts_out(1);
		if (m_select)
		{
			m_tx_active = true;
			m_tx_timer->adjust(attotime::from_hz(9600 / 10));
		}
	}

	// data was queued
	return true;
}

void vsmile_ctrl_device_base::select_w(int state)
{
	m_select = bool(state);
	if (m_select && !m_tx_fifo_empty && !m_tx_active)
	{
		m_tx_active = true;
		m_tx_timer->adjust(attotime::from_hz(9600 / 10));
	}
}

void vsmile_ctrl_device_base::data_w(uint8_t data)
{
	rx_complete(data, m_select);
}

TIMER_CALLBACK_MEMBER(vsmile_ctrl_device_base::tx_timer_expired)
{
	assert(!m_tx_fifo_empty);
	assert(m_tx_active);

	// deliver the byte to the host (bits have shifted out now)
	uint8_t const data(m_tx_fifo[m_tx_fifo_head]);
	m_tx_fifo_head = (m_tx_fifo_head + 1) % ARRAY_LENGTH(m_tx_fifo);
	if (m_tx_fifo_head == m_tx_fifo_tail)
		m_tx_fifo_empty = true;
	data_out(data);

	// if queue is drained give implmentation a chance to queue more before dropping RTS
	if (m_tx_fifo_empty)
		tx_complete();

	// drop RTS if no more data, otherwise keep transmitting if CTS is still high
	if (m_tx_fifo_empty)
	{
		m_tx_active = false;
		rts_out(0);
	}
	else if (m_select)
	{
		m_tx_timer->adjust(attotime::from_hz(9600 / 10));
	}
	else
	{
		m_tx_active = false;
	}
}


#include "pad.h"

void vsmile_controllers(device_slot_interface &device)
{
	device.option_add("pad", VSMILE_PAD);
}
