// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
// thanks-to:rfka01
/***************************************************************************

    K801 RS-232 Switchable Interface
    K211 RS-232 Communications Interface
    K212 RS-232 Printer Interface
    K213 RS-232 Plotter Interface

    K211, K212 and K213 have same board, but different cables.
    K801 uses a 2661 instead of the 2651 and has 4 switches for
    select the IFSEL.

***************************************************************************/

#include "emu.h"
#include "k801.h"

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/


static INPUT_PORTS_START( dmv_k801 )
	PORT_START("DSW")
	PORT_DIPNAME( 0x0f, 0x00, "K801 IFSEL" )  PORT_DIPLOCATION("S:!4,S:!3,S:!2,S:!1")
	PORT_DIPSETTING( 0x00, "0A" )
	PORT_DIPSETTING( 0x01, "0B" )
	PORT_DIPSETTING( 0x02, "1A" )
	PORT_DIPSETTING( 0x03, "1B" )
	PORT_DIPSETTING( 0x04, "2A" )
	PORT_DIPSETTING( 0x05, "2B" )
	PORT_DIPSETTING( 0x06, "3A" )
	PORT_DIPSETTING( 0x07, "3B" )
	PORT_DIPSETTING( 0x08, "4A" )
	PORT_DIPSETTING( 0x09, "4B" )
INPUT_PORTS_END

static INPUT_PORTS_START( dmv_k211 )
	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x02, "K211 Jumpers" )  PORT_DIPLOCATION("J:1,J:2")
	PORT_DIPSETTING( 0x01, "IFSEL 0" )
	PORT_DIPSETTING( 0x02, "IFSEL 1" )
INPUT_PORTS_END

static INPUT_PORTS_START( dmv_k212 )
	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x01, "K212 Jumpers" )  PORT_DIPLOCATION("J:1,J:2")
	PORT_DIPSETTING( 0x01, "IFSEL 0" )
	PORT_DIPSETTING( 0x02, "IFSEL 1" )
INPUT_PORTS_END

static INPUT_PORTS_START( dmv_k213 )
	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x01, "K213 Jumpers" )  PORT_DIPLOCATION("J:1,J:2")
	PORT_DIPSETTING( 0x01, "IFSEL 0" )
	PORT_DIPSETTING( 0x02, "IFSEL 1" )
INPUT_PORTS_END


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(DMV_K801, dmv_k801_device, "dmv_k801", "K801 RS-232 Switchable Interface")
DEFINE_DEVICE_TYPE(DMV_K211, dmv_k211_device, "dmv_k211", "K211 RS-232 Communications Interface")
DEFINE_DEVICE_TYPE(DMV_K212, dmv_k212_device, "dmv_k212", "K212 RS-232 Printer Interface")
DEFINE_DEVICE_TYPE(DMV_K213, dmv_k213_device, "dmv_k213", "K213 RS-232 Plotter Interface")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  dmv_k801_device - constructor
//-------------------------------------------------

dmv_k801_device::dmv_k801_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: dmv_k801_device(mconfig, DMV_K801, tag, owner, clock)
{
}

dmv_k801_device::dmv_k801_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock),
	device_dmvslot_interface( mconfig, *this ),
	m_epci(*this, "epci"),
	m_rs232(*this, "rs232"),
	m_dsw(*this, "DSW")
{
}

//-------------------------------------------------
//  dmv_k211_device - constructor
//-------------------------------------------------

dmv_k211_device::dmv_k211_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: dmv_k211_device(mconfig, DMV_K211, tag, owner, clock)
{
}


dmv_k211_device::dmv_k211_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: dmv_k801_device(mconfig, type, tag, owner, clock)
{
}

//-------------------------------------------------
//  dmv_k212_device - constructor
//-------------------------------------------------

dmv_k212_device::dmv_k212_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: dmv_k211_device(mconfig, DMV_K212, tag, owner, clock)
{
}

