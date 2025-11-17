// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Acorn Atom Econet Interface

    Part No. 102,002

**********************************************************************/

#include "emu.h"
#include "econet.h"

#include "bus/econet/econet.h"
#include "machine/mc6854.h"


namespace {

class atom_econet_device : public device_t, public device_acorn_bus_interface
{
public:
	atom_econet_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, ATOM_ECONET, tag, owner, clock)
		, device_acorn_bus_interface(mconfig, *this)
		, m_adlc(*this, "mc6854")
		, m_econet(*this, "econet")
	{
	}

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD;

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

void atom_econet_device::device_add_mconfig(machine_config &config)
{
	MC6854(config, m_adlc);
	m_adlc->out_txd_cb().set(m_econet, FUNC(econet_device::host_data_w));
	m_adlc->out_irq_cb().set(FUNC(atom_econet_device::bus_irq_w));

	ECONET(config, m_econet, 0);
	m_econet->clk_wr_callback().set(m_adlc, FUNC(mc6854_device::txc_w));
	m_econet->clk_wr_callback().append(m_adlc, FUNC(mc6854_device::rxc_w));
	m_econet->data_wr_callback().set(m_adlc, FUNC(mc6854_device::set_rx));

	ECONET_SLOT(config, "econet254", m_econet, econet_devices);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void atom_econet_device::device_start()
{
	address_space &space = m_bus->memspace();

	space.install_readwrite_handler(0xb400, 0xb403, emu::rw_delegate(*m_adlc, FUNC(mc6854_device::read)), emu::rw_delegate(*m_adlc, FUNC(mc6854_device::write)));
	space.install_read_handler(0xb404, 0xb404, emu::rw_delegate(*this, FUNC(atom_econet_device::statid_r)));
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(ATOM_ECONET, device_acorn_bus_interface, atom_econet_device, "atom_econet", "Atom Econet Interface")
