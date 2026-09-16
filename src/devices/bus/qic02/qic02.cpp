// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * QIC-02 connector and controller interface.
 *
 * All signals (except data) are specified and emulated active low.
 *
 * Sources:
 *  - QIC-02 Rev D 1/4 inch Cartridge Tape Drive Intelligent Interface Standard, September 23, 1982, Archive Corporation
 *
 */

#include "emu.h"
#include "qic02.h"

//#define VERBOSE (LOG_GENERAL)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(QIC02_CONNECTOR, qic02_connector_device, "qic02_connector", "QIC-02 Connector")

qic02_connector_device::qic02_connector_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, QIC02_CONNECTOR, tag, owner, clock)
	, device_single_card_slot_interface<device_qic02_interface>(mconfig, *this)
	, m_ack(*this)
	, m_rdy(*this)
	, m_exc(*this)
	, m_dir(*this)
{
}

qic02_connector_device::~qic02_connector_device()
{
}

void qic02_connector_device::device_start()
{
}

void qic02_connector_device::ack_w(int state)
{
	m_ack(state);
}

void qic02_connector_device::rdy_w(int state)
{
	m_rdy(state);
}

void qic02_connector_device::exc_w(int state)
{
	m_exc(state);
}

void qic02_connector_device::dir_w(int state)
{
	m_dir(state);
}

void qic02_connector_device::onl_w(int state)
{
	if (device_qic02_interface *const c = get_card_device())
		c->onl_w(state);
}

void qic02_connector_device::req_w(int state)
{
	if (device_qic02_interface *const c = get_card_device())
		c->req_w(state);
}

void qic02_connector_device::rst_w(int state)
{
	if (device_qic02_interface *const c = get_card_device())
		c->rst_w(state);
}

void qic02_connector_device::xfr_w(int state)
{
	if (device_qic02_interface *const c = get_card_device())
		c->xfr_w(state);
}

u8 qic02_connector_device::data_r()
{
	if (device_qic02_interface *const c = get_card_device())
		return c->data_r();
	else
		return 0;
}

void qic02_connector_device::data_w(u8 data)
{
	if (device_qic02_interface *const c = get_card_device())
		c->data_w(data);
}

device_qic02_interface::device_qic02_interface(machine_config const &mconfig, device_t &device)
	: device_interface(device, "qic02_controller")
	, m_qic(dynamic_cast<qic02_connector_device *>(device.owner()))
{
}

device_qic02_interface::~device_qic02_interface()
{
}
