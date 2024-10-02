// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/*
 A T-connector, strangely enough.
 */

#include "emu.h"
#include "teeconn.h"


namespace {

class tee_connector_device : public device_t, public device_ti8x_link_port_interface
{
public:
	tee_connector_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	virtual void input_tip(int state) override;
	virtual void input_ring(int state) override;

	void tip_a_w(int state);
	void ring_a_w(int state);
	void tip_b_w(int state);
	void ring_b_w(int state);

	required_device<ti8x_link_port_device>  m_port_a;
	required_device<ti8x_link_port_device>  m_port_b;

	bool    m_tip_host, m_tip_a, m_tip_b;
	bool    m_ring_host, m_ring_a, m_ring_b;
};


tee_connector_device::tee_connector_device(
		machine_config const &mconfig,
		char const *tag,
		device_t *owner,
		uint32_t clock)
	: device_t(mconfig, TI8X_TEE_CONNECTOR, tag, owner, clock)
	, device_ti8x_link_port_interface(mconfig, *this)
	, m_port_a(*this, "a")
	, m_port_b(*this, "b")
	, m_tip_host(true)
	, m_tip_a(true)
	, m_tip_b(true)
	, m_ring_host(true)
	, m_ring_a(true)
	, m_ring_b(true)
{
}


void tee_connector_device::tip_a_w(int state)
{
	m_tip_a = bool(state);
	output_tip((m_tip_a && m_tip_b) ? 1 : 0);
	m_port_b->tip_w((m_tip_host && m_tip_a) ? 1 : 0);
}


void tee_connector_device::ring_a_w(int state)
{
	m_ring_a = bool(state);
	output_ring((m_ring_a && m_ring_b) ? 1 : 0);
	m_port_b->ring_w((m_ring_host && m_ring_a) ? 1 : 0);
}


void tee_connector_device::tip_b_w(int state)
{
	m_tip_b = bool(state);
	output_tip((m_tip_a && m_tip_b) ? 1 : 0);
	m_port_a->tip_w((m_tip_host && m_tip_b) ? 1 : 0);
}


void tee_connector_device::ring_b_w(int state)
{
	m_ring_b = bool(state);
	output_ring((m_ring_a && m_ring_b) ? 1 : 0);
	m_port_a->ring_w((m_ring_host && m_ring_b) ? 1 : 0);
}


void tee_connector_device::device_add_mconfig(machine_config &config)
{
	TI8X_LINK_PORT(config, m_port_a, default_ti8x_link_devices, nullptr);
	m_port_a->tip_handler().set(FUNC(tee_connector_device::tip_a_w));
	m_port_a->ring_handler().set(FUNC(tee_connector_device::ring_a_w));

	TI8X_LINK_PORT(config, m_port_b, default_ti8x_link_devices, nullptr);
	m_port_b->tip_handler().set(FUNC(tee_connector_device::tip_b_w));
	m_port_b->ring_handler().set(FUNC(tee_connector_device::ring_b_w));
}


void tee_connector_device::device_start()
{
	save_item(NAME(m_tip_host));
	save_item(NAME(m_tip_a));
	save_item(NAME(m_tip_b));
	save_item(NAME(m_ring_host));
	save_item(NAME(m_ring_a));
	save_item(NAME(m_ring_b));

	m_tip_host = m_tip_a = m_tip_b = true;
	m_ring_host = m_ring_a = m_ring_b = true;
}


void tee_connector_device::input_tip(int state)
{
	m_tip_host = bool(state);
	m_port_a->tip_w((m_tip_host && m_tip_b) ? 1 : 0);
	m_port_b->tip_w((m_tip_host && m_tip_a) ? 1 : 0);
}


void tee_connector_device::input_ring(int state)
{
	m_ring_host = bool(state);
	m_port_a->ring_w((m_ring_host && m_ring_b) ? 1 : 0);
	m_port_b->ring_w((m_ring_host && m_ring_a) ? 1 : 0);
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(TI8X_TEE_CONNECTOR, device_ti8x_link_port_interface, tee_connector_device, "ti8x_tconn", "TI-8x T-connector")