//-------------------------------------------------
//  dmv_k213_device - constructor
//-------------------------------------------------

dmv_k213_device::dmv_k213_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: dmv_k211_device(mconfig, DMV_K213, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void dmv_k801_device::device_start()
{
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void dmv_k801_device::device_reset()
{
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void dmv_k801_device::device_add_mconfig(machine_config &config)
{
	MC2661(config, m_epci, XTAL(5'068'800));
	m_epci->txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	m_epci->rts_handler().set("rs232", FUNC(rs232_port_device::write_rts));
	m_epci->dtr_handler().set("rs232", FUNC(rs232_port_device::write_dtr));
	m_epci->rxrdy_handler().set(FUNC(dmv_k801_device::epci_irq_w));
	m_epci->txrdy_handler().set(FUNC(dmv_k801_device::epci_irq_w));

	RS232_PORT(config, m_rs232, default_rs232_devices, "printer");
	m_rs232->rxd_handler().set(m_epci, FUNC(mc2661_device::rx_w));
	m_rs232->dcd_handler().set(m_epci, FUNC(mc2661_device::dcd_w));
	m_rs232->dsr_handler().set(m_epci, FUNC(mc2661_device::dsr_w));
	m_rs232->cts_handler().set(m_epci, FUNC(mc2661_device::cts_w));
}

void dmv_k211_device::device_add_mconfig(machine_config &config)
{
	dmv_k801_device::device_add_mconfig(config);
	m_rs232->set_default_option("null_modem");
}

void dmv_k212_device::device_add_mconfig(machine_config &config)
{
	dmv_k801_device::device_add_mconfig(config);
	m_rs232->set_default_option("printer");
}

void dmv_k213_device::device_add_mconfig(machine_config &config)
{
	dmv_k801_device::device_add_mconfig(config);
	m_rs232->set_default_option(nullptr);
}

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor dmv_k801_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( dmv_k801 );
}

ioport_constructor dmv_k211_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( dmv_k211 );
}

ioport_constructor dmv_k212_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( dmv_k212 );
}

ioport_constructor dmv_k213_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( dmv_k213 );
}

WRITE_LINE_MEMBER(dmv_k801_device::epci_irq_w)
{
	out_irq(state);
}

void dmv_k801_device::io_read(int ifsel, offs_t offset, uint8_t &data)
{
	uint8_t dsw = m_dsw->read() & 0x0f;
	if ((dsw >> 1) == ifsel && BIT(offset, 3) == BIT(dsw, 0))
	{
		if (offset & 0x04)
			m_epci->write(offset & 0x03, data);
		else
			data = m_epci->read(offset & 0x03);
	}
}

void dmv_k801_device::io_write(int ifsel, offs_t offset, uint8_t data)
{
	uint8_t dsw = m_dsw->read() & 0x0f;
	if ((dsw >> 1) == ifsel && BIT(offset, 3) == BIT(dsw, 0))
	{
		if (offset & 0x04)
			m_epci->write(offset & 0x03, data);
		else
			data = m_epci->read(offset & 0x03);
	}
}

void dmv_k211_device::io_read(int ifsel, offs_t offset, uint8_t &data)
{
	uint8_t jumpers = m_dsw->read() & 0x03;
	if ((BIT(jumpers, 0) && ifsel == 0) || (BIT(jumpers, 1) && ifsel == 1))
	{
		if (offset & 0x04)
			m_epci->write(offset & 0x03, data);
		else
			data = m_epci->read(offset & 0x03);
	}
}

void dmv_k211_device::io_write(int ifsel, offs_t offset, uint8_t data)
{
	uint8_t jumpers = m_dsw->read() & 0x03;
	if ((BIT(jumpers, 0) && ifsel == 0) || (BIT(jumpers, 1) && ifsel == 1))
	{
		if (offset & 0x04)
			m_epci->write(offset & 0x03, data);
		else
			data = m_epci->read(offset & 0x03);
	}
}
