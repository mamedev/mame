// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Acorn Econet Interface

    Part No. 200,024

    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Acorn_Econet.html

**********************************************************************/

#include "emu.h"
#include "econet.h"

#include "bus/econet/econet.h"
#include "machine/mc6854.h"


namespace {

class acorn_econet_device : public device_t, public device_acorn_bus_interface
{
public:
	acorn_econet_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, ACORN_ECONET, tag, owner, clock)
		, device_acorn_bus_interface(mconfig, *this)
		, m_adlc(*this, "mc6854")
		, m_econet(*this, "econet")
	{
	}

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<mc6854_device> m_adlc;
	required_device<econet_device> m_econet;

	uint8_t statid_r()
	{
		return 0xfe;
	}

	void bus_irq_w(int state)
	{
		m_bus->irq_w(state);
	}
};


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void acorn_econet_device::device_add_mconfig(machine_config &config)
{
	MC6854(config, m_adlc);
	m_adlc->out_txd_cb().set(m_econet, FUNC(econet_device::host_data_w));
	m_adlc->out_irq_cb().set(FUNC(acorn_econet_device::bus_irq_w));

	ECONET(config, m_econet, 0);
	m_econet->clk_wr_callback().set(m_adlc, FUNC(mc6854_device::txc_w));
	m_econet->clk_wr_callback().append(m_adlc, FUNC(mc6854_device::rxc_w));
	m_econet->data_wr_callback().set(m_adlc, FUNC(mc6854_device::set_rx));

	ECONET_SLOT(config, "econet254", m_econet, econet_devices);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void acorn_econet_device::device_start()
{
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void acorn_econet_device::device_reset()
{
	address_space &space = m_bus->memspace();

	space.install_readwrite_handler(0x1940, 0x1943, emu::rw_delegate(*m_adlc, FUNC(mc6854_device::read)), emu::rw_delegate(*m_adlc, FUNC(mc6854_device::write)));
	space.install_read_handler(0x1944, 0x1944, emu::rw_delegate(*this, FUNC(acorn_econet_device::statid_r)));
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(ACORN_ECONET, device_acorn_bus_interface, acorn_econet_device, "acorn_econet", "Acorn Econet Interface")
