// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Acorn Atom Econet Interface

    Part No. 102,002

**********************************************************************/

#include "emu.h"
#include "econet.h"
#include "bus/econet/e01.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ATOM_ECONET, atom_econet_device, "atom_econet", "Acorn Atom Econet Interface")

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void atom_econet_device::device_add_mconfig(machine_config &config)
{
	/* econet */
	MC6854(config, m_adlc);
	m_adlc->out_txd_cb().set(m_econet, FUNC(econet_device::host_data_w));
	m_adlc->out_irq_cb().set(FUNC(atom_econet_device::bus_irq_w));

	ECONET(config, m_econet, 0);
	m_econet->clk_wr_callback().set(m_adlc, FUNC(mc6854_device::txc_w));
	m_econet->clk_wr_callback().append(m_adlc, FUNC(mc6854_device::rxc_w));
	m_econet->data_wr_callback().set(m_adlc, FUNC(mc6854_device::set_rx));

	econet_slot_device &slot(ECONET_SLOT(config, "econet254", 0));
	econet_devices(slot);
	slot.set_slot(254);
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  atom_econet_device - constructor
//-------------------------------------------------

atom_econet_device::atom_econet_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ATOM_ECONET, tag, owner, clock)
	, device_acorn_bus_interface(mconfig, *this)
	, m_adlc(*this, "mc6854")
	, m_econet(*this, "econet")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void atom_econet_device::device_start()
{
	address_space &space = m_bus->memspace();

	space.install_readwrite_handler(0xb400, 0xb403, read8sm_delegate(*m_adlc, FUNC(mc6854_device::read)), write8sm_delegate(*m_adlc, FUNC(mc6854_device::write)));
	space.install_read_handler(0xb404, 0xb404, read8_delegate(*this, FUNC(atom_econet_device::statid_r)));
}

//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

READ8_MEMBER(atom_econet_device::statid_r)
{
	return 0xfe;
}

WRITE_LINE_MEMBER(atom_econet_device::bus_irq_w)
{
	m_bus->irq_w(state);
}
