// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
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


static MACHINE_CONFIG_FRAGMENT( dmv_k801 )
	MCFG_DEVICE_ADD("epci", MC2661, XTAL_5_0688MHz)
	MCFG_MC2661_TXD_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_txd))
	MCFG_MC2661_RTS_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_rts))
	MCFG_MC2661_DTR_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_dtr))
	MCFG_MC2661_RXRDY_HANDLER(WRITELINE(dmv_k801_device, epci_irq_w))
	MCFG_MC2661_TXRDY_HANDLER(WRITELINE(dmv_k801_device, epci_irq_w))

	MCFG_RS232_PORT_ADD("rs232", default_rs232_devices, "printer")
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("epci", mc2661_device, rx_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE("epci", mc2661_device, dcd_w))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE("epci", mc2661_device, dsr_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("epci", mc2661_device, cts_w))
MACHINE_CONFIG_END

static MACHINE_CONFIG_FRAGMENT( dmv_k211 )
	MCFG_FRAGMENT_ADD( dmv_k801 )

	MCFG_DEVICE_MODIFY("rs232")
	MCFG_SLOT_DEFAULT_OPTION("null_modem")
MACHINE_CONFIG_END

static MACHINE_CONFIG_FRAGMENT( dmv_k212 )
	MCFG_FRAGMENT_ADD( dmv_k801 )

	MCFG_DEVICE_MODIFY("rs232")
	MCFG_SLOT_DEFAULT_OPTION("printer")
MACHINE_CONFIG_END

static MACHINE_CONFIG_FRAGMENT( dmv_k213 )
	MCFG_FRAGMENT_ADD( dmv_k801 )

	MCFG_DEVICE_MODIFY("rs232")
	MCFG_SLOT_DEFAULT_OPTION(NULL)
MACHINE_CONFIG_END

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

const device_type DMV_K801 = &device_creator<dmv_k801_device>;
const device_type DMV_K211 = &device_creator<dmv_k211_device>;
const device_type DMV_K212 = &device_creator<dmv_k212_device>;
const device_type DMV_K213 = &device_creator<dmv_k213_device>;

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  dmv_k801_device - constructor
//-------------------------------------------------

dmv_k801_device::dmv_k801_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: device_t(mconfig, DMV_K801, "K801 RS-232 Switchable Interface", tag, owner, clock, "dmv_k801", __FILE__),
		device_dmvslot_interface( mconfig, *this ),
		m_epci(*this, "epci"),
		m_dsw(*this, "DSW"), m_bus(nullptr)
	{
}

dmv_k801_device::dmv_k801_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
		: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
		device_dmvslot_interface( mconfig, *this ),
		m_epci(*this, "epci"),
		m_dsw(*this, "DSW"), m_bus(nullptr)
	{
}

//-------------------------------------------------
//  dmv_k211_device - constructor
//-------------------------------------------------

dmv_k211_device::dmv_k211_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: dmv_k801_device(mconfig, DMV_K211, "K211 RS-232 Communications Interface", tag, owner, clock, "dmv_k211", __FILE__)
{
}


dmv_k211_device::dmv_k211_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
		: dmv_k801_device(mconfig, type, name, tag, owner, clock, shortname, source)
{
}

//-------------------------------------------------
//  dmv_k212_device - constructor
//-------------------------------------------------

dmv_k212_device::dmv_k212_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: dmv_k211_device(mconfig, DMV_K212, "K212 RS-232 Printer Interface", tag, owner, clock, "dmv_k212", __FILE__)
{
}

//-------------------------------------------------
//  dmv_k213_device - constructor
//-------------------------------------------------

dmv_k213_device::dmv_k213_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: dmv_k211_device(mconfig, DMV_K213, "K213 RS-232 Plotter Interface", tag, owner, clock, "dmv_k213", __FILE__)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void dmv_k801_device::device_start()
{
	m_bus = static_cast<dmvcart_slot_device*>(owner());
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void dmv_k801_device::device_reset()
{
}

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor dmv_k801_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( dmv_k801 );
}

machine_config_constructor dmv_k211_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( dmv_k211 );
}

machine_config_constructor dmv_k212_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( dmv_k212 );
}

machine_config_constructor dmv_k213_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( dmv_k213 );
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
	m_bus->m_out_irq_cb(state);
}

void dmv_k801_device::io_read(address_space &space, int ifsel, offs_t offset, UINT8 &data)
{
	UINT8 dsw = m_dsw->read() & 0x0f;
	if ((dsw >> 1) == ifsel && BIT(offset, 3) == BIT(dsw, 0))
	{
		if (offset & 0x04)
			m_epci->write(space, offset & 0x03, data);
		else
			data = m_epci->read(space, offset & 0x03);
	}
}

void dmv_k801_device::io_write(address_space &space, int ifsel, offs_t offset, UINT8 data)
{
	UINT8 dsw = m_dsw->read() & 0x0f;
	if ((dsw >> 1) == ifsel && BIT(offset, 3) == BIT(dsw, 0))
	{
		if (offset & 0x04)
			m_epci->write(space, offset & 0x03, data);
		else
			data = m_epci->read(space, offset & 0x03);
	}
}

void dmv_k211_device::io_read(address_space &space, int ifsel, offs_t offset, UINT8 &data)
{
	UINT8 jumpers = m_dsw->read() & 0x03;
	if ((BIT(jumpers, 0) && ifsel == 0) || (BIT(jumpers, 1) && ifsel == 1))
	{
		if (offset & 0x04)
			m_epci->write(space, offset & 0x03, data);
		else
			data = m_epci->read(space, offset & 0x03);
	}
}

void dmv_k211_device::io_write(address_space &space, int ifsel, offs_t offset, UINT8 data)
{
	UINT8 jumpers = m_dsw->read() & 0x03;
	if ((BIT(jumpers, 0) && ifsel == 0) || (BIT(jumpers, 1) && ifsel == 1))
	{
		if (offset & 0x04)
			m_epci->write(space, offset & 0x03, data);
		else
			data = m_epci->read(space, offset & 0x03);
	}
}
