// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    SV-805 RS-232 Interface for SVI 318/328

***************************************************************************/

#include "sv805.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type SV805 = &device_creator<sv805_device>;

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( sv805 )
	MCFG_DEVICE_ADD("uart", INS8250, XTAL_3_072MHz)
	MCFG_INS8250_OUT_INT_CB(WRITELINE(sv805_device, uart_intr_w))
	MCFG_INS8250_OUT_TX_CB(DEVWRITELINE("rs232", rs232_port_device, write_txd))
	MCFG_INS8250_OUT_DTR_CB(DEVWRITELINE("rs232", rs232_port_device, write_dtr))
	MCFG_INS8250_OUT_RTS_CB(DEVWRITELINE("rs232", rs232_port_device, write_rts))

	MCFG_RS232_PORT_ADD("rs232", default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("uart", ins8250_uart_device, rx_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE("uart", ins8250_uart_device, dcd_w))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE("uart", ins8250_uart_device, dsr_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("uart", ins8250_uart_device, cts_w))
MACHINE_CONFIG_END

machine_config_constructor sv805_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( sv805 );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  sv805_device - constructor
//-------------------------------------------------

sv805_device::sv805_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, SV805, "SV-805 RS-232 Interface", tag, owner, clock, "sv805", __FILE__),
	device_svi_slot_interface(mconfig, *this),
	m_uart(*this, "uart"),
	m_rs232(*this, "rs232")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sv805_device::device_start()
{
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

READ8_MEMBER( sv805_device::iorq_r )
{
	switch (offset)
	{
	case 0x28:
	case 0x29:
	case 0x2a:
	case 0x2b:
	case 0x2c:
	case 0x2d:
	case 0x2e:
	case 0x2f:
		return m_uart->ins8250_r(space, offset & 0x07);
	}

	return 0xff;
}

WRITE8_MEMBER( sv805_device::iorq_w )
{
	switch (offset)
	{
	case 0x28:
	case 0x29:
	case 0x2a:
	case 0x2b:
	case 0x2c:
	case 0x2d:
	case 0x2e:
	case 0x2f:
		m_uart->ins8250_w(space, offset & 0x07, data);
	}
}

WRITE_LINE_MEMBER( sv805_device::uart_intr_w )
{
	m_bus->int_w(state);
}
