// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    SWTPC MP-S Serial Interface

    TODO: Add baud rate selection switch

**********************************************************************/

#include "emu.h"
#include "bus/ss50/mps.h"
#include "bus/ss50/interface.h"

#include "bus/rs232/rs232.h"
#include "machine/6850acia.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> ss50_mps_device

class ss50_mps_device : public device_t, public ss50_card_interface
{
public:
	// construction/destruction
	ss50_mps_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: device_t(mconfig, SS50_MPS, tag, owner, clock),
			ss50_card_interface(mconfig, *this),
			m_acia(*this, "acia")
	{
	}

protected:
	// device-specific overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override { }

	// interface-specific overrides
	virtual DECLARE_READ8_MEMBER(register_read) override;
	virtual DECLARE_WRITE8_MEMBER(register_write) override;
	virtual DECLARE_WRITE_LINE_MEMBER(f600_1200_w) override;

private:
	required_device<acia6850_device> m_acia;
};


static DEVICE_INPUT_DEFAULTS_START( terminal )
	DEVICE_INPUT_DEFAULTS("RS232_RXBAUD", 0xff, RS232_BAUD_1200)
	DEVICE_INPUT_DEFAULTS("RS232_TXBAUD", 0xff, RS232_BAUD_1200)
	DEVICE_INPUT_DEFAULTS("RS232_STARTBITS", 0xff, RS232_STARTBITS_1)
	DEVICE_INPUT_DEFAULTS("RS232_DATABITS", 0xff, RS232_DATABITS_8)
	DEVICE_INPUT_DEFAULTS("RS232_PARITY", 0xff, RS232_PARITY_NONE)
	DEVICE_INPUT_DEFAULTS("RS232_STOPBITS", 0xff, RS232_STOPBITS_1)
DEVICE_INPUT_DEFAULTS_END


//-------------------------------------------------
//  device_add_mconfig - add device-specific
//  machine configuration
//-------------------------------------------------

MACHINE_CONFIG_MEMBER(ss50_mps_device::device_add_mconfig)
	MCFG_DEVICE_ADD("acia", ACIA6850, 0)
	MCFG_ACIA6850_TXD_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_txd))
	//MCFG_ACIA6850_RTS_HANDLER(WRITELINE(ss50_mps_device, reader_control_w))
	//MCFG_ACIA6850_IRQ_HANDLER(WRITELINE(ss50_mps_device, write_irq))

	MCFG_RS232_PORT_ADD("rs232", default_rs232_devices, "terminal")
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("acia", acia6850_device, write_rxd))
	MCFG_DEVICE_CARD_DEVICE_INPUT_DEFAULTS("terminal", terminal)
MACHINE_CONFIG_END


//-------------------------------------------------
//  register_read - read from a port register
//-------------------------------------------------

READ8_MEMBER(ss50_mps_device::register_read)
{
	return m_acia->read(space, offset & 1, 0);
}

//-------------------------------------------------
//  register_write - write to a port register
//-------------------------------------------------

WRITE8_MEMBER(ss50_mps_device::register_write)
{
	m_acia->write(space, offset & 1, data);
}


WRITE_LINE_MEMBER(ss50_mps_device::f600_1200_w)
{
	m_acia->write_txc(state);
	m_acia->write_rxc(state);
}


// device type definition
DEFINE_DEVICE_TYPE(SS50_MPS, ss50_mps_device, "ss50_mps", "MP-S Serial Interface")
