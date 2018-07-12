// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Acorn Econet Interface

    Part No. 200,024

    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Acorn_Econet.html

**********************************************************************/

#include "emu.h"
#include "econet.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ACORN_ECONET, acorn_econet_device, "acorn_econet", "Acorn Econet Interface")

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

MACHINE_CONFIG_START(acorn_econet_device::device_add_mconfig)
	/* econet */
	MCFG_DEVICE_ADD(m_adlc, MC6854, 0)
	MCFG_MC6854_OUT_TXD_CB(WRITELINE(m_econet, econet_device, host_data_w))
	MCFG_MC6854_OUT_IRQ_CB(WRITELINE(*this, acorn_econet_device, bus_irq_w))

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
//  acorn_econet_device - constructor
//-------------------------------------------------

acorn_econet_device::acorn_econet_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ACORN_ECONET, tag, owner, clock)
	, device_acorn_bus_interface(mconfig, *this)
	, m_adlc(*this, "mc6854")
	, m_econet(*this, "econet")
{
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

	space.install_readwrite_handler(0x1940, 0x1943, read8_delegate(FUNC(mc6854_device::read), m_adlc.target()), write8_delegate(FUNC(mc6854_device::write), m_adlc.target()));
	space.install_read_handler(0x1944, 0x1944, read8_delegate(FUNC(acorn_econet_device::statid_r), this));
}

//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

READ8_MEMBER(acorn_econet_device::statid_r)
{
	return 0xfe;
}

WRITE_LINE_MEMBER(acorn_econet_device::bus_irq_w)
{
	m_bus->irq_w(state);
}
