// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Acorn Atom Econet Interface

    Part No. 102,002

**********************************************************************/

#include "emu.h"
#include "econet.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ATOM_ECONET, atom_econet_device, "atom_econet", "Acorn Atom Econet Interface")

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

MACHINE_CONFIG_START(atom_econet_device::device_add_mconfig)
	/* econet */
	MCFG_DEVICE_ADD(m_adlc, MC6854, 0)
	MCFG_MC6854_OUT_TXD_CB(WRITELINE(m_econet, econet_device, host_data_w))
	MCFG_MC6854_OUT_IRQ_CB(WRITELINE(*this, atom_econet_device, bus_irq_w))

	ECONET(config, m_econet, 0);
	m_econet->clk_wr_callback().set(m_adlc, FUNC(mc6854_device::txc_w));
	m_econet->clk_wr_callback().append(m_adlc, FUNC(mc6854_device::rxc_w));
	m_econet->data_wr_callback().set(m_adlc, FUNC(mc6854_device::set_rx));
	MCFG_ECONET_SLOT_ADD("econet254", 254, econet_devices, nullptr)
MACHINE_CONFIG_END

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

	space.install_readwrite_handler(0xb400, 0xb403, read8_delegate(FUNC(mc6854_device::read), m_adlc.target()), write8_delegate(FUNC(mc6854_device::write), m_adlc.target()));
	space.install_read_handler(0xb404, 0xb404, read8_delegate(FUNC(atom_econet_device::statid_r), this));
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
