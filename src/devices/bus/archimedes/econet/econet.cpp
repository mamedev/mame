// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Acorn Archimedes Econet Module

**********************************************************************/

#include "emu.h"
#include "econet.h"
#include "bus/econet/econet.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ARC_ECONET, arc_econet_device, "arc_econet", "Acorn ADF10/AEH52 Econet Module");


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void arc_econet_device::device_add_mconfig(machine_config &config)
{
	MC6854(config, m_adlc);
	m_adlc->out_txd_cb().set("econet", FUNC(econet_device::host_data_w));
	m_adlc->out_irq_cb().set(DEVICE_SELF_OWNER, FUNC(archimedes_econet_slot_device::efiq_w));

	econet_device &econet(ECONET(config, "econet", 0));
	econet.clk_wr_callback().set(m_adlc, FUNC(mc6854_device::txc_w));
	econet.clk_wr_callback().append(m_adlc, FUNC(mc6854_device::rxc_w));
	econet.data_wr_callback().set(m_adlc, FUNC(mc6854_device::set_rx));

	ECONET_SLOT(config, "econet254", "econet", econet_devices);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  archimedes_econet_device - constructor
//-------------------------------------------------

arc_econet_device::arc_econet_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, ARC_ECONET, tag, owner, clock)
	, device_archimedes_econet_interface(mconfig, *this)
	, m_adlc(*this, "mc6854")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void arc_econet_device::device_start()
{
}

//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

u8 arc_econet_device::read(offs_t offset)
{
	return m_adlc->read(offset);
}

void arc_econet_device::write(offs_t offset, u8 data)
{
	m_adlc->write(offset, data);
}
