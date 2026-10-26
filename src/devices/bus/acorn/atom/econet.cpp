// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Acorn Atom Econet Interface

    Part No. 102,002

**********************************************************************/

#include "emu.h"
#include "econet.h"

#include "bus/econet/econet.h"
#include "machine/74123.h"
#include "machine/input_merger.h"
#include "machine/mc6854.h"
#include "machine/rescap.h"


namespace {

class atom_econet_device : public device_t, public device_acorn_bus_interface
{
public:
	atom_econet_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, ATOM_ECONET, tag, owner, clock)
		, device_acorn_bus_interface(mconfig, *this)
		, m_adlc(*this, "mc6854")
		, m_econet(*this, "econet")
		, m_irq(*this, "irq")
		, m_links(*this, "LK")
	{
	}

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	required_device<mc6854_device> m_adlc;
	required_device<econet_device> m_econet;
	required_device<input_merger_device> m_irq;
	required_ioport m_links;

	uint8_t statid_r()
	{
		return m_links->read();
	}
};


//-------------------------------------------------
//  INPUT_PORTS( atom_econet )
//-------------------------------------------------

static INPUT_PORTS_START( atom_econet )
	PORT_START("LK")
	PORT_CONFNAME(0x01, 0x01, "LK0") PORT_CONFSETTING(0x00, "Fitted") PORT_CONFSETTING(0x01, "Open")
	PORT_CONFNAME(0x02, 0x02, "LK1") PORT_CONFSETTING(0x00, "Fitted") PORT_CONFSETTING(0x02, "Open")
	PORT_CONFNAME(0x04, 0x04, "LK2") PORT_CONFSETTING(0x00, "Fitted") PORT_CONFSETTING(0x04, "Open")
	PORT_CONFNAME(0x08, 0x08, "LK3") PORT_CONFSETTING(0x00, "Fitted") PORT_CONFSETTING(0x08, "Open")
	PORT_CONFNAME(0x10, 0x10, "LK4") PORT_CONFSETTING(0x00, "Fitted") PORT_CONFSETTING(0x10, "Open")
	PORT_CONFNAME(0x20, 0x20, "LK5") PORT_CONFSETTING(0x00, "Fitted") PORT_CONFSETTING(0x20, "Open")
	PORT_CONFNAME(0x40, 0x40, "LK6") PORT_CONFSETTING(0x00, "Fitted") PORT_CONFSETTING(0x40, "Open")
	PORT_CONFNAME(0x80, 0x80, "LK7") PORT_CONFSETTING(0x00, "Fitted") PORT_CONFSETTING(0x80, "Open")
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor atom_econet_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( atom_econet );
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void atom_econet_device::device_add_mconfig(machine_config &config)
{
	MC6854(config, m_adlc);
	m_adlc->out_txd_cb().set(m_econet, FUNC(econet_device::host_data_w));
	m_adlc->out_irq_cb().set(m_irq, FUNC(input_merger_device::in_w<0>));
	m_adlc->out_rts_cb().set(m_econet, FUNC(econet_device::host_rts_w));
	m_adlc->out_dtr_cb().set(m_irq, FUNC(input_merger_device::in_w<1>)).invert();

	INPUT_MERGER_ANY_HIGH(config, m_irq).output_handler().set([this] (int state) { m_bus->irq_w(state); });

	ttl74123_device &ic5(TTL74123(config, "ic5", RES_K(39), CAP_N(2.2)));
	ic5.set_connection_type(TTL74123_NOT_GROUNDED_NO_DIODE);
	ic5.set_b_pin_value(1);
	ic5.set_clear_pin_value(1);
	ic5.out_cb().set(m_adlc, FUNC(mc6854_device::set_dcd)).invert();
	ic5.out_cb().append("ic9", FUNC(input_merger_device::in_w<0>));

	INPUT_MERGER_ALL_HIGH(config, "ic9").output_handler().set(m_adlc, FUNC(mc6854_device::set_cts)).invert();

	ECONET(config, m_econet);
	m_econet->clk_wr_callback().set(m_adlc, FUNC(mc6854_device::txc_w));
	m_econet->clk_wr_callback().append(m_adlc, FUNC(mc6854_device::rxc_w));
	m_econet->clk_wr_callback().append(ic5, FUNC(ttl74123_device::a_w));
	m_econet->data_wr_callback().set(m_adlc, FUNC(mc6854_device::set_rx));
	m_econet->valid_wr_callback().set("ic9", FUNC(input_merger_device::in_w<1>));

	ECONET_SLOT(config, "econet254", m_econet, econet_devices);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void atom_econet_device::device_start()
{
	address_space &space = m_bus->memspace();

	space.install_readwrite_handler(0xb400, 0xb403, 0, 0x3f8, 0, emu::rw_delegate(*m_adlc, FUNC(mc6854_device::read)), emu::rw_delegate(*m_adlc, FUNC(mc6854_device::write)));
	space.install_read_handler(0xb404, 0xb404, 0, 0x3fb, 0, emu::rw_delegate(*this, FUNC(atom_econet_device::statid_r)));
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(ATOM_ECONET, device_acorn_bus_interface, atom_econet_device, "atom_econet", "Atom Econet Interface")
